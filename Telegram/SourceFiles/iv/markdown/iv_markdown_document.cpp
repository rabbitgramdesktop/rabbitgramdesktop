/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "iv/markdown/iv_markdown_document.h"

#include <utility>

namespace Iv::Markdown {

PreparedDocument EmptyDocument(QString sourceName) {
	auto document = PreparedDocument();
	document.sourceName = std::move(sourceName);
	document.document.kind = NodeKind::Document;
	return document;
}

} // namespace Iv::Markdown
