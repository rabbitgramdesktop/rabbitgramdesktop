/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "data/data_peer_bot_command.h"

namespace Data {

BotCommand BotCommandFromTL(const MTPBotCommand &result) {
	return result.match([](const MTPDbotCommand &data) {
		return BotCommand {
			.command = qs(data.vcommand().v),
			.description = qs(data.vdescription().v),
			.ephemeral = data.is_ephemeral(),
		};
	});
}

} // namespace Data
