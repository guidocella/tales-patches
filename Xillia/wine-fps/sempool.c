/*
 * version.dll proxy for Tales of Xillia Remastered (Unity 2022.3.59f1).
 *
 * UnityPlayer's baselib::UnityClassic::Lock constructor calls
 * CreateSemaphoreExW and its destructor calls CloseHandle. URP constructs
 * ~100 of these per frame (RenderNodeQueue in ExecuteScriptableRenderLoop).
 * On Windows that is cheap; under Wine each create/close is a wineserver
 * round trip, which costs ~2.5 ms per frame on the main thread and caps the
 * game at ~100 FPS. Since the game's animation is frame-locked, that makes
 * everything run slow.
 *
 * This DLL patches UnityPlayer's IAT so those anonymous semaphores are
 * recycled from a pool instead of being created and destroyed every frame.
 *
 * version.rc gives the DLL a non-Microsoft CompanyName, so Wine prefers it
 * over the builtin version.dll without a DllOverrides entry.
 */

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef void *HANDLE;
typedef int BOOL;

#define WINAPI __stdcall
#define DLL_PROCESS_ATTACH 1
#define PAGE_READWRITE 0x04
#define WAIT_OBJECT_0 0
#define LOCK_SEMAPHORE_ACCESS 0x1F0002
#define LOCK_SEMAPHORE_MAX 0x7FFFFFFF

typedef struct { void *p; } SRWLOCK;

__declspec(dllimport) HANDLE WINAPI CreateSemaphoreExW(void *, long, long, const u16 *, u32, u32);
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE);
__declspec(dllimport) u32 WINAPI WaitForSingleObject(HANDLE, u32);
__declspec(dllimport) void WINAPI AcquireSRWLockExclusive(SRWLOCK *);
__declspec(dllimport) void WINAPI ReleaseSRWLockExclusive(SRWLOCK *);
__declspec(dllimport) void *WINAPI GetModuleHandleW(const u16 *);
__declspec(dllimport) void *WINAPI LoadLibraryW(const u16 *);
__declspec(dllimport) void *WINAPI GetProcAddress(void *, const char *);
__declspec(dllimport) u32 WINAPI GetSystemDirectoryW(u16 *, u32);
__declspec(dllimport) BOOL WINAPI VirtualProtect(void *, u64, u32, u32 *);

/* ---- version.dll forwarding ---- */

#define VERSION_EXPORTS(X) \
    X(GetFileVersionInfoA) X(GetFileVersionInfoExA) X(GetFileVersionInfoExW) \
    X(GetFileVersionInfoSizeA) X(GetFileVersionInfoSizeExA) X(GetFileVersionInfoSizeExW) \
    X(GetFileVersionInfoSizeW) X(GetFileVersionInfoW) X(VerFindFileA) X(VerFindFileW) \
    X(VerInstallFileA) X(VerInstallFileW) X(VerLanguageNameA) X(VerLanguageNameW) \
    X(VerQueryValueA) X(VerQueryValueW)

#define DECLARE_PTR(name) void *p_##name;
VERSION_EXPORTS(DECLARE_PTR)

#define DECLARE_STUB(name) \
    __attribute__((naked)) void name(void) { __asm__("jmpq *p_" #name "(%rip)"); }
VERSION_EXPORTS(DECLARE_STUB)

static void load_real_version(void)
{
    static const u16 suffix[] = u"\\version.dll";
    u16 path[300];
    u32 len = GetSystemDirectoryW(path, 260);
    if (!len || len >= 260)
        return;
    for (u32 i = 0; i < sizeof(suffix) / sizeof(*suffix); i++)
        path[len + i] = suffix[i];
    void *real = LoadLibraryW(path);
    if (!real)
        return;
#define RESOLVE(name) p_##name = GetProcAddress(real, #name);
    VERSION_EXPORTS(RESOLVE)
}

/* ---- semaphore pool ---- */

#define POOL_MAX 4096
#define LIVE_SLOTS 16384 /* power of two, open addressing */
#define TOMBSTONE ((HANDLE)1)

static SRWLOCK lock;
static HANDLE pool[POOL_MAX];
static u32 pool_count;
static HANDLE live[LIVE_SLOTS];
static u32 live_count;

static HANDLE(WINAPI *real_CreateSemaphoreExW)(void *, long, long, const u16 *, u32, u32);
static BOOL(WINAPI *real_CloseHandle)(HANDLE);

static u32 slot_of(HANDLE h) { return ((u32)((u64)h >> 2) * 2654435761u) & (LIVE_SLOTS - 1); }

/* Caller holds lock. */
static BOOL live_insert(HANDLE h)
{
    if (live_count >= LIVE_SLOTS / 2)
        return 0;
    u32 i = slot_of(h);
    while (live[i] && live[i] != TOMBSTONE)
        i = (i + 1) & (LIVE_SLOTS - 1);
    live[i] = h;
    live_count++;
    return 1;
}

/* Caller holds lock. */
static BOOL live_remove(HANDLE h)
{
    for (u32 i = slot_of(h); live[i]; i = (i + 1) & (LIVE_SLOTS - 1)) {
        if (live[i] == h) {
            live[i] = TOMBSTONE;
            live_count--;
            return 1;
        }
    }
    return 0;
}

static HANDLE WINAPI hook_CreateSemaphoreExW(void *attrs, long initial, long max, const u16 *name,
                                             u32 flags, u32 access)
{
    if (attrs || initial || max != LOCK_SEMAPHORE_MAX || name || flags || access != LOCK_SEMAPHORE_ACCESS)
        return real_CreateSemaphoreExW(attrs, initial, max, name, flags, access);

    HANDLE h = 0;
    AcquireSRWLockExclusive(&lock);
    if (pool_count) {
        h = pool[--pool_count];
        live_insert(h);
    }
    ReleaseSRWLockExclusive(&lock);
    if (h)
        return h;

    h = real_CreateSemaphoreExW(attrs, initial, max, name, flags, access);
    if (!h)
        return h;
    AcquireSRWLockExclusive(&lock);
    live_insert(h); /* if the table is full the handle is simply not pooled */
    ReleaseSRWLockExclusive(&lock);
    return h;
}

static BOOL WINAPI hook_CloseHandle(HANDLE h)
{
    AcquireSRWLockExclusive(&lock);
    BOOL ours = live_remove(h);
    ReleaseSRWLockExclusive(&lock);
    if (!ours)
        return real_CloseHandle(h);

    /* Lock semaphores are normally at count 0 when destroyed; make sure. */
    int drained = 0;
    while (WaitForSingleObject(h, 0) == WAIT_OBJECT_0)
        if (++drained > 64)
            break;

    AcquireSRWLockExclusive(&lock);
    if (drained <= 64 && pool_count < POOL_MAX) {
        pool[pool_count++] = h;
        h = 0;
    }
    ReleaseSRWLockExclusive(&lock);
    return h ? real_CloseHandle(h) : 1;
}

static int str_eq_i(const char *a, const char *b)
{
    for (;; a++, b++) {
        char ca = *a >= 'A' && *a <= 'Z' ? *a + 32 : *a;
        char cb = *b >= 'A' && *b <= 'Z' ? *b + 32 : *b;
        if (ca != cb)
            return 0;
        if (!ca)
            return 1;
    }
}

static int patch_iat(char *base, const char *dll, const char *func, void *hook, void **orig)
{
    char *nt = base + *(u32 *)(base + 0x3C);
    u32 imports = *(u32 *)(nt + 0x90);
    for (u32 *desc = (u32 *)(base + imports); desc[3]; desc += 5) {
        if (!str_eq_i(base + desc[3], dll))
            continue;
        u64 *names = (u64 *)(base + (desc[0] ? desc[0] : desc[4]));
        void **iat = (void **)(base + desc[4]);
        for (int i = 0; names[i]; i++) {
            if ((names[i] >> 63) || !str_eq_i(base + (u32)names[i] + 2, func))
                continue;
            u32 old;
            if (!VirtualProtect(&iat[i], sizeof(void *), PAGE_READWRITE, &old))
                return 0;
            *orig = iat[i];
            iat[i] = hook;
            VirtualProtect(&iat[i], sizeof(void *), old, &old);
            return 1;
        }
    }
    return 0;
}

BOOL WINAPI DllMain(void *instance, u32 reason, void *reserved)
{
    if (reason != DLL_PROCESS_ATTACH)
        return 1;

    load_real_version();

    char *unity = GetModuleHandleW(u"UnityPlayer.dll");
    if (unity) {
        real_CreateSemaphoreExW = CreateSemaphoreExW;
        real_CloseHandle = CloseHandle;
        void *orig_create, *orig_close;
        if (patch_iat(unity, "kernel32.dll", "CreateSemaphoreExW", hook_CreateSemaphoreExW, &orig_create)) {
            if (patch_iat(unity, "kernel32.dll", "CloseHandle", hook_CloseHandle, &orig_close)) {
                real_CreateSemaphoreExW = orig_create;
                real_CloseHandle = orig_close;
            } else {
                patch_iat(unity, "kernel32.dll", "CreateSemaphoreExW", orig_create, &orig_close);
            }
        }
    }
    return 1;
}
