#!/usr/bin/env python3
# Bundle rabbitGram Desktop Linux release artifacts (.tar.xz).
#
# Usage:
#   python scripts/bundle_linux.py                 Bundle release (.tar.xz) with stripped binaries
#   python scripts/bundle_linux.py --no-strip      Do not strip binaries
#
# Environment Variables:
#   XZ_OPT      Options passed to xz compression (default: "-T0 -6")
#
# Requirements:
#   - Python 3.10+
#   - Completed build in out/Release/ (rabbitGram)
#   - strip utility (optional, enabled by default)

import argparse
import os
import re
import shutil
import subprocess
import sys
import tarfile
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

    def finish(self, message: str = "Linux release bundling completed successfully") -> None:
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
        "version_num": r"AppVersion = (.*?);",
        "is_beta": r"AppBetaVersion = (.*?);",
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


def format_size(size_bytes: int) -> str:
    if size_bytes >= 1024 * 1024:
        return f"{size_bytes / (1024 * 1024):.1f} MB"
    return f"{size_bytes / 1024:.1f} KB"


def strip_binary(binary_path: Path, pbar: SingleLineProgressBar, step_idx: int, ratio: float) -> None:
    strip_cmd = shutil.which("strip")
    if not strip_cmd:
        pbar.update(step_idx, ratio, f"strip not found, keeping original size for {binary_path.name}")
        return

    original_size = binary_path.stat().st_size
    pbar.update(step_idx, ratio, f"Stripping {binary_path.name} ({format_size(original_size)})...")
    res = subprocess.run([strip_cmd, "--strip-all", str(binary_path)], capture_output=True, text=True)
    if res.returncode == 0:
        new_size = binary_path.stat().st_size
        pbar.update(
            step_idx,
            ratio,
            f"Stripped {binary_path.name}: {format_size(original_size)} -> {format_size(new_size)}",
        )
    else:
        pbar.update(step_idx, ratio, f"Warning: strip returned {res.returncode}: {res.stderr.strip()}")


def create_tar_xz(source_dir: Path, output_file: Path, pbar: SingleLineProgressBar, step_idx: int) -> None:
    tar_cmd = shutil.which("tar")
    if tar_cmd:
        # Use system tar with multithreaded xz if available for speed and reliability
        pbar.update(step_idx, 0.2, f"Creating {output_file.name} using tar...")
        env = os.environ.copy()
        if "XZ_OPT" not in env:
            env["XZ_OPT"] = "-T0 -6"
        res = subprocess.run(
            [tar_cmd, "-cJf", str(output_file), source_dir.name],
            cwd=source_dir.parent,
            capture_output=True,
            text=True,
            env=env,
        )
        if res.returncode != 0:
            sys.exit(f"tar command failed: {res.stderr}")
    else:
        # Fallback to Python tarfile
        pbar.update(step_idx, 0.5, f"Packing {output_file.name} using python tarfile...")
        with tarfile.open(output_file, "w:xz") as tar:
            tar.add(source_dir, arcname=source_dir.name)


def prepare_bundle_directory(
    root: Path,
    version: VersionInfo,
    do_strip: bool,
    pbar: SingleLineProgressBar,
    step_idx: int,
) -> tuple[Path, Path]:
    release_dir = root / "out" / "Release"
    output_dir = release_dir / "releases" / f"rtgdrelease-{version.index}"
    output_dir.mkdir(parents=True, exist_ok=True)

    bundle_name = f"rabbitGram-linux-x64-{version.index}"
    staging_root = release_dir / "staging_linux"
    staging_dir = staging_root / bundle_name

    if staging_root.exists():
        shutil.rmtree(staging_root)
    staging_dir.mkdir(parents=True)

    # 1. Main executable
    pbar.update(step_idx, 0.1, "Copying rabbitGram executable...")
    src_bin = release_dir / "rabbitGram"
    if not src_bin.exists():
        sys.exit(
            f"Executable not found: {src_bin}\n"
            "Build target 'rabbitGram' in out/Release before running bundling script."
        )

    target_bin = staging_dir / "rabbitGram"
    shutil.copy2(src_bin, target_bin)
    target_bin.chmod(0o755)

    if do_strip:
        strip_binary(target_bin, pbar, step_idx, 0.25)

    # 2. Updater (if built)
    src_updater = release_dir / "Updater"
    if src_updater.exists():
        pbar.update(step_idx, 0.35, "Copying Updater executable...")
        target_updater = staging_dir / "Updater"
        shutil.copy2(src_updater, target_updater)
        target_updater.chmod(0o755)
        if do_strip:
            strip_binary(target_updater, pbar, step_idx, 0.45)

    # 3. Installer script
    pbar.update(step_idx, 0.55, "Adding install-rabbitgram.sh...")
    install_script = root / "install-rabbitgram.sh"
    if not install_script.exists():
        sys.exit(f"Error: Required installer script not found: {install_script}")
    target_install = staging_dir / "install-rabbitgram.sh"
    shutil.copy2(install_script, target_install)
    target_install.chmod(0o755)

    # 4. Icon assets for offline/portable installation
    pbar.update(step_idx, 0.7, "Packaging icons...")
    icons_staging = staging_dir / "icons"
    icons_staging.mkdir(parents=True, exist_ok=True)

    art_src = root / "Telegram" / "Resources" / "art"
    if art_src.exists():
        for size in [16, 32, 48, 64, 128, 256, 512]:
            icon_file = art_src / f"icon{size}.png"
            if icon_file.exists():
                shutil.copy2(icon_file, icons_staging / f"icon{size}.png")

    tray_src = root / "Telegram" / "Resources" / "icons"
    if tray_src.exists():
        tray_staging = icons_staging / "tray"
        tray_staging.mkdir(parents=True, exist_ok=True)
        for sym_svg in ["tray_monochrome.svg", "tray_monochrome_attention.svg", "tray_monochrome_mute.svg"]:
            f = tray_src / sym_svg
            if f.exists():
                shutil.copy2(f, tray_staging / sym_svg)

    # 5. README / instructions
    readme_text = f"""rabbitGram Desktop for Linux (x86_64)
Version: {version.version_str} ({version.index})

Quick Start:
1. Run directly (portable):
   ./rabbitGram

2. Install with application menu entry & tg:// link handler:
   ./install-rabbitgram.sh

3. System-wide install (requires root):
   sudo ./install-rabbitgram.sh --system

4. Clean uninstall:
   ./install-rabbitgram.sh --uninstall

Application Data:
- Session data (tdata) is stored in:
  ~/.local/share/rabbitGramDesktop/tdata
"""
    (staging_dir / "README.txt").write_text(readme_text, encoding="utf-8")

    pbar.update(step_idx, 1.0, f"Staging directory prepared ({bundle_name})")
    return staging_dir, output_dir


def bundle_linux(
    root: Path,
    version: VersionInfo,
    do_strip: bool,
    pbar: SingleLineProgressBar,
) -> None:
    staging_dir, output_dir = prepare_bundle_directory(
        root=root,
        version=version,
        do_strip=do_strip,
        pbar=pbar,
        step_idx=0,
    )

    bundle_name = staging_dir.name
    tar_path = output_dir / f"{bundle_name}.tar.xz"
    pbar.update(1, 0.1, f"Creating {tar_path.name}...")
    create_tar_xz(staging_dir, tar_path, pbar, step_idx=1)

    # Cleanup staging directory
    shutil.rmtree(staging_dir.parent, ignore_errors=True)
    pbar.update(1, 1.0, f"Release archive created in {output_dir.relative_to(root)}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Bundle rabbitGram Desktop Linux release artifacts.")
    parser.add_argument(
        "--no-strip",
        action="store_true",
        help="Do not strip binary debugging symbols (not recommended for distribution)",
    )
    # Cross-platform dispatcher compatibility flags (ignored on Linux)
    parser.add_argument("--no-portable", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--no-installer", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("--no-update", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()

    root = Path(__file__).resolve().parent.parent
    version = parse_version(root)

    pbar = SingleLineProgressBar(total_steps=2, length=30)
    pbar.update(0, 0.0, f"Starting Linux release bundle for v{version.version_str} ({version.index})...")

    bundle_linux(
        root=root,
        version=version,
        do_strip=not args.no_strip,
        pbar=pbar,
    )

    pbar.finish(f"Linux release bundle complete for v{version.version_str} ({version.index})")


if __name__ == "__main__":
    main()
