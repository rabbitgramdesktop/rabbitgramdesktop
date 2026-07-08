/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#ifdef Q_OS_WIN

#include "windows_utils.h"

#include "storage/localstorage.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <ShlObj_core.h>

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
                                      reinterpret_cast<void**>(&pShellLink));
        if (SUCCEEDED(hr))
        {
            hr = pShellLink->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&pPersistFile));
            if (SUCCEEDED(hr))
            {
                const auto nativeShortcut = QDir::toNativeSeparators(shortcut).toStdWString();
                if (SUCCEEDED(pPersistFile->Load(nativeShortcut.c_str(), STGM_READWRITE)))
                {
                    const auto nativeIconPath = QDir::toNativeSeparators(rabbitgramIconPath).toStdWString();
                    pShellLink->SetIconLocation(nativeIconPath.c_str(), 0);
                    pPersistFile->Save(nativeShortcut.c_str(), TRUE);
                }

                pPersistFile->Release();
            }

            pShellLink->Release();
        }

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    }
}

#endif
