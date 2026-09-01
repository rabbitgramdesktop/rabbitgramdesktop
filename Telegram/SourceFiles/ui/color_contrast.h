/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class QColor;

namespace Ui {

[[nodiscard]] float64 CountContrast(const QColor &a, const QColor &b);
[[nodiscard]] float64 CountPerceivedBrightness(const QColor &color);
[[nodiscard]] bool IsLightBackground(const QColor &background);

} // namespace Ui
