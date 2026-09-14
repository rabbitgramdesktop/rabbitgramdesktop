# Bundle rabbitGram Desktop release artifacts.
#
# Usage:
#   python scripts/bundle_release.py                 Bundle all (portable + update + installer)
#   python scripts/bundle_release.py --no-portable   Skip portable archive
#   python scripts/bundle_release.py --no-update     Skip OTA update package
#   python scripts/bundle_release.py --no-installer  Skip installer
#
# The script reads the version from Telegram/SourceFiles/core/version.h,
# updates Telegram/build/setup.iss with the resolved version, then
# produces a portable .zip archive, an OTA update package (via Packer.exe),
# and/or an Inno Setup installer into out/Release/releases/rtgdrelease-<version>/.
#
# Requirements:
#   - Python 3.10+
#   - Inno Setup (iscc) on PATH or standard installation path
#   - Packer.exe and Updater.exe in out/Release/ for OTA updates
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

    release_path = str(root / "out" / "Release")
    replacements = {
        r'#define MyAppVersion ".*?"': f'#define MyAppVersion "{version.version_str}"',
        r'#define MyAppVersionFull ".*?"': f'#define MyAppVersionFull "{version.index}"',
        r'#define ReleasePath ".*?"': f'#define ReleasePath "{release_path}"',
    }
    for pattern, replacement in replacements.items():
        if not re.search(pattern, content):
            sys.exit(f"Failed to find pattern {pattern} in {iss_path}")
        content = re.sub(pattern, lambda match, r=replacement: r, content)

    iss_path.write_text(content, encoding="utf-8", newline="\r\n")


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

    updater_src = release_dir / "Updater.exe"
    if updater_src.exists():
        shutil.copy2(updater_src, portable_dir / "Updater.exe")

    # Create tdata directory marker required for Telegram portable mode
    (portable_dir / "tdata").mkdir(parents=True, exist_ok=True)

    output_dir = release_dir / "releases" / f"rtgdrelease-{version.index}"
    output_dir.mkdir(parents=True, exist_ok=True)

    archive_base = output_dir / f"rtgdportable-x64-{version.index}"
    shutil.make_archive(str(archive_base), "zip", str(portable_dir))
    print(f"Portable archive created: {archive_base}.zip")


def bundle_update(root: Path, version: VersionInfo) -> None:
    print("Bundling OTA update package...")

    release_dir = root / "out" / "Release"
    packer = release_dir / "Packer.exe"
    if not packer.exists():
        sys.exit(
            f"Packer.exe not found: {packer}\n"
            "Build Telegram with target 'Packer' to enable update bundling."
        )

    output_dir = release_dir / "releases" / f"rtgdrelease-{version.index}"
    output_dir.mkdir(parents=True, exist_ok=True)

    cmd = [
        str(packer),
        "-target",
        "win64",
        "-path",
        str(release_dir / "rabbitGram.exe"),
        "-version",
        version.version_num,
    ]

    print(f"Running Packer: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=output_dir, capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"Packer failed with exit code {result.returncode}:\n{result.stderr}")

    # Move any tupdate* / tx64upd* / tarm64upd* files from root/scripts to output_dir
    possible_dirs = [root, root / "scripts", Path.cwd()]
    for search_dir in possible_dirs:
        if not search_dir.exists() or search_dir.resolve() == output_dir.resolve():
            continue
        for item in search_dir.iterdir():
            if item.is_file() and (item.name.startswith("tupdate") or item.name.startswith("tx64upd") or item.name.startswith("tarm64upd")):
                dest = output_dir / item.name
                shutil.move(str(item), str(dest))
                print(f"Moved {item.name} from {search_dir} to {output_dir}")

    tupdate_files = [
        f for f in output_dir.iterdir()
        if f.is_file() and (f.name.startswith("tupdate") or f.name.startswith("tx64upd") or f.name.startswith("tarm64upd"))
    ]

    for tup in tupdate_files:
        print(f"OTA update package created in releases: {tup}")


def bundle_installer(root: Path) -> None:
    print("Bundling Inno Setup installer...")

    iscc = shutil.which("iscc")
    if not iscc:
        standard_path = Path("C:/Program Files (x86)/Inno Setup 6/ISCC.exe")
        if standard_path.exists():
            iscc = str(standard_path)
        else:
            sys.exit("Inno Setup compiler (iscc) not found on PATH or standard location.")

    iss_path = root / "Telegram" / "build" / "setup.iss"
    cmd = [iscc, str(iss_path)]

    print(f"Running Inno Setup: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"Inno Setup compiler failed with exit code {result.returncode}")
    print("Installer created successfully.")


def main() -> None:
    parser = argparse.ArgumentParser(description="Bundle rabbitGram release artifacts.")
    parser.add_argument("--no-portable", action="store_true", help="Skip portable bundle")
    parser.add_argument("--no-installer", action="store_true", help="Skip installer bundle")
    parser.add_argument("--no-update", action="store_true", help="Skip OTA update package")
    args = parser.parse_args()

    root = Path(__file__).resolve().parent.parent
    version = parse_version(root)
    print(f"Version: {version.version_str} ({version.index})")

    update_installer_script(root, version)

    if not args.no_portable:
        bundle_portable(root, version)
    if not args.no_update:
        bundle_update(root, version)
    if not args.no_installer:
        bundle_installer(root)


if __name__ == "__main__":
    main()
