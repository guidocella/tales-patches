#!/usr/bin/env python3
"""Remove the delay before skit prompts in Tales of Xillia Remastered.

Skits that require time spent in a location, up to 30 minutes, become available as soon as
their other conditions are met, and the game checks all skits in about 3 seconds at 30 FPS
instead of 25. Close the game before patching.

Usage: python skit_delay.py path/to/GameAssembly.dll [--revert]

A backup is saved as GameAssembly.dll.skit.bak. The bytes are verified before writing, so a
different game version is refused rather than broken.
"""
import os
import shutil
import sys

# (file offset, original, patched)
PATCHES = [
    (0x79FE4E, "7617", "eb17"),  # TO11ChatLinkList.Check: ignore the time requirement
    (0x7A29D0, "0f8440010000", "0f8496ffffff"),  # TaskWorkChatLink: go on after a failed check
]

args = [a for a in sys.argv[1:] if not a.startswith("--")]
revert = "--revert" in sys.argv
if len(args) != 1:
    sys.exit(__doc__)
path = args[0]

with open(path, "r+b") as f:
    states = set()
    for offset, original, patched in PATCHES:
        f.seek(offset)
        current = f.read(len(original) // 2)
        if current == bytes.fromhex(original):
            states.add("original")
        elif current == bytes.fromhex(patched):
            states.add("patched")
        else:
            sys.exit(f"unexpected bytes at {offset:#x}: {current.hex(' ')} (different game version?)")

    if states == {"original" if revert else "patched"}:
        print("already", "reverted" if revert else "patched")
        sys.exit(0)

    if not revert:
        backup = path + ".skit.bak"
        if not os.path.exists(backup):
            shutil.copy2(path, backup)
            print("backup:", backup)

    for offset, original, patched in PATCHES:
        f.seek(offset)
        f.write(bytes.fromhex(original if revert else patched))
print("reverted" if revert else "patched")
