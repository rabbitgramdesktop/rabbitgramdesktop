/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/quick_actions/quick_action_box.h"

#include "quick_action_ui.h"
#include "rabbit/settings/rabbit_settings.h"
#include "settings/settings_common.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "lang_auto.h"

#include <vector>

#include "styles/style_boxes.h"
#include "styles/style_settings.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"

#include "ui/wrap/vertical_layout_reorder.h"
#include "base/algorithm.h"

namespace {

void BuildQuickActionBox(
	not_null<Ui::GenericBox *> box,
	const QString &titleKey,
	const Fn<void(RabbitSettings::QuickAction)> &setter) {
	box->setTitle(rktr(titleKey));

	const auto layout = box->verticalLayout();

	const auto quickActions = std::vector<RabbitSettings::QuickAction>{
		RabbitSettings::QuickAction::Disable,
		RabbitSettings::QuickAction::Reaction,
		RabbitSettings::QuickAction::Reply,
		RabbitSettings::QuickAction::Copy,
		RabbitSettings::QuickAction::Forward,
		RabbitSettings::QuickAction::Edit,
		RabbitSettings::QuickAction::Save,
		RabbitSettings::QuickAction::Delete,
		RabbitSettings::QuickAction::Repeat,
		RabbitSettings::QuickAction::Translate
	};

	const auto addAction = [&](
		rpl::producer<QString> label,
		RabbitSettings::QuickAction quickAction,
		Settings::IconDescriptor &&descriptor) {
		Settings::AddButtonWithIcon(
			layout,
			std::move(label),
			st::settingsButton,
			std::move(descriptor)
		)->addClickHandler([=] {
			setter(quickAction);
			box->closeBox();
		});
	};

	for (const auto &action : quickActions) {
		addAction(
			RabbitSettings::QuickActionString(action),
			action,
			RabbitSettings::QuickActionIcon(action));
	}

	box->addButton(tr::lng_close(), [=] { box->closeBox(); });
	box->setWidth(st::aboutWidth);
}

} // namespace

void OutgoingQuickActionBox(not_null<Ui::GenericBox *> box) {
	BuildQuickActionBox(
		box,
		u"outgoing_quick_action"_q,
		[=](RabbitSettings::QuickAction action) {
			RabbitSettings::setOutgoingQuickAction(action);
		});
}

void IncomingQuickActionBox(not_null<Ui::GenericBox *> box) {
	BuildQuickActionBox(
		box,
		u"incoming_quick_action"_q,
		[=](RabbitSettings::QuickAction action) {
			RabbitSettings::setIncomingQuickAction(action);
		});
}

void ChannelBottomButtonBox(not_null<Ui::GenericBox *> box) {
	box->setTitle(rktr(u"rtg_chats_channel_bottom_button"_q));

	const auto layout = box->verticalLayout();

	const auto addOption = [&](rpl::producer<QString> label, int value) {
		Settings::AddButtonWithIcon(
			layout,
			std::move(label),
			st::settingsButton
		)->addClickHandler([=] {
			RabbitSettings::setChannelBottomButton(value);
			box->closeBox();
		});
	};

	addOption(rktr(u"rtg_chats_channel_bottom_button_mute"_q), 0);
	addOption(rktr(u"rtg_chats_channel_bottom_button_discuss"_q), 1);
	addOption(rktr(u"rtg_chats_channel_bottom_button_hide"_q), 2);

	box->addButton(tr::lng_close(), [=] { box->closeBox(); });
	box->setWidth(st::aboutWidth);
}


