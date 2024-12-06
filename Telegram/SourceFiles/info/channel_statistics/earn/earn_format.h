/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "data/data_channel_earn.h"

namespace Info::ChannelEarn {

[[nodiscard]] QString MajorPart(Data::EarnInt value);
[[nodiscard]] QString MinorPart(Data::EarnInt value);
[[nodiscard]] QString ToUsd(
	Data::EarnInt value,
	float64 rate,
	int afterFloat);
[[nodiscard]] QString ToUsd(
	StarsAmount value,
	float64 rate,
	int afterFloat);

} // namespace Info::ChannelEarn
