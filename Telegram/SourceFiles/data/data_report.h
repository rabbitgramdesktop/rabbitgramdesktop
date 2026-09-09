/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Data {

struct ReportInput final {
	QByteArray optionId;
	QString optionText;
	QString comment;
	std::vector<MsgId> ids;
	std::vector<StoryId> stories;

	inline bool operator==(const ReportInput &other) const {
		return optionId == other.optionId && comment == other.comment;
	}
};

} // namespace Data
