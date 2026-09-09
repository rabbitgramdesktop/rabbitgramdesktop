/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "iv/markdown/iv_markdown_parse.h"

namespace Iv::Markdown {

[[nodiscard]] ParseResult Failure(QString sourceName, QString error);
[[nodiscard]] MarkdownSourceValidationResult ValidationFailure(
	QString sourceName,
	QString error);

} // namespace Iv::Markdown
