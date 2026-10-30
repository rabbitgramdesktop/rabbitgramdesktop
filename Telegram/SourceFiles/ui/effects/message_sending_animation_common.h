/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Ui {

struct MessageSendingAnimationFrom {
	enum class Type {
		None,
		Sticker,
		Gif,
		Emoji,
		Text,
	};
	struct FieldText {
		QImage frame;
		QRect globalGeometry;
		crl::time capturedAt = 0;
		bool singleLine = false;
		bool scrolled = false;
		bool rightToLeft = false;
	};
	Type type = Type::None;
	std::optional<MsgId> localId;
	QRect globalStartGeometry;
	QImage frame;
	bool crop = false;
	std::optional<FieldText> fieldText;
};

} // namespace Ui
