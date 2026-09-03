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

void SwipeActionsBox(not_null<Ui::GenericBox *> box) {
	box->setTitle(rktr(u"rtg_chats_swipe_actions"_q));

	box->addRow(
		object_ptr<Ui::FlatLabel>(
			box,
			rktr(u"rtg_chats_swipe_actions_about"_q),
			st::boxDividerLabel),
		st::defaultSubsectionTitlePadding);

	const auto currentList = RabbitSettings::swipeActionsList();
	const auto show = box->uiShow();

	// 1. Active Section Header
	Ui::AddSubsectionTitle(box->verticalLayout(), rktr(u"rtg_chats_swipe_actions_active"_q));

	const auto activeContainer = box->verticalLayout()->add(
		object_ptr<Ui::VerticalLayout>(box->verticalLayout()));

	auto activeButtons = std::vector<not_null<Settings::Button*>>();
	auto activeHandles = std::vector<not_null<Ui::RpWidget*>>();

	const auto canRemove = (currentList.size() > 1);

	for (int i = 0; i < int(currentList.size()); ++i) {
		const auto action = currentList[i];
		const auto button = Settings::AddButtonWithIcon(
			activeContainer,
			RabbitSettings::QuickActionString(action),
			st::settingsButton,
			RabbitSettings::QuickActionIcon(action));

		activeButtons.push_back(button);

		const auto handle = Ui::CreateChild<Ui::RpWidget>(button.get());
		const auto &reorderIcon = st::menuIconReorderHandle;
		handle->resize(reorderIcon.width(), reorderIcon.height());
		handle->setCursor(style::cur_sizeall);
		handle->paintRequest(
		) | rpl::on_next([=] {
			auto p = QPainter(handle);
			reorderIcon.paint(p, 0, 0, handle->width());
		}, handle->lifetime());

		handle->events(
		) | rpl::on_next([=](not_null<QEvent*> e) {
			if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease) {
				e->accept();
			}
		}, handle->lifetime());

		handle->show();
		activeHandles.push_back(handle);

		Ui::IconButton *removeIcon = nullptr;
		if (canRemove) {
			removeIcon = Ui::CreateChild<Ui::IconButton>(
				button,
				st::defaultIconButton);
			removeIcon->setIconOverride(&st::menuIconRemove);
			removeIcon->show();

			removeIcon->addClickHandler([=] {
				auto newList = RabbitSettings::swipeActionsList();
				if (newList.size() > 1 && i < int(newList.size())) {
					newList.erase(newList.begin() + i);
					RabbitSettings::setSwipeActionsList(newList);
					show->showBox(Box(SwipeActionsBox), Ui::LayerOption::CloseOther);
				}
			});
		}

		button->addClickHandler([=] {
			auto newList = RabbitSettings::swipeActionsList();
			if (newList.size() > 1 && i < int(newList.size())) {
				newList.erase(newList.begin() + i);
				RabbitSettings::setSwipeActionsList(newList);
				show->showBox(Box(SwipeActionsBox), Ui::LayerOption::CloseOther);
			}
		});

		button->sizeValue(
		) | rpl::on_next([=](QSize size) {
			const auto rightMargin = st::settingsButton.padding.right();
			const auto handleX = size.width() - handle->width() - rightMargin;
			handle->move(handleX, (size.height() - handle->height()) / 2);

			if (removeIcon) {
				const auto removeX = handleX - removeIcon->width();
				removeIcon->move(removeX, (size.height() - removeIcon->height()) / 2);
			}
		}, button->lifetime());
	}

	// Attach drag-and-drop reorder controller for active actions
	if (currentList.size() > 1) {
		const auto reorder = box->lifetime().make_state<Ui::VerticalLayoutReorder>(
			activeContainer
		);
		reorder->setMouseEventProxy([=](int index) -> not_null<Ui::RpWidget*> {
			if (index >= 0 && index < int(activeHandles.size())) {
				return activeHandles[index];
			}
			return activeButtons[index];
		});
		reorder->start();
		reorder->updates(
		) | rpl::on_next([=](Ui::VerticalLayoutReorder::Single data) {
			if (data.state == Ui::VerticalLayoutReorder::State::Applied) {
				auto list = RabbitSettings::swipeActionsList();
				if (data.oldPosition >= 0 && data.oldPosition < int(list.size()) &&
					data.newPosition >= 0 && data.newPosition < int(list.size())) {
					base::reorder(list, data.oldPosition, data.newPosition);
					RabbitSettings::setSwipeActionsList(list);
					show->showBox(Box(SwipeActionsBox), Ui::LayerOption::CloseOther);
				}
			}
		}, activeContainer->lifetime());
	}

	// 2. Disabled / Available Section
	const auto allPossibleActions = std::vector<RabbitSettings::QuickAction>{
		RabbitSettings::QuickAction::Reply,
		RabbitSettings::QuickAction::Reaction,
		RabbitSettings::QuickAction::Copy,
		RabbitSettings::QuickAction::Forward,
		RabbitSettings::QuickAction::Edit,
		RabbitSettings::QuickAction::Save,
		RabbitSettings::QuickAction::Repeat,
		RabbitSettings::QuickAction::Delete,
		RabbitSettings::QuickAction::Translate,
	};

	auto disabledActions = std::vector<RabbitSettings::QuickAction>();
	for (const auto act : allPossibleActions) {
		if (!ranges::contains(currentList, act)) {
			disabledActions.push_back(act);
		}
	}

	if (!disabledActions.empty()) {
		Ui::AddSubsectionTitle(box->verticalLayout(), rktr(u"rtg_chats_swipe_actions_disabled"_q));
		const auto disabledContainer = box->verticalLayout()->add(
			object_ptr<Ui::VerticalLayout>(box->verticalLayout()));

		for (const auto act : disabledActions) {
			const auto button = Settings::AddButtonWithIcon(
				disabledContainer,
				RabbitSettings::QuickActionString(act),
				st::settingsButton,
				RabbitSettings::QuickActionIcon(act));

			const auto addIcon = Ui::CreateChild<Ui::IconButton>(
				button,
				st::defaultIconButton);
			addIcon->setIconOverride(&st::menuIconInvite);
			addIcon->show();

			button->sizeValue(
			) | rpl::on_next([=](QSize size) {
				addIcon->move(
					size.width() - addIcon->width() - st::settingsButton.padding.right(),
					(size.height() - addIcon->height()) / 2);
			}, addIcon->lifetime());

			button->addClickHandler([=] {
				auto newList = RabbitSettings::swipeActionsList();
				newList.push_back(act);
				RabbitSettings::setSwipeActionsList(newList);
				show->showBox(Box(SwipeActionsBox), Ui::LayerOption::CloseOther);
			});
		}
	}

	box->addButton(tr::lng_close(), [=] { box->closeBox(); });
	box->setWidth(st::aboutWidth);
}
