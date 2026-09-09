/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

struct PollData;

namespace Ui {
class ChatStyle;
} // namespace Ui

namespace Window {
class SessionController;
} // namespace Window

namespace HistoryView {

class Element;
class ElementOverlayHost;

void ShowAddPollOptionOverlay(
	ElementOverlayHost &host,
	not_null<QWidget*> parent,
	not_null<Element*> view,
	not_null<PollData*> poll,
	FullMsgId context,
	not_null<Window::SessionController*> controller,
	not_null<const Ui::ChatStyle*> st);

} // namespace HistoryView
