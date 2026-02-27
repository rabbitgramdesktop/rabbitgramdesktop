/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/settings_menu/rabbit_context_menu.h"

#include "base/event_filter.h"
#include "boxes/share_box.h"
#include "lang/lang_keys.h"
#include "rabbit/deep_links/deep_links_rtg.h"
#include "rabbit/lang/rabbit_lang.h"
#include "ui/widgets/popup_menu.h"
#include "window/window_session_controller.h"

#include <QGuiApplication>
#include <QClipboard>

namespace Settings::RtgMenu {
namespace {

base::unique_qptr<Ui::PopupMenu> ContextMenu;

} // namespace

void AttachSettingsContextMenu(
		not_null<QWidget*> widget,
		const QString &controlId,
		not_null<Window::SessionController*> controller) {
	const auto deepLink = Core::DeepLinks::RtgSettingsDeepLink(controlId);
	if (deepLink.isEmpty()) {
		return;
	}

	base::install_event_filter(widget, [=](not_null<QEvent*> e) {
		if (e->type() != QEvent::ContextMenu) {
			return base::EventFilterResult::Continue;
		}

		ContextMenu = base::make_unique_q<Ui::PopupMenu>(widget);

		ContextMenu->addAction(
			tr::lng_context_copy_link(tr::now),
			[=] {
				QGuiApplication::clipboard()->setText(deepLink);
				controller->showToast(
					tr::lng_background_link_copied(tr::now));
			});

		ContextMenu->addAction(
			ktr("rtg_share"),
			[=] {
				FastShareLink(controller, deepLink);
			});

		ContextMenu->popup(QCursor::pos());
		return base::EventFilterResult::Cancel;
	});
}

} // namespace Settings::RtgMenu
