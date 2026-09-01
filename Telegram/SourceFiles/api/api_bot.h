/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

struct ClickHandlerContext;
class HistoryItem;
struct HistoryMessageMarkupButton;

namespace Window {
class SessionController;
} // namespace Window

namespace Api {

using BotButtonLookup = Fn<const HistoryMessageMarkupButton*()>;

void SendBotCallbackData(
	not_null<Window::SessionController*> controller,
	not_null<HistoryItem*> item,
	BotButtonLookup lookup);

void SendBotCallbackDataWithPassword(
	not_null<Window::SessionController*> controller,
	not_null<HistoryItem*> item,
	BotButtonLookup lookup);

bool SwitchInlineBotButtonReceived(
	not_null<Window::SessionController*> controller,
	const QByteArray &queryWithPeerTypes,
	UserData *samePeerBot = nullptr,
	MsgId samePeerReplyTo = 0);

void ActivateBotButton(ClickHandlerContext context, BotButtonLookup lookup);
void ActivateBotCommand(ClickHandlerContext context, int row, int column);
void ActivateRichPageBotButton(
	ClickHandlerContext context,
	const HistoryMessageMarkupButton &button);

} // namespace Api
