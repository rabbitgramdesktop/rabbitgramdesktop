/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

class ClickHandler;

namespace Main {
class SessionShow;
} // namespace Main

namespace Api {

[[nodiscard]] std::shared_ptr<ClickHandler> AcceptClickHandler(
	not_null<HistoryItem*> item);
[[nodiscard]] std::shared_ptr<ClickHandler> DeclineClickHandler(
	not_null<HistoryItem*> item);
[[nodiscard]] std::shared_ptr<ClickHandler> SuggestChangesClickHandler(
	not_null<HistoryItem*> item);

void AddOfferToMessage(
	std::shared_ptr<Main::SessionShow> show,
	FullMsgId itemId);

} // namespace Api
