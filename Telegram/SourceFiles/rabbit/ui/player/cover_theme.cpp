/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/ui/player/cover_theme.h"

#include "ui/effects/animations.h"
#include "data/data_document.h"
#include "data/data_document_media.h"
#include "data/data_session.h"
#include "main/main_session.h"
#include "core/file_location.h"
#include "media/audio/media_audio.h"
#include "ui/chat/attach/attach_prepare.h"
#include <rpl/variable.h>
#include <QtCore/QFileInfo>
#include <algorithm>
#include <cmath>

namespace Rabbit::Player {
namespace {

rpl::variable<CoverTheme> &CurrentThemeVariable() {
	static rpl::variable<CoverTheme> stream;
	return stream;
}

} // namespace

CoverTheme GenerateCoverTheme(const QImage &image, bool isDark) {
	if (image.isNull()) {
		return CoverTheme();
	}

	const auto scaled = image.scaled(48, 48, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	if (scaled.isNull()) {
		return CoverTheme();
	}

	struct Bucket {
		float64 totalR = 0;
		float64 totalG = 0;
		float64 totalB = 0;
		float64 weight = 0;
	};

	constexpr int kHueBuckets = 16;
	Bucket buckets[kHueBuckets];
	float64 maxWeight = 0;
	int bestBucket = -1;

	for (int y = 0; y < scaled.height(); ++y) {
		const auto line = reinterpret_cast<const QRgb*>(scaled.constScanLine(y));
		for (int x = 0; x < scaled.width(); ++x) {
			const auto rgb = line[x];
			if (qAlpha(rgb) < 128) {
				continue;
			}
			const auto color = QColor::fromRgb(rgb);
			const auto h = color.hsvHueF();
			const auto s = color.hsvSaturationF();
			const auto v = color.valueF();

			// Skip near-black, near-white, or desaturated pixels for dominant hue selection
			if (v < 0.15 || v > 0.95 || s < 0.15) {
				continue;
			}

			// Score: prefer saturated and vibrant colors
			const auto weight = s * (1.0 - std::abs(v - 0.6));
			const auto bucketIdx = std::clamp(static_cast<int>(h * kHueBuckets), 0, kHueBuckets - 1);
			buckets[bucketIdx].totalR += color.redF() * weight;
			buckets[bucketIdx].totalG += color.greenF() * weight;
			buckets[bucketIdx].totalB += color.blueF() * weight;
			buckets[bucketIdx].weight += weight;

			if (buckets[bucketIdx].weight > maxWeight) {
				maxWeight = buckets[bucketIdx].weight;
				bestBucket = bucketIdx;
			}
		}
	}

	QColor baseColor;
	if (bestBucket >= 0 && maxWeight > 0) {
		const auto &b = buckets[bestBucket];
		baseColor = QColor::fromRgbF(
			std::clamp(b.totalR / b.weight, 0.0, 1.0),
			std::clamp(b.totalG / b.weight, 0.0, 1.0),
			std::clamp(b.totalB / b.weight, 0.0, 1.0));
	} else {
		// Fallback: average color of the image
		int64 r = 0, g = 0, b = 0, count = 0;
		for (int y = 0; y < scaled.height(); ++y) {
			const auto line = reinterpret_cast<const QRgb*>(scaled.constScanLine(y));
			for (int x = 0; x < scaled.width(); ++x) {
				if (qAlpha(line[x]) >= 128) {
					r += qRed(line[x]);
					g += qGreen(line[x]);
					b += qBlue(line[x]);
					++count;
				}
			}
		}
		if (count > 0) {
			baseColor = QColor(r / count, g / count, b / count);
		} else {
			baseColor = QColor(60, 60, 80);
		}
	}

	const auto h = baseColor.hslHue();
	const auto s = baseColor.hslSaturation();

	CoverTheme theme;
	theme.valid = true;

	if (isDark) {
		theme.backgroundTop = QColor::fromHsl(
			h >= 0 ? h : 220,
			std::clamp(static_cast<int>(s * 0.45), 20, 120),
			24);
		theme.backgroundBottom = QColor::fromHsl(
			h >= 0 ? h : 220,
			std::clamp(static_cast<int>(s * 0.35), 15, 90),
			13);
		theme.accent = QColor::fromHsl(
			h >= 0 ? h : 220,
			std::clamp(static_cast<int>(s * 1.2), 160, 255),
			155);
		theme.accentSecondary = QColor::fromHsl(
			h >= 0 ? h : 220,
			std::clamp(static_cast<int>(s * 0.9), 120, 220),
			180);
		theme.text = QColor(255, 255, 255);
		theme.textSecondary = QColor(255, 255, 255, 185);
		theme.buttonBg = QColor(255, 255, 255, 36);
		theme.buttonBgOver = QColor(255, 255, 255, 60);
		theme.outline = QColor(255, 255, 255, 80);
	} else {
		theme.backgroundTop = QColor::fromHsl(
			h >= 0 ? h : 220,
			std::clamp(static_cast<int>(s * 0.40), 20, 100),
			238);
		theme.backgroundBottom = QColor::fromHsl(
			h >= 0 ? h : 220,
			std::clamp(static_cast<int>(s * 0.30), 15, 80),
			248);
		theme.accent = QColor::fromHsl(
			h >= 0 ? h : 220,
			std::clamp(static_cast<int>(s * 1.1), 160, 255),
			110);
		theme.accentSecondary = QColor::fromHsl(
			h >= 0 ? h : 220,
			std::clamp(static_cast<int>(s * 0.9), 120, 220),
			85);
		theme.text = QColor(20, 20, 25);
		theme.textSecondary = QColor(20, 20, 25, 175);
		theme.buttonBg = QColor(0, 0, 0, 18);
		theme.buttonBgOver = QColor(0, 0, 0, 36);
		theme.outline = QColor(0, 0, 0, 60);
	}

	return theme;
}

CoverTheme InterpolateTheme(const CoverTheme &from, const CoverTheme &to, float64 progress) {
	if (!from.valid) return to;
	if (!to.valid) return from;
	const auto p = std::clamp(progress, 0.0, 1.0);
	return CoverTheme{
		.backgroundTop = anim::color(from.backgroundTop, to.backgroundTop, p),
		.backgroundBottom = anim::color(from.backgroundBottom, to.backgroundBottom, p),
		.text = anim::color(from.text, to.text, p),
		.textSecondary = anim::color(from.textSecondary, to.textSecondary, p),
		.accent = anim::color(from.accent, to.accent, p),
		.accentSecondary = anim::color(from.accentSecondary, to.accentSecondary, p),
		.buttonBg = anim::color(from.buttonBg, to.buttonBg, p),
		.buttonBgOver = anim::color(from.buttonBgOver, to.buttonBgOver, p),
		.outline = anim::color(from.outline, to.outline, p),
		.valid = true,
	};
}

void SetCurrentCoverTheme(const CoverTheme &theme) {
	CurrentThemeVariable() = theme;
}

CoverTheme CurrentCoverTheme() {
	return CurrentThemeVariable().current();
}

rpl::producer<CoverTheme> CurrentCoverThemeValue() {
	return CurrentThemeVariable().value();
}

void ExtractEmbeddedMetadata(
		not_null<DocumentData*> document,
		Fn<void(EmbeddedMetadata)> callback) {
	const auto guard = base::make_weak(&document->session());
	const auto &loc = document->location(true);
	if (loc.accessEnable()) {
		const auto path = loc.name();
		if (!path.isEmpty() && QFileInfo::exists(path)) {
			crl::async([=, callback = std::move(callback)] {
				auto info = Media::Player::PrepareForSending(path, QByteArray());
				if (const auto song = std::get_if<Ui::PreparedFileInformation::Song>(&info.media)) {
					EmbeddedMetadata meta{
						.cover = song->cover,
						.album = song->album,
						.title = song->title,
						.performer = song->performer,
					};
					crl::on_main(guard, [=, meta = std::move(meta)]() mutable {
						callback(std::move(meta));
					});
					return;
				}
				crl::on_main(guard, [=] {
					callback(EmbeddedMetadata());
				});
			});
			return;
		}
	}
	if (const auto media = document->activeMediaView()) {
		const auto bytes = media->bytes();
		if (!bytes.isEmpty()) {
			crl::async([=, callback = std::move(callback), bytes = bytes] {
				auto info = Media::Player::PrepareForSending(QString(), bytes);
				if (const auto song = std::get_if<Ui::PreparedFileInformation::Song>(&info.media)) {
					EmbeddedMetadata meta{
						.cover = song->cover,
						.album = song->album,
						.title = song->title,
						.performer = song->performer,
					};
					crl::on_main(guard, [=, meta = std::move(meta)]() mutable {
						callback(std::move(meta));
					});
					return;
				}
				crl::on_main(guard, [=] {
					callback(EmbeddedMetadata());
				});
			});
			return;
		}
	}
	document->owner().cache().get(document->cacheKey(), [=, callback = std::move(callback)](QByteArray data) {
		if (data.isEmpty()) {
			callback(EmbeddedMetadata());
			return;
		}
		crl::async([=, data = std::move(data), callback = std::move(callback)]() mutable {
			auto info = Media::Player::PrepareForSending(QString(), data);
			if (const auto song = std::get_if<Ui::PreparedFileInformation::Song>(&info.media)) {
				EmbeddedMetadata meta{
					.cover = song->cover,
					.album = song->album,
					.title = song->title,
					.performer = song->performer,
				};
				crl::on_main(guard, [=, meta = std::move(meta)]() mutable {
					callback(std::move(meta));
				});
				return;
			}
			crl::on_main(guard, [=] {
				callback(EmbeddedMetadata());
			});
		});
	});
}

void ExtractEmbeddedCover(
		not_null<DocumentData*> document,
		Fn<void(QImage)> callback) {
	ExtractEmbeddedMetadata(document, [callback = std::move(callback)](EmbeddedMetadata meta) {
		callback(std::move(meta.cover));
	});
}

} // namespace Rabbit::Player
