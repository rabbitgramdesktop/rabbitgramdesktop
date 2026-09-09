/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Calls::Group {

struct PreparedMessage {
	uint64 randomId = 0;
	MTPTextWithEntities message;
};

[[nodiscard]] QByteArray SerializeMessage(const PreparedMessage &data);
[[nodiscard]] std::optional<PreparedMessage> DeserializeMessage(
	const QByteArray &data);

} // namespace Calls::Group
