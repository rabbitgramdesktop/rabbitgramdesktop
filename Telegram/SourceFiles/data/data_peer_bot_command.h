/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Data {

struct BotCommand final {
	QString command;
	QString description;
	bool ephemeral = false;

	friend inline bool operator==(
		const BotCommand &,
		const BotCommand &) = default;
};

[[nodiscard]] BotCommand BotCommandFromTL(const MTPBotCommand &result);

} // namespace Data
