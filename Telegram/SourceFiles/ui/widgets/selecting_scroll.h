/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Ui {

class RpWidget;

void SetupSelectingScroll(
	not_null<RpWidget*> widget,
	Fn<void(int pixels)> scrollBy);

} // namespace Ui
