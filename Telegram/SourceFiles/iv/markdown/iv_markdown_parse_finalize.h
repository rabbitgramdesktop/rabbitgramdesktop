/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "iv/markdown/iv_markdown_parse_convert.h"

namespace Iv::Markdown {

void NormalizeDisplayMathBlocks(
	PreparedDocument *document,
	const QByteArray &source,
	const std::vector<int> &lineStarts);
void FinalizeDocumentSemantics(PreparedDocument *document);

} // namespace Iv::Markdown
