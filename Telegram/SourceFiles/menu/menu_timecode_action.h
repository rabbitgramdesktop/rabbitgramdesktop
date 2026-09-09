/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "base/basic_types.h"

class QAction;

namespace Ui {
class InputField;
class PopupMenu;
} // namespace Ui

namespace Menu {

void InsertTextAtCursor(
	not_null<Ui::InputField*> field,
	const QString &text);

not_null<QAction*> AddTimecodeAction(
	not_null<Ui::PopupMenu*> menu,
	const QString &timecode,
	rpl::producer<QString> updates,
	Fn<void()> callback);

} // namespace Menu
