/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "base/basic_types.h"
#include "iv/markdown/iv_markdown_prepare.h"

namespace Ui {
class PopupMenu;
} // namespace Ui

namespace Iv::Editor {

class State;

void FillListChangeMenu(
	not_null<Ui::PopupMenu*> menu,
	not_null<State*> state,
	const Markdown::PreparedEditListItemRange &range,
	Fn<void(Fn<bool()>)> applyChange);

void FillListItemChangeMenu(
	not_null<Ui::PopupMenu*> menu,
	not_null<State*> state,
	const Markdown::PreparedEditListItemRange &range,
	Fn<void(Fn<bool()>)> applyChange);

void FillTableChangeMenu(
	not_null<Ui::PopupMenu*> menu,
	not_null<State*> state,
	const Markdown::PreparedEditTableCellRange &range,
	Fn<void(Fn<bool()>)> applyChange);

} // namespace Iv::Editor
