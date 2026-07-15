/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings_menu/sections/rabbit_other.h"
#include "rabbit/settings_menu/rabbit_settings_menu.h"
#include "rabbit/settings_menu/rabbit_context_menu.h"

#include "mainwindow.h"
#include "settings/settings_common.h"
#include "settings/settings_builder.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "window/window_session_controller.h"
#include "storage/localstorage.h"
#include "data/data_session.h"
#include "main/main_session.h"
#include "styles/style_settings.h"
#include "styles/style_menu_icons.h"

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

        void BuildOther(SectionBuilder &builder)
        {
            builder.addSubsectionTitle({
                .id = u"rabbit/other/other"_q,
                .title = rktr("rtg_settings_other"),
                .keywords = { u"other"_q },
            });
            
            AddJsonToggle(
                builder,
                u"rabbit/other/analytics"_q,
                u"rtg_other_analytics"_q,
                u"analytics"_q,
                { u"analytics"_q, u"telemetry"_q });
        }

        void BuildRabbitOther(SectionBuilder &builder)
        {
            builder.addSkip();
            BuildOther(builder);
        }

        const auto kMeta = BuildHelper({
            .id = RabbitOther::Id(),
            .parentId = Rabbit::Id(),
            .title = rktr_phrase(u"rtg_settings_other"_q),
            .icon = &st::menuIconShowAll,
        }, [](SectionBuilder &builder) {
            BuildRabbitOther(builder);
        });

        const SectionBuildMethod kRabbitOtherSection = kMeta.build;
    } // namespace

    rpl::producer<QString> RabbitOther::title()
    {
        return rktr("rtg_settings_other");
    }

    RabbitOther::RabbitOther(
        QWidget* parent,
        not_null<Window::SessionController*> controller)
        : Section(parent, controller)
    {
        setupContent();
    }

    void RabbitOther::setupContent()
    {
        const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
        build(content, kRabbitOtherSection);
        Ui::ResizeFitChild(this, content);
    }
} // namespace Settings
