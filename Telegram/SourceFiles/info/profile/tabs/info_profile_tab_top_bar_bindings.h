/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "info/info_wrap_widget.h"

namespace Ui::Menu {
struct MenuCallback;
} // namespace Ui::Menu

namespace Info::Profile {

struct TabTopBarBindings {
	rpl::producer<TextWithEntities> title;
	rpl::producer<TextWithEntities> subtitle;
	Fn<void(const Ui::Menu::MenuCallback&)> fillMenu;
	rpl::producer<SelectedItems> selectedItems;
	rpl::producer<bool> searchEnabledByContent;
	Fn<void(SelectionAction)> selectionAction;
	Fn<void(QString)> applySearchQuery;
	rpl::producer<bool> groupByRoleState;
	Fn<void(bool)> setGroupByRole;
	rpl::producer<bool> groupByRoleAvailable;
};

} // namespace Info::Profile
