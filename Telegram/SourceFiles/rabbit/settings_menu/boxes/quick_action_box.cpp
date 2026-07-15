/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/settings_menu/boxes/quick_action_box.h"

#include "rabbit/settings/quick_action_ui.h"
#include "rabbit/settings/rabbit_settings.h"
#include "settings/settings_common.h"
#include "ui/widgets/buttons.h"
#include "lang_auto.h"

#include <vector>

#include "styles/style_boxes.h"
#include "styles/style_settings.h"

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
		RabbitSettings::QuickAction::Delete
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
