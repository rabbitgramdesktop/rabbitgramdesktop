# Bundle rabbitGram Desktop release artifacts.
#
# Usage:
#   python bundle_release.py                 Bundle both portable and installer
#   python bundle_release.py --no-portable   Bundle installer only
#   python bundle_release.py --no-installer  Bundle portable only
#
# The script reads the version from Telegram/SourceFiles/core/version.h,
# updates Telegram/build/setup.iss with the resolved version, then
# produces a portable .zip archive and/or an Inno Setup installer
# into out/Release/releases/rtgdrelease-<version>/.
#
# Requirements:
#   - Python 3.10+
#   - Inno Setup (iscc) on PATH when building the installer
#   - A completed Release build in out/Release/

import argparse
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass
from datetime import date
from pathlib import Path


@dataclass
class VersionInfo:
    version_str: str
    version_num: str
    is_beta: bool
    index: str = ""


def parse_version(root: Path) -> VersionInfo:
    version_header = root / "Telegram" / "SourceFiles" / "core" / "version.h"
    data = version_header.read_text(encoding="utf-8")

    patterns = {
        "version_str": r'AppVersionStr = "(.*?)";',
        "version_num": r'AppVersion = (.*?);',
        "is_beta": r'AppBetaVersion = (.*?);',
    }
    results: dict[str, str] = {}
    for key, pattern in patterns.items():
        match = re.search(pattern, data)
        if not match:
            sys.exit(f"Failed to find {key} pattern in {version_header}")
        results[key] = match.group(1)

    info = VersionInfo(
        version_str=results["version_str"],
        version_num=results["version_num"],
        is_beta=results["is_beta"] == "true",
    )
    suffix = "-beta" if info.is_beta else ""
    info.index = f"{info.version_str}-{date.today().strftime('%d%m%Y')}{suffix}"
    return info


def update_installer_script(root: Path, version: VersionInfo) -> None:
    iss_path = root / "Telegram" / "build" / "setup.iss"
    content = iss_path.read_text(encoding="utf-8")

    release_path = str(root / "out" / "Release").replace("\\", "\\\\")
    replacements = {
        r'#define MyAppVersion ".*?"': f'#define MyAppVersion "{version.version_str}"',
        r'#define MyAppVersionFull ".*?"': f'#define MyAppVersionFull "{version.index}"',
        r'#define ReleasePath ".*?"': f'#define ReleasePath "{release_path}"',
    }
    for pattern, replacement in replacements.items():
        content = re.sub(pattern, replacement, content)

    iss_path.write_text(content, encoding="utf-8")


def bundle_portable(root: Path, version: VersionInfo) -> None:
    print("Bundling portable version...")

    release_dir = root / "out" / "Release"
    portable_dir = release_dir / "portable"

    if portable_dir.exists():
        shutil.rmtree(portable_dir)
    portable_dir.mkdir(parents=True)

    exe_src = release_dir / "rabbitGram.exe"
    if not exe_src.exists():
        sys.exit(f"Executable not found: {exe_src}")

    shutil.copy2(exe_src, portable_dir / "rabbitGram.exe")

    modules_src = release_dir / "modules"
    if modules_src.exists():
        shutil.copytree(modules_src, portable_dir / "modules")

    output_dir = release_dir / "releases" / f"rtgdrelease-{version.index}"
    output_dir.mkdir(parents=True, exist_ok=True)

    archive_base = output_dir / f"rtgdportable-x64-{version.index}"
    shutil.make_archive(str(archive_base), "zip", str(portable_dir))
    print(f"Portable archive created: {archive_base}.zip")


def bundle_installer(root: Path) -> None:
    print("Bundling installer...")

    iss_path = root / "Telegram" / "build" / "setup.iss"
    result = subprocess.run(["iscc", str(iss_path)])
    if result.returncode != 0:
        sys.exit(f"Inno Setup compiler failed with exit code {result.returncode}")
    print("Installer created successfully.")


def main() -> None:
    parser = argparse.ArgumentParser(description="Bundle rabbitGram release artifacts.")
    parser.add_argument("--no-portable", action="store_true", help="Skip portable bundle")
    parser.add_argument("--no-installer", action="store_true", help="Skip installer bundle")
    args = parser.parse_args()

    root = Path(__file__).resolve().parent
    version = parse_version(root)
    print(f"Version: {version.version_str} ({version.index})")

    update_installer_script(root, version)

    if not args.no_portable:
        bundle_portable(root, version)
    if not args.no_installer:
        bundle_installer(root)


if __name__ == "__main__":
    main()