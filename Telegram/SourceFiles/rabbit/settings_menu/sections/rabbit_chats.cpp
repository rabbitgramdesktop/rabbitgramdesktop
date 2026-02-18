/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include <ui/boxes/single_choice_box.h>

#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/settings/quick_action_ui.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings_menu/sections/rabbit_chats.h"
#include "rabbit/settings_menu/rabbit_settings_menu.h"
#include "rabbit/ui/settings/previews.h"

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
#include "ui/boxes/confirm_box.h"
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
#include "rabbit/settings_menu/boxes/quick_action_box.h"
#include "ui/widgets/continuous_sliders.h"

namespace Settings
{
    namespace {

    using namespace Builder;

    [[nodiscard]] RabbitSettings::QuickAction QuickActionFrom(int value) {
        return static_cast<RabbitSettings::QuickAction>(value);
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
        }
    }

    void BuildChats(SectionBuilder &builder)
    {
        builder.addSubsectionTitle({
            .id = u"rabbit/chats/settings"_q,
            .title = rktr("rtg_settings_chats"),
            .keywords = { u"chats"_q, u"messages"_q },
        });

        builder.add([](const WidgetContext &ctx) {
            auto wrap = object_ptr<Ui::VerticalLayout>(ctx.container);

            const auto chatPreview = wrap->add(
                object_ptr<ChatPreview>(wrap),
                st::defaultSubsectionTitlePadding);

            const auto stickerSizeLabel = wrap->add(
                object_ptr<Ui::LabelSimple>(wrap, st::settingsAudioVolumeLabel),
                st::settingsAudioVolumeLabelPadding);
            const auto stickerSizeSlider = wrap->add(
                object_ptr<Ui::MediaSlider>(wrap, st::settingsAudioVolumeSlider),
                st::settingsAudioVolumeSliderPadding);
            const auto updateStickerSizeLabel = [=](int value) {
                const auto pixels = QString::number(value);
                stickerSizeLabel->setText(ktr(
                    "rtg_chats_sticker_size",
                    { "pixels", pixels }));
            };
            const auto updateStickerSize = [=](int value) {
                updateStickerSizeLabel(value);
                chatPreview->repaint();
                RabbitSettings::setStickerSize(value);
            };
            stickerSizeSlider->resize(st::settingsAudioVolumeSlider.seekSize);
            stickerSizeSlider->setPseudoDiscrete(
                193,
                [](int val) { return val + 64; },
                RabbitSettings::stickerSize(),
                updateStickerSize);
            updateStickerSizeLabel(RabbitSettings::stickerSize());

            const auto messageRoundnessLabel = wrap->add(
                object_ptr<Ui::LabelSimple>(wrap, st::settingsAudioVolumeLabel),
                st::settingsAudioVolumeLabelPadding);
            const auto messageRoundnessSlider = wrap->add(
                object_ptr<Ui::MediaSlider>(wrap, st::settingsAudioVolumeSlider),
                st::settingsAudioVolumeSliderPadding);
            const auto updateMessageRoundnessLabel = [=](int value) {
                const auto radius = QString::number(value);
                messageRoundnessLabel->setText(ktr(
                    "rtg_chats_message_rounding",
                    { "radius", radius }));
            };
            const auto previousMessageRoundness = wrap->lifetime().make_state<int>(
                RabbitSettings::messageRoundness());
            const auto updateMessageRoundness = [=](int value) {
                updateMessageRoundnessLabel(value);
                chatPreview->repaint();
                RabbitSettings::setMessageRoundness(value);
            };
            const auto showRestartRequired = [=](int value) {
                if (value == *previousMessageRoundness) {
                    return;
                }
                *previousMessageRoundness = value;
                ctx.controller->show(Ui::MakeConfirmBox({
                    .text = tr::lng_settings_need_restart(),
                    .confirmed = [] { Core::Restart(); },
                    .confirmText = tr::lng_settings_restart_now(),
                    .cancelText = tr::lng_settings_restart_later(),
                }));
            };
            messageRoundnessSlider->resize(
                st::settingsAudioVolumeSlider.seekSize);
            messageRoundnessSlider->setPseudoDiscrete(
                51,
                [](int val) { return val; },
                RabbitSettings::messageRoundness(),
                updateMessageRoundness,
                showRestartRequired);
            updateMessageRoundnessLabel(RabbitSettings::messageRoundness());

            return SectionBuilder::WidgetToAdd{ .widget = std::move(wrap) };
        }, [] {
            return SearchEntry{
                .id = u"rabbit/chats/sliders"_q,
                .title = ktr("rtg_settings_chats"),
            };
        });

        builder.add(nullptr, [] {
            return SearchEntry{
                .id = u"rabbit/chats/sticker_size"_q,
                .title = ktr("rtg_chats_sticker_size"),
                .keywords = { u"stickers"_q, u"size"_q },
            };
        });
        builder.add(nullptr, [] {
            return SearchEntry{
                .id = u"rabbit/chats/message_rounding"_q,
                .title = ktr("rtg_chats_message_rounding"),
                .keywords = { u"messages"_q, u"rounding"_q, u"radius"_q },
            };
        });

        AddToggle(
            builder,
            u"rabbit/chats/show_actions_time"_q,
            rktr("rtg_show_actions_time"),
            &st::menuIconReschedule,
            [] { return RabbitSettings::showActionsTime(); },
            [](bool value) { RabbitSettings::setShowActionsTime(value); },
            { u"actions"_q, u"time"_q });
        AddToggle(
            builder,
            u"rabbit/chats/show_seconds"_q,
            rktr("rtg_show_seconds"),
            nullptr,
            [] { return RabbitSettings::showSeconds(); },
            [](bool value) { RabbitSettings::setShowSeconds(value); },
            { u"seconds"_q, u"time"_q });
        AddToggle(
            builder,
            u"rabbit/chats/comma_after_mention"_q,
            rktr("rtg_comma_after_mention"),
            nullptr,
            [] { return RabbitSettings::commaAfterMention(); },
            [](bool value) { RabbitSettings::setCommaAfterMention(value); },
            { u"mention"_q, u"comma"_q });
        AddToggle(
            builder,
            u"rabbit/chats/hide_bubble_tails"_q,
            rktr("rtg_hide_bubble_tails"),
            nullptr,
            [] { return RabbitSettings::hideBubbleTails(); },
            [](bool value) { RabbitSettings::setHideBubbleTails(value); },
            { u"bubble"_q, u"tails"_q });
    }

    void BuildStickerShape(SectionBuilder &builder)
    {
        builder.addSubsectionTitle({
            .id = u"rabbit/chats/sticker_shape"_q,
            .title = rktr("rtg_chats_sticker_shape"),
            .keywords = { u"sticker"_q, u"shape"_q },
        });

        builder.add([](const WidgetContext &ctx) {
            return SectionBuilder::WidgetToAdd{
                .widget = object_ptr<StickerShapePicker>(ctx.container),
                .margin = st::defaultSubsectionTitlePadding,
            };
        });
        builder.add(nullptr, [] {
            return SearchEntry{
                .id = u"rabbit/chats/sticker_shape_picker"_q,
                .title = ktr("rtg_chats_sticker_shape"),
                .keywords = { u"sticker"_q, u"shape"_q },
            };
        });
    }

    void BuildQuickActions(
        SectionBuilder &builder,
        Window::SessionController *controller)
    {
        builder.addSubsectionTitle({
            .id = u"rabbit/chats/quick_actions"_q,
            .title = rktr("rtg_chats_quick_actions"),
            .keywords = { u"quick"_q, u"actions"_q },
        });
        const auto outgoingAction = QuickActionFrom(
            RabbitSettings::outgoingQuickAction());
        const auto incomingAction = QuickActionFrom(
            RabbitSettings::incomingQuickAction());

        builder.addButton({
            .id = u"rabbit/chats/quick_actions/outgoing"_q,
            .title = rktr("rtg_outgoing_quick_actions"),
            .st = &st::settingsButtonNoIcon,
            .label = RabbitSettings::QuickActionString(outgoingAction),
            .onClick = controller
                ? [=] { controller->show(Box(OutgoingQuickActionBox)); }
                : Fn<void()>(),
            .keywords = { u"outgoing"_q, u"actions"_q },
        });
        builder.addButton({
            .id = u"rabbit/chats/quick_actions/incoming"_q,
            .title = rktr("rtg_incoming_quick_actions"),
            .st = &st::settingsButtonNoIcon,
            .label = RabbitSettings::QuickActionString(incomingAction),
            .onClick = controller
                ? [=] { controller->show(Box(IncomingQuickActionBox)); }
                : Fn<void()>(),
            .keywords = { u"incoming"_q, u"actions"_q },
        });
    }

    void BuildStickers(SectionBuilder &builder)
    {
        builder.addSubsectionTitle({
            .id = u"rabbit/chats/stickers"_q,
            .title = rktr("rtg_chats_stickers"),
            .keywords = { u"stickers"_q },
        });

        AddToggle(
            builder,
            u"rabbit/chats/more_recent_stickers"_q,
            rktr("rtg_chats_more_recent_stickers"),
            nullptr,
            [] { return RabbitSettings::moreRecentStickers(); },
            [](bool value) { RabbitSettings::setMoreRecentStickers(value); },
            { u"stickers"_q, u"recent"_q });
    }

    void BuildRabbitChats(
        SectionBuilder &builder,
        Window::SessionController *controller)
    {
        builder.addSkip();
        BuildChats(builder);

        builder.addSkip();
        builder.addDivider();
        builder.addSkip();
        BuildStickerShape(builder);

        builder.addSkip();
        builder.addDivider();
        builder.addSkip();
        BuildQuickActions(builder, controller);

        builder.addSkip();
        builder.addDivider();
        builder.addSkip();
        BuildStickers(builder);
    }

    const auto kMeta = BuildHelper({
        .id = RabbitChats::Id(),
        .parentId = Rabbit::Id(),
        .title = rktr_phrase(u"rtg_settings_chats"_q),
        .icon = &st::menuIconChatBubble,
    }, [](SectionBuilder &builder) {
        BuildRabbitChats(builder, builder.controller());
    });

    const SectionBuildMethod kRabbitChatsSection = kMeta.build;

    } // namespace

    rpl::producer<QString> RabbitChats::title()
    {
        return rktr("rtg_settings_chats");
    }

    RabbitChats::RabbitChats(
        QWidget* parent,
        not_null<Window::SessionController*> controller)
        : Section(parent, controller)
    {
        setupContent();
    }

    void RabbitChats::setupContent()
    {
        const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
        build(content, kRabbitChatsSection);
        Ui::ResizeFitChild(this, content);
    }
} // namespace Settings
