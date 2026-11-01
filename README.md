# Collection of cheats for Tales of games

## spell pause.txt

Disable the spell pause in Phantasia PS1, GBA and FVE and all of versions of Destiny PS1.

## Eternia bosses.txt

Lists the numbers of the encounter groups and stages of every boss fight in the game to be selected in the debug mode menu, and which music plays.

## Destiny PS2, original release

`SLPS-25172_A86BA5E4.pnach` includes miscellaneous cheats, ucuckic's cheat for 4 extra arte slots, and cheats to re-fight every boss.

## Rebirth

`SLPS-25450_B4EC196F.pnach` includes a cheat for 8 arte slots, miscellaneous cheats, and cheats to re-fight every boss and unique encounter.

## Legendia (Japanese)

`SLPS-25533_1F8640E0.pnach` includes a cheat for 60 FPS battles.

## Phantasia Full Voice Edition

`ULJS00079.ini` includes cheats for 16 arte slots using PPSSPP's extended controls, uncropping the world map restoring the PS1 zoom level, and manually starting random encounters.

See https://github.com/lifebottle/Tales-of-Phantasia-Full-Voice-Edition/releases for more cheats.

## Phantasia Cross Edition

`ULJS00293-TOPX.ini` includes cheats for 16 arte slots using PPSSPP's extended controls, uncropping the world map restoring the PS1 zoom level, and cheats with the encounter group and stage of every boss.

For playing in Japanese, there is Life Bottle's 60 FPS battles cheat modified to no longer crash in the NDX start menu, and a cheat to fix Suzu's normal attacks that can be used without the translation patch.

See https://github.com/lifebottle/Tales-of-Phantasia-X/releases for more cheats.

## Narikiri Dungeon X

`ULJS00293-NDX.ini` includes cheats for 60 FPS, 28 arte slots using PPSSPP's extended controls, and co-op using https://github.com/Armonte/ppsspp-tales-mp.

See https://ritzcode.web.fc2.com/cwcpsp/topndx.txt for more cheats.

## Tales of the Heroes: Twin Brave

`NPJH50501.ini` includes cheats for 60 FPS and 3 more arte slots on L2+○/×/▲, using PPSSPP's extended controls.

## Xillia/

Patches for Tales of Xillia remastered:

- `python skit_delay.py /path/to/GameAssembly.dll` makes timed skits appear instantly to fix the bug in the remastered that delays consecutive skits with the same trigger.
- `python projectile_speed.py /path/to/GameAssembly.dll` fixes the speed of certain projectile artes at 30 and 120 FPS (https://www.youtube.com/watch?v=ztST83z0tM4).
- `python hide_cursor.py /path/to/GameAssembly.dll` disables the mouse cursor in menus when playing with a keyboard, which prevents it from stealing focus when scrolling.
- `wine-fps/version.dll` fixes frame rate drops/slowdown at 120 FPS when running in Wine caused by Unity calling CreateSemaphoreExW and its destructor ~100 times per frame, which requires a wineserver round trip. It recycles the anonymous semaphores from a pool instead. This should also work for other Unity games.

## See also:

https://github.com/Sora3100/Tales_of_Cheat_Tables

https://github.com/ucuckic/tales_patches
