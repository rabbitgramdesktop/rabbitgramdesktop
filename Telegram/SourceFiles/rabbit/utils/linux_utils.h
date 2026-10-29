/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/
#pragma once

#if !defined Q_OS_WIN && !defined Q_OS_MAC

#include <QtCore/QString>

namespace RabbitLinuxUtils {

[[nodiscard]] QString ApplicationsDirectory();
[[nodiscard]] QString DesktopFilePath();
[[nodiscard]] QString IconsRootDirectory();
[[nodiscard]] QString BinDirectory();
[[nodiscard]] QString SymlinkPath();

[[nodiscard]] bool IsInstalledToSystem();
[[nodiscard]] bool IsDesktopEntryInstalled();
[[nodiscard]] bool IsSymlinkInstalled();

void ApplyAppIcon();
void RefreshDesktopDatabase();

bool InstallDesktopEntry(const QString &customExecPath = QString());
bool RemoveDesktopEntry();

bool InstallSymlink(const QString &targetExe = QString());
bool RemoveSymlink();

bool InstallAppToUserLocal(bool createDesktop, bool createSymlink);

} // namespace RabbitLinuxUtils

#endif // !defined Q_OS_WIN && !defined Q_OS_MAC
