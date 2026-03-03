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
#include "rabbit/settings_menu/rabbit_settings_menu.h"
#include "rabbit/settings_menu/rabbit_context_menu.h"

#include "lang_auto.h"
#include "mainwindow.h"
#include "settings/settings_common.h"
#include "settings/settings_builder.h"
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

namespace Settings
{
    namespace
    {
        using namespace Builder;

        void AddJsonToggle(
            SectionBuilder &builder,
            QString id,
            const QString &titleKey,
            const QString &optionKey,
            QStringList keywords = {})
        {
            const auto current = RabbitSettings::JsonSettings::GetBool(optionKey);
            const auto controlId = id;
            if (const auto button = builder.addButton({
                .id = std::move(id),
                .title = rktr(titleKey),
                .st = &st::settingsButtonNoIcon,
                .toggled = rpl::single(current),
                .keywords = std::move(keywords),
            })) {
                button->toggledValue()
                    | rpl::filter([=](bool enabled) {
                        return enabled
                            != RabbitSettings::JsonSettings::GetBool(optionKey);
                    })
                    | rpl::on_next([=](bool enabled) {
                        RabbitSettings::JsonSettings::Set(optionKey, enabled);
                    }, button->lifetime());
                if (const auto controller = builder.controller()) {
                    RtgMenu::AttachSettingsContextMenu(
                        button, controlId, controller);
                }
            }
        }

        void AddToggle(
            SectionBuilder &builder,
            QString id,
            rpl::producer<QString> title,
            const style::icon *icon,
            Fn<bool()> getter,
            Fn<void(bool)> setter,
            QStringList keywords = {})
        {
            const auto st = icon
                ? &st::settingsButton
                : &st::settingsButtonNoIcon;
            const auto controlId = id;
            if (const auto button = builder.addButton({
                .id = std::move(id),
                .title = std::move(title),
                .st = st,
                .icon = { icon },
                .toggled = rpl::single(getter()),
                .keywords = std::move(keywords),
            })) {
                button->toggledValue()
                    | rpl::filter([=](bool enabled) { return enabled != getter(); })
                    | rpl::on_next([=](bool enabled) { setter(enabled); },
                        button->lifetime());
                if (const auto controller = builder.controller()) {
                    RtgMenu::AttachSettingsContextMenu(
                        button, controlId, controller);
                }
            }
        }

        void BuildGeneral(SectionBuilder &builder)
        {
            builder.addSubsectionTitle({
                .id = u"rabbit/general/general"_q,
                .title = rktr("rtg_settings_general"),
                .keywords = { u"general"_q },
            });
            
            AddToggle(
                builder,
                u"rabbit/general/show_seconds"_q,
                rktr("rtg_show_seconds"),
                nullptr,
                [] { return RabbitSettings::showSeconds(); },
                [](bool value) { RabbitSettings::setShowSeconds(value); },
                { u"seconds"_q, u"time"_q });
            
            AddJsonToggle(
                builder,
                u"rabbit/general/auto_hide_notifications"_q,
                u"rtg_general_auto_hide_notifications"_q,
                u"auto_hide_notifications"_q,
                { u"notifications"_q, u"auto"_q });
            AddJsonToggle(
                builder,
                u"rabbit/general/userpic_in_top_bar"_q,
                u"rtg_general_userpic_in_top_bar"_q,
                u"userpic_in_top_bar"_q,
                { u"userpic"_q, u"top"_q });
        }
        
        void BuildProfile(SectionBuilder &builder)
        {
            builder.addSubsectionTitle({
                .id = u"rabbit/general/profile"_q,
                .title = rktr("rtg_profile"),
                .keywords = { u"profile"_q, u"privacy"_q },
            });
            
            AddJsonToggle(
                builder,
                u"rabbit/general/streamer_mode"_q,
                u"rtg_general_streamer_mode"_q,
                u"streamer_mode"_q,
                { u"streamer"_q, u"privacy"_q });
        }

        void BuildConnectionBar(SectionBuilder &builder)
        {
            builder.addSubsectionTitle({
                .id = u"rabbit/general/connection_bar"_q,
                .title = rktr("rtg_connection_bar"),
                .keywords = { u"connection"_q, u"status"_q },
            });

            AddToggle(
                builder,
                u"rabbit/general/connection_bar_lost"_q,
                rktr("rtg_connection_bar_lost"),
                &st::menuIconNetwork,
                [] { return RabbitSettings::connectionBarLost(); },
                [](bool value) { RabbitSettings::setConnectionBarLost(value); },
                { u"connection"_q, u"lost"_q });
            AddToggle(
                builder,
                u"rabbit/general/connection_bar_proxy"_q,
                rktr("rtg_connection_bar_proxy"),
                &st::menuIconAntispam,
                [] { return RabbitSettings::connectionBarProxy(); },
                [](bool value) { RabbitSettings::setConnectionBarProxy(value); },
                { u"proxy"_q, u"connection"_q });
        }

        void BuildRabbitGeneral(SectionBuilder &builder)
        {
            builder.addSkip();
            BuildGeneral(builder);
            
            builder.addSkip();
            builder.addDivider();
            builder.addSkip();
            BuildProfile(builder);

            builder.addSkip();
            builder.addDivider();
            builder.addSkip();
            BuildConnectionBar(builder);
        }

        const auto kMeta = BuildHelper({
            .id = RabbitGeneral::Id(),
            .parentId = Rabbit::Id(),
            .title = rktr_phrase(u"rtg_settings_general"_q),
            .icon = &st::menuIconShowAll,
        }, [](SectionBuilder &builder) {
            BuildRabbitGeneral(builder);
        });

        const SectionBuildMethod kRabbitGeneralSection = kMeta.build;
    } // namespace

    rpl::producer<QString> RabbitGeneral::title()
    {
        return rktr("rtg_settings_general");
    }

    RabbitGeneral::RabbitGeneral(
        QWidget* parent,
        not_null<Window::SessionController*> controller)
        : Section(parent, controller)
    {
        setupContent();
    }

    void RabbitGeneral::setupContent()
    {
        const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
        build(content, kRabbitGeneralSection);
        Ui::ResizeFitChild(this, content);
    }
} // namespace Settings
