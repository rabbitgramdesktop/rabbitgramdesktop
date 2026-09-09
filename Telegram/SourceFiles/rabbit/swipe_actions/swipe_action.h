/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace RabbitSettings {

enum class SwipeAction {
	Disable,
	Reaction,
	Reply,
	Copy,
	Forward,
	Edit,
	Save,
	Delete,
	Repeat,
	Translate
};

inline SwipeAction SwipeActionFrom(int value) {
	return static_cast<SwipeAction>(value);
}

} // namespace RabbitSettings
