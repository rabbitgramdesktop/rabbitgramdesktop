/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class QWidget;

namespace Window {
class SessionController;
} // namespace Window

namespace Settings::RtgMenu {

void AttachSettingsContextMenu(
	not_null<QWidget*> widget,
	const QString &controlId,
	not_null<Window::SessionController*> controller);

} // namespace Settings::RtgMenu
