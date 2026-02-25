/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#include "icon_picker.h"

#include "rabbit/settings/rabbit_settings.h"

#include "tray.h"
#include "core/application.h"
#include "styles/style_layers.h"
#include "rabbit/ui/rabbit_assets.h"
#include "main/main_domain.h"
#include "styles/palette.h"
#include "styles/style_rabbit_assets.h"
#include "ui/painter.h"
#include "window/main_window.h"

#ifdef Q_OS_WIN
#include "rabbit/utils/windows_utils.h"
#endif

const QVector icon_variants {
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

const auto kColumnsCount = 4;

[[nodiscard]] int rowsCount() {
	const auto count = static_cast<int>(icon_variants.size());
	return (count + kColumnsCount - 1) / kColumnsCount;
}

void drawIcon(QPainter& p, const QImage& icon, int xOffset, int yOffset, float strokeOpacity)
{
    xOffset += st::cpPadding;

    p.save();
    p.setPen(QPen(st::boxDividerBg, 0));
    p.setBrush(QBrush(st::boxDividerBg));
    p.setOpacity(strokeOpacity);
    p.drawRoundedRect(
        xOffset + st::cpSelectedPadding,
        yOffset + st::cpSelectedPadding,
        st::cpIconSize + st::cpSelectedPadding * 2,
        st::cpIconSize + st::cpSelectedPadding * 2,
        st::cpSelectedRounding,
        st::cpSelectedRounding
    );
    p.restore();

    auto rect = QRect(
        xOffset + st::cpImagePadding,
        yOffset + st::cpImagePadding,
        st::cpIconSize,
        st::cpIconSize
    );
    p.drawImage(rect, icon);
}

void applyIcon()
{
#ifdef Q_OS_WIN
    RabbitAssets::loadAppIco();
    reloadAppIconFromTaskBar();
#endif

    Window::OverrideApplicationIcon(RabbitAssets::currentAppLogo());
    Core::App().refreshApplicationIcon();
    Core::App().tray().updateIconCounters();
    Core::App().domain().notifyUnreadBadgeChanged();
}

IconPicker::IconPicker(QWidget* parent)
    : RpWidget(parent)
{
	const auto rows = rowsCount();
	const auto cellHeight = st::cpIconSize + st::cpSelectedPadding * 2;
	const auto totalHeight = cellHeight + (rows - 1) * (st::cpIconSize + st::cpSpacingY);
	setMinimumSize(st::boxWidth, totalHeight);
}

void IconPicker::paintEvent(QPaintEvent* e)
{
    Painter p(this);
    PainterHighQualityEnabler hq(p);

    const auto rows = rowsCount();
    const auto iconsCount = static_cast<int>(icon_variants.size());
    auto offset = st::boxWidth / 2 - (st::cpIconSize + st::cpSpacingX) * 2;

    for (int row = 0; row < rows; row++)
    {
        const auto columns = std::min(kColumnsCount, iconsCount - row * kColumnsCount);
        for (int i = 0; i < columns; i++)
        {
            const auto idx = i + row * kColumnsCount;

            const auto& iconName = icon_variants[idx];
            if (iconName.isEmpty())
            {
                continue;
            }

            auto icon = RabbitAssets::loadPreview(iconName)
                .scaled(st::cpIconSize, st::cpIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            auto opacity = 0.0f;
            if (iconName == wasSelected)
            {
                opacity = 1.0f - animation.value(1.0f);
            }
            else if (iconName == RabbitAssets::currentAppLogoName())
            {
                opacity = wasSelected.isEmpty() ? 1.0f : animation.value(1.0f);
            }

            drawIcon(
                p,
                icon,
                (st::cpIconSize + st::cpSpacingX) * i + offset,
                row * (st::cpIconSize + st::cpSpacingY),
                opacity
            );
        }
    }
}

void IconPicker::mousePressEvent(QMouseEvent* e)
{
    auto appIcon = RabbitSettings::appIcon();
    auto changed = false;

    auto x = e->pos().x();
    const auto rows = rowsCount();
    const auto iconsCount = static_cast<int>(icon_variants.size());
    for (int row = 0; row < rows; row++)
    {
        const auto columns = std::min(kColumnsCount, iconsCount - row * kColumnsCount);
        for (int i = 0; i < columns; i++)
        {
            const auto idx = i + row * kColumnsCount;
            const auto xOffset = (st::cpIconSize + st::cpSpacingX) * i + st::cpPadding;
            const auto yOffset = row * (st::cpIconSize + st::cpSpacingY);

            if (x >= xOffset && x <= xOffset + st::cpIconSize && e->pos().y() >= yOffset
                && e->pos().y() <= yOffset + st::cpIconSize)
            {
                const auto& iconName = icon_variants[idx];
                if (iconName.isEmpty())
                {
                    break;
                }

                if (appIcon != iconName)
                {
                    wasSelected = appIcon;
                    animation.start(
                        [=]
                        {
                            update();
                        },
                        0.0,
                        1.0,
                        200,
                        anim::easeOutCubic
                    );

                    RabbitSettings::setAppIcon(iconName);
                    changed = true;
                    break;
                }
            }
        }
    }

    if (changed)
    {
        applyIcon();

        repaint();
    }
}
