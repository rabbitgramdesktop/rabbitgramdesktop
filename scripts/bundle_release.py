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
import zipfile
from dataclasses import dataclass
from datetime import date
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except Exception:
        pass


class SingleLineProgressBar:
    def __init__(self, total_steps: int, length: int = 25):
        self.total_steps = max(1, total_steps)
        self.current_step = 0
        self.sub_progress = 0.0
        self.length = length
        self.status_text = ""
        self._is_tty = sys.stdout.isatty() if hasattr(sys.stdout, "isatty") else False

    def update(self, step_index: int, sub_progress: float = 0.0, status: str = "") -> None:
        self.current_step = min(max(0, step_index), self.total_steps)
        self.sub_progress = min(max(0.0, sub_progress), 1.0)
        self.status_text = status

        overall_ratio = (self.current_step + self.sub_progress) / self.total_steps
        overall_ratio = min(max(0.0, overall_ratio), 1.0)
        percent = overall_ratio * 100.0

        if not self._is_tty:
            return

        cols = shutil.get_terminal_size((80, 24)).columns
        max_line_len = max(20, cols - 1)
        bar_len = max(10, min(self.length, (max_line_len // 4)))
        filled_len = int(bar_len * overall_ratio)

        try:
            bar = "█" * filled_len + "░" * (bar_len - filled_len)
            prefix = f"\r[{bar}] {percent:5.1f}% | "
        except UnicodeEncodeError:
            bar = "#" * filled_len + "-" * (bar_len - filled_len)
            prefix = f"\r[{bar}] {percent:5.1f}% | "

        avail_status_len = max_line_len - (len(prefix) - 1)
        if avail_status_len > 3 and len(self.status_text) > avail_status_len:
            truncated_status = self.status_text[: avail_status_len - 3] + "..."
        else:
            truncated_status = self.status_text[: max(0, avail_status_len)]

        full_output = prefix + truncated_status
        sys.stdout.write(full_output.ljust(max_line_len))
        sys.stdout.flush()

    def finish(self, message: str = "Release bundling completed successfully") -> None:
        self.current_step = self.total_steps
        self.sub_progress = 0.0
        self.update(self.total_steps, 0.0, message)
        if self._is_tty:
            sys.stdout.write("\n")
        else:
            print(f"Completed: {message}")
        sys.stdout.flush()


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


def update_installer_script(root: Path, version: VersionInfo, pbar: SingleLineProgressBar, step_idx: int) -> None:
    pbar.update(step_idx, 0.1, "Updating setup.iss version info...")
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
    pbar.update(step_idx, 1.0, "Updated setup.iss")


def create_zip_archive(source_dir: Path, output_zip: Path, pbar: SingleLineProgressBar, step_idx: int) -> None:
    files = [f for f in source_dir.rglob("*") if f.is_file()]
    total_bytes = sum(f.stat().st_size for f in files) or 1
    written_bytes = 0

    with zipfile.ZipFile(output_zip, "w", zipfile.ZIP_DEFLATED) as zf:
        for f in files:
            arcname = f.relative_to(source_dir)
            zf.write(f, arcname)
            written_bytes += f.stat().st_size
            sub_ratio = written_bytes / total_bytes
            pbar.update(step_idx, sub_ratio, f"Zipping portable: {arcname.name}")


def bundle_portable(root: Path, version: VersionInfo, pbar: SingleLineProgressBar, step_idx: int) -> None:
    pbar.update(step_idx, 0.05, "Preparing portable directory...")
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

    (portable_dir / "tdata").mkdir(parents=True, exist_ok=True)

    output_dir = release_dir / "releases" / f"rtgdrelease-{version.index}"
    output_dir.mkdir(parents=True, exist_ok=True)

    archive_base = output_dir / f"rtgdportable-x64-{version.index}"
    output_zip = Path(f"{archive_base}.zip")
    create_zip_archive(portable_dir, output_zip, pbar, step_idx)
    pbar.update(step_idx, 1.0, f"Portable bundle created ({output_zip.name})")


def bundle_update(root: Path, version: VersionInfo, pbar: SingleLineProgressBar, step_idx: int) -> None:
    pbar.update(step_idx, 0.1, "Checking Packer.exe...")
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

    pbar.update(step_idx, 0.3, "Running Packer.exe...")
    result = subprocess.run(cmd, cwd=output_dir, capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"Packer failed with exit code {result.returncode}:\n{result.stderr}")

    pbar.update(step_idx, 0.8, "Moving update packages...")

    possible_dirs = [root, root / "scripts", Path.cwd()]
    for search_dir in possible_dirs:
        if not search_dir.exists() or search_dir.resolve() == output_dir.resolve():
            continue
        for item in search_dir.iterdir():
            if item.is_file() and (item.name.startswith("tupdate") or item.name.startswith("tx64upd") or item.name.startswith("tarm64upd")):
                dest = output_dir / item.name
                shutil.move(str(item), str(dest))

    tupdate_files = [
        f for f in output_dir.iterdir()
        if f.is_file() and (f.name.startswith("tupdate") or f.name.startswith("tx64upd") or f.name.startswith("tarm64upd"))
    ]

    pbar.update(step_idx, 1.0, f"OTA update package created ({len(tupdate_files)} package(s))")


def bundle_installer(root: Path, pbar: SingleLineProgressBar, step_idx: int) -> None:
    pbar.update(step_idx, 0.05, "Locating Inno Setup (iscc)...")
    iscc = shutil.which("iscc")
    if not iscc:
        standard_path = Path("C:/Program Files (x86)/Inno Setup 6/ISCC.exe")
        if standard_path.exists():
            iscc = str(standard_path)
        else:
            sys.exit("Inno Setup compiler (iscc) not found on PATH or standard location.")

    iss_path = root / "Telegram" / "build" / "setup.iss"
    cmd = [iscc, str(iss_path)]

    pbar.update(step_idx, 0.1, "Compiling Inno Setup script...")
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)
    lines = []
    sub_ratio = 0.1

    if process.stdout:
        for line in iter(process.stdout.readline, ""):
            lines.append(line)
            line_str = line.strip()
            if line_str:
                match = re.search(r"(\d+(?:\.\d+)?)%", line_str)
                if match:
                    pct = float(match.group(1)) / 100.0
                    sub_ratio = 0.1 + (pct * 0.85)
                else:
                    sub_ratio = min(sub_ratio + 0.02, 0.95)
                pbar.update(step_idx, sub_ratio, f"Inno Setup: {line_str}")

    process.wait()
    if process.returncode != 0:
        err_output = "".join(lines)
        sys.exit(f"Inno Setup compiler failed with exit code {process.returncode}:\n{err_output}")

    pbar.update(step_idx, 1.0, "Installer created successfully")


def main() -> None:
    parser = argparse.ArgumentParser(description="Bundle rabbitGram release artifacts.")
    parser.add_argument("--no-portable", action="store_true", help="Skip portable bundle")
    parser.add_argument("--no-installer", action="store_true", help="Skip installer bundle")
    parser.add_argument("--no-update", action="store_true", help="Skip OTA update package")
    args = parser.parse_args()

    root = Path(__file__).resolve().parent.parent
    version = parse_version(root)

    steps = [
        ("Update setup.iss", lambda p, idx: update_installer_script(root, version, p, idx)),
    ]
    if not args.no_portable:
        steps.append(("Portable bundle", lambda p, idx: bundle_portable(root, version, p, idx)))
    if not args.no_update:
        steps.append(("OTA update bundle", lambda p, idx: bundle_update(root, version, p, idx)))
    if not args.no_installer:
        steps.append(("Installer bundle", lambda p, idx: bundle_installer(root, p, idx)))

    total_steps = len(steps)
    pbar = SingleLineProgressBar(total_steps=total_steps, length=30)
    pbar.update(0, 0.0, f"Starting release bundle for v{version.version_str} ({version.index})...")

    for i, (name, step_fn) in enumerate(steps):
        pbar.update(i, 0.0, f"Starting {name}...")
        step_fn(pbar, i)

    pbar.finish(f"Release bundling complete for v{version.version_str} ({version.index})")


if __name__ == "__main__":
    main()


