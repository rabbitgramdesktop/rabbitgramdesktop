/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Data {

struct UnreviewedAuth {
	uint64 hash = 0;
	bool unconfirmed = false;
	TimeId date = 0;
	QString device;
	QString location;
};

} // namespace Data
