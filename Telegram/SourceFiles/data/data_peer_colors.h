/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Data {

struct ColorProfileSet {
	std::vector<QColor> palette;
	std::vector<QColor> bg;
	std::vector<QColor> story;
};

struct ColorProfileData {
	ColorProfileSet light;
	ColorProfileSet dark;
};

} // namespace Data
