using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using BepInEx;
using BepInEx.Configuration;
using BepInEx.Unity.IL2CPP;

namespace XilliaPatches;

// Fixes for Tales of Xillia Remastered, applied to GameAssembly.dll in memory at startup.
// Each patch checks the original bytes first and is skipped with a warning if they differ,
// so a different game version is left alone rather than broken.
[BepInPlugin("XilliaPatches", "XilliaPatches", "1.0.0")]
public class Plugin : BasePlugin
{
    // A byte change at an RVA of GameAssembly.dll.
    record Patch(uint Rva, string Original, string Patched);

    public override void Load()
    {
        var skits = Config.Bind("Skits", "InstantSkits", true,
            "To avoid the bug in the Remastered that delays consecutive skits with the same trigger for minutes, " +
            "make skits that require time spent in a location appear as soon as their other conditions are met, " +
            "and check all skits in about 3 seconds at 30 FPS instead of 25.");
        var projectiles = Config.Bind("Battle", "FixProjectileSpeed", true,
            "Fix the speed at 30 and 120 FPS of certain projectile artes, such as Alvin's Chase Bullet.");
        var counter = Config.Bind("Battle", "BossCounterTime", true,
            "Make bosses tech out after a configurable number of seconds of hitstun instead of after a few hits.");
        var counterSeconds = Config.Bind("Battle", "BossCounterSeconds", 15f, new ConfigDescription(
            "Seconds of hitstun in one combo before a boss can counter.",
            new AcceptableValueRange<float>(0.5f, 3600f)));
        var cursor = Config.Bind("Menus", "HideMouseCursor", false,
            "Hide the mouse cursor in menus, so it doesn't appear in the middle of the screen and steal the selection " +
            "when scrolling when playing with a keyboard.");

        if (skits.Value)
            Apply("skit delay", new Patch[]
            {
                // TO11ChatLinkList.Check: ignore the time spent in the location (jbe -> jmp)
                new(0x7A164E, "7617", "eb17"),
                // TO11ChatLinkList.TaskWorkChatLink: go on to the next skit after a failed check
                new(0x7A41D0, "0f8440010000", "0f8496ffffff"),
            });

        if (projectiles.Value)
            Apply("projectile speed", new Patch[]
            {
                // TO11Script.GetFramerate for gadget scripts: return 1.0 instead of 60 / FPS
                new(0x82C0FF, "f30f10802c010000660f7ec0", "b80000803f0f1f8000000000"),
            });

        if (counter.Value)
            Apply("boss counter time", BossCounter(counterSeconds.Value));

        if (cursor.Value)
            Apply("hidden mouse cursor", new Patch[]
            {
                // QuillPenCursor.SetCursorView: view = false
                new(0x5FDFEF, "0fb6f2488bd98bce", "31f690488bd98bce"),
                // Cursor.set_lockState: always None, so the system cursor isn't warped to the middle
                new(0x26AB44D, "8bd94885c07513", "31db4885c07513"),
                // InputSystemUIInputModule.ProcessPointer: keep the pointer off the UI as if locked,
                // and call Cursor.set_visible(false) instead of Cursor.get_lockState
                new(0x22CD4CB, "e840df3d00413bc60f859600000083bed0000000007527",
                    "e8b0df3d00660f1f84000000000083bed0000000009090"),
            });
    }

    // Enemy AI scripts counter (CUnit.SetAIEmergencyAttack) when the semi combo hit count reaches a
    // random number between UNIT_STATUS.emergencyDamageMin and emergencyDamageMax, which they read
    // through Script_GetParam 0x7007/0x7008. New code in the zero padding at the end of the il2cpp
    // section times the hitstun of each unit (CUnit.PreExecState, adding BTL.CTask.s_FrameTime) and
    // makes min/max read 0 (no counter) until that time reaches the setting, and 1 after.
    // Source: cave.asm.
    static Patch[] BossCounter(float seconds)
    {
        const uint cave = 0x2A20EE0, emergencyMin = cave + 0x6D, emergencyMax = cave + 0x75;
        const int threshold = 0x8B; // float seconds, imm32 of a cmp
        var code = Convert.FromHexString(
            "83bbdc030000007445803dbefada0000744f488b051f5bbc00488b80b80000004885c0743c48" +
            "83ec10f30f7f0424f30f10402cf30f5883f4020000f30f1183f4020000f30f6f04244883c410" +
            "eb1383bbe403000000750ac783f402000000000000488b43404885c0e900d103fe8b80b00100" +
            "00eb068b80b401000085c0741c837b5800741681bbf402000000007041b8000000007205b801" +
            "000000e9258300fe");
        BitConverter.GetBytes(seconds).CopyTo(code, threshold);
        return new Patch[]
        {
            // the code first, so the jumps never lead to a half-written cave
            new(cave, new string('0', code.Length * 2), Convert.ToHexString(code)),
            // CUnit.PreExecState (inlined UpdateCombo): mov rax,[rbx+0x40]; test rax,rax -> jmp combo_timer
            new(0xA5E046, "488b43404885c0", Jmp(0xA5E046, cave, 7)),
            // Script_GetParam 0x7007: mov eax,[rax+0x1b0] -> jmp emergency_min
            new(0xA28C6B, "8b80b0010000", Jmp(0xA28C6B, emergencyMin, 6)),
            // Script_GetParam 0x7008: mov eax,[rax+0x1b4] -> jmp emergency_max
            new(0xA28C83, "8b80b4010000", Jmp(0xA28C83, emergencyMax, 6)),
        };
    }

    // jmp rel32 padded with nops, as hex
    static string Jmp(uint from, uint to, int size) =>
        "E9" + Convert.ToHexString(BitConverter.GetBytes((int)(to - (from + 5)))) +
        string.Concat(Enumerable.Repeat("90", size - 5));

    void Apply(string name, Patch[] patches)
    {
        IntPtr module = GetModuleHandle("GameAssembly.dll");
        var writes = new List<(IntPtr, byte[])>();
        foreach (var p in patches)
        {
            var original = Convert.FromHexString(p.Original);
            var patched = Convert.FromHexString(p.Patched);
            IntPtr address = module + (int)p.Rva;
            var current = new byte[original.Length];
            Marshal.Copy(address, current, 0, current.Length);
            if (current.AsSpan().SequenceEqual(original))
                writes.Add((address, patched));
            else
            {
                Log.LogWarning($"{name}: unexpected bytes at RVA 0x{p.Rva:X} ({Convert.ToHexString(current)}), " +
                               "different game version? Not applied.");
                return;
            }
        }
        foreach (var (address, data) in writes)
            Write(address, data);
        Log.LogInfo($"{name}: applied");
    }

    static void Write(IntPtr address, byte[] data)
    {
        if (!VirtualProtect(address, (UIntPtr)data.Length, PAGE_EXECUTE_READWRITE, out uint old))
            throw new InvalidOperationException($"VirtualProtect failed: {Marshal.GetLastWin32Error()}");
        Marshal.Copy(data, 0, address, data.Length);
        VirtualProtect(address, (UIntPtr)data.Length, old, out _);
        FlushInstructionCache(GetCurrentProcess(), address, (UIntPtr)data.Length);
    }

    const uint PAGE_EXECUTE_READWRITE = 0x40;

    [DllImport("kernel32", SetLastError = true)]
    static extern IntPtr GetModuleHandle(string name);

    [DllImport("kernel32", SetLastError = true)]
    static extern bool VirtualProtect(IntPtr address, UIntPtr size, uint protect, out uint old);

    [DllImport("kernel32")]
    static extern IntPtr GetCurrentProcess();

    [DllImport("kernel32")]
    static extern bool FlushInstructionCache(IntPtr process, IntPtr address, UIntPtr size);
}
