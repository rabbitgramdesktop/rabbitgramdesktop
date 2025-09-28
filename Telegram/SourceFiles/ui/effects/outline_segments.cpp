/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "ui/effects/outline_segments.h"

#include "rabbit/settings/rabbit_settings.h"

namespace Ui {

void PaintOutlineSegments(
		QPainter &p,
		QRectF ellipse,
		const std::vector<OutlineSegment> &segments,
		float64 fromFullProgress) {
	Expects(!segments.empty());

	auto radius = ellipse.height() * RabbitSettings::userpicRoundness() * .0102;

	p.setBrush(Qt::NoBrush);
	p.setPen(QPen(segments.front().brush, segments.front().width));
	p.drawRoundedRect(
		ellipse, radius, radius);
	return;
}

void PaintOutlineSegments(
		QPainter &p,
		QRectF rect,
		float64 radius,
		const std::vector<OutlineSegment> &segments) {
	Expects(!segments.empty());

	p.setBrush(Qt::NoBrush);
	const auto count = std::min(int(segments.size()), kOutlineSegmentsMax);
	if (count == 1 || true) {
		p.setPen(QPen(segments.back().brush, segments.back().width));
		p.drawRoundedRect(rect, radius, radius);
		return;
	}
}

QLinearGradient UnreadStoryOutlineGradient(QRectF rect) {
	auto result = QLinearGradient(rect.topRight(), rect.bottomLeft());
	result.setStops({
		{ 0., st::groupCallLive1->c },
		{ 1., st::groupCallMuted1->c },
	});
	return result;
}

} // namespace Ui
