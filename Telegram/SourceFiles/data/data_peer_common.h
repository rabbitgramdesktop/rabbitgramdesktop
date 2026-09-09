/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Data {

struct StarsRating {
	int level = 0;
	int stars = 0;
	int thisLevelStars = 0;
	int nextLevelStars = 0;

	explicit operator bool() const {
		return level != 0 || thisLevelStars != 0;
	}

	friend inline bool operator==(StarsRating, StarsRating) = default;
};

struct StarsRatingPending {
	StarsRating value;
	TimeId date = 0;

	explicit operator bool() const {
		return value && date;
	}
};

} // namespace Data
