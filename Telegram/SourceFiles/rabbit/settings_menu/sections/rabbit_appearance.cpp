/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings_menu/sections/rabbit_appearance.h"
#include "rabbit/settings_menu/rabbit_settings_menu.h"
#include "rabbit/settings_menu/rabbit_context_menu.h"
#include "rabbit/ui/settings/icon_picker.h"
#include "rabbit/ui/settings/previews.h"

#include "lang_auto.h"
#include "mainwindow.h"
#include "settings/settings_common.h"
#include "settings/settings_builder.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "window/window_session_controller.h"
#include "data/data_session.h"
#include "main/main_session.h"
#include "styles/style_settings.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "ui/widgets/continuous_sliders.h"

namespace Settings
{
    namespace
    {
        using namespace Builder;

        void AddToggle(
            SectionBuilder &builder,
            QString id,
            rpl::producer<QString> title,
            const style::icon *icon,
            const Fn<bool()>& getter,
            const Fn<void(bool)>& setter,
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

        void BuildAppIcon(SectionBuilder &builder)
        {
            builder.addSubsectionTitle({
                .id = u"rabbit/appearance/app_icon"_q,
                .title = rktr("rtg_settings_app_icon"),
                .keywords = { u"icon"_q, u"app"_q },
            });
            builder.add([](const WidgetContext &ctx) {
                return SectionBuilder::WidgetToAdd{
                    .widget = object_ptr<IconPicker>(ctx.container),
                    .margin = st::settingsCheckboxPadding,
                };
            }, [] {
                return SearchEntry{
                    .id = u"rabbit/appearance/app_icon_picker"_q,
                    .title = ktr("rtg_settings_app_icon"),
                    .keywords = { u"icon"_q, u"app"_q },
                };
            });
        }

        void BuildUserpic(SectionBuilder &builder)
        {
            builder.addSubsectionTitle({
                .id = u"rabbit/appearance/roundness"_q,
                .title = rktr("rtg_settings_userpic"),
                .keywords = { u"roundness"_q, u"radius"_q },
            });

            builder.add([](const WidgetContext &ctx) {
                auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);

                const auto roundnessPreview = wrap->add(
                    object_ptr<RoundnessPreview>(wrap),
                    st::defaultSubsectionTitlePadding);

                const auto userpicRoundnessLabel = wrap->add(
                    object_ptr<Ui::LabelSimple>(wrap, st::settingsAudioVolumeLabel),
                    st::settingsAudioVolumeLabelPadding);
                const auto userpicRoundnessSlider = wrap->add(
                    object_ptr<Ui::MediaSlider>(wrap, st::settingsAudioVolumeSlider),
                    st::settingsAudioVolumeSliderPadding);
                const auto updateUserpicRoundnessLabel = [=](int value) {
                    const auto radius = QString::number(value);
                    userpicRoundnessLabel->setText(ktr(
                        "rtg_settings_userpic_rounding",
                        { "radius", radius }));
                };
                const auto updateUserpicRoundness = [=](int value) {
                    updateUserpicRoundnessLabel(value);
                    roundnessPreview->repaint();
                    RabbitSettings::setUserpicRoundness(value);
                };
                userpicRoundnessSlider->resize(
                    st::settingsAudioVolumeSlider.seekSize);
                userpicRoundnessSlider->setPseudoDiscrete(
                    51,
                    [](int val) { return val; },
                    RabbitSettings::userpicRoundness(),
                    updateUserpicRoundness);
                updateUserpicRoundnessLabel(RabbitSettings::userpicRoundness());

                return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
            }, [] {
                return SearchEntry{
                    .id = u"rabbit/appearance/userpic_rounding"_q,
                    .title = ktr("rtg_settings_userpic_rounding"),
                    .keywords = { u"userpic"_q, u"roundness"_q },
                };
            });

            AddJsonToggle(
                builder,
                u"rabbit/appearance/general_roundness"_q,
                u"rtg_general_roundness"_q,
                u"general_roundness"_q,
                { u"roundness"_q, u"general"_q });
        }

        void BuildSidebar(SectionBuilder &builder)
        {
            builder.addSubsectionTitle({
                .id = u"rabbit/appearance/sidebar"_q,
                .title = rktr("rtg_sidebar_editor"),
                .keywords = { u"sidebar"_q, u"menu"_q },
            });

            AddToggle(
                builder,
                u"rabbit/appearance/sidebar_my_profile"_q,
                tr::lng_menu_my_profile(),
                &st::menuIconProfile,
                [] { return RabbitSettings::sidebarMyProfile(); },
                [](bool value) { RabbitSettings::setSidebarMyProfile(value); },
                { u"sidebar"_q, u"profile"_q });
            AddToggle(
                builder,
                u"rabbit/appearance/sidebar_bots"_q,
                tr::lng_filters_type_bots(),
                &st::menuIconBot,
                [] { return RabbitSettings::sidebarBots(); },
                [](bool value) { RabbitSettings::setSidebarBots(value); },
                { u"sidebar"_q, u"bots"_q });
            AddToggle(
                builder,
                u"rabbit/appearance/sidebar_create_group"_q,
                tr::lng_create_group_title(),
                &st::menuIconGroups,
                [] { return RabbitSettings::sidebarCreateGroup(); },
                [](bool value) { RabbitSettings::setSidebarCreateGroup(value); },
                { u"sidebar"_q, u"group"_q });
            AddToggle(
                builder,
                u"rabbit/appearance/sidebar_create_channel"_q,
                tr::lng_create_channel_title(),
                &st::menuIconChannel,
                [] { return RabbitSettings::sidebarCreateChannel(); },
                [](bool value) { RabbitSettings::setSidebarCreateChannel(value); },
                { u"sidebar"_q, u"channel"_q });
            AddToggle(
                builder,
                u"rabbit/appearance/sidebar_contacts"_q,
                tr::lng_menu_contacts(),
                &st::menuIconProfile,
                [] { return RabbitSettings::sidebarContacts(); },
                [](bool value) { RabbitSettings::setSidebarContacts(value); },
                { u"sidebar"_q, u"contacts"_q });
            AddToggle(
                builder,
                u"rabbit/appearance/sidebar_calls"_q,
                tr::lng_menu_calls(),
                &st::menuIconPhone,
                [] { return RabbitSettings::sidebarCalls(); },
                [](bool value) { RabbitSettings::setSidebarCalls(value); },
                { u"sidebar"_q, u"calls"_q });
            AddToggle(
                builder,
                u"rabbit/appearance/sidebar_saved_messages"_q,
                tr::lng_saved_messages(),
                &st::menuIconSavedMessages,
                [] { return RabbitSettings::sidebarSavedMessages(); },
                [](bool value) { RabbitSettings::setSidebarSavedMessages(value); },
                { u"sidebar"_q, u"saved"_q });
            AddToggle(
                builder,
                u"rabbit/appearance/sidebar_night_mode"_q,
                tr::lng_menu_night_mode(),
                &st::menuIconNightMode,
                [] { return RabbitSettings::sidebarNightMode(); },
                [](bool value) { RabbitSettings::setSidebarNightMode(value); },
                { u"sidebar"_q, u"night"_q, u"theme"_q });
        }

        void BuildRabbitAppearance(SectionBuilder &builder)
        {
            builder.addSkip();
            BuildAppIcon(builder);
            
            builder.addSkip();
            builder.addDivider();
            builder.addSkip();
            BuildUserpic(builder);

            builder.addSkip();
            builder.addDivider();
            builder.addSkip();
            BuildSidebar(builder);
        }

        const auto kMeta = BuildHelper({
            .id = RabbitAppearance::Id(),
            .parentId = Rabbit::Id(),
            .title = rktr_phrase(u"rtg_settings_appearance"_q),
            .icon = &st::menuIconPalette,
        }, [](SectionBuilder &builder) {
            BuildRabbitAppearance(builder);
        });

        const SectionBuildMethod kRabbitAppearanceSection = kMeta.build;
    } // namespace

    rpl::producer<QString> RabbitAppearance::title()
    {
        return rktr("rtg_settings_appearance");
    }

    RabbitAppearance::RabbitAppearance(
        QWidget* parent,
        not_null<Window::SessionController*> controller)
        : Section(parent, controller)
    {
        setupContent();
    }

    void RabbitAppearance::setupContent()
    {
        const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
        build(content, kRabbitAppearanceSection);
        Ui::ResizeFitChild(this, content);
    }
} // namespace Settings
