/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Ui::Premium {

[[nodiscard]] float64 CubicBezier(
	float64 x1,
	float64 y1,
	float64 x2,
	float64 y2,
	float64 x);

[[nodiscard]] bool Object3dSupported();

} // namespace Ui::Premium
