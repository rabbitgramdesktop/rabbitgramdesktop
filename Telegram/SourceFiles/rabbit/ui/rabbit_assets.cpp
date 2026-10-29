/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/ui/rabbit_assets.h"

#include <QtCore/QDir>
#include <QtCore/QFile>

#include "core/application.h"
#include "main/main_domain.h"
#include "rabbit/settings/rabbit_settings.h"
#include "tray.h"
#include "window/main_window.h"

#ifdef Q_OS_WIN
#include "rabbit/utils/windows_utils.h"
#elif !defined Q_OS_MAC
#include "rabbit/utils/linux_utils.h"
#endif

namespace RabbitAssets {
namespace {

QString LastLoadedName;
QImage LastLoaded;
QImage LastLoadedNoMargin;

void LoadIcons() {
	const auto appIcon = RabbitSettings::appIcon();
	if (LastLoadedName != appIcon) {
		LastLoadedName = appIcon;

		LastLoaded = QImage(u":/gui/art/rabbit/%1/app.png"_q.arg(appIcon));
		LastLoadedNoMargin = QImage(u":/gui/art/rabbit/%1/app_preview.png"_q.arg(appIcon));
	}
}

} // namespace

void loadAppIco() {
	const auto appIcon = RabbitSettings::appIcon();

	const auto appDataPath = QDir::fromNativeSeparators(QString::fromLocal8Bit(qgetenv("APPDATA")));
	const auto tempIconPath = appDataPath + u"/rabbitGram.ico"_q;

	// workaround for read-only file
	auto f = QFile(tempIconPath);
	if (f.exists()) {
		f.setPermissions(QFile::WriteOther);
		f.remove();
	}
	f.close();
	QFile::copy(u":/gui/art/rabbit/%1/app_icon.ico"_q.arg(appIcon), tempIconPath);
}

QImage loadPreview(const QString &name) {
	return QImage(u":/gui/art/rabbit/%1/app_preview.png"_q.arg(name));
}

QString currentAppLogoName() {
	return LastLoadedName;
}

QImage currentAppLogo() {
	LoadIcons();
	return LastLoaded;
}

QImage currentAppLogoNoMargin() {
	LoadIcons();
	return LastLoadedNoMargin;
}

void applyAppIcon() {
#ifdef Q_OS_WIN
	loadAppIco();
	reloadAppIconFromTaskBar();
#elif !defined Q_OS_MAC
	RabbitLinuxUtils::ApplyAppIcon();
#endif

	Window::OverrideApplicationIcon(currentAppLogo());
	Core::App().refreshApplicationIcon();
	Core::App().tray().updateIconCounters();
	Core::App().domain().notifyUnreadBadgeChanged();
}

} // namespace RabbitAssets
