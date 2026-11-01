#!/usr/bin/env python3
"""Hide the mouse cursor and ignore mouse hover in Tales of Xillia Remastered.

In menus the game draws its own cursor, which appears in the middle of the screen, and when
a menu opens or scrolls, the item under it takes focus away from the one selected with the
keyboard. With this patch the cursor is hidden, but mouse clicks and hover in menus stop
working; gamepad and keyboard are unaffected. Close the game before patching.

Usage: python hide_cursor.py path/to/GameAssembly.dll [--revert]

A backup is saved as GameAssembly.dll.cursor.bak. The bytes are verified before writing, so
a different game version is refused rather than broken.
"""
import os
import shutil
import sys

# (file offset, original, patched)
PATCHES = [
    (0x5FC7EF, "0fb6f2488bd98bce", "31f690488bd98bce"),  # QuillPenCursor.SetCursorView: view = false
    (0x26A9C4D, "8bd94885c07513", "31db4885c07513"),  # Cursor.set_lockState: always None
    # InputSystemUIInputModule.ProcessPointer: move the pointer off the UI as if locked, and
    # replace Cursor.get_lockState with Cursor.set_visible(false)
    (0x22CBCCB, "e840df3d00413bc60f859600000083bed0000000007527",
     "e8b0df3d00660f1f84000000000083bed0000000009090"),
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
        backup = path + ".cursor.bak"
        if not os.path.exists(backup):
            shutil.copy2(path, backup)
            print("backup:", backup)

    for offset, original, patched in PATCHES:
        f.seek(offset)
        f.write(bytes.fromhex(original if revert else patched))
print("reverted" if revert else "patched")
