/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "iv/markdown/iv_markdown_article_selection.h"

class QColor;

namespace Iv::Markdown {

struct MarkdownArticlePaintContext;

[[nodiscard]] QColor NonPullquoteQuoteCaptionColor(
	const MarkdownArticlePaintContext &context,
	const style::Markdown &st);
void PaintBlocks(
	Painter &p,
	const std::vector<LaidOutBlock> &blocks,
	std::vector<PreparedFormulaSlot> *formulas,
	std::vector<RenderedFormula> *renderedFormulas,
	MathRenderer *renderer,
	int devicePixelRatio,
	int outerWidth,
	const style::Markdown &st,
	const MarkdownArticlePaintContext &context);

} // namespace Iv::Markdown
