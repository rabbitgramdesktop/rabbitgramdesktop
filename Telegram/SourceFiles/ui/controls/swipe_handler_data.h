/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "ui/style/style_core_types.h"

namespace Ui::Controls {

struct SwipeContextData final {
	[[nodiscard]] bool empty() const {
		return !ratio
			&& !reachRatio
			&& !translation
			&& !cursorTop;
	}
	[[nodiscard]] explicit operator bool() const {
		return !empty();
	}

	float64 ratio = 0.;
	float64 reachRatio = 0.;
	float64 exactTranslation = 0.;
	int64 msgBareId = 0;
	int translation = 0;
	int cursorTop = 0;
	float64 carouselOffset = 0.;
	const style::icon *customIcon = nullptr;
	const style::icon *beforeIcon = nullptr;
	const style::icon *afterIcon = nullptr;
	const style::icon *farBeforeIcon = nullptr;
	const style::icon *farAfterIcon = nullptr;
};

struct SwipeHandlerInitData final {
	QPoint cursorPosition;
	Qt::LayoutDirection direction = Qt::LeftToRight;
};

struct SwipeBackResult final {
	rpl::lifetime lifetime;
	Fn<void(SwipeContextData)> callback;
};

} // namespace Ui::Controls
