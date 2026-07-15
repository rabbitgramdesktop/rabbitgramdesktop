/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "rabbit/ui/settings/icon_picker.h"

#include "core/application.h"
#include "main/main_domain.h"
#include "rabbit/lang/rabbit_lang.h"
#include "rabbit/settings/rabbit_settings.h"
#include "rabbit/ui/rabbit_assets.h"
#include "tray.h"
#include "ui/painter.h"
#include "window/main_window.h"

#ifdef Q_OS_WIN
#include "rabbit/utils/windows_utils.h"
#endif

#include "styles/palette.h"
#include "styles/style_layers.h"
#include "styles/style_rabbit_assets.h"

const auto icon_variants = std::vector<QString>{
	RabbitAssets::DEFAULT_ICON,
	RabbitAssets::ANGEL_ICON,
	RabbitAssets::BLACKOUT_ICON,
	RabbitAssets::COLOR_ICON,
	RabbitAssets::IMPOSTOR_ICON,
	RabbitAssets::MATRIX_ICON,
	RabbitAssets::MOON_ICON,
	RabbitAssets::OLD_ICON,
	RabbitAssets::SEASONAL_ICON,
	RabbitAssets::TWITCH_ICON,
};

constexpr auto kColumnsCount = 4;

[[nodiscard]] static int rowsCount() {
	const auto count = static_cast<int>(icon_variants.size());
	return (count + kColumnsCount - 1) / kColumnsCount;
}

static void drawIcon(QPainter &p, const QImage &icon, int xOffset, int yOffset, float strokeOpacity) {
	if (strokeOpacity > 0.0f) {
		p.save();
		auto pen = QPen(st::windowBgActive, 2);
		p.setPen(pen);
		p.setBrush(Qt::NoBrush);
		p.setOpacity(strokeOpacity);
		p.drawEllipse(
			xOffset,
			yOffset + st::cpSelectedPadding,
			st::cpIconSize + st::cpSelectedPadding * 2,
			st::cpIconSize + st::cpSelectedPadding * 2);
		p.restore();
	}

	const auto rect = QRect(
		xOffset + st::cpSelectedPadding,
		yOffset + st::cpImagePadding,
		st::cpIconSize,
		st::cpIconSize);
	p.drawImage(rect, icon);
}

static void applyIcon() {
#ifdef Q_OS_WIN
	RabbitAssets::loadAppIco();
	reloadAppIconFromTaskBar();
#endif

	Window::OverrideApplicationIcon(RabbitAssets::currentAppLogo());
	Core::App().refreshApplicationIcon();
	Core::App().tray().updateIconCounters();
	Core::App().domain().notifyUnreadBadgeChanged();
}

IconPicker::IconPicker(QWidget *parent)
	: RpWidget(parent) {
	const auto rows = rowsCount();
	const auto textFont = st::normalFont;
	const auto textHeight = textFont->height;
	const auto textSpacing = 4;
	const auto rowGap = 16;
	const auto rowHeight = st::cpIconSize + st::cpSelectedPadding * 3 + textSpacing + textHeight + rowGap;
	const auto totalHeight = (rows - 1) * rowHeight + st::cpIconSize + st::cpSelectedPadding * 3 + textSpacing + textHeight;
	setMinimumSize(st::boxWidth, totalHeight);
}

void IconPicker::paintEvent(QPaintEvent *e) {
	Painter p(this);
	PainterHighQualityEnabler hq(p);

	const auto rows = rowsCount();
	const auto iconsCount = static_cast<int>(icon_variants.size());
	const auto totalWidth = width();

	const auto textFont = st::normalFont;
	const auto textHeight = textFont->height;
	const auto textSpacing = 4;
	const auto rowGap = 16;
	const auto rowHeight = st::cpIconSize + st::cpSelectedPadding * 3 + textSpacing + textHeight + rowGap;

	const auto cellWidth = st::cpIconSize + st::cpSelectedPadding * 2;
	const auto padding = st::cpPadding + st::cpSelectedPadding;
	const auto spacingX = (totalWidth - padding * 2 - cellWidth) / (kColumnsCount - 1);

	for (int row = 0; row < rows; row++) {
		const auto columns = std::min(kColumnsCount, iconsCount - row * kColumnsCount);
		const auto yOffset = row * rowHeight;

		for (int i = 0; i < columns; i++) {
			const auto idx = i + row * kColumnsCount;

			const auto &iconName = icon_variants[idx];
			if (iconName.isEmpty()) {
				continue;
			}

			auto icon = RabbitAssets::loadPreview(iconName)
				.scaled(st::cpIconSize, st::cpIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

			auto opacity = 0.0f;
			if (iconName == _wasSelected) {
				opacity = 1.0f - _animation.value(1.0f);
			} else if (iconName == RabbitAssets::currentAppLogoName()) {
				opacity = _wasSelected.isEmpty() ? 1.0f : _animation.value(1.0f);
			}

			const auto xOffset = padding + i * spacingX;

			drawIcon(
				p,
				icon,
				xOffset,
				yOffset,
				opacity);

			const auto nameKey = u"rtg_settings_app_icon_"_q + iconName;
			const auto nameText = RabbitLang::Lang::Translate(nameKey);

			p.setFont(textFont);
			p.setPen(st::windowFg);

			const auto textWidth = spacingX;
			const auto textX = xOffset - (spacingX - cellWidth) / 2;
			const auto textY = yOffset + st::cpSelectedPadding * 3 + st::cpIconSize + textSpacing;

			p.drawText(
				QRect(textX, textY, textWidth, textHeight),
				Qt::AlignCenter,
				nameText);
		}
	}
}

void IconPicker::mousePressEvent(QMouseEvent *e) {
	auto appIcon = RabbitSettings::appIcon();
	auto changed = false;

	const auto x = e->pos().x();
	const auto rows = rowsCount();
	const auto iconsCount = static_cast<int>(icon_variants.size());
	const auto totalWidth = width();

	const auto textFont = st::normalFont;
	const auto textHeight = textFont->height;
	const auto textSpacing = 4;
	const auto rowGap = 16;
	const auto rowHeight = st::cpIconSize + st::cpSelectedPadding * 3 + textSpacing + textHeight + rowGap;

	const auto cellWidth = st::cpIconSize + st::cpSelectedPadding * 2;
	const auto padding = st::cpPadding + st::cpSelectedPadding;
	const auto spacingX = (totalWidth - padding * 2 - cellWidth) / (kColumnsCount - 1);

	for (int row = 0; row < rows; row++) {
		const auto columns = std::min(kColumnsCount, iconsCount - row * kColumnsCount);
		const auto yOffset = row * rowHeight;

		for (int i = 0; i < columns; i++) {
			const auto idx = i + row * kColumnsCount;
			const auto xOffset = padding + i * spacingX;

			if (x >= xOffset + st::cpSelectedPadding
				&& x <= xOffset + st::cpSelectedPadding + st::cpIconSize
				&& e->pos().y() >= yOffset + st::cpImagePadding
				&& e->pos().y() <= yOffset + st::cpImagePadding + st::cpIconSize) {
				const auto &iconName = icon_variants[idx];
				if (iconName.isEmpty()) {
					break;
				}

				if (appIcon != iconName) {
					_wasSelected = appIcon;
					_animation.start(
						[=] {
							update();
						},
						0.0,
						1.0,
						200,
						anim::easeOutCubic);

					RabbitSettings::setAppIcon(iconName);
					changed = true;
					break;
				}
			}
		}
	}

	if (changed) {
		applyIcon();
		repaint();
	}
}
