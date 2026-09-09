#!/usr/bin/env python3
"""
Script to replace tdesktop license headers with rabbitGram license headers
across the codebase.
"""

import os
import sys
import re

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

def process_file(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()

        new_content = OLD_C_HEADER_RE.sub(NEW_C_HEADER, content, count=1)
        new_content = OLD_CMAKE_HEADER_RE.sub(NEW_CMAKE_HEADER, new_content, count=1)

        if new_content != content:
            with open(filepath, 'w', encoding='utf-8', newline='') as f:
                f.write(new_content)
            return True
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
    return False

def main(target_path):
    updated_count = 0
    total_scanned = 0

    if os.path.isfile(target_path):
        total_scanned += 1
        if process_file(target_path):
            updated_count += 1
    else:
        for dirpath, dirnames, filenames in os.walk(target_path):
            dirnames[:] = [d for d in dirnames if d not in EXCLUDE_DIRS]
            for filename in filenames:
                ext = os.path.splitext(filename)[1].lower()
                if ext in EXTENSIONS or filename == 'CMakeLists.txt':
                    filepath = os.path.join(dirpath, filename)
                    total_scanned += 1
                    if process_file(filepath):
                        updated_count += 1

    print(f"Scanned {total_scanned} file(s).")
    print(f"Updated {updated_count} file(s) to rabbitGram license header.")

if __name__ == '__main__':
    target = sys.argv[1] if len(sys.argv) > 1 else os.getcwd()
    main(target)
