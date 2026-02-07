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
#include "rabbit/ui/settings/previews.h"

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

    [[nodiscard]] RabbitSettings::QuickAction QuickActionFrom(int value) {
        return static_cast<RabbitSettings::QuickAction>(value);
    }

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
        setupContent(controller);
    }

    void AddToggle(
        not_null<Ui::VerticalLayout*> container,
        const QString& key,
        const style::icon* icon,
        Fn<bool()> getter,
        Fn<void(bool)> setter)
    {
        Ui::SettingsButton* button = icon
                                         ? static_cast<Ui::SettingsButton*>(AddButtonWithIcon(
                                             container, rktr(key), st::settingsButton, {icon}))
                                         : container->add(
                                             object_ptr<Ui::SettingsButton>(
                                                 container, rktr(key), st::settingsButtonNoIcon));

        button->toggleOn(rpl::single(getter()))
              ->toggledValue()
            | rpl::filter([=](bool enabled) { return enabled != getter(); })
            | rpl::on_next([=](bool enabled) { setter(enabled); }, container->lifetime());
    }

    void RabbitChats::SetupChats(
        not_null<Ui::VerticalLayout*> container,
        not_null<Window::SessionController*> controller)
    {
        Ui::AddSubsectionTitle(container, rktr("rtg_settings_chats"));

        const auto chatPreview = container->add(
            object_ptr<ChatPreview>(container),
            st::defaultSubsectionTitlePadding);

        const auto stickerSizeLabel = container->add(
            object_ptr<Ui::LabelSimple>(
                container,
                st::settingsAudioVolumeLabel),
            st::settingsAudioVolumeLabelPadding);
        const auto stickerSizeSlider = container->add(
            object_ptr<Ui::MediaSlider>(
                container,
                st::settingsAudioVolumeSlider),
            st::settingsAudioVolumeSliderPadding);
        const auto updateStickerSizeLabel = [=](int value)
        {
            const auto pixels = QString::number(value);
            stickerSizeLabel->setText(ktr("rtg_chats_sticker_size", {"pixels", pixels}));
        };
        const auto updateStickerSize = [=](int value)
        {
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

        const auto messageRoundnessLabel = container->add(
            object_ptr<Ui::LabelSimple>(
                container,
                st::settingsAudioVolumeLabel),
            st::settingsAudioVolumeLabelPadding);
        const auto messageRoundnessSlider = container->add(
            object_ptr<Ui::MediaSlider>(
                container,
                st::settingsAudioVolumeSlider),
            st::settingsAudioVolumeSliderPadding);
        const auto updateMessageRoundnessLabel = [=](int value)
        {
            const auto radius = QString::number(value);
            messageRoundnessLabel->setText(ktr("rtg_chats_message_rounding", {"radius", radius}));
        };
        const auto previousMessageRoundness = container->lifetime().make_state<int>(
            RabbitSettings::messageRoundness());
        const auto updateMessageRoundness = [=](int value)
        {
            updateMessageRoundnessLabel(value);
            chatPreview->repaint();
            RabbitSettings::setMessageRoundness(value);
        };
        const auto showRestartRequired = [=](int value)
        {
            if (value == *previousMessageRoundness) {
                return;
            }
            *previousMessageRoundness = value;
            controller->show(Ui::MakeConfirmBox({
                .text = tr::lng_settings_need_restart(),
                .confirmed = [] { Core::Restart(); },
                .confirmText = tr::lng_settings_restart_now(),
                .cancelText = tr::lng_settings_restart_later(),
            }));
        };
        messageRoundnessSlider->resize(st::settingsAudioVolumeSlider.seekSize);
        messageRoundnessSlider->setPseudoDiscrete(
            51,
            [](int val) { return val; },
            RabbitSettings::messageRoundness(),
            updateMessageRoundness,
            showRestartRequired);
        updateMessageRoundnessLabel(RabbitSettings::messageRoundness());

        AddToggle(
            container, "rtg_show_actions_time", &st::menuIconReschedule,
            [] { return RabbitSettings::showActionsTime(); },
            [](bool value) { RabbitSettings::setShowActionsTime(value); }
        );

        AddToggle(
            container, "rtg_show_seconds", nullptr,
            [] { return RabbitSettings::showSeconds(); },
            [](bool value) { RabbitSettings::setShowSeconds(value); }
        );

        AddToggle(
            container, "rtg_comma_after_mention", nullptr,
            [] { return RabbitSettings::commaAfterMention(); },
            [](bool value) { RabbitSettings::setCommaAfterMention(value); }
        );

        AddToggle(
            container, "rtg_hide_bubble_tails", nullptr,
            [] { return RabbitSettings::hideBubbleTails(); },
            [](bool value) { RabbitSettings::setHideBubbleTails(value); }
        );
    }

    void RabbitChats::SetupStickerShape(not_null<Ui::VerticalLayout*> container)
    {
        Ui::AddSubsectionTitle(container, rktr("rtg_chats_sticker_shape"));

        container->add(
            object_ptr<StickerShapePicker>(container),
            st::defaultSubsectionTitlePadding);
    }
    
    void RabbitChats::SetupQuickActions(not_null<Ui::VerticalLayout*> container,
                                        not_null<Window::SessionController*> controller)
    {
        Ui::AddSubsectionTitle(container, rktr("rtg_chats_quick_actions"));
        const auto outgoingAction = QuickActionFrom(
            RabbitSettings::outgoingQuickAction());
        const auto incomingAction = QuickActionFrom(
            RabbitSettings::incomingQuickAction());
        
        AddButtonWithLabel(
            container,
            rktr("rtg_outgoing_quick_actions"),
            RabbitSettings::QuickActionString(outgoingAction),
            st::settingsButtonNoIcon
        )->addClickHandler([=] {
            controller->show(Box(OutgoingQuickActionBox));
        });

        AddButtonWithLabel(
            container,
            rktr("rtg_incoming_quick_actions"),
            RabbitSettings::QuickActionString(incomingAction),
            st::settingsButtonNoIcon
        )->addClickHandler([=] {
            controller->show(Box(IncomingQuickActionBox));
        });
    }

    void RabbitChats::SetupStickers(not_null<Ui::VerticalLayout*> container)
    {
        Ui::AddSubsectionTitle(container, rktr("rtg_chats_stickers"));

        AddToggle(
            container, "rtg_chats_more_recent_stickers", nullptr,
            [] { return RabbitSettings::moreRecentStickers(); },
            [](bool value) { RabbitSettings::setMoreRecentStickers(value); }
        );
    }

    void RabbitChats::SetupRabbitChats(
        not_null<Ui::VerticalLayout*> container,
        not_null<Window::SessionController*> controller)
    {
        Ui::AddSkip(container);
        SetupChats(container, controller);

        Ui::AddSkip(container);
        Ui::AddDivider(container);
        Ui::AddSkip(container);
        SetupStickerShape(container);
        
        Ui::AddSkip(container);
        Ui::AddDivider(container);
        Ui::AddSkip(container);
        SetupQuickActions(container, controller);

        Ui::AddSkip(container);
        Ui::AddDivider(container);
        Ui::AddSkip(container);
        SetupStickers(container);
    }

    void RabbitChats::setupContent(not_null<Window::SessionController*> controller)
    {
        const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);

        SetupRabbitChats(content, controller);

        Ui::ResizeFitChild(this, content);
    }
} // namespace Settings
