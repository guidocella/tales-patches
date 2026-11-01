#!/usr/bin/env python3
"""Fix the speed of projectile artes at 30 and 120 FPS in Tales of Xillia Remastered.

Some effect scripts, e.g. Alvin's Chase Bullet, scale their movement by the script function
get_framerate() on top of a path that already compensates for the frame rate, so projectiles
move at half speed at 120 FPS and double speed at 30 FPS. This makes get_framerate() return
1.0 for those scripts. Close the game before patching.

Usage: python projectile_speed.py path/to/GameAssembly.dll [--revert]

A backup is saved as GameAssembly.dll.projectile.bak. The bytes are verified before writing,
so a different game version is refused rather than broken.
"""
import os
import shutil
import sys

# (file offset, original, patched)
PATCHES = [
    (0x82A8FF, "f30f10802c010000660f7ec0", "b80000803f0f1f8000000000"),  # TO11Script.GetFramerate
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
        backup = path + ".projectile.bak"
        if not os.path.exists(backup):
            shutil.copy2(path, backup)
            print("backup:", backup)

    for offset, original, patched in PATCHES:
        f.seek(offset)
        f.write(bytes.fromhex(original if revert else patched))
print("reverted" if revert else "patched")
