/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "dialogs/ui/dialogs_quick_action.h"
#include "ui/controls/swipe_handler_data.h"

namespace Lottie {
class Icon;
} // namespace Lottie

namespace Ui {
class RippleAnimation;
} // namespace Ui

namespace Dialogs::Ui {

using namespace ::Ui;

enum class QuickDialogActionLabel {
	Mute,
	Unmute,
	Pin,
	Unpin,
	Read,
	Unread,
	Archive,
	Unarchive,
	Delete,
	Ungroup,
	Disabled,
};

struct QuickActionContext {
	::Ui::Controls::SwipeContextData data;
	std::unique_ptr<Lottie::Icon> icon;
	std::unique_ptr<Ui::RippleAnimation> ripple;
	std::unique_ptr<Ui::RippleAnimation> rippleFg;
	QuickDialogAction action;
	crl::time finishedAt = 0;
};

} // namespace Dialogs::Ui
