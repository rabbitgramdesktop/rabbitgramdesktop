#!/usr/bin/env python3
"""
Script to replace tdesktop license headers with rabbitGram license headers
across the codebase and display detailed statistics.
Ignores all git submodules under all circumstances.
"""

import os
import sys
import re
import subprocess

if hasattr(sys.stdout, 'reconfigure'):
    try:
        sys.stdout.reconfigure(encoding='utf-8')
    except Exception:
        pass

OLD_C_HEADER_RE = re.compile(
    r'/\*\s*\n'
    r'This file is part of Telegram Desktop,\s*\n'
    r'(?:the official desktop application for the Telegram messaging service\.\s*\n)?'
    r'\s*\n'
    r'For license and copyright information please follow this link:\s*\n'
    r'https://github\.com/telegramdesktop/tdesktop/blob/master/LEGAL\s*\n'
    r'\*/',
    re.MULTILINE
)

NEW_C_HEADER = """/*
This file is part of rabbitGram Desktop,
the unofficial app based on Telegram Desktop.

For license and copyright information please follow this link:
https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL
*/"""

OLD_CMAKE_HEADER_RE = re.compile(
    r'# This file is part of Telegram Desktop,\s*\n'
    r'(?:# the official desktop application for the Telegram messaging service\.\s*\n)?'
    r'#\s*\n'
    r'# For license and copyright information please follow this link:\s*\n'
    r'# https://github\.com/telegramdesktop/tdesktop/blob/master/LEGAL',
    re.MULTILINE
)

NEW_CMAKE_HEADER = """# This file is part of rabbitGram Desktop,
# the unofficial app based on Telegram Desktop.
#
# For license and copyright information please follow this link:
# https://github.com/rabbitgramdesktop/rabbitgramdesktop/blob/dev/LEGAL"""

EXTENSIONS = {'.cpp', '.h', '.hpp', '.c', '.mm', '.m', '.style', '.cmake', '.py', '.sh', '.iss'}
EXCLUDE_DIRS = {'.git', 'out', 'Libraries', 'win64', 'ThirdParty'}

RABBIT_HEADER_MARKER = "This file is part of rabbitGram Desktop,"


def get_repo_root(start_path):
    start_dir = start_path if os.path.isdir(start_path) else os.path.dirname(start_path)
    start_dir = os.path.normpath(os.path.abspath(start_dir))

    # Try git rev-parse --show-superproject-working-tree first
    try:
        res = subprocess.run(
            ['git', 'rev-parse', '--show-superproject-working-tree'],
            cwd=start_dir,
            capture_output=True,
            text=True,
            errors='ignore'
        )
        if res.returncode == 0 and res.stdout.strip():
            return os.path.normpath(os.path.abspath(res.stdout.strip()))
    except Exception:
        pass

    # Try git rev-parse --show-toplevel
    try:
        res = subprocess.run(
            ['git', 'rev-parse', '--show-toplevel'],
            cwd=start_dir,
            capture_output=True,
            text=True,
            errors='ignore'
        )
        if res.returncode == 0 and res.stdout.strip():
            return os.path.normpath(os.path.abspath(res.stdout.strip()))
    except Exception:
        pass

    # Fallback search upwards for root directory with AGENTS.md or top-level .git
    curr = start_dir
    candidate = curr
    while curr:
        if os.path.exists(os.path.join(curr, 'AGENTS.md')) or os.path.exists(os.path.join(curr, '.gitmodules')):
            return curr
        if os.path.isdir(os.path.join(curr, '.git')):
            candidate = curr
        parent = os.path.dirname(curr)
        if parent == curr:
            break
        curr = parent

    return candidate


def get_submodule_paths(repo_root):
    submodules = set()

    # 1. git submodule status --recursive
    try:
        res = subprocess.run(
            ['git', 'submodule', 'status', '--recursive'],
            cwd=repo_root,
            capture_output=True,
            text=True,
            errors='ignore'
        )
        if res.returncode == 0:
            for line in res.stdout.splitlines():
                line = line.strip()
                if line:
                    parts = line.split()
                    if len(parts) >= 2:
                        rel_path = parts[1]
                        abs_p = os.path.normpath(os.path.abspath(os.path.join(repo_root, rel_path)))
                        submodules.add(abs_p)
    except Exception:
        pass

    # 2. Parse all .gitmodules files under repo_root recursively
    for dirpath, dirnames, filenames in os.walk(repo_root):
        dirnames[:] = [d for d in dirnames if d not in EXCLUDE_DIRS]
        if '.gitmodules' in filenames:
            gitmodules_file = os.path.join(dirpath, '.gitmodules')
            try:
                with open(gitmodules_file, 'r', encoding='utf-8', errors='ignore') as f:
                    for line in f:
                        line = line.strip()
                        if line.startswith('path =') or line.startswith('path='):
                            rel_path = line.split('=', 1)[1].strip()
                            abs_p = os.path.normpath(os.path.abspath(os.path.join(dirpath, rel_path)))
                            submodules.add(abs_p)
            except Exception:
                pass

    return submodules


def is_submodule_path(target_path, repo_root, submodule_paths):
    norm_target = os.path.normpath(os.path.abspath(target_path))
    norm_root = os.path.normpath(os.path.abspath(repo_root))

    # 1. Exact match or inside any known submodule path
    for sub in submodule_paths:
        if norm_target == sub or norm_target.startswith(sub + os.sep):
            return True

    # 2. Check if target or any parent directory between repo_root and target has a .git entry
    curr = norm_target if os.path.isdir(norm_target) else os.path.dirname(norm_target)
    while curr and len(curr) >= len(norm_root):
        if curr != norm_root:
            git_entry = os.path.join(curr, '.git')
            if os.path.exists(git_entry):
                return True
        parent = os.path.dirname(curr)
        if parent == curr:
            break
        curr = parent

    return False


def process_file(filepath):
    """
    Returns one of:
    - 'updated': Header replaced from tdesktop to rabbitGram
    - 'already_rabbit': Already contained rabbitGram header
    - 'no_header': No recognized header found
    - 'error': Failed to read/write file
    """
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        if RABBIT_HEADER_MARKER in content:
            return 'already_rabbit'

        new_content = OLD_C_HEADER_RE.sub(NEW_C_HEADER, content, count=1)
        new_content = OLD_CMAKE_HEADER_RE.sub(NEW_CMAKE_HEADER, new_content, count=1)

        if new_content != content:
            with open(filepath, 'w', encoding='utf-8', newline='') as f:
                f.write(new_content)
            return 'updated'
        
        return 'no_header'
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return 'error'


def main(target_path):
    stats = {
        'total_scanned': 0,
        'already_rabbit': 0,
        'updated': 0,
        'no_header': 0,
        'error': 0,
        'skipped_submodules': 0,
    }

    repo_root = get_repo_root(target_path)
    submodule_paths = get_submodule_paths(repo_root)

    def process_and_count(path):
        if is_submodule_path(path, repo_root, submodule_paths):
            stats['skipped_submodules'] += 1
            return
        stats['total_scanned'] += 1
        res = process_file(path)
        if res in stats:
            stats[res] += 1

    if os.path.isfile(target_path):
        process_and_count(target_path)
    elif is_submodule_path(target_path, repo_root, submodule_paths):
        print(f"Ignored: Target path '{target_path}' is a submodule or inside a submodule.")
        stats['skipped_submodules'] += 1
    else:
        for dirpath, dirnames, filenames in os.walk(target_path):
            pruned_dirnames = []
            for d in dirnames:
                if d in EXCLUDE_DIRS:
                    continue
                full_d = os.path.join(dirpath, d)
                if is_submodule_path(full_d, repo_root, submodule_paths):
                    stats['skipped_submodules'] += 1
                    continue
                pruned_dirnames.append(d)
            dirnames[:] = pruned_dirnames

            for filename in filenames:
                ext = os.path.splitext(filename)[1].lower()
                if ext in EXTENSIONS or filename == 'CMakeLists.txt':
                    filepath = os.path.join(dirpath, filename)
                    process_and_count(filepath)

    total_scanned = stats['total_scanned']
    already_rabbit = stats['already_rabbit']
    updated = stats['updated']
    total_rabbit_now = already_rabbit + updated
    no_header = stats['no_header']

    print("=" * 50)
    print("Header Update Statistics")
    print("=" * 50)
    print(f"Total files scanned:           {total_scanned}")
    print(f"Already rabbitGram (Was):      {already_rabbit}")
    print(f"Updated from tdesktop (New):   {updated}")
    print(f"Total rabbitGram now (Now):    {total_rabbit_now}")
    print(f"Files without header/other:    {no_header}")
    if stats['skipped_submodules'] > 0:
        print(f"Skipped submodules:            {stats['skipped_submodules']}")
    if stats['error'] > 0:
        print(f"Errors:                        {stats['error']}")
    print("=" * 50)


if __name__ == '__main__':
    target = sys.argv[1] if len(sys.argv) > 1 else os.getcwd()
    main(target)
