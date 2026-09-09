/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "ui/effects/round_checkbox.h"

namespace style {
struct RoundCheckbox;
} // namespace style

namespace Overview::Layout {

class Checkbox {
public:
	template <typename UpdateCallback>
	Checkbox(UpdateCallback callback, const style::RoundCheckbox &st)
	: _updateCallback(callback)
	, _check(st, _updateCallback) {
	}

	void paint(
		QPainter &p,
		QPoint position,
		int outerWidth,
		bool selected,
		bool selecting);

	void setActive(bool active);
	void setPressed(bool pressed);
	void setChecked(bool checked, anim::type animated = anim::type::normal);
	void finishAnimating();

	void invalidateCache() {
		_check.invalidateCache();
	}

private:
	void startAnimation();

	Fn<void()> _updateCallback;
	Ui::RoundCheckbox _check;

	Ui::Animations::Simple _pression;
	bool _active = false;
	bool _pressed = false;

};

} // namespace Overview::Layout
