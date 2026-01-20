/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "ui/text/format_song_name.h"

class DocumentData;

namespace Ui::Text {

[[nodiscard]] FormatSongName FormatSongNameFor(
	not_null<DocumentData*> document);

[[nodiscard]] TextWithEntities FormatDownloadsName(
	not_null<DocumentData*> document);

[[nodiscard]] FormatSongName FormatVoiceName(
	not_null<DocumentData*> document,
	FullMsgId contextId);

} // namespace Ui::Text
