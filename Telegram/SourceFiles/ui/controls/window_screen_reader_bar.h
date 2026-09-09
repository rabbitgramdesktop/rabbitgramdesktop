/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "base/object_ptr.h"

namespace Ui {
class RpWidget;

[[nodiscard]] object_ptr<RpWidget> CreateScreenReaderBar(
	not_null<QWidget*> parent,
	Fn<void()> disableCallback);

} // namespace Ui
