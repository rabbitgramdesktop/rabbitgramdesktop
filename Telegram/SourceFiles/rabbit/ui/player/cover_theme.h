/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <QtGui/QColor>
#include <QtGui/QImage>
#include <rpl/producer.h>

class DocumentData;

namespace Rabbit::Player {

struct CoverTheme {
	QColor backgroundTop;
	QColor backgroundBottom;
	QColor text;
	QColor textSecondary;
	QColor accent;
	QColor accentSecondary;
	QColor buttonBg;
	QColor buttonBgOver;
	QColor outline;
	bool valid = false;
};

[[nodiscard]] CoverTheme GenerateCoverTheme(const QImage &image, bool isDark);
[[nodiscard]] CoverTheme InterpolateTheme(const CoverTheme &from, const CoverTheme &to, float64 progress);

void SetCurrentCoverTheme(const CoverTheme &theme);
[[nodiscard]] CoverTheme CurrentCoverTheme();
[[nodiscard]] rpl::producer<CoverTheme> CurrentCoverThemeValue();

void ExtractEmbeddedCover(
	not_null<DocumentData*> document,
	Fn<void(QImage)> callback);

} // namespace Rabbit::Player
