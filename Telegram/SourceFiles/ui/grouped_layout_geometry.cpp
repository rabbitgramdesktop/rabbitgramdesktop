/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "ui/grouped_layout_geometry.h"

#include "ui/grouped_layout.h"

namespace Ui {

std::vector<QRect> LayoutMediaGroupGeometry(
		const std::vector<QSize> &sizes,
		int maxWidth,
		int minWidth,
		int spacing) {
	const auto layout = LayoutMediaGroup(sizes, maxWidth, minWidth, spacing);
	auto result = std::vector<QRect>();
	result.reserve(layout.size());
	for (const auto &part : layout) {
		result.push_back(part.geometry);
	}
	return result;
}

} // namespace Ui
