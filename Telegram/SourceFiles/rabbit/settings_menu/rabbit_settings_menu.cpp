/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include <ui/boxes/single_choice_box.h>

#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings_menu/rabbit_settings_menu.h"

#include "rabbit/settings_menu/sections/rabbit_appearance.h"
#include "rabbit/settings_menu/sections/rabbit_chats.h"
#include "rabbit/settings_menu/sections/rabbit_general.h"

#include "core/application.h"
#include "lang_auto.h"
#include "mainwindow.h"
#include "settings/settings_common.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/checkbox.h"
#include "ui/vertical_list.h"
#include "boxes/connection_box.h"
#include "platform/platform_specific.h"
#include "window/window_session_controller.h"
#include "lang/lang_instance.h"
#include "core/application.h"
#include "storage/localstorage.h"
#include "data/data_session.h"
#include "main/main_session.h"
#include "styles/style_settings.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "apiwrap.h"
#include "api/api_blocked_peers.h"
#include "ui/widgets/continuous_sliders.h"
#include "ui/basic_click_handlers.h"

#define SettingsMenuJsonSwitch(LangKey, Option) container->add(object_ptr<Button>( \
	container, \
	rktr(#LangKey), \
	st::settingsButtonNoIcon \
))->toggleOn( \
	rpl::single(RabbitSettings::JsonSettings::GetBool(#Option)) \
)->toggledValue( \
) | rpl::filter([](bool enabled) { \
	return (enabled != RabbitSettings::JsonSettings::GetBool(#Option)); \
}) | rpl::start_with_next([](bool enabled) { \
	RabbitSettings::JsonSettings::Set(#Option, enabled); \
}, container->lifetime());

namespace Settings {

    rpl::producer<QString> Rabbit::title() {
        return rktr("rtg_settings");
    }

    Rabbit::Rabbit(
            QWidget *parent,
            not_null<Window::SessionController *> controller)
            : Section(parent) {
        setupContent(controller);
    }

    void Rabbit::SetupRabbitSettings(not_null<Ui::VerticalLayout *> container, not_null<Window::SessionController *> controller) {
    	AddSubsectionTitle(container, rktr("rtg_settings_categories"));
    	
		const auto addSection = [&](
				rpl::producer<QString> label,
				Type type,
				IconDescriptor &&descriptor) {
			AddButtonWithIcon(
				container,
				std::move(label),
				st::settingsButton,
				std::move(descriptor)
			)->addClickHandler([=] {
				showOther(type);
			});
		};

		Ui::AddSkip(container);
    	addSection(
			rktr("rtg_settings_general"),
			RabbitGeneral::Id(),
			{ &st::menuIconShowAll });

		addSection(
			rktr("rtg_settings_appearance"),
			RabbitAppearance::Id(),
			{ &st::menuIconPalette });

		addSection(
			rktr("rtg_settings_chats"),
			RabbitChats::Id(),
			{ &st::menuIconChatBubble });
    }

	void Rabbit::SetupRabbitLinks(not_null<Ui::VerticalLayout *> container, not_null<Window::SessionController *> controller) {
		AddSubsectionTitle(container, rktr("rtg_links"));

		Ui::AddSkip(container);

		AddButtonWithLabel(
			container,
			rktr("rtg_channel_title"),
			rktr("rtg_channel_label"),
			st::settingsButton,
			{ &st::menuIconChannel }
		)->setClickedCallback([=] {
			Core::App().openLocalUrl("tg://resolve?domain=rabbitGramUpdates", {});
		});

    	AddButtonWithLabel(
			container,
			rktr("rtg_group_title"),
			rktr("rtg_group_label"),
			st::settingsButton,
			{ &st::menuIconGroups }
		)->setClickedCallback([=] {
			Core::App().openLocalUrl("tg://resolve?domain=rabbitGramDesktop", {});
		});

    	AddButtonWithLabel(
			container,
			rktr("rtg_translate_title"),
			rktr("rtg_translate_label"),
			st::settingsButton,
			{ &st::menuIconTranslate }
		)->setClickedCallback([=] {
			UrlClickHandler::Open("https://crowdin.com/project/rabbitgramdesktop");
		});

    	AddButtonWithLabel(
			container,
			rktr("rtg_source_title"),
			rktr("rtg_source_label"),
			st::settingsButton,
			{ &st::menuIconDelete }
		)->setClickedCallback([=] {
			UrlClickHandler::Open("https://github.com/rabbitgramdesktop/rabbitgramdesktop");
		});
	}

    void Rabbit::setupContent(not_null<Window::SessionController *> controller) {
        const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);

    	Ui::AddSkip(content);
        SetupRabbitSettings(content, controller);

		Ui::AddSkip(content);
    	Ui::AddDivider(content);
    	Ui::AddSkip(content);
    	SetupRabbitLinks(content, controller);

        Ui::ResizeFitChild(this, content);
    }
} // namespace Settings
