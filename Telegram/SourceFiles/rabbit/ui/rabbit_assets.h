/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#define ICON(name, value) const auto name##_ICON = QStringLiteral(value)

namespace RabbitAssets
{
    ICON(DEFAULT, "default");
    ICON(ANGEL, "angel");
    ICON(BLACKOUT, "blackout");
    ICON(COLOR, "color");
    ICON(IMPOSTOR, "impostor");
    ICON(MATRIX, "matrix");
    ICON(MOON, "moon");
    ICON(OLD, "old");
    ICON(SEASONAL, "seasonal");
    ICON(TWITCH, "twitch");

    void loadAppIco();

    QImage loadPreview(QString name);

    QString currentAppLogoName();
    QImage currentAppLogo();
    QImage currentAppLogoNoMargin();
}
