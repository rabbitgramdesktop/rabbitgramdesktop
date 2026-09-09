/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <QtCore/QRect>

namespace Ui {

// LayoutMediaGroup without the per-item sides, so that targets which do
// not link lib_ui, like td_export, can use the same album geometry.
[[nodiscard]] std::vector<QRect> LayoutMediaGroupGeometry(
	const std::vector<QSize> &sizes,
	int maxWidth,
	int minWidth,
	int spacing);

} // namespace Ui
