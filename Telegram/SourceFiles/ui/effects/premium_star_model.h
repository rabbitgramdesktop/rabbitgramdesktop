/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <vector>

namespace Ui::Premium {

struct StarModel {
	std::vector<float> vertices;
	int vertexCount = 0;

	[[nodiscard]] bool isNull() const {
		return vertices.empty();
	}
};

[[nodiscard]] StarModel LoadStarModel();

} // namespace Ui::Premium
