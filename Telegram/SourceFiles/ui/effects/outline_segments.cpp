/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "ui/effects/outline_segments.h"

#include "rabbit/settings/rabbit_settings.h"

#include <algorithm>
#include <cmath>

namespace Ui {

namespace {

constexpr auto kPi = float64(3.14159265358979323846);
constexpr auto kRadiansToDegrees = float64(180.) / kPi;

struct RoundedRectPerimeter {
	QRectF rect;
	float64 radius = 0.;
	float64 horizontal = 0.;
	float64 vertical = 0.;
	float64 corner = 0.;
	float64 topHalf = 0.;
	float64 full = 0.;
};

[[nodiscard]] RoundedRectPerimeter ComputePerimeter(QRectF rect, float64 radius) {
	auto result = RoundedRectPerimeter{ .rect = rect };
	result.radius = std::clamp(radius, 0., std::min(rect.width(), rect.height()) / 2.);
	result.horizontal = std::max(0., rect.width() - 2. * result.radius);
	result.vertical = std::max(0., rect.height() - 2. * result.radius);
	result.corner = result.radius * (kPi / 2.);
	result.topHalf = result.horizontal / 2.;
	result.full = (2. * (result.horizontal + result.vertical)) + (4. * result.corner);
	return result;
}

void DrawRoundedRectSpan(
		QPainter &p,
		const RoundedRectPerimeter &shape,
		float64 start,
		float64 span) {
	if (span <= 0. || shape.full <= 0.) {
		return;
	}
	auto at = std::fmod(start, shape.full);
	if (at < 0.) {
		at += shape.full;
	}
	auto left = std::min(span, shape.full);
	const auto x = shape.rect.x();
	const auto y = shape.rect.y();
	const auto w = shape.rect.width();
	const auto h = shape.rect.height();
	const auto r = shape.radius;
	const auto q = shape.corner;
	while (left > 0.) {
		if (at < shape.topHalf) {
			const auto take = std::min(left, shape.topHalf - at);
			if (take > 0.) {
				const auto x1 = x + (w / 2.) - at;
				const auto x2 = x1 - take;
				p.drawLine(QPointF(x1, y), QPointF(x2, y));
			}
			at += take;
			left -= take;
			continue;
		}
		if (at < shape.topHalf + q) {
			const auto part = at - shape.topHalf;
			const auto take = std::min(left, (shape.topHalf + q) - at);
			if (take > 0. && r > 0.) {
				const auto from = 90. + (part / r) * kRadiansToDegrees;
				const auto len = (take / r) * kRadiansToDegrees;
				p.drawArc(
					QRectF(x, y, 2. * r, 2. * r),
					int(base::SafeRound(from * 16.)),
					int(base::SafeRound(len * 16.)));
			}
			at += take;
			left -= take;
			continue;
		}
		if (at < shape.topHalf + q + shape.vertical) {
			const auto part = at - (shape.topHalf + q);
			const auto take = std::min(left, (shape.topHalf + q + shape.vertical) - at);
			if (take > 0.) {
				const auto y1 = y + r + part;
				const auto y2 = y1 + take;
				p.drawLine(QPointF(x, y1), QPointF(x, y2));
			}
			at += take;
			left -= take;
			continue;
		}
		if (at < shape.topHalf + (2. * q) + shape.vertical) {
			const auto part = at - (shape.topHalf + q + shape.vertical);
			const auto take = std::min(
				left,
				(shape.topHalf + (2. * q) + shape.vertical) - at);
			if (take > 0. && r > 0.) {
				const auto from = 180. + (part / r) * kRadiansToDegrees;
				const auto len = (take / r) * kRadiansToDegrees;
				p.drawArc(
					QRectF(x, y + h - 2. * r, 2. * r, 2. * r),
					int(base::SafeRound(from * 16.)),
					int(base::SafeRound(len * 16.)));
			}
			at += take;
			left -= take;
			continue;
		}
		if (at < shape.topHalf + (2. * q) + shape.vertical + shape.horizontal) {
			const auto part = at - (shape.topHalf + (2. * q) + shape.vertical);
			const auto take = std::min(
				left,
				(shape.topHalf + (2. * q) + shape.vertical + shape.horizontal) - at);
			if (take > 0.) {
				const auto x1 = x + r + part;
				const auto x2 = x1 + take;
				p.drawLine(QPointF(x1, y + h), QPointF(x2, y + h));
			}
			at += take;
			left -= take;
			continue;
		}
		if (at < shape.topHalf + (3. * q) + shape.vertical + shape.horizontal) {
			const auto part = at - (shape.topHalf + (2. * q) + shape.vertical + shape.horizontal);
			const auto take = std::min(
				left,
				(shape.topHalf + (3. * q) + shape.vertical + shape.horizontal) - at);
			if (take > 0. && r > 0.) {
				const auto from = 270. + (part / r) * kRadiansToDegrees;
				const auto len = (take / r) * kRadiansToDegrees;
				p.drawArc(
					QRectF(x + w - 2. * r, y + h - 2. * r, 2. * r, 2. * r),
					int(base::SafeRound(from * 16.)),
					int(base::SafeRound(len * 16.)));
			}
			at += take;
			left -= take;
			continue;
		}
		if (at < shape.topHalf + (3. * q) + (2. * shape.vertical) + shape.horizontal) {
			const auto part = at - (shape.topHalf + (3. * q) + shape.vertical + shape.horizontal);
			const auto take = std::min(
				left,
				(shape.topHalf + (3. * q) + (2. * shape.vertical) + shape.horizontal) - at);
			if (take > 0.) {
				const auto y1 = y + h - r - part;
				const auto y2 = y1 - take;
				p.drawLine(QPointF(x + w, y1), QPointF(x + w, y2));
			}
			at += take;
			left -= take;
			continue;
		}
		if (at < shape.topHalf + (4. * q) + (2. * shape.vertical) + shape.horizontal) {
			const auto part = at - (shape.topHalf + (3. * q) + (2. * shape.vertical) + shape.horizontal);
			const auto take = std::min(
				left,
				(shape.topHalf + (4. * q) + (2. * shape.vertical) + shape.horizontal) - at);
			if (take > 0. && r > 0.) {
				const auto from = (part / r) * kRadiansToDegrees;
				const auto len = (take / r) * kRadiansToDegrees;
				p.drawArc(
					QRectF(x + w - 2. * r, y, 2. * r, 2. * r),
					int(base::SafeRound(from * 16.)),
					int(base::SafeRound(len * 16.)));
			}
			at += take;
			left -= take;
			continue;
		}
		const auto part = at - (shape.topHalf + (4. * q) + (2. * shape.vertical) + shape.horizontal);
		const auto take = std::min(left, shape.full - at);
		if (take > 0.) {
			const auto x1 = x + w - r - part;
			const auto x2 = x1 - take;
			p.drawLine(QPointF(x1, y), QPointF(x2, y));
		}
		at += take;
		left -= take;
		if (at >= shape.full) {
			at = 0.;
		}
	}
}

void PaintOutlineSegmentsRoundedRect(
		QPainter &p,
		QRectF rect,
		float64 radius,
		const std::vector<OutlineSegment> &segments,
		float64 fromFullProgress) {
	const auto count = std::min(int(segments.size()), kOutlineSegmentsMax);
	if (count == 1) {
		p.setPen(QPen(segments.front().brush, segments.front().width));
		p.drawRoundedRect(rect, radius, radius);
		return;
	}
	const auto shape = ComputePerimeter(rect, radius);
	if (shape.full <= 0.) {
		return;
	}
	const auto full = shape.full;
	const auto small = full / 36.;
	const auto separator = (full > 1.1 * small * count)
		? small
		: (full / (count * 1.1));
	const auto left = full - (separator * count);
	const auto length = left / float64(count);
	const auto spin = separator * (1. - fromFullProgress);

	auto start = (full / 4.) + (separator / 2.) + (3. * spin);
	auto pen = QPen(
		segments.back().brush,
		segments.back().width,
		Qt::SolidLine,
		Qt::RoundCap);
	p.setPen(pen);
	for (auto i = 0; i != count;) {
		const auto &segment = segments[count - (++i)];
		if (!segment.width) {
			start += length + separator;
			continue;
		} else if (pen.brush() != segment.brush
			|| pen.widthF() != segment.width) {
			pen = QPen(
				segment.brush,
				segment.width,
				Qt::SolidLine,
				Qt::RoundCap);
			p.setPen(pen);
		}
		const auto from = start;
		auto added = spin;
		for (; i != count;) {
			start += length + separator;
			const auto &next = segments[count - (++i)];
			if (next.width) {
				--i;
				break;
			}
			added += (separator + length) * (1. - fromFullProgress);
		}
		DrawRoundedRectSpan(p, shape, from, length + added);
	}
}

} // namespace

void PaintOutlineSegments(
		QPainter &p,
		QRectF ellipse,
		const std::vector<OutlineSegment> &segments,
		float64 fromFullProgress) {
	Expects(!segments.empty());

	p.setBrush(Qt::NoBrush);
	const auto radius = std::min(ellipse.width(), ellipse.height())
		* RabbitSettings::userpicRoundness()
		/ 100.;
	PaintOutlineSegmentsRoundedRect(
		p,
		ellipse,
		radius,
		segments,
		fromFullProgress);
}

void PaintOutlineSegments(
		QPainter &p,
		QRectF rect,
		float64 radius,
		const std::vector<OutlineSegment> &segments) {
	Expects(!segments.empty());

	p.setBrush(Qt::NoBrush);
	PaintOutlineSegmentsRoundedRect(p, rect, radius, segments, 1.);
}

QLinearGradient UnreadStoryOutlineGradient(
		QRectF rect,
		const QColor &c1,
		const QColor &c2) {
	auto result = QLinearGradient(rect.topRight(), rect.bottomLeft());
	result.setStops({ { 0., c1 }, { 1., c2 } });
	return result;
}

QLinearGradient UnreadStoryOutlineGradient(QRectF rect) {
	return UnreadStoryOutlineGradient(
		std::move(rect),
		st::groupCallLive1->c,
		st::groupCallMuted1->c);
}

} // namespace Ui
