/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "iv/markdown/iv_markdown_document.h"

#include <QtCore/QSize>
#include <QtGui/QImage>

namespace Iv::Markdown {

struct MicrotexMeasureRequest {
	QString trimmedTex;
	MathKind kind = MathKind::Display;
	int textSize = 0;
	int renderWidthCap = 0;
	int renderHeightCap = 0;
};

struct MicrotexRenderRequest {
	QString trimmedTex;
	MathKind kind = MathKind::Display;
	int textSize = 0;
	int renderWidthCap = 0;
	int renderHeightCap = 0;
	int devicePixelRatio = 1;
};

struct MicrotexRenderResult {
	QImage image;
	MeasuredFormula measured;
};

[[nodiscard]] bool EnsureMicrotexInitialized(QString *error = nullptr);
[[nodiscard]] MeasuredFormula MeasureWithMicrotex(
	const MicrotexMeasureRequest &request);
[[nodiscard]] MicrotexRenderResult RenderWithMicrotex(
	const MicrotexRenderRequest &request);
[[nodiscard]] bool MicrotexBackendLinked();

} // namespace Iv::Markdown
