/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Editor {

struct VideoQualityLevel {
	// Target shorter side, zero means the original resolution is kept.
	int shorterSide = 0;
	QSize resolution;

	friend inline bool operator==(
		const VideoQualityLevel &,
		const VideoQualityLevel &) = default;
};

[[nodiscard]] std::vector<VideoQualityLevel> VideoQualityLevels(QSize edited);

[[nodiscard]] QString VideoQualityLabel(const VideoQualityLevel &level);

} // namespace Editor
