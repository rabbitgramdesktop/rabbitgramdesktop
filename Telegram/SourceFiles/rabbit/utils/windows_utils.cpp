/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#ifdef Q_OS_WIN

#include "windows_utils.h"

#include "core/application.h"
#include "core/core_settings.h"
#include "storage/localstorage.h"
#include "window/themes/window_theme.h"
#include "window/themes/window_themes_embedded.h"

#include <ShlObj_core.h>
#include <dwmapi.h>

void reloadAppIconFromTaskBar()
{
    QString appdata = QDir::fromNativeSeparators(qgetenv("APPDATA"));
    QString rabbitgramIconPath = appdata + "/rabbitGram.ico";

    QString shortcut = appdata + "/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/rabbitGram Desktop.lnk";
    if (!QFile::exists(shortcut))
    {
        shortcut = appdata + "/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/rabbitGram.lnk";
    }

    if (QFile::exists(shortcut))
    {
        IShellLink* pShellLink = nullptr;
        IPersistFile* pPersistFile = nullptr;

        HRESULT hr = CoCreateInstance(CLSID_ShellLink,
                                      nullptr,
                                      CLSCTX_INPROC_SERVER,
                                      IID_IShellLink,
                                      (void**)&pShellLink);
        if (SUCCEEDED(hr))
        {
            hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
            if (SUCCEEDED(hr))
            {
                WCHAR wszShortcutPath[MAX_PATH];
                shortcut.toWCharArray(wszShortcutPath);
                wszShortcutPath[shortcut.length()] = '\0';

                if (SUCCEEDED(pPersistFile->Load(wszShortcutPath, STGM_READWRITE)))
                {
                    pShellLink->SetIconLocation(rabbitgramIconPath.toStdWString().c_str(), 0);
                    pPersistFile->Save(wszShortcutPath, TRUE);
                }

                pPersistFile->Release();
            }

            pShellLink->Release();
        }

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    }
}

void setAccentTheme()
{
    DWORD accentColorDWORD = 0;
    DWORD size = sizeof(accentColorDWORD);
    DWORD type = 0;
    HKEY hKey = nullptr;
    auto result = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\DWM",
        0,
        KEY_READ,
        &hKey);
    if (result != ERROR_SUCCESS) {
        return;
    }
    result = RegQueryValueExW(
        hKey,
        L"AccentColor",
        nullptr,
        &type,
        reinterpret_cast<LPBYTE>(&accentColorDWORD),
        &size);
    RegCloseKey(hKey);
    if (result != ERROR_SUCCESS || type != REG_DWORD) {
        return;
    }

    const auto a = (accentColorDWORD >> 24) & 0xFF;
    const auto b = (accentColorDWORD >> 16) & 0xFF;
    const auto g = (accentColorDWORD >> 8) & 0xFF;
    const auto r = accentColorDWORD & 0xFF;
    const auto accentColor = QColor(r, g, b, a);
    if (!accentColor.isValid()) {
        return;
    }

    using namespace Window::Theme;

    const auto themes = EmbeddedThemes();
    const auto &object = Background()->themeObject();
    auto currentType = EmbeddedType(-1);
    for (const auto &scheme : themes) {
        if (object.pathAbsolute == scheme.path) {
            currentType = scheme.type;
            break;
        }
    }
    if (currentType == EmbeddedType(-1)) {
        currentType = IsNightMode()
            ? EmbeddedType::Night
            : EmbeddedType::DayBlue;
        const auto scheme = ranges::find(themes, currentType, &EmbeddedScheme::type);
        if (scheme != end(themes)) {
            const auto isNight = (currentType != EmbeddedType::DayBlue)
                && (currentType != EmbeddedType::Default);
            if (IsNightMode() == isNight) {
                ApplyDefaultWithPath(scheme->path);
            } else {
                ToggleNightMode(scheme->path);
            }
            KeepApplied();
        }
    }

    auto &colors = Core::App().settings().themesAccentColors();
    if (colors.get(currentType) != accentColor) {
        colors.set(currentType, accentColor);
        Local::writeSettings();
    }
    const auto scheme = ranges::find(themes, currentType, &EmbeddedScheme::type);
    if (scheme != end(themes)) {
        const auto isNight = (currentType != EmbeddedType::DayBlue)
            && (currentType != EmbeddedType::Default);
        if (IsNightMode() == isNight) {
            ApplyDefaultWithPath(scheme->path);
        } else {
            ToggleNightMode(scheme->path);
        }
        KeepApplied();
    }
}

#endif
