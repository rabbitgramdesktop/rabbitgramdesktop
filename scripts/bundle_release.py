#!/usr/bin/env python3
# Universal dispatcher for bundling rabbitGram Desktop releases.
#
# Detects running OS and executes the appropriate release bundler:
#   - Windows: scripts/bundle_win.py
#   - Linux:   scripts/bundle_linux.py
#
# Usage:
#   python scripts/bundle_release.py [options]

import os
import subprocess
import sys
from pathlib import Path

def main() -> None:
    scripts_dir = Path(__file__).resolve().parent

    if sys.platform.startswith("win"):
        target_script = scripts_dir / "bundle_win.py"
    elif sys.platform.startswith("linux"):
        target_script = scripts_dir / "bundle_linux.py"
    else:
        sys.exit(f"Unsupported operating system: {sys.platform}")

    if not target_script.exists():
        sys.exit(f"Bundler script not found: {target_script}")

    cmd = [sys.executable, str(target_script)] + sys.argv[1:]
    res = subprocess.run(cmd)
    sys.exit(res.returncode)

if __name__ == "__main__":
    main()
