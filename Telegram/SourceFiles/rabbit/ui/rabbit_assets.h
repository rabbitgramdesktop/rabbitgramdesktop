/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include <QtGui/QImage>

namespace RabbitAssets {

inline const auto DEFAULT_ICON = u"default"_q;
inline const auto ANGEL_ICON = u"angel"_q;
inline const auto BLACKOUT_ICON = u"blackout"_q;
inline const auto COLOR_ICON = u"color"_q;
inline const auto IMPOSTOR_ICON = u"impostor"_q;
inline const auto MATRIX_ICON = u"matrix"_q;
inline const auto MOON_ICON = u"moon"_q;
inline const auto OLD_ICON = u"old"_q;
inline const auto SEASONAL_ICON = u"seasonal"_q;
inline const auto TWITCH_ICON = u"twitch"_q;

void loadAppIco();

[[nodiscard]] QImage loadPreview(const QString &name);

[[nodiscard]] QString currentAppLogoName();
[[nodiscard]] QImage currentAppLogo();
[[nodiscard]] QImage currentAppLogoNoMargin();

} // namespace RabbitAssets
