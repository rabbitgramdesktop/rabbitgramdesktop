/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Data {
struct StarGift;
struct UniqueGift;
struct UniqueGiftAttributes;
enum class GiftAttributeIdType;
} // namespace Data

namespace Window {
class SessionController;
} // namespace Window

namespace Ui {

class GenericBox;

void StarGiftPreviewBox(
	not_null<GenericBox*> box,
	const QString &title,
	Data::UniqueGiftAttributes attributes,
	Data::GiftAttributeIdType tab,
	std::shared_ptr<Data::UniqueGift> selected);

} // namespace Ui
