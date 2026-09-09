/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#include "translate_provider.h"

namespace Platform {

[[nodiscard]] bool IsTranslateProviderAvailable();
[[nodiscard]] std::unique_ptr<Ui::TranslateProvider> CreateTranslateProvider();

} // namespace Platform

#if defined Q_OS_WINRT || defined Q_OS_WIN
#include "platform/win/translate_provider_win.h"
#elif defined Q_OS_MAC
#include "platform/mac/translate_provider_mac.h"
#else
#include "platform/linux/translate_provider_linux.h"
#endif
