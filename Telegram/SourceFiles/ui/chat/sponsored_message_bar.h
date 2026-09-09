/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

namespace Data {
struct SponsoredFrom;
} // namespace Data

namespace Main {
class Session;
} // namespace Main

namespace Ui {

class RpWidget;

void FillSponsoredMessageBar(
	not_null<RpWidget*> container,
	not_null<Main::Session*> session,
	FullMsgId fullId,
	Data::SponsoredFrom from,
	const TextWithEntities &textWithEntities);

} // namespace Ui
