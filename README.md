# Collection of cheats for Tales of games

## spell pause.txt

Disable the spell pause in Phantasia PS1, GBA and FVE and all of versions of Destiny PS1.

## Destiny bosses.txt

Lists the numbers of the encounter groups of every boss fight in the game to be selected in the debug mode menu.

## Eternia bosses.txt

Lists the numbers of the encounter groups and stages of every boss fight in the game to be selected in the debug mode menu, and which music plays.

## Destiny 2 PS2, original release

`SLPS-25172_A86BA5E4.pnach` includes miscellaneous cheats, ucuckic's cheat for 4 extra arte slots, and cheats to re-fight every boss.

## Rebirth PS2

`SLPS-25450_B4EC196F.pnach` includes a cheat for 8 arte slots, miscellaneous cheats, and cheats to re-fight every boss and unique encounter.

## Legendia (Japan)

`SLPS-25533_1F8640E0.pnach` includes a cheat for 60 FPS battles.

## Phantasia Full Voice Edition

`ULJS00079.ini` includes cheats for 32 arte slots, uncropping the world map restoring the PS1 zoom level, and manually starting random encounters.

The arte slots cheat uses PPSSPP's extended controls. You navigate the menu pages with left and right, and use the new slots with right stick, L2, R2, or L2+R2.

See https://github.com/lifebottle/Tales-of-Phantasia-Full-Voice-Edition/releases for more cheats.

## Phantasia Cross Edition

`ULJS00293-TOPX.ini` includes cheats for 32 arte slots, uncropping the world map restoring the PS1 zoom level, and cheats with the encounter group and stage of every boss.

The arte slots cheat uses PPSSPP's extended controls. You navigate the menu pages with left and right, and use the new slots with right stick, L2, R2, or L2+R2. This is compatible with the multiplayer patch.

For playing in Japanese, there is Life Bottle's 60 FPS battles cheat modified to no longer crash in the NDX start menu, and a cheat to fix Suzu's normal attacks that can be used without the translation patch.

See https://github.com/lifebottle/Tales-of-Phantasia-X/releases for more cheats.

## Narikiri Dungeon X

`ULJS00293-NDX.ini` includes cheats for 60 FPS, 28 arte slots using PPSSPP's extended controls, and co-op using https://github.com/Armonte/ppsspp-tales-mp.

See https://ritzcode.web.fc2.com/cwcpsp/topndx.txt for more cheats.

## Tales of the Heroes: Twin Brave

`NPJH50501.ini` includes cheats for 60 FPS and 3 more arte slots on L2+○/×/▲, using PPSSPP's extended controls.

## Tales of Innocence R

`PCSG00009_60fps.txt` is a 60 FPS patch.

`PCSG00009_artes.txt` implements 32 arte slots on L2, R2 and L2+R2. Navigate the arte menu with left and right to access the new pages.

Both are compatible with the co-op patch.

Put them in `%AppData%\Vita3K\patch` (Windows) or `~/.local/share/Vita3K/patch` (Linux).

# Tales of Hearts R

`PCSE00429_60fps.txt` (US) / `PCSB00550_60fps.txt` (EU) / `PCSG00125_60fps.txt` (Japan) is a 60 FPS patch.

`PCSE00429_artes.txt` (US) / `PCSB00550_artes.txt` (EU) / `PCSG00125_artes.txt` (Japan) implements 36 arte slots on L2 and R2. Navigate the arte menu with left and right to access the new pages. This also unlocks the page with slots for the normal attack button since the start of the game.

Both are compatible with the co-op patch.

Put them in `%AppData%\Vita3K\patch` (Windows) or `~/.local/share/Vita3K/patch` (Linux).

## Xillia/

Patches for Tales of Xillia remastered:

- `Xillia/XilliaPatches.dll` is a BepInEx plugin with these fixes, which can each be turned on or off in `BepInEx/config/XilliaPatches.cfg` (created when the game starts with the plugin):
  - To avoid the bug in the Remastered that delays consecutive skits with the same trigger for minutes, make skits that require time spent in a location appear as soon as their other conditions are met.
  - Fix the speed at 30 and 120 FPS of certain projectile artes, such as Alvin's Chase Bullet. (https://www.youtube.com/watch?v=ztST83z0tM4).
  - Make bosses tech out after a configurable number of seconds of hitstun instead of after a few hits.
  - Hide the mouse cursor in menus, so it doesn't appear in the middle of the screen and steal the selection when scrolling when playing with a keyboard.

  If you don't already have it from [TalesofXilliaFix](https://codeberg.org/Lyall/TalesofXilliaFix), install [BepInEx 6 for IL2CPP](https://builds.bepinex.dev/projects/bepinex_be), and copy the DLL to `BepInEx/plugins`.
- `Xillia/wine-fps/version.dll` fixes frame rate drops/slowdown at 120 FPS when running in Wine caused by Unity calling CreateSemaphoreExW and its destructor ~100 times per frame, which requires a wineserver round trip. It recycles the anonymous semaphores from a pool instead. This should also work for other Unity games.

## See also:

https://github.com/Sora3100/Tales_of_Cheat_Tables

https://github.com/ucuckic/tales_patches
