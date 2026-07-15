/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#ifdef Q_OS_WIN

#include "rabbit/utils/windows_utils.h"

#include "storage/localstorage.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <ShlObj_core.h>
#include <gsl/gsl>

void reloadAppIconFromTaskBar() {
	const auto appdata = QDir::fromNativeSeparators(QString::fromLocal8Bit(qgetenv("APPDATA")));
	const auto rabbitgramIconPath = appdata + u"/rabbitGram.ico"_q;

	auto shortcut = appdata + u"/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/rabbitGram Desktop.lnk"_q;
	if (!QFile::exists(shortcut)) {
		shortcut = appdata + u"/Microsoft/Internet Explorer/Quick Launch/User Pinned/TaskBar/rabbitGram.lnk"_q;
	}

	if (QFile::exists(shortcut)) {
		IShellLink *pShellLink = nullptr;
		HRESULT hr = CoCreateInstance(
			CLSID_ShellLink,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_IShellLink,
			reinterpret_cast<void **>(&pShellLink));
		if (SUCCEEDED(hr)) {
			const auto shellLinkGuard = gsl::finally([&] {
				pShellLink->Release();
			});

			IPersistFile *pPersistFile = nullptr;
			hr = pShellLink->QueryInterface(
				IID_IPersistFile,
				reinterpret_cast<void **>(&pPersistFile));
			if (SUCCEEDED(hr)) {
				const auto persistFileGuard = gsl::finally([&] {
					pPersistFile->Release();
				});

				const auto nativeShortcut = QDir::toNativeSeparators(shortcut).toStdWString();
				if (SUCCEEDED(pPersistFile->Load(nativeShortcut.c_str(), STGM_READWRITE))) {
					const auto nativeIconPath = QDir::toNativeSeparators(rabbitgramIconPath).toStdWString();
					pShellLink->SetIconLocation(nativeIconPath.c_str(), 0);
					pPersistFile->Save(nativeShortcut.c_str(), TRUE);
				}
			}
		}

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	}
}

#endif
