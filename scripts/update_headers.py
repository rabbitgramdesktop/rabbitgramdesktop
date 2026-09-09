#!/usr/bin/env python3
"""
Script to replace tdesktop license headers with rabbitGram license headers
across the codebase and display detailed statistics.
"""

import os
import sys
import re

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
    }

    def process_and_count(path):
        stats['total_scanned'] += 1
        res = process_file(path)
        if res in stats:
            stats[res] += 1

    if os.path.isfile(target_path):
        process_and_count(target_path)
    else:
        for dirpath, dirnames, filenames in os.walk(target_path):
            dirnames[:] = [d for d in dirnames if d not in EXCLUDE_DIRS]
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
    if stats['error'] > 0:
        print(f"Errors:                        {stats['error']}")
    print("=" * 50)

if __name__ == '__main__':
    target = sys.argv[1] if len(sys.argv) > 1 else os.getcwd()
    main(target)
