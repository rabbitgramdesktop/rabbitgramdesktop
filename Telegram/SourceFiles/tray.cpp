/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "tray.h"
#include "tray_accounts_menu.h"

#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings/rabbit_settings.h"
#include "window/notifications_manager.h"
#include "base/unixtime.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "core/version.h"
#include "platform/platform_notifications_manager.h"
#include "platform/platform_specific.h"
#include "lang/lang_keys.h"

#include <QtWidgets/QApplication>

namespace Core {

QString TrayIconToolTip() {
	const auto counter = Core::App().unreadBadge();
	return (counter > 0)
		? u"%1 (%2)"_q.arg(AppName.utf16()).arg(counter)
		: AppName.utf16();
}

Tray::Tray()
	: _quietModeTimer([=] { rebuildMenu(); }) {
}

void Tray::create() {
	const auto deadline = RabbitSettings::quietModeDeadline();
	const auto now = base::unixtime::now();
	if (deadline > now) {
		_quietModeTimer.callOnce((deadline - now) * 1000);
	}

	RabbitSettings::JsonSettings::Events(
		"quiet_mode_deadline"
	) | rpl::on_next([=] {
		const auto deadline = RabbitSettings::quietModeDeadline();
		const auto now = base::unixtime::now();
		if (deadline > now) {
			_quietModeTimer.callOnce((deadline - now) * 1000);
		} else {
			_quietModeTimer.cancel();
		}
		rebuildMenu();
		updateIconCounters();
	}, _tray.lifetime());

	RabbitSettings::JsonSettings::Events(
		"quiet_mode_enabled"
	) | rpl::on_next([=] {
		rebuildMenu();
	}, _tray.lifetime());

	Core::App().notifications().settingsChanged(
	) | rpl::on_next([=](Window::Notifications::ChangeType type) {
		if (type == Window::Notifications::ChangeType::DesktopEnabled) {
			rebuildMenu();
		}
	}, _tray.lifetime());

	rebuildMenu();
	using WorkMode = Settings::WorkMode;
	if (Core::App().settings().workMode() != WorkMode::WindowOnly) {
		_tray.createIcon();
	}

	Core::App().settings().workModeValue(
	) | rpl::combine_previous(
	) | rpl::on_next([=](WorkMode previous, WorkMode state) {
		const auto wasHasIcon = (previous != WorkMode::WindowOnly);
		const auto nowHasIcon = (state != WorkMode::WindowOnly);
		if (wasHasIcon != nowHasIcon) {
			if (nowHasIcon) {
				_tray.createIcon();
			} else {
				_tray.destroyIcon();
			}
		}
	}, _tray.lifetime());

	Core::App().settings().trayIconMonochromeChanges(
	) | rpl::on_next([=] {
		updateIconCounters();
	}, _tray.lifetime());

	Core::App().passcodeLockChanges(
	) | rpl::on_next([=] {
		rebuildMenu();
	}, _tray.lifetime());

	TrayAccountsMenu::SetupChangesSubscription(
		[=] { rebuildMenu(); },
		_tray.lifetime());

	_tray.iconClicks(
	) | rpl::on_next([=] {
		const auto skipTrayClick = (_lastTrayClickTime > 0)
			&& (crl::now() - _lastTrayClickTime
				< QApplication::doubleClickInterval());
		if (!skipTrayClick) {
			_activeForTrayIconAction = Core::App().isActiveForTrayMenu();
			_minimizeMenuItemClicks.fire({});
			_lastTrayClickTime = crl::now();
		}
	}, _tray.lifetime());
}

void Tray::rebuildMenu() {
	_tray.destroyMenu();
	_tray.createMenu();

	{
		auto minimizeText = _textUpdates.events(
		) | rpl::map([=] {
			_activeForTrayIconAction = Core::App().isActiveForTrayMenu();
			return _activeForTrayIconAction
				? tr::lng_minimize_to_tray(tr::now)
				: ktr("rtg_open");
		});

		_tray.addAction(
			std::move(minimizeText),
			[=] { _minimizeMenuItemClicks.fire({}); });
	}

	if (!Core::App().passcodeLocked()) {
		auto notificationsText = _textUpdates.events(
		) | rpl::map([=] {
			return Core::App().settings().desktopNotify()
				? tr::lng_disable_notifications_from_tray(tr::now)
				: tr::lng_enable_notifications_from_tray(tr::now);
		});

		_tray.addAction(
			std::move(notificationsText),
			[=] { toggleSoundNotifications(); });

		if (Core::App().settings().desktopNotify() && RabbitSettings::quietModeEnabled()) {
			std::vector<std::pair<rpl::producer<QString>, Fn<void()>>> quietOptions;
			const auto addOption = [&](const QString &labelKey, int seconds) {
				quietOptions.push_back({
					rktr(labelKey),
					[=] {
						const auto deadline = base::unixtime::now() + seconds;
						RabbitSettings::setQuietModeDeadline(deadline);
						Core::App().notifications().updateAll();
					}
				});
			};
			addOption("rtg_quiet_mode_30m", 30 * 60);
			addOption("rtg_quiet_mode_1h", 60 * 60);
			addOption("rtg_quiet_mode_3h", 3 * 60 * 60);
			addOption("rtg_quiet_mode_8h", 8 * 60 * 60);
			addOption("rtg_quiet_mode_24h", 24 * 60 * 60);

			_tray.addSubmenu(rktr("rtg_quiet_mode_for"), std::move(quietOptions));
		}
	}

	_tray.addAction(rktr("rtg_close"), [] { Core::Quit(); });

	TrayAccountsMenu::Fill(_tray);

	updateMenuText();
}

void Tray::updateMenuText() {
	_textUpdates.fire({});
}

void Tray::updateIconCounters() {
	_tray.updateIcon();
}

rpl::producer<> Tray::aboutToShowRequests() const {
	return _tray.aboutToShowRequests();
}

rpl::producer<> Tray::showFromTrayRequests() const {
	return rpl::merge(
		_tray.showFromTrayRequests(),
		_minimizeMenuItemClicks.events() | rpl::filter([=] {
			return !_activeForTrayIconAction;
		})
	);
}

rpl::producer<> Tray::hideToTrayRequests() const {
	auto triggers = rpl::merge(
		_tray.hideToTrayRequests(),
		_minimizeMenuItemClicks.events() | rpl::filter([=] {
			return _activeForTrayIconAction;
		})
	);
	if (_tray.hasTrayMessageSupport()) {
		return std::move(triggers) | rpl::map([=]() -> rpl::empty_value {
			_tray.showTrayMessage();
			return {};
		});
	} else {
		return triggers;
	}
}

void Tray::toggleSoundNotifications() {
	auto soundNotifyChanged = false;
	auto flashBounceNotifyChanged = false;
	auto &settings = Core::App().settings();
	settings.setDesktopNotify(!settings.desktopNotify());
	if (settings.desktopNotify()) {
		if (settings.rememberedSoundNotifyFromTray()
			&& !settings.soundNotify()) {
			settings.setSoundNotify(true);
			settings.setRememberedSoundNotifyFromTray(false);
			soundNotifyChanged = true;
		}
		if (settings.rememberedFlashBounceNotifyFromTray()
			&& !settings.flashBounceNotify()) {
			settings.setFlashBounceNotify(true);
			settings.setRememberedFlashBounceNotifyFromTray(false);
			flashBounceNotifyChanged = true;
		}
	} else {
		if (settings.soundNotify()) {
			settings.setSoundNotify(false);
			settings.setRememberedSoundNotifyFromTray(true);
			soundNotifyChanged = true;
		} else {
			settings.setRememberedSoundNotifyFromTray(false);
		}
		if (settings.flashBounceNotify()) {
			settings.setFlashBounceNotify(false);
			settings.setRememberedFlashBounceNotifyFromTray(true);
			flashBounceNotifyChanged = true;
		} else {
			settings.setRememberedFlashBounceNotifyFromTray(false);
		}
	}
	Core::App().saveSettingsDelayed();
	using Change = Window::Notifications::ChangeType;
	auto &notifications = Core::App().notifications();
	notifications.notifySettingsChanged(Change::DesktopEnabled);
	if (soundNotifyChanged) {
		notifications.notifySettingsChanged(Change::SoundEnabled);
	}
	if (flashBounceNotifyChanged) {
		notifications.notifySettingsChanged(Change::FlashBounceEnabled);
	}
}

bool Tray::has() const {
	return _tray.hasIcon() && Platform::TrayIconSupported();
}

} // namespace Core
