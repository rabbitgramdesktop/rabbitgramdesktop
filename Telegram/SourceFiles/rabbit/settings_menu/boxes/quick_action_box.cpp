/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/settings_menu/boxes/quick_action_box.h"

#include "lang_auto.h"
#include "history/view/controls/history_view_ttl_button.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/settings_menu/rabbit_settings_menu.h"

#include "settings/settings_common.h"
#include "styles/style_boxes.h"
#include "styles/style_settings.h"

void OutcomingQuickActionBox(not_null<Ui::GenericBox*> box)
{
    box->setTitle(rktr("outcoming_quick_action"));
    
    auto layout = box->verticalLayout();
    
    std::vector quick_actions {
        RabbitSettings::QuickAction::Disable,
        RabbitSettings::QuickAction::Reaction,
        RabbitSettings::QuickAction::Reply,
        RabbitSettings::QuickAction::Copy,
        RabbitSettings::QuickAction::Forward,
        RabbitSettings::QuickAction::Edit,
        RabbitSettings::QuickAction::Save,
        RabbitSettings::QuickAction::Delete
    };
    
    const auto addAction = [&](
            rpl::producer<QString> label,
            RabbitSettings::QuickAction quick_action,
            Settings::IconDescriptor &&descriptor)
    {
        Settings::AddButtonWithIcon(
            layout,
            std::move(label),
            st::settingsButton,
            std::move(descriptor)
        )->addClickHandler([=] {
            RabbitSettings::setOutcomingQuickAction(quick_action);
            box->closeBox();
        });
    };
    
    for (auto action: quick_actions)
    {
        addAction(
            RabbitSettings::QuickActionString(action),
            action,
            RabbitSettings::QuickActionIcon(action));
    }
    
    box->addButton(tr::lng_close(), [=] { box->closeBox(); });
    box->setWidth(st::aboutWidth);
}
