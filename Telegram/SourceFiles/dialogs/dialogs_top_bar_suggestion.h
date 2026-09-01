/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

template <typename Object>
class object_ptr;

namespace Main {
class Session;
} // namespace Main

namespace Ui {
class RpWidget;
template <typename Widget>
class SlideWrap;
} // namespace Ui

namespace Dialogs {

[[nodiscard]] auto TopBarSuggestionValue(
	not_null<Ui::RpWidget*> parent,
	not_null<Main::Session*>,
	rpl::producer<bool> outerWrapToggleValue,
	rpl::producer<float64> childListShown,
	rpl::producer<> prepareCollapseSnapshot)
-> rpl::producer<Ui::SlideWrap<Ui::RpWidget>*>;

[[nodiscard]] Fn<void()> ExpandChatsListCallback(
	not_null<Ui::RpWidget*> widget);

} // namespace Dialogs

