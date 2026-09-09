/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "data/data_star_gift.h"

namespace Data {
struct UniqueGift;
} // namespace Data

namespace Window {
class SessionController;
} // namespace Window

namespace Ui {

class Show;

struct GiftForCraftEntry {
	std::shared_ptr<Data::UniqueGift> unique;
	Data::SavedStarGiftId manageId;
};

void ShowGiftCraftInfoBox(
	not_null<Window::SessionController*> controller,
	std::shared_ptr<Data::UniqueGift> gift,
	Data::SavedStarGiftId savedId);

void ShowTestGiftCraftBox(
	not_null<Window::SessionController*> controller,
	std::vector<GiftForCraftEntry> gifts);

[[nodiscard]] bool ShowCraftLaterError(
	std::shared_ptr<Show> show,
	std::shared_ptr<Data::UniqueGift> gift);

void ShowCraftLaterError(
	std::shared_ptr<Show> show,
	TimeId when);

[[nodiscard]] bool ShowCraftAddressError(
	std::shared_ptr<Show> show,
	std::shared_ptr<Data::UniqueGift> gift);

} // namespace Ui
