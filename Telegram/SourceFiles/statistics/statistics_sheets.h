/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "statistics/statistics_xlsx.h"

namespace Data {
struct AnyStatistics;
struct StatisticalGraph;
} // namespace Data

namespace Statistic {

[[nodiscard]] std::vector<Xlsx::Sheet> Sheets(
	const Data::AnyStatistics &stats,
	const Data::StatisticalGraph &pollVotes);

} // namespace Statistic
