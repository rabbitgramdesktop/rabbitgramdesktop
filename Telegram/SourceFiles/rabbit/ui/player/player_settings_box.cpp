/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/ui/player/player_settings_box.h"

#include "rabbit/settings/rabbit_settings.h"
#include "ui/layers/generic_box.h"
#include "ui/widgets/checkbox.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/popup_menu.h"
#include "settings/settings_common.h"
#include "window/window_session_controller.h"
#include "styles/style_boxes.h"
#include "styles/style_settings.h"
#include "styles/style_media_player.h"
#include "styles/style_menu_icons.h"
#include "lang/lang_keys.h"
#include <rpl/variable.h>

namespace Rabbit {

void ShowPlayerSettingsBox(Window::SessionController *controller) {
	if (!controller) {
		return;
	}

	controller->show(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(u"Настройки плеера"_q);

		const auto layout = box->verticalLayout();

		const auto getSummaryText = [] {
			const auto big = RabbitSettings::playerCoverColorsFullscreen();
			const auto mini = RabbitSettings::playerCoverColorsMini();
			if (big && mini) {
				return u"Большой плеер, мини-плеер"_q;
			} else if (big) {
				return u"Большой плеер"_q;
			} else if (mini) {
				return u"Мини-плеер"_q;
			}
			return u"Выключено"_q;
		};

		const auto summary = box->lifetime().make_state<rpl::variable<QString>>(getSummaryText());

		const auto button = Settings::AddButtonWithLabel(
			layout,
			rpl::single(u"Цвета обложки"_q),
			summary->value(),
			st::settingsButton,
			{ &st::menuIconPalette });

		const auto checkFullscreen = layout->add(
			object_ptr<Ui::Checkbox>(
				layout,
				u"Большой плеер"_q,
				RabbitSettings::playerCoverColorsFullscreen(),
				st::defaultCheckbox),
			QMargins(
				st::defaultCheckbox.margin.left() + 20,
				st::defaultCheckbox.margin.top(),
				st::defaultCheckbox.margin.right(),
				st::defaultCheckbox.margin.bottom() / 2));

		const auto checkMini = layout->add(
			object_ptr<Ui::Checkbox>(
				layout,
				u"Мини-плеер"_q,
				RabbitSettings::playerCoverColorsMini(),
				st::defaultCheckbox),
			QMargins(
				st::defaultCheckbox.margin.left() + 20,
				st::defaultCheckbox.margin.top() / 2,
				st::defaultCheckbox.margin.right(),
				st::defaultCheckbox.margin.bottom()));

		checkFullscreen->checkedChanges(
		) | rpl::on_next([=](bool checked) {
			if (RabbitSettings::playerCoverColorsFullscreen() != checked) {
				RabbitSettings::setPlayerCoverColorsFullscreen(checked);
				*summary = getSummaryText();
			}
		}, checkFullscreen->lifetime());

		checkMini->checkedChanges(
		) | rpl::on_next([=](bool checked) {
			if (RabbitSettings::playerCoverColorsMini() != checked) {
				RabbitSettings::setPlayerCoverColorsMini(checked);
				*summary = getSummaryText();
			}
		}, checkMini->lifetime());

		button->setClickedCallback([=] {
			auto menu = base::make_unique_q<Ui::PopupMenu>(button, st::defaultPopupMenu);
			const auto big = RabbitSettings::playerCoverColorsFullscreen();
			const auto mini = RabbitSettings::playerCoverColorsMini();

			menu->addAction(
				u"Большой плеер"_q,
				[=] {
					const auto next = !RabbitSettings::playerCoverColorsFullscreen();
					RabbitSettings::setPlayerCoverColorsFullscreen(next);
					checkFullscreen->setChecked(next);
					*summary = getSummaryText();
				},
				big ? &st::mediaPlayerMenuCheck : nullptr);

			menu->addAction(
				u"Мини-плеер"_q,
				[=] {
					const auto next = !RabbitSettings::playerCoverColorsMini();
					RabbitSettings::setPlayerCoverColorsMini(next);
					checkMini->setChecked(next);
					*summary = getSummaryText();
				},
				mini ? &st::mediaPlayerMenuCheck : nullptr);

			menu->popup(button->mapToGlobal(QPoint(button->width(), button->height())));
		});

		box->addButton(tr::lng_close(), [=] { box->closeBox(); });
	}));
}

} // namespace Rabbit
