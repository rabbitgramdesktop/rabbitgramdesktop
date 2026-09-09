/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class WindowListDelegate;

namespace HistoryView {
class ListWidget;
} // namespace HistoryView

namespace Ui {
class ElasticScroll;
class RpWidget;
} // namespace Ui

namespace Window {

class SectionWidget;

void SetupSwipeBackSection(
	not_null<Ui::RpWidget*> parent,
	not_null<Ui::ElasticScroll*> scroll,
	not_null<HistoryView::ListWidget*> list);

} // namespace Window
