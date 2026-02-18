/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "ui/chat/chat_style_radius.h"
#include "ui/chat/chat_style.h"
#include "base/options.h"
#include "rabbit/settings/rabbit_settings.h"

#include "ui/chat/chat_theme.h"
#include "ui/painter.h"
#include "ui/ui_utility.h"
#include "styles/style_chat.h"
#include <algorithm>

namespace Ui {
namespace {

base::options::toggle UseSmallMsgBubbleRadius({
	.id = kOptionUseSmallMsgBubbleRadius,
	.name = "Use small message bubble radius",
	.description = "Makes most message bubbles square-ish.",
	.restartRequired = true,
});

} // namespace

const char kOptionUseSmallMsgBubbleRadius[] = "use-small-msg-bubble-radius";

int BubbleRadiusSmall() {
	const auto setting = std::clamp(RabbitSettings::messageRoundness(), 0, 50);
	return (st::bubbleRadiusSmall * setting + 25) / 50;
}

int BubbleRadiusLarge() {
	const auto setting = std::clamp(RabbitSettings::messageRoundness(), 0, 50);
	const auto base = UseSmallMsgBubbleRadius.value()
		? st::bubbleRadiusSmall
		: st::bubbleRadiusLarge;
	return (base * setting + 25) / 50;
}

int MsgFileThumbRadiusSmall() {
	const auto setting = std::clamp(RabbitSettings::messageRoundness(), 0, 50);
	return (st::msgFileThumbRadiusSmall * setting + 25) / 50;
}

int MsgFileThumbRadiusLarge() {
	const auto setting = std::clamp(RabbitSettings::messageRoundness(), 0, 50);
	const auto base = UseSmallMsgBubbleRadius.value()
		? st::msgFileThumbRadiusSmall
		: st::msgFileThumbRadiusLarge;
	return (base * setting + 25) / 50;
}

}
