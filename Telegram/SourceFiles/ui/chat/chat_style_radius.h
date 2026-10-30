/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Ui {

enum class BubbleCornerRounding : uchar;

[[nodiscard]] int BubbleRadiusSmall();
[[nodiscard]] int BubbleRadiusLarge();
[[nodiscard]] int BubbleCornerRadius(BubbleCornerRounding corner);

[[nodiscard]] int MsgFileThumbRadiusSmall();
[[nodiscard]] int MsgFileThumbRadiusLarge();

extern const char kOptionUseSmallMsgBubbleRadius[];

} // namespace Ui
