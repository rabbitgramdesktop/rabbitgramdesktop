/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "emoji.h"
#include "ui/text/text.h"

namespace Ui {

struct ChatsFilterTagContext {
	base::flat_map<QString, std::unique_ptr<Text::CustomEmoji>> emoji;
	Text::MarkedContext textContext;
	QColor color;
	bool active = false;
	bool loading = false;
};

[[nodiscard]] QImage ChatsFilterTag(
	const TextWithEntities &text,
	ChatsFilterTagContext &context);

[[nodiscard]] std::unique_ptr<Text::CustomEmoji> MakeScaledSimpleEmoji(
	EmojiPtr emoji);

[[nodiscard]] std::unique_ptr<Text::CustomEmoji> MakeScaledCustomEmoji(
	std::unique_ptr<Text::CustomEmoji> wrapped);

} // namespace Ui
