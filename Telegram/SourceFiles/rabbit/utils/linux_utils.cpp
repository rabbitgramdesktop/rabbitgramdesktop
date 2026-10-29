/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#if !defined Q_OS_WIN && !defined Q_OS_MAC

#include "rabbit/utils/linux_utils.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtGui/QImage>

#include "rabbit/ui/rabbit_assets.h"
#include "settings.h"

namespace RabbitLinuxUtils {

QString ApplicationsDirectory() {
	return QStandardPaths::writableLocation(
		QStandardPaths::ApplicationsLocation);
}

QString DesktopFilePath() {
	return ApplicationsDirectory() + u"/rabbitGram.desktop"_q;
}

QString IconsRootDirectory() {
	return QStandardPaths::writableLocation(
		QStandardPaths::GenericDataLocation) + u"/icons/hicolor/"_q;
}

QString BinDirectory() {
	return QDir::homePath() + u"/.local/bin"_q;
}

QString SymlinkPath() {
	return BinDirectory() + u"/rabbitgram"_q;
}

bool IsInstalledToSystem() {
	const auto exeDir = cExeDir();
	if (exeDir.startsWith(u"/usr/"_q) || exeDir.startsWith(u"/opt/"_q)) {
		return true;
	}
	const auto home = QDir::homePath();
	if (exeDir.startsWith(home + u"/.local/bin"_q)
		|| exeDir.startsWith(home + u"/.local/opt"_q)
		|| exeDir.startsWith(home + u"/.local/share/rabbitgram"_q)) {
		return true;
	}
	const auto symlink = SymlinkPath();
	const auto fi = QFileInfo(symlink);
	if (fi.isSymLink() && fi.symLinkTarget() == (cExeDir() + cExeName())) {
		return true;
	}
	return false;
}

bool IsDesktopEntryInstalled() {
	return QFile::exists(DesktopFilePath());
}

bool IsSymlinkInstalled() {
	const auto path = SymlinkPath();
	return QFileInfo(path).isSymLink() || QFile::exists(path);
}

void RefreshDesktopDatabase() {
	QProcess::startDetached(
		u"update-desktop-database"_q,
		{ ApplicationsDirectory() });
	QProcess::startDetached(
		u"gtk-update-icon-cache"_q,
		{ u"-f"_q, u"-t"_q, IconsRootDirectory() });
}

void ApplyAppIcon() {
	const auto iconsPath = IconsRootDirectory();
	const auto logo = RabbitAssets::currentAppLogo();
	if (logo.isNull()) {
		return;
	}

	constexpr int sizes[] = { 16, 24, 32, 48, 64, 128, 256, 512 };
	for (const auto size : sizes) {
		const auto dir = QString(u"%1%2x%2/apps/"_q).arg(iconsPath).arg(size);
		QDir().mkpath(dir);
		const auto scaled = logo.scaled(
			size,
			size,
			Qt::IgnoreAspectRatio,
			Qt::SmoothTransformation);
		scaled.save(dir + u"rabbitGram.png"_q, "PNG");
		scaled.save(dir + u"rabbitgram.png"_q, "PNG");
	}

	const auto desktopPath = DesktopFilePath();
	if (QFile::exists(desktopPath)) {
		auto f = QFile(desktopPath);
		if (f.open(QIODevice::Append)) {
			f.close();
		}
	}

	RefreshDesktopDatabase();
}

bool InstallDesktopEntry(const QString &customExecPath) {
	const auto appDir = ApplicationsDirectory();
	QDir().mkpath(appDir);

	auto exec = customExecPath;
	if (exec.isEmpty()) {
		exec = (cExeDir() + cExeName());
	}

	const auto execString = exec.contains(' ')
		? (u"\"%1\" -- %%u"_q.arg(exec))
		: (u"%1 -- %%u"_q.arg(exec));
	const auto execQuitString = exec.contains(' ')
		? (u"\"%1\" -quit"_q.arg(exec))
		: (u"%1 -quit"_q.arg(exec));
	const auto tryExec = exec.contains(' ')
		? (u"\"%1\""_q.arg(exec))
		: exec;

	const auto content = QString(
		u"[Desktop Entry]\n"
		"Version=1.0\n"
		"Name=rabbitGram\n"
		"Comment=rabbitGram Desktop Messaging Client\n"
		"TryExec=%1\n"
		"Exec=%2\n"
		"Icon=rabbitGram\n"
		"Terminal=false\n"
		"StartupWMClass=rabbitGram\n"
		"Type=Application\n"
		"Categories=Chat;Network;InstantMessaging;Qt;\n"
		"MimeType=x-scheme-handler/tg;x-scheme-handler/tonsite;\n"
		"Keywords=tg;chat;im;messaging;messenger;sms;tdesktop;rabbitgram;rabbitGram;\n"
		"Actions=quit;\n"
		"SingleMainWindow=true\n"
		"X-GNOME-UsesNotifications=true\n"
		"X-GNOME-SingleWindow=true\n\n"
		"[Desktop Action quit]\n"
		"Exec=%3\n"
		"Name=Quit rabbitGram\n"
		"Icon=application-exit\n"_q
	).arg(tryExec, execString, execQuitString);

	const auto desktopPath = DesktopFilePath();
	auto file = QFile(desktopPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		return false;
	}
	file.write(content.toUtf8());
	file.close();

	QFile::setPermissions(
		desktopPath,
		QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
		| QFileDevice::ReadGroup | QFileDevice::ExeGroup
		| QFileDevice::ReadOther | QFileDevice::ExeOther);

	ApplyAppIcon();
	return true;
}

bool RemoveDesktopEntry() {
	const auto desktopPath = DesktopFilePath();
	const auto removed = QFile::remove(desktopPath);
	RefreshDesktopDatabase();
	return removed;
}

bool InstallSymlink(const QString &targetExe) {
	const auto binDir = BinDirectory();
	QDir().mkpath(binDir);

	const auto target = targetExe.isEmpty()
		? (cExeDir() + cExeName())
		: targetExe;
	const auto symlink = SymlinkPath();
	const auto symlinkCapital = binDir + u"/rabbitGram"_q;

	QFile::remove(symlink);
	QFile::remove(symlinkCapital);

	const auto ok = QFile::link(target, symlink);
	QFile::link(target, symlinkCapital);
	return ok;
}

bool RemoveSymlink() {
	const auto symlink = SymlinkPath();
	const auto symlinkCapital = BinDirectory() + u"/rabbitGram"_q;
	const auto r1 = QFile::remove(symlink);
	QFile::remove(symlinkCapital);
	return r1;
}

bool InstallAppToUserLocal(bool createDesktop, bool createSymlink) {
	const auto binDir = BinDirectory();
	QDir().mkpath(binDir);

	const auto currentExe = cExeDir() + cExeName();
	const auto targetExe = binDir + u"/rabbitgram"_q;

	if (QFileInfo(currentExe).canonicalFilePath() != QFileInfo(targetExe).canonicalFilePath()) {
		QFile::remove(targetExe);
		if (!QFile::copy(currentExe, targetExe)) {
			return false;
		}
		QFile::setPermissions(
			targetExe,
			QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
			| QFileDevice::ReadGroup | QFileDevice::ExeGroup
			| QFileDevice::ReadOther | QFileDevice::ExeOther);
	}

	if (createDesktop) {
		InstallDesktopEntry(targetExe);
	}
	if (createSymlink) {
		const auto symlinkCapital = binDir + u"/rabbitGram"_q;
		if (!QFile::exists(symlinkCapital)) {
			QFile::link(targetExe, symlinkCapital);
		}
	}
	return true;
}

} // namespace RabbitLinuxUtils

#endif // !defined Q_OS_WIN && !defined Q_OS_MAC
