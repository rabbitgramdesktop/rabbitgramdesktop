/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#if !defined Q_OS_WIN && !defined Q_OS_MAC

#include "rabbit/settings_menu/sections/rabbit_linux.h"

#include <rpl/rpl.h>
#include "ui/layers/generic_box.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/settings_menu/rabbit_settings_menu.h"
#include "rabbit/utils/linux_utils.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "settings.h"
#include "ui/toast/toast.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

#include "styles/style_boxes.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "lang/lang_keys.h"

namespace Settings {
namespace {

using namespace Builder;

void ShowInstallBox(not_null<Window::SessionController *> controller) {
	controller->show(Box([=](not_null<Ui::GenericBox *> box) {
		box->setTitle(rktr(u"rtg_linux_install_title"_q));
		box->setWidth(st::aboutWidth);

		const auto content = box->verticalLayout();
		content->add(object_ptr<Ui::FlatLabel>(
			content,
			rktr(u"rtg_linux_install_description"_q),
			st::aboutLabel));

		content->add(object_ptr<Ui::RpWidget>(content), { 0, 8, 0, 8 });

		const auto copyBinaryCheck = content->add(
			object_ptr<Ui::Checkbox>(
				content,
				rktr(u"rtg_linux_install_option_binary"_q),
				true,
				st::defaultCheckbox));

		const auto desktopCheck = content->add(
			object_ptr<Ui::Checkbox>(
				content,
				rktr(u"rtg_linux_install_option_desktop"_q),
				true,
				st::defaultCheckbox));

		box->addButton(rktr(u"rtg_linux_install_button"_q), [=] {
			const auto copy = copyBinaryCheck->checked();
			const auto desktop = desktopCheck->checked();
			if (copy) {
				RabbitLinuxUtils::InstallAppToUserLocal(desktop, true);
			} else {
				if (desktop) {
					RabbitLinuxUtils::InstallDesktopEntry();
				}
				RabbitLinuxUtils::InstallSymlink();
			}
			Ui::Toast::Show(ktr(u"rtg_linux_install_success"_q));
			box->closeBox();
		});

		box->addButton(tr::lng_cancel(), [=] { box->closeBox(); });
	}));
}

void BuildRabbitLinux(SectionBuilder &builder) {
	const auto controller = builder.controller();
	const auto isInstalled = RabbitLinuxUtils::IsInstalledToSystem();

	builder.addSubsectionTitle({
		.id = u"rabbit/linux/app"_q,
		.title = rktr(u"rtg_linux_integration"_q),
		.keywords = { u"linux"_q, u"install"_q, u"desktop"_q },
	});

	if (isInstalled) {
		builder.addButton({
			.id = u"rabbit/linux/installed_status"_q,
			.title = rktr(u"rtg_linux_status_installed"_q),
			.st = &st::settingsButton,
			.icon = { &st::menuIconPermissions },
			.label = rpl::single(cExeDir() + cExeName()),
			.keywords = { u"status"_q, u"installed"_q },
		});
	} else if (controller) {
		builder.addButton({
			.id = u"rabbit/linux/install"_q,
			.title = rktr(u"rtg_linux_install_app"_q),
			.st = &st::settingsButton,
			.icon = { &st::menuIconDownload },
			.label = rktr(u"rtg_linux_status_portable"_q),
			.onClick = [=] {
				ShowInstallBox(controller);
			},
			.keywords = { u"install"_q, u"setup"_q },
		});
	}

	builder.addSkip();
	builder.addDivider();
	builder.addSkip();

	if (const auto desktopButton = builder.addButton({
		.id = u"rabbit/linux/desktop_entry"_q,
		.title = rktr(u"rtg_linux_desktop_entry"_q),
		.st = &st::settingsButton,
		.icon = { &st::menuIconShortcut },
		.toggled = rpl::single(RabbitLinuxUtils::IsDesktopEntryInstalled()),
		.keywords = { u"desktop"_q, u"menu"_q, u"shortcut"_q },
	})) {
		desktopButton->toggledValue()
			| rpl::filter([=](bool enabled) {
				return enabled != RabbitLinuxUtils::IsDesktopEntryInstalled();
			})
			| rpl::on_next([=](bool enabled) {
				if (enabled) {
					RabbitLinuxUtils::InstallDesktopEntry();
					Ui::Toast::Show(ktr(u"rtg_linux_desktop_created"_q));
				} else {
					RabbitLinuxUtils::RemoveDesktopEntry();
					Ui::Toast::Show(ktr(u"rtg_linux_desktop_removed"_q));
				}
			}, desktopButton->lifetime());
	}

	if (const auto cmdButton = builder.addButton({
		.id = u"rabbit/linux/terminal_cmd"_q,
		.title = rktr(u"rtg_linux_terminal_command"_q),
		.st = &st::settingsButton,
		.icon = { &st::menuIconManage },
		.toggled = rpl::single(RabbitLinuxUtils::IsSymlinkInstalled()),
		.keywords = { u"terminal"_q, u"command"_q, u"bin"_q, u"path"_q },
	})) {
		cmdButton->toggledValue()
			| rpl::filter([=](bool enabled) {
				return enabled != RabbitLinuxUtils::IsSymlinkInstalled();
			})
			| rpl::on_next([=](bool enabled) {
				if (enabled) {
					RabbitLinuxUtils::InstallSymlink();
					Ui::Toast::Show(ktr(u"rtg_linux_cmd_created"_q));
				} else {
					RabbitLinuxUtils::RemoveSymlink();
					Ui::Toast::Show(ktr(u"rtg_linux_cmd_removed"_q));
				}
			}, cmdButton->lifetime());
	}

	builder.addButton({
		.id = u"rabbit/linux/refresh"_q,
		.title = rktr(u"rtg_linux_refresh_desktop"_q),
		.st = &st::settingsButton,
		.icon = { &st::menuIconRestore },
		.onClick = [] {
			RabbitLinuxUtils::ApplyAppIcon();
			Ui::Toast::Show(ktr(u"rtg_linux_refreshed"_q));
		},
		.keywords = { u"refresh"_q, u"icon"_q, u"database"_q },
	});

	builder.addSkip();
	builder.addDivider();
	builder.addSkip();

	if (const auto xcbButton = builder.addButton({
		.id = u"rabbit/linux/force_xcb"_q,
		.title = rktr(u"rtg_linux_force_xcb"_q),
		.st = &st::settingsButton,
		.icon = { &st::menuIconNewWindow },
		.toggled = rpl::single(RabbitSettings::linuxForceXcb()),
		.keywords = { u"xcb"_q, u"x11"_q, u"wayland"_q, u"notifications"_q },
	})) {
		xcbButton->toggledValue()
			| rpl::filter([=](bool enabled) {
				return enabled != RabbitSettings::linuxForceXcb();
			})
			| rpl::on_next([=](bool enabled) {
				RabbitSettings::setLinuxForceXcb(enabled);
				Ui::Toast::Show(tr::lng_settings_restart_now(tr::now));
			}, xcbButton->lifetime());
	}
	builder.addSkip();
	builder.addDividerText(rktr(u"rtg_linux_force_xcb_about"_q));
}

const auto kMeta = BuildHelper({
	.id = RabbitLinux::Id(),
	.parentId = Rabbit::Id(),
	.title = rktr_phrase(u"rtg_settings_linux"_q),
	.icon = &st::menuIconSettings,
}, [](SectionBuilder &builder) {
	BuildRabbitLinux(builder);
});

const SectionBuildMethod kRabbitLinuxSection = kMeta.build;

} // namespace

rpl::producer<QString> RabbitLinux::title() {
	return rktr(u"rtg_settings_linux"_q);
}

RabbitLinux::RabbitLinux(
	QWidget *parent,
	not_null<Window::SessionController *> controller)
	: Section(parent, controller) {
	setupContent();
}

void RabbitLinux::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	build(content, kRabbitLinuxSection);
	Ui::ResizeFitChild(this, content);
}

} // namespace Settings

#endif // !defined Q_OS_WIN && !defined Q_OS_MAC
