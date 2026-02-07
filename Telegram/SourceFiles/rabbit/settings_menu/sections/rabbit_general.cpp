/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include <ui/boxes/single_choice_box.h>

#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings_menu/sections/rabbit_general.h"

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
#include "styles/style_window.h"
#include "apiwrap.h"
#include "api/api_blocked_peers.h"
#include "ui/widgets/continuous_sliders.h"

#define SettingsMenuJsonSwitch(LangKey, Option) container->add(object_ptr<Button>( \
	container, \
	rktr(#LangKey), \
	st::settingsButtonNoIcon \
))->toggleOn( \
	rpl::single(RabbitSettings::JsonSettings::GetBool(#Option)) \
)->toggledValue( \
) | rpl::filter([](bool enabled) { \
	return (enabled != RabbitSettings::JsonSettings::GetBool(#Option)); \
}) | rpl::on_next([](bool enabled) { \
	RabbitSettings::JsonSettings::Set(#Option, enabled); \
}, container->lifetime());

namespace Settings
{
    rpl::producer<QString> RabbitGeneral::title()
    {
        return rktr("rtg_settings_general");
    }

    RabbitGeneral::RabbitGeneral(
        QWidget* parent,
        not_null<Window::SessionController*> controller)
        : Section(parent, controller)
    {
        setupContent(controller);
    }

    void RabbitGeneral::SetupGeneral(not_null<Ui::VerticalLayout*> container)
    {
        SettingsMenuJsonSwitch(rtg_general_streamer_mode, streamer_mode);
        SettingsMenuJsonSwitch(rtg_general_auto_hide_notifications, auto_hide_notifications);
        SettingsMenuJsonSwitch(rtg_general_userpic_in_top_bar, userpic_in_top_bar);
    }

    void RabbitGeneral::SetupConnectionBar(not_null<Ui::VerticalLayout*> container)
    {
        Ui::AddSubsectionTitle(container, rktr("rtg_connection_bar"));

        AddButtonWithIcon(
            container,
            rktr("rtg_connection_bar_lost"),
            st::settingsButton,
            IconDescriptor{&st::menuIconNetwork}
        )->toggleOn(
            rpl::single(RabbitSettings::connectionBarLost())
        )->toggledValue(
        ) | rpl::filter([](bool enabled)
        {
            return (enabled != RabbitSettings::connectionBarLost());
        }) | rpl::on_next([](bool enabled)
        {
            RabbitSettings::setConnectionBarLost(enabled);
        }, container->lifetime());

        AddButtonWithIcon(
            container,
            rktr("rtg_connection_bar_proxy"),
            st::settingsButton,
            IconDescriptor{&st::menuIconAntispam}
        )->toggleOn(
            rpl::single(RabbitSettings::connectionBarProxy())
        )->toggledValue(
        ) | rpl::filter([](bool enabled)
        {
            return (enabled != RabbitSettings::connectionBarProxy());
        }) | rpl::on_next([](bool enabled)
        {
            RabbitSettings::setConnectionBarProxy(enabled);
        }, container->lifetime());
    }

    void RabbitGeneral::SetupRabbitGeneral(not_null<Ui::VerticalLayout*> container,
                                           not_null<Window::SessionController*> controller)
    {
        Ui::AddSkip(container);
        SetupGeneral(container);

        Ui::AddSkip(container);
        Ui::AddDivider(container);
        Ui::AddSkip(container);
        SetupConnectionBar(container);
    }

    void RabbitGeneral::setupContent(not_null<Window::SessionController*> controller)
    {
        const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);

        SetupRabbitGeneral(content, controller);

        Ui::ResizeFitChild(this, content);
    }
} // namespace Settings
