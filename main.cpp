// OnyxNotch v5 (features 1 + 2 + 3: SMTC track titles, saved
// settings + auto-start, themes) - Mac-style notch for Windows,
// native Win32 C++.
// v5-themes changes over v5-settings (feature 3 only, nothing
// else is added here: no position/size customisation beyond
// the existing Size menu, no weather, no calendar events, no
// installer):
//  1. Theme system: three themes (Onyx = the v4 look, default;
//     Midnight = deep blue; Graphite = lighter grey, still dark),
//     each defining gradient top/bottom, border, primary and
//     secondary text, and an accent (today pill, progress-bar
//     fill, active tab underline). Painting reads the current
//     theme everywhere — no element keeps a hardcoded v4 colour.
//  2. Themes submenu in the tray menu (shared with the gear
//     menu, which opens the same menu): tick on the current
//     theme, instant repaint of all notches on switch, saved
//     via the existing settings file mechanism (theme key).
//  3. SMTC behaviour and settings/auto-start behaviour are
//     untouched except for the added theme key; build.bat is
//     unchanged (themes need no new library).
// v5-settings changes over v5-SMTC:
//  1. Saved settings: a plain key=value file at
//     %APPDATA%\OnyxNotch\settings.ini (no JSON library), loaded
//     in wWinMain BEFORE the manager window creates the first
//     notch, and saved whenever a size/toggle changes and again
//     on clean quit. Persisted keys: size, hide_on_fullscreen,
//     media, widget_stats, widget_calendar, animations, theme
//     (theme=onyx|midnight|graphite; an unknown value falls back
//     to Onyx on load, see LoadSettings).
//  2. Auto-start: a tray toggle "Start with Windows" (default
//     OFF, in the same tray menu the gear opens) that writes or
//     removes HKCU\Software\Microsoft\Windows\CurrentVersion\Run
//     \OnyxNotch with the quoted current .exe path. At startup
//     the registry value is read to initialise the toggle; the
//     registry is the source of truth for auto-start, the file
//     for everything else. Auto-start is never written to the
//     settings file.
//  3. Robustness: a missing or corrupt settings file falls back
//     to the old defaults and never prevents startup; unknown
//     keys and malformed lines are ignored, and out-of-range
//     values are ignored. The settings directory is created if
//     missing. Saving is a full rewrite of the known keys only
//     (see SaveSettings); a failed save is silently ignored so
//     it can never block a toggle or a quit. Registry calls need
//     advapi32.lib (see build.bat); file calls need no new lib.
// v5-SMTC changes over tabbed v4 (themes are feature 3, added
// after settings — see the v5-themes note at the top):
//  1. When Media controls are on, the current SMTC session's track
//     title/artist are read on the existing 1-second timer (no new
//     timer, thread, or polling) via C++/WinRT
//     (GlobalSystemMediaTransportControlsSessionManager) and shown
//     in the MEDIA tab (title line, artist line) and as one compact
//     title line in the HOME media summary. Text that does not fit
//     the notch width is truncated with an ellipsis by GDI.
//  2. The play/pause glyph now reflects SMTC playback status when
//     it is known: playing shows pause bars, paused shows a play
//     triangle. Any other/unknown status keeps the old combined
//     play+pause glyph, so nothing is claimed that is not known.
//  3. Failure fallback is exact v4 behaviour: no session, no
//     properties, or any WinRT failure means no title/artist lines
//     and no glyph change, no crash, no blank hole — the expanded
//     heights shrink back to the v4 sizes. Media keys remain the
//     only control path; SMTC is read-only here, never used to
//     play/pause/seek. No network: SMTC is the local media
//     session broker only.
//  4. Build requirement (new): MSVC with C++17 and the Windows SDK
//     C++/WinRT headers, linked against windowsapp.lib. MinGW
//     cannot build the WinRT part, so MSVC is the supported path
//     for v5-SMTC (see README). When Media controls are off, the
//     SMTC code is never called, so idle cost is unchanged.
// OnyxNotch v4 - Mac-style notch for Windows, native Win32 C++.
// v4 changes over v3 (UI ONLY: no new features, no behaviour changes):
//  1. Premium surface: vertical near-black gradient, subtle light
//     border, and a soft inner top highlight, instead of flat black.
//  2. Premium typography: Segoe UI, lighter big-clock weight, more
//     generous spacing, near-white primary text, muted secondary text.
//  3. Circular media buttons (diameter = media-row height) with the
//     hit test updated to match the drawn circles exactly.
//  4. Thin divider between the clock/date/battery block and the
//     media/widgets sections; a drawn battery glyph next to the
//     battery text; today in the calendar as a white pill.
//  5. No acrylic/blur: deliberately skipped, not faked (see README).
//     The window is region-shaped and non-layered, and acrylic
//     interplay with that shape cannot be verified without a
//     Windows test, so the gradient version is the shipped look.
//  6. No new timers, threads, polling, or dependencies; animation
//     timing and the 1-second clock timer are exactly as in v3.
// v4 reorganization (in place, after the v4 push — no v5):
//  1. Compact two-column body: below the full-width header (big
//     clock, date, battery, divider), media and system-stats sit
//     side by side when both are on, each about half the notch
//     width; a lone section spans the full body width. Calendar
//     stays full-width below the two-column row. Total expanded
//     height drops whenever media + stats are both on, versus
//     the old stacked layout, and the always-visible footer is
//     gone (pinning is unchanged; its explanation moves to the
//     About box and the README).
//  2. Grouped section headers: MEDIA, SYSTEM, CALENDAR — small
//     uppercase muted-grey labels, left-aligned in each section.
//     Stats is split from one dense line into two labelled rows
//     (CPU %, RAM % with GB used/total), each with a thin bar.
//  3. Settings gear button in the expanded header, top-right:
//     plain-GDI gear (teeth + centre hole, no image/font glyph).
//     Clicking it opens the existing tray menu at the gear's
//     screen position; it is hit-tested before the pin click, so
//     it never pins/unpins. No new settings window is built.
// v4 tabs (in place, after the reorganization — no v5):
//  1. Tab bar in the expanded notch: HOME, MEDIA, SYSTEM,
//     CALENDAR (only the enabled ones, after HOME). No tab bar
//     when nothing is enabled: the expanded view is just the
//     header, as before.
//  2. HOME is a compact summary of every enabled feature:
//     smaller media buttons, one stats line, the current week
//     only. Each other tab shows that feature's full version.
//  3. Clicking a tab switches the body (never pins, never fires
//     a media key, never opens the gear menu); height animates
//     via the existing animation path. The selected tab resets
//     to HOME on the next expand after a collapse.
// v3 changes over v2 (v2 behaviour is otherwise intact):
//  1. Smooth expand/collapse animation (~220 ms, ease-out cubic),
//     driven by a short-lived per-notch timer that stops when the
//     animation finishes. Turning the Animations tray tick off
//     restores the instant v2 behaviour everywhere.
//  2. Media-button hover and pressed highlights, plus a subtle
//     outline when the mouse is over the notch. Hit-test rects
//     are unchanged; only the drawing changes.
//  3. Anti-aliased text (ClearType font quality) and the same
//     rounded-corner region as v2. No fade/slide-in on first show:
//     that would need a layered-window rewrite, so first show
//     stays instant (see README). No new threads, no polling,
//     no network, no new dependencies.
// No Electron, no Python, no .NET runtime. One .exe, one source file
// (plus a small .rc/.ico for the custom tray/app icon).
// Build on Windows: run build.bat in a Visual Studio Developer Command Prompt.
//   cl /std:c++17 /O2 /MT main.cpp onyxnotch.res user32.lib gdi32.lib shell32.lib windowsapp.lib
// MinGW cannot build the WinRT (SMTC) part; MSVC is the supported
// path for v5-SMTC (see README). No MinGW command is given for v5.
//
// v2 changes over v1 (v1 behaviour is otherwise intact):
//  1. Multi-monitor: one notch per monitor, top-centre of each monitor's
//     work area, rebuilt on display change, per-monitor DPI aware.
//  2. Hide on fullscreen, OPT-IN (tray, default OFF): a notch hides on a
//     monitor while a fullscreen/borderless window covers that monitor,
//     and returns when fullscreen ends. Checked once per second.
//  3. Custom tray/app icon: a black-pill icon (onyxnotch.ico, via
//     onyxnotch.rc) instead of the default Windows application icon.
//  4. Media controls, OPT-IN (tray, default OFF): play/pause, previous,
//     next via plain Win32 media keys (VK_MEDIA_*). See the README for
//     the honest SMTC limit: no track title/artist is shown, because
//     title retrieval is not available in plain Win32 without C++/WinRT,
//     and this build deliberately does not add that dependency.
//  5. Optional widgets, each toggleable from the tray (default OFF,
//     except the v1 date/battery expanded view, which stays always on):
//     - System stats: CPU % (from GetSystemTimes deltas) and RAM use
//       (GlobalMemoryStatusEx). Sampled only while the widget is on.
//     - Calendar: a small month grid for the current month, today
//       highlighted. No network, no telemetry, no extra runtime.
//  Idle cost stays low: one 1-second timer, owned by a hidden manager
//  window. Nothing is sampled or drawn for a widget that is off, and
//  the clock is the only thing refreshed when everything is off.

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <tchar.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdio.h> // v5-settings: sprintf_s in SaveSettings
#include <string.h> // v5-settings: strlen/strcmp in LoadSettings
// v5-SMTC: C++/WinRT headers for the local SMTC session manager.
// These need the Windows SDK with C++/WinRT and /std:c++17 (see
// build.bat). No other WinRT API is used; there is no network use.
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "windowsapp.lib")
#pragma comment(lib, "advapi32.lib") // v5-settings: Reg* calls for auto-start

#define ID_TRAY_ICON            1
#define WM_TRAY                 (WM_APP + 1)
#define WM_APP_REBUILD          (WM_APP + 2)
#define ID_TIMER_CLOCK          1
#define ID_TIMER_ANIM           2
#define ID_TRAY_ANIMATIONS      1013
#define ID_TRAY_QUIT            1001
#define ID_TRAY_SIZE_S          1002
#define ID_TRAY_SIZE_M          1003
#define ID_TRAY_SIZE_L          1004
#define ID_TRAY_ABOUT           1005
#define ID_TRAY_HIDE_FULLSCREEN 1006
#define ID_TRAY_MEDIA           1007
#define ID_TRAY_WIDGET_STATS    1008
#define ID_TRAY_WIDGET_CALENDAR 1009
#define ID_TRAY_MEDIA_PREV      1010
#define ID_TRAY_MEDIA_PLAYPAUSE 1011
#define ID_TRAY_MEDIA_NEXT      1012
#define ID_TRAY_AUTOSTART       1014
#define ID_TRAY_THEME_ONYX      1015
#define ID_TRAY_THEME_MIDNIGHT  1016
#define ID_TRAY_THEME_GRAPHITE  1017
#define IDI_ONYXNOTCH_ICON      100

static const wchar_t kNotchClass[] = L"OnyxNotch";
static const wchar_t kManagerClass[] = L"OnyxNotchManager";
static const wchar_t kWindowTitle[] = L"OnyxNotch";

// ---------------------------------------------------------------------------
// Settings (tray toggles). Defaults preserve v1 behaviour.
// ---------------------------------------------------------------------------
static int  g_size = 1;                 // 0 = Small, 1 = Medium, 2 = Large
static bool g_hideOnFullscreen = false; // opt-in
static bool g_mediaEnabled = false;     // opt-in
static bool g_widgetStats = false;      // opt-in
static bool g_widgetCalendar = false;   // opt-in
static bool g_animations = true;        // v3: ON by default, tray tick
// v5-settings: auto-start is NOT stored in the settings file.
// The registry (see below) is its only source of truth, so this
// starts OFF and is overwritten by LoadAutoStart() at startup.
static bool g_startWithWindows = false; // v5-settings: OFF by default

// ---------------------------------------------------------------------------
// v5-themes: theme table. Onyx reproduces the v4 colours exactly
// (gradient 32,32,36 -> 10,10,12; border 58 normal / 104 hover;
// primary 248,248,250; secondary 165,165,170; muted 140,140,140;
// buttons 44 / hover 66 / pressed 94; gear 165 / 210 / 240),
// so Onyx is pixel-identical to v4. Midnight is a deep blue
// gradient with a blue accent; Graphite is a lighter grey
// gradient that is still dark, with a light-grey accent.
// accent = today pill fill, progress-bar fill, active tab
// underline (all three share it, per theme). pillText is the
// dark text drawn on an accent pill; glyph is the media glyph
// colour; batteryFill is the battery fill (primary in all
// themes); gearHole is the gear's dark centre hole. Painting
// functions read CurrentTheme() — they hold no colour literals.
// ---------------------------------------------------------------------------
struct Theme {
    COLORREF gradTop, gradBottom;
    COLORREF border, borderHover, highlight;
    COLORREF primary, secondary, muted, tabHover;
    COLORREF accent;
    COLORREF divider;
    COLORREF btnBg, btnHover, btnPressed;
    COLORREF glyph;
    COLORREF gear, gearHover, gearPressed, gearHole;
    COLORREF pillText;
    COLORREF batteryOutline, batteryFill, batteryBolt;
};

static const Theme kThemes[3] = {
    // Onyx (default): the v4 look, exactly.
    { RGB(32, 32, 36), RGB(10, 10, 12),
      RGB(58, 58, 62), RGB(104, 104, 110), RGB(78, 78, 84),
      RGB(248, 248, 250), RGB(165, 165, 170), RGB(140, 140, 140), RGB(210, 210, 214),
      RGB(248, 248, 250),
      RGB(52, 52, 56),
      RGB(44, 44, 48), RGB(66, 66, 70), RGB(94, 94, 98),
      RGB(255, 255, 255),
      RGB(165, 165, 170), RGB(210, 210, 214), RGB(240, 240, 242), RGB(18, 18, 20),
      RGB(18, 18, 20),
      RGB(220, 220, 224), RGB(248, 248, 250), RGB(24, 24, 26) },
    // Midnight: deep blue gradient, blue accent.
    { RGB(28, 38, 66), RGB(10, 16, 32),
      RGB(64, 84, 128), RGB(110, 140, 190), RGB(88, 110, 160),
      RGB(240, 244, 252), RGB(165, 178, 205), RGB(130, 145, 175), RGB(205, 218, 240),
      RGB(96, 165, 250),
      RGB(45, 58, 88),
      RGB(38, 52, 84), RGB(58, 74, 112), RGB(84, 102, 142),
      RGB(240, 244, 252),
      RGB(165, 178, 205), RGB(205, 218, 240), RGB(240, 244, 252), RGB(12, 18, 34),
      RGB(10, 16, 32),
      RGB(200, 212, 235), RGB(240, 244, 252), RGB(10, 16, 32) },
    // Graphite: lighter grey gradient, still dark, light-grey accent.
    { RGB(72, 72, 78), RGB(38, 38, 42),
      RGB(110, 110, 116), RGB(150, 150, 158), RGB(130, 130, 138),
      RGB(250, 250, 252), RGB(200, 200, 205), RGB(170, 170, 176), RGB(225, 225, 230),
      RGB(230, 230, 235),
      RGB(88, 88, 94),
      RGB(84, 84, 90), RGB(106, 106, 112), RGB(134, 134, 142),
      RGB(250, 250, 252),
      RGB(200, 200, 205), RGB(225, 225, 230), RGB(250, 250, 252), RGB(30, 30, 34),
      RGB(30, 30, 34),
      RGB(235, 235, 240), RGB(250, 250, 252), RGB(38, 38, 42) },
};

static int g_theme = 0; // 0 = Onyx (default), 1 = Midnight, 2 = Graphite

static const Theme& CurrentTheme()
{
    if (g_theme < 0 || g_theme > 2) return kThemes[0];
    return kThemes[g_theme];
}

static const char* ThemeKey(int t)
{
    if (t == 1) return "midnight";
    if (t == 2) return "graphite";
    return "onyx";
}

// ---------------------------------------------------------------------------
// v5-settings: saved settings file + auto-start registry value.
// File: %APPDATA%\OnyxNotch\settings.ini, plain key=value, no
// JSON library. Auto-start: HKCU\Software\Microsoft\Windows\
// CurrentVersion\Run, value name "OnyxNotch", REG_SZ quoted exe
// path. Registry is the source of truth for auto-start; the file
// is the source of truth for everything else, including the theme.
// All failures here are silent by design: a settings problem must
// never stop the app starting, break a toggle, or block a quit.
// ---------------------------------------------------------------------------
static bool GetOnyxSettingsDir(wchar_t* out, size_t count)
{
    wchar_t appData[MAX_PATH] = { 0 };
    DWORD len = GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return false;
    if (wcscpy_s(out, count, appData) != 0) return false;
    if (wcscat_s(out, count, L"\\OnyxNotch") != 0) return false;
    return true;
}

static bool GetSettingsPath(wchar_t* out, size_t count)
{
    if (!GetOnyxSettingsDir(out, count)) return false;
    if (wcscat_s(out, count, L"\\settings.ini") != 0) return false;
    return true;
}

static bool StrEqualsIgnoreCase(const char* a, const char* b)
{
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca + 32);
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb + 32);
        if (ca != cb) return false;
        a++; b++;
    }
    return *a == *b;
}

// Parses a boolean value. Returns false (caller keeps the old
// value) for anything that is not recognisably true/false, so a
// corrupt value cannot flip a toggle to a surprising state.
static bool ParseBoolValue(const char* v, bool& out)
{
    if (StrEqualsIgnoreCase(v, "1") || StrEqualsIgnoreCase(v, "true") ||
        StrEqualsIgnoreCase(v, "yes") || StrEqualsIgnoreCase(v, "on")) {
        out = true; return true;
    }
    if (StrEqualsIgnoreCase(v, "0") || StrEqualsIgnoreCase(v, "false") ||
        StrEqualsIgnoreCase(v, "no") || StrEqualsIgnoreCase(v, "off")) {
        out = false; return true;
    }
    return false;
}

static void LoadSettings()
{
    // Globals already hold the old defaults; this only overrides
    // them for keys that parse cleanly. Missing file, unreadable
    // file, unknown keys, malformed lines and out-of-range values
    // all leave the corresponding default untouched.
    wchar_t path[MAX_PATH] = { 0 };
    if (!GetSettingsPath(path, MAX_PATH)) return;
    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return; // missing = defaults
    char buf[4096];
    DWORD read = 0;
    BOOL ok = ReadFile(h, buf, (DWORD)sizeof(buf) - 1, &read, NULL);
    CloseHandle(h);
    if (!ok) return;
    buf[read] = '\0';
    char* p = buf;
    while (*p) {
        char* lineEnd = p;
        while (*lineEnd && *lineEnd != '\n') lineEnd++;
        char saved = *lineEnd;
        *lineEnd = '\0';
        // Trim leading whitespace; skip blank / comment lines.
        char* line = p;
        while (*line == ' ' || *line == '\t' || *line == '\r') line++;
        if (*line && *line != ';' && *line != '#' && *line != '[') {
            char* eq = line;
            while (*eq && *eq != '=') eq++;
            if (*eq == '=') {
                *eq = '\0';
                char* key = line;
                // Trim trailing whitespace from the key.
                char* kend = key + strlen(key);
                while (kend > key && (kend[-1] == ' ' || kend[-1] == '\t' || kend[-1] == '\r')) { kend--; *kend = '\0'; }
                char* val = eq + 1;
                while (*val == ' ' || *val == '\t') val++;
                char* vend = val + strlen(val);
                while (vend > val && (vend[-1] == ' ' || vend[-1] == '\t' || vend[-1] == '\r')) { vend--; *vend = '\0'; }
                bool b = false;
                if (StrEqualsIgnoreCase(key, "size")) {
                    if (strcmp(val, "0") == 0) g_size = 0;
                    else if (strcmp(val, "1") == 0) g_size = 1;
                    else if (strcmp(val, "2") == 0) g_size = 2;
                    // Any other value is ignored (default kept).
                } else if (StrEqualsIgnoreCase(key, "hide_on_fullscreen")) {
                    if (ParseBoolValue(val, b)) g_hideOnFullscreen = b;
                } else if (StrEqualsIgnoreCase(key, "media")) {
                    if (ParseBoolValue(val, b)) g_mediaEnabled = b;
                } else if (StrEqualsIgnoreCase(key, "widget_stats")) {
                    if (ParseBoolValue(val, b)) g_widgetStats = b;
                } else if (StrEqualsIgnoreCase(key, "widget_calendar")) {
                    if (ParseBoolValue(val, b)) g_widgetCalendar = b;
                } else if (StrEqualsIgnoreCase(key, "animations")) {
                    if (ParseBoolValue(val, b)) g_animations = b;
                } else if (StrEqualsIgnoreCase(key, "theme")) {
                    // v5-themes: unknown/corrupt values are ignored,
                    // so the current value (Onyx by default) stands.
                    if (StrEqualsIgnoreCase(val, "onyx")) g_theme = 0;
                    else if (StrEqualsIgnoreCase(val, "midnight")) g_theme = 1;
                    else if (StrEqualsIgnoreCase(val, "graphite")) g_theme = 2;
                }
                // Unknown keys are ignored.
            }
        }
        if (saved == '\0') break;
        p = lineEnd + 1;
    }
}

static void SaveSettings()
{
    // Full rewrite of the known keys only. That is the documented
    // trade-off: any unrelated lines a user hand-added to the
    // file are NOT preserved across a save. The file this app
    // writes is tiny (under 300 bytes), so the rewrite is atomic
    // enough in practice; a failed save is ignored silently.
    wchar_t dir[MAX_PATH] = { 0 };
    wchar_t path[MAX_PATH] = { 0 };
    if (!GetOnyxSettingsDir(dir, MAX_PATH)) return;
    if (!GetSettingsPath(path, MAX_PATH)) return;
    CreateDirectoryW(dir, NULL); // already-exists is fine
    HANDLE h = CreateFileW(path, GENERIC_WRITE, 0, NULL,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    char buf[512];
    int len = sprintf_s(buf, sizeof(buf),
        "; OnyxNotch settings - rewritten on every change and on clean quit.\r\n"
        "; Unknown keys are ignored on load; values are 0 = off, 1 = on.\r\n"
        "size=%d\r\n"
        "hide_on_fullscreen=%d\r\n"
        "media=%d\r\n"
        "widget_stats=%d\r\n"
        "widget_calendar=%d\r\n"
        "animations=%d\r\n"
        "theme=%s\r\n",
        g_size,
        g_hideOnFullscreen ? 1 : 0,
        g_mediaEnabled ? 1 : 0,
        g_widgetStats ? 1 : 0,
        g_widgetCalendar ? 1 : 0,
        g_animations ? 1 : 0,
        ThemeKey(g_theme));
    if (len > 0) {
        DWORD written = 0;
        WriteFile(h, buf, (DWORD)len, &written, NULL);
    }
    CloseHandle(h);
}

static const wchar_t kRunKeyPath[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t kRunValueName[] = L"OnyxNotch";

static void LoadAutoStart()
{
    // Registry is the source of truth: a present, non-empty REG_SZ
    // value means ON, whatever path it holds. If the .exe has
    // moved since the value was written, toggling off and on
    // again rewrites it to the current path (see SetAutoStart).
    g_startWithWindows = false;
    HKEY key = NULL;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) return;
    wchar_t value[MAX_PATH] = { 0 };
    DWORD size = sizeof(value);
    DWORD type = 0;
    LONG rc = RegQueryValueExW(key, kRunValueName, NULL, &type, (LPBYTE)value, &size);
    RegCloseKey(key);
    if (rc == ERROR_SUCCESS && type == REG_SZ && size > 0 && value[0] != L'\0') {
        g_startWithWindows = true;
    }
}

static bool SetAutoStart(bool on)
{
    if (!on) {
        HKEY key = NULL;
        LONG rc = RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_SET_VALUE, &key);
        if (rc == ERROR_FILE_NOT_FOUND) return true; // already gone = desired state
        if (rc != ERROR_SUCCESS) return false;
        rc = RegDeleteValueW(key, kRunValueName);
        RegCloseKey(key);
        if (rc == ERROR_SUCCESS || rc == ERROR_FILE_NOT_FOUND) return true;
        return false;
    }
    wchar_t exePath[MAX_PATH] = { 0 };
    DWORD len = GetModuleFileNameW(NULL, exePath, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return false;
    wchar_t quoted[MAX_PATH + 2] = { 0 };
    if (wcscpy_s(quoted, MAX_PATH + 2, L"\"") != 0) return false;
    if (wcscat_s(quoted, MAX_PATH + 2, exePath) != 0) return false;
    if (wcscat_s(quoted, MAX_PATH + 2, L"\"") != 0) return false;
    HKEY key = NULL;
    LONG rc = RegCreateKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, NULL, 0,
                              KEY_SET_VALUE, NULL, &key, NULL);
    if (rc != ERROR_SUCCESS) return false;
    DWORD bytes = (DWORD)((wcslen(quoted) + 1) * sizeof(wchar_t));
    rc = RegSetValueExW(key, kRunValueName, 0, REG_SZ, (const BYTE*)quoted, bytes);
    RegCloseKey(key);
    return rc == ERROR_SUCCESS;
}

// v5-SMTC cached state. Declared here (before ComputeLayout,
// which reads hasTrack/artist for heights) and filled only by
// RefreshSmtc on the 1-second timer; painting only reads the cache.
// Status: 0 = unknown/none, 1 = playing, 2 = paused.
static bool g_smtcInitTried = false;
static bool g_smtcInitOk = false;
static bool g_smtcHasTrack = false;
static wchar_t g_smtcTitle[256] = { 0 };
static wchar_t g_smtcArtist[256] = { 0 };
static int g_smtcStatus = 0;
static int g_smtcLayoutKey = 0;

static HWND g_managerHwnd = NULL;
static NOTIFYICONDATAW g_nid = { 0 };
static bool g_rebuilding = false;
static ULONGLONG g_lastRebuildTick = 0;

// Stats sampling state (only used while the stats widget is on).
static bool g_havePrevTimes = false;
static FILETIME g_prevIdle = { 0, 0 }, g_prevKernel = { 0, 0 }, g_prevUser = { 0, 0 };
static int g_cpuPercent = 0;
static int g_memPercent = 0;
static double g_memUsedGB = 0.0, g_memTotalGB = 0.0;

// ---------------------------------------------------------------------------
// Per-monitor notch state.
// ---------------------------------------------------------------------------
struct Notch {
    HWND hwnd = NULL;
    HMONITOR hMonitor = NULL;
    RECT monitorRect = { 0, 0, 0, 0 }; // full monitor rect (fullscreen test)
    RECT workRect = { 0, 0, 0, 0 };    // work area (notch position)
    int dpi = 96;
    HFONT hFontClock = NULL;
    HFONT hFontBig = NULL;
    HFONT hFontSmall = NULL;
    HFONT hFontTiny = NULL;
    bool expanded = false;
    bool pinned = false;
    bool trackingLeave = false;
    bool hiddenByFullscreen = false;
    // v3 animation / hover state. paintExpanded is what PaintNotch
    // draws: it becomes true as soon as an expand starts, and only
    // becomes false when a collapse animation finishes, so content
    // does not pop out mid-collapse. hoverBtn/pressedBtn are -1 for
    // none, else 0 = Prev, 1 = Play/Pause, 2 = Next.
    bool paintExpanded = false;
    bool hoverNotch = false;
    int hoverBtn = -1;
    int pressedBtn = -1;
    // v4 reorg: settings-gear hover/pressed state (gear only).
    bool hoverGear = false;
    bool pressedGear = false;
    // v4 tabs: per-notch selected tab (0 = HOME, 1 = MEDIA,
    // 2 = SYSTEM, 3 = CALENDAR) and hovered tab (-1 = none).
    int activeTab = 0;
    int hoverTab = -1;
    bool animActive = false;
    ULONGLONG animStartTick = 0;
    int animFromW = 0, animFromH = 0;
    int animToW = 0, animToH = 0;
};

static std::vector<Notch*> g_notches;

static Notch* FindNotch(HWND hwnd)
{
    for (size_t i = 0; i < g_notches.size(); i++) {
        if (g_notches[i]->hwnd == hwnd) return g_notches[i];
    }
    return NULL;
}

static int Scale(int value, int dpi)
{
    return MulDiv(value, dpi, 96);
}

static int CollapsedWidth(int dpi)
{
    int base = (g_size == 0) ? 200 : (g_size == 2) ? 300 : 240;
    return Scale(base, dpi);
}

// ---------------------------------------------------------------------------
// Expanded layout. One shared computation drives both painting and the
// media-button hit test, so the clickable buttons match what is drawn.
// ---------------------------------------------------------------------------
struct Layout {
    int totalH = 0;
    int gearCX = 0, gearCY = 0, gearR = 0;
    // v4 tabs: tab bar below the header divider. showTabs is
    // false when no feature is enabled (header-only view).
    bool showTabs = false;
    int tabY = 0, tabH = 0, tabCount = 0;
    int tabIds[4] = { 0, 0, 0, 0 };
    RECT tabRects[4] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
    int bodyY = 0;
    // Detailed sections (full width on their own tab).
    int mediaX = 0, mediaW = 0;
    int mediaLabelY = 0;
    // v5-SMTC: title/artist lines between the MEDIA label and the
    // buttons. -1 = line not shown (no track, or no artist).
    int mediaTitleY = -1, mediaArtistY = -1;
    int mediaButtonY = 0;
    int mediaButtonH = 0;
    RECT btnPrev = { 0, 0, 0, 0 };
    RECT btnPlay = { 0, 0, 0, 0 };
    RECT btnNext = { 0, 0, 0, 0 };
    int statsX = 0, statsW = 0;
    int statsLabelY = 0;
    int cpuLabelY = 0, cpuBarY = 0;
    int ramLabelY = 0, ramBarY = 0;
    int calendarY = 0;
    int calendarH = 0;
    // HOME summaries (compact, stacked).
    // v5-SMTC: one compact title line above the HOME media buttons.
    // -1 = not shown (no track).
    int homeMediaTitleY = -1;
    int homeMediaButtonY = 0, homeMediaButtonH = 0;
    RECT homeBtnPrev = { 0, 0, 0, 0 };
    RECT homeBtnPlay = { 0, 0, 0, 0 };
    RECT homeBtnNext = { 0, 0, 0, 0 };
    int homeStatsY = 0;
    int homeWeekY = 0;
};

static int DaysInMonth(int year, int month)
{
    static const int kDays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2) {
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return leap ? 29 : 28;
    }
    return kDays[(month - 1) % 12];
}

static const int kTabHome = 0;
static const int kTabMedia = 1;
static const int kTabSystem = 2;
static const int kTabCalendar = 3;

// v4 tabs: the tab actually shown. A selected tab whose feature
// was toggled off falls back to HOME, so layout, painting and
// hit tests can never show a dead tab.
static int EffectiveTab(Notch* n)
{
    if (!g_mediaEnabled && !g_widgetStats && !g_widgetCalendar) return kTabHome;
    int t = n->activeTab;
    if (t == kTabMedia && !g_mediaEnabled) return kTabHome;
    if (t == kTabSystem && !g_widgetStats) return kTabHome;
    if (t == kTabCalendar && !g_widgetCalendar) return kTabHome;
    return t;
}

static Layout ComputeLayout(Notch* n, int clientW)
{
    Layout L;
    int dpi = n->dpi;
    // Header geometry is unchanged: gear in the header's
    // top-right corner, divider at y 118, body at y 126 when
    // there is no tab bar. With a tab bar, the body moves
    // below it (see below).
    L.gearR = Scale(11, dpi);
    L.gearCX = clientW - Scale(16, dpi) - L.gearR;
    L.gearCY = Scale(24, dpi);
    int margin = Scale(16, dpi);
    int bodyW = clientW - 2 * margin;
    if (bodyW < 40) bodyW = 40;
    bool anyFeature = g_mediaEnabled || g_widgetStats || g_widgetCalendar;
    if (!anyFeature) {
        L.showTabs = false;
        L.bodyY = Scale(126, dpi);
        L.totalH = L.bodyY + Scale(12, dpi);
        return L;
    }
    // Tab bar: below the header divider (y 118), above the body.
    // Tabs are evenly spaced across the body width, in order:
    // HOME first, then each enabled feature.
    L.showTabs = true;
    L.tabY = Scale(126, dpi);
    L.tabH = Scale(24, dpi);
    L.bodyY = L.tabY + L.tabH + Scale(8, dpi);
    int ids[4] = { kTabHome, kTabMedia, kTabSystem, kTabCalendar };
    bool tabOn[4] = { true, g_mediaEnabled, g_widgetStats, g_widgetCalendar };
    int count = 0;
    for (int i = 0; i < 4; i++) if (tabOn[i]) count++;
    L.tabCount = count;
    int segW = bodyW / count;
    int ti = 0;
    for (int i = 0; i < 4; i++) if (tabOn[i]) {
        L.tabIds[ti] = ids[i];
        L.tabRects[ti] = { margin + ti * segW, L.tabY, margin + (ti + 1) * segW, L.tabY + L.tabH };
        ti++;
    }
    int effTab = EffectiveTab(n);
    int btnGap = Scale(8, dpi);
    if (effTab == kTabHome) {
        // HOME: compact summaries, stacked, each section only
        // as tall as it needs (media buttons 20 px, stats line
        // 20 px, week row 20 px, 8 px gaps between sections).
        int y = L.bodyY;
        int lastBottom = y;
        if (g_mediaEnabled) {
            // v5-SMTC: compact title line above the mini buttons,
            // only when a track is known. It costs one 20 px row
            // plus a 4 px gap; with no track the layout is exactly
            // the v4 HOME layout.
            if (g_smtcHasTrack) {
                L.homeMediaTitleY = y;
                y += Scale(20, dpi) + Scale(4, dpi);
            }
            L.homeMediaButtonY = y;
            L.homeMediaButtonH = Scale(20, dpi);
            int bw = (bodyW - 2 * btnGap) / 3;
            if (bw < 20) bw = 20;
            L.homeBtnPrev = { margin, y, margin + bw, y + L.homeMediaButtonH };
            L.homeBtnPlay = { margin + bw + btnGap, y, margin + 2 * bw + btnGap, y + L.homeMediaButtonH };
            L.homeBtnNext = { margin + 2 * bw + 2 * btnGap, y, margin + 3 * bw + 2 * btnGap, y + L.homeMediaButtonH };
            lastBottom = y + L.homeMediaButtonH;
            y = lastBottom + Scale(8, dpi);
        }
        if (g_widgetStats) {
            L.homeStatsY = y;
            lastBottom = y + Scale(20, dpi);
            y = lastBottom + Scale(8, dpi);
        }
        if (g_widgetCalendar) {
            L.homeWeekY = y;
            lastBottom = y + Scale(20, dpi);
        }
        L.totalH = lastBottom + Scale(12, dpi);
        return L;
    }
    if (effTab == kTabMedia) {
        // Full media section, full body width (as before the
        // reorganization's two-column row, but behind its tab).
        L.mediaX = margin;
        L.mediaW = bodyW;
        L.mediaLabelY = L.bodyY;
        // v5-SMTC: title line, then artist line (only if an
        // artist string exists), then the buttons. Each line is
        // 20 px plus a 4 px gap before the buttons; with no track
        // the buttons sit at the old v4 offset, unchanged.
        if (g_smtcHasTrack) {
            int y = L.bodyY + Scale(20, dpi);
            L.mediaTitleY = y;
            y += Scale(20, dpi);
            if (g_smtcArtist[0] != L'\0') {
                L.mediaArtistY = y;
                y += Scale(20, dpi);
            }
            L.mediaButtonY = y + Scale(4, dpi);
        } else {
            L.mediaButtonY = L.bodyY + Scale(22, dpi);
        }
        L.mediaButtonH = Scale(28, dpi);
        int bw = (bodyW - 2 * btnGap) / 3;
        if (bw < 20) bw = 20;
        L.btnPrev = { margin, L.mediaButtonY, margin + bw, L.mediaButtonY + L.mediaButtonH };
        L.btnPlay = { margin + bw + btnGap, L.mediaButtonY, margin + 2 * bw + btnGap, L.mediaButtonY + L.mediaButtonH };
        L.btnNext = { margin + 2 * bw + 2 * btnGap, L.mediaButtonY, margin + 3 * bw + 2 * btnGap, L.mediaButtonY + L.mediaButtonH };
        L.totalH = L.mediaButtonY + L.mediaButtonH + Scale(12, dpi);
        return L;
    }
    if (effTab == kTabSystem) {
        // Full stats section, full body width: header, CPU row
        // + bar, RAM row + bar (same geometry as the reorg).
        L.statsX = margin;
        L.statsW = bodyW;
        L.statsLabelY = L.bodyY;
        L.cpuLabelY = L.bodyY + Scale(20, dpi);
        L.cpuBarY = L.cpuLabelY + Scale(18, dpi);
        L.ramLabelY = L.cpuBarY + Scale(10, dpi);
        L.ramBarY = L.ramLabelY + Scale(18, dpi);
        L.totalH = L.ramBarY + Scale(6, dpi) + Scale(12, dpi);
        return L;
    }
    // Full calendar section, full body width: month header +
    // weekday row + all weeks (same geometry as the reorg).
    SYSTEMTIME st; GetLocalTime(&st);
    int firstWeekday = (int)((st.wDayOfWeek - (st.wDay - 1) % 7) % 7);
    if (firstWeekday < 0) firstWeekday += 7;
    int cells = firstWeekday + DaysInMonth(st.wYear, st.wMonth);
    int rows = (cells + 6) / 7;
    L.calendarY = L.bodyY;
    L.calendarH = Scale(24 + 20 + rows * 20, dpi);
    L.totalH = L.calendarY + L.calendarH + Scale(12, dpi);
    return L;
}

static void ApplyRegion(HWND hwnd, int w, int h, bool expanded)
{
    HRGN rgn = CreateRoundRectRgn(0, 0, w, h, 28, 28);
    if (!expanded) {
        HRGN top = CreateRectRgn(0, 0, w, 18);
        CombineRgn(rgn, rgn, top, RGN_OR);
        DeleteObject(top);
    }
    SetWindowRgn(hwnd, rgn, TRUE); // SetWindowRgn takes ownership of rgn.
}

static void ApplyNotchSizeInstant(Notch* n, bool expanded)
{
    if (!n || !n->hwnd) return;
    int dpi = n->dpi;
    int w = expanded ? Scale(340, dpi) : CollapsedWidth(dpi);
    int h;
    if (expanded) {
        Layout L = ComputeLayout(n, w);
        h = L.totalH;
    } else {
        h = Scale(34, dpi);
    }
    int workW = n->workRect.right - n->workRect.left;
    int x = n->workRect.left + (workW - w) / 2;
    if (x < n->workRect.left) x = n->workRect.left;
    int y = n->workRect.top;
    SetWindowPos(n->hwnd, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    ApplyRegion(n->hwnd, w, h, expanded);
}

// v3 animation: 220 ms, ease-out cubic, ~16 ms frames. The timer is
// per-notch, started only for a size change and killed on finish,
// so an idle notch has no animation timer at all.
static const int kAnimDurationMs = 220;
static const int kAnimIntervalMs = 16;

static void TargetSizeFor(Notch* n, bool expanded, int& outW, int& outH)
{
    int dpi = n->dpi;
    outW = expanded ? Scale(340, dpi) : CollapsedWidth(dpi);
    if (expanded) {
        Layout L = ComputeLayout(n, outW);
        outH = L.totalH;
    } else {
        outH = Scale(34, dpi);
    }
}

static void StopAnimation(Notch* n)
{
    if (!n) return;
    if (n->hwnd) KillTimer(n->hwnd, ID_TIMER_ANIM);
    n->animActive = false;
}

// Animate (or instantly apply, when animations are off) to the size
// for targetExpanded. n->expanded must already be targetExpanded
// when this is called from SetNotchExpanded; for size-only changes
// (tray Size, widget toggles, DPI) it is already correct too.
static void ApplyTargetSize(Notch* n, bool targetExpanded)
{
    if (!n || !n->hwnd) return;
    int toW, toH;
    TargetSizeFor(n, targetExpanded, toW, toH);
    if (!g_animations) {
        StopAnimation(n);
        n->paintExpanded = targetExpanded;
        ApplyNotchSizeInstant(n, targetExpanded);
        InvalidateRect(n->hwnd, NULL, TRUE);
        return;
    }
    RECT cur = { 0, 0, 0, 0 };
    GetWindowRect(n->hwnd, &cur);
    int fromW = cur.right - cur.left;
    int fromH = cur.bottom - cur.top;
    if (fromW == toW && fromH == toH) {
        StopAnimation(n);
        n->paintExpanded = targetExpanded;
        ApplyNotchSizeInstant(n, targetExpanded);
        InvalidateRect(n->hwnd, NULL, TRUE);
        return;
    }
    if (targetExpanded) n->paintExpanded = true; // collapsing keeps
    // paintExpanded true until the collapse finishes (see Step).
    n->animFromW = fromW; n->animFromH = fromH;
    n->animToW = toW; n->animToH = toH;
    n->animStartTick = GetTickCount64();
    n->animActive = true;
    SetTimer(n->hwnd, ID_TIMER_ANIM, kAnimIntervalMs, NULL);
}

static void StepAnimation(Notch* n)
{
    if (!n || !n->hwnd || !n->animActive) return;
    ULONGLONG elapsed = GetTickCount64() - n->animStartTick;
    if (elapsed >= (ULONGLONG)kAnimDurationMs) {
        // Finish exactly on target: snap, then switch painting to
        // the final state (this is where a collapse stops drawing
        // expanded content).
        StopAnimation(n);
        n->paintExpanded = n->expanded;
        ApplyNotchSizeInstant(n, n->expanded);
        InvalidateRect(n->hwnd, NULL, TRUE);
        return;
    }
    double t = (double)elapsed / (double)kAnimDurationMs;
    double e = 1.0 - (1.0 - t) * (1.0 - t) * (1.0 - t); // ease-out cubic
    int w = (int)(n->animFromW + (n->animToW - n->animFromW) * e + 0.5);
    int h = (int)(n->animFromH + (n->animToH - n->animFromH) * e + 0.5);
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    int workW = n->workRect.right - n->workRect.left;
    int x = n->workRect.left + (workW - w) / 2;
    if (x < n->workRect.left) x = n->workRect.left;
    SetWindowPos(n->hwnd, HWND_TOPMOST, x, n->workRect.top, w, h,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    // Region follows the current height, so the shape stays rounded
    // while tall and matches the v2 collapsed shape once short.
    ApplyRegion(n->hwnd, w, h, h > Scale(34, n->dpi) + 2);
    InvalidateRect(n->hwnd, NULL, FALSE);
    UpdateWindow(n->hwnd);
}

static void SetNotchExpanded(Notch* n, bool expanded)
{
    if (!n) return;
    // Already at target and not animating: nothing to do. Already
    // animating to this target: let it finish there (pinning
    // mid-expand settles at expanded; hovering away mid-expand
    // is handled by the caller changing the target instead).
    if (n->expanded == expanded && !n->animActive) return;
    if (n->expanded == expanded && n->animActive) return;
    n->expanded = expanded;
    // v4 tabs: expanding always starts on HOME (a collapse
    // resets the tab on the next expand, never mid-collapse).
    if (expanded) n->activeTab = kTabHome;
    ApplyTargetSize(n, expanded);
}

static void RecreateFonts(Notch* n)
{
    if (n->hFontClock) DeleteObject(n->hFontClock);
    if (n->hFontBig) DeleteObject(n->hFontBig);
    if (n->hFontSmall) DeleteObject(n->hFontSmall);
    if (n->hFontTiny) DeleteObject(n->hFontTiny);
    int dpi = n->dpi;
    // v4 typography: Segoe UI throughout, lighter weights for a
    // premium feel. The big clock drops from semibold to light;
    // the collapsed clock drops from medium to normal. Sizes are
    // unchanged, so layout positions do not move for weight alone.
    n->hFontClock = CreateFontW(Scale(20, dpi), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    n->hFontBig = CreateFontW(Scale(36, dpi), 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    n->hFontSmall = CreateFontW(Scale(17, dpi), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    n->hFontTiny = CreateFontW(Scale(14, dpi), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

// ---------------------------------------------------------------------------
// Tray icon (owned by the hidden manager window, so it survives rebuilds).
// ---------------------------------------------------------------------------
static HICON LoadOnyxIcon(HINSTANCE hInst)
{
    HICON icon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ONYXNOTCH_ICON));
    if (!icon) icon = LoadIcon(NULL, IDI_APPLICATION);
    return icon;
}

static void AddTrayIcon(HWND hwnd, HINSTANCE hInst)
{
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = hwnd;
    g_nid.uID = ID_TRAY_ICON;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAY;
    g_nid.hIcon = LoadOnyxIcon(hInst);
    wcscpy_s(g_nid.szTip, L"OnyxNotch");
    Shell_NotifyIconW(NIM_ADD, &g_nid);
}

static void AppendCheck(HMENU menu, UINT id, const wchar_t* text, bool checked)
{
    AppendMenuW(menu, MF_STRING | (checked ? MF_CHECKED : MF_UNCHECKED), id, text);
}

// v4 reorg: the tray menu is shared by two entry points — the
// tray icon (at the cursor) and the notch gear button (at the
// gear's screen position). Commands always go to the manager
// window, which owns the tray icon and handles WM_COMMAND, even
// when the menu was opened from a notch window.
static void ShowTrayMenuAt(POINT pt, UINT alignFlags)
{
    HMENU menu = CreatePopupMenu();
    AppendCheck(menu, ID_TRAY_SIZE_S, L"Size: Small", g_size == 0);
    AppendCheck(menu, ID_TRAY_SIZE_M, L"Size: Medium", g_size == 1);
    AppendCheck(menu, ID_TRAY_SIZE_L, L"Size: Large", g_size == 2);
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendCheck(menu, ID_TRAY_HIDE_FULLSCREEN, L"Hide on fullscreen", g_hideOnFullscreen);
    AppendCheck(menu, ID_TRAY_MEDIA, L"Media controls", g_mediaEnabled);
    AppendCheck(menu, ID_TRAY_WIDGET_STATS, L"Widget: System stats", g_widgetStats);
    AppendCheck(menu, ID_TRAY_WIDGET_CALENDAR, L"Widget: Calendar", g_widgetCalendar);
    AppendCheck(menu, ID_TRAY_ANIMATIONS, L"Animations", g_animations);
    AppendCheck(menu, ID_TRAY_AUTOSTART, L"Start with Windows", g_startWithWindows);
    // v5-themes: submenu, so it appears in both the tray menu and
    // the gear menu (the gear opens this same menu). Tick marks
    // the current theme; switching is handled in ManagerWndProc.
    {
        HMENU themeMenu = CreatePopupMenu();
        AppendCheck(themeMenu, ID_TRAY_THEME_ONYX, L"Onyx", g_theme == 0);
        AppendCheck(themeMenu, ID_TRAY_THEME_MIDNIGHT, L"Midnight", g_theme == 1);
        AppendCheck(themeMenu, ID_TRAY_THEME_GRAPHITE, L"Graphite", g_theme == 2);
        AppendMenuW(menu, MF_POPUP, (UINT_PTR)themeMenu, L"Themes");
    }
    if (g_mediaEnabled) {
        AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(menu, MF_STRING, ID_TRAY_MEDIA_PREV, L"Media: Previous track");
        AppendMenuW(menu, MF_STRING, ID_TRAY_MEDIA_PLAYPAUSE, L"Media: Play / Pause");
        AppendMenuW(menu, MF_STRING, ID_TRAY_MEDIA_NEXT, L"Media: Next track");
    }
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, ID_TRAY_ABOUT, L"About OnyxNotch");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, ID_TRAY_QUIT, L"Quit");
    HWND owner = g_managerHwnd ? g_managerHwnd : GetDesktopWindow();
    SetForegroundWindow(owner);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | alignFlags, pt.x, pt.y, 0, owner, NULL);
    PostMessage(owner, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

static void ShowTrayMenu(HWND /*hwnd*/)
{
    POINT pt; GetCursorPos(&pt);
    ShowTrayMenuAt(pt, 0);
}

// ---------------------------------------------------------------------------
// Formatting helpers (clock, date, battery, stats, calendar names).
// ---------------------------------------------------------------------------
static void FormatClock(wchar_t* out, size_t count)
{
    SYSTEMTIME st; GetLocalTime(&st);
    swprintf(out, count, L"%02d:%02d", st.wHour, st.wMinute);
}

static void FormatDate(wchar_t* out, size_t count)
{
    static const wchar_t* kDays[] = { L"Sunday", L"Monday", L"Tuesday", L"Wednesday",
        L"Thursday", L"Friday", L"Saturday" };
    static const wchar_t* kMonths[] = { L"January", L"February", L"March", L"April", L"May", L"June",
        L"July", L"August", L"September", L"October", L"November", L"December" };
    SYSTEMTIME st; GetLocalTime(&st);
    swprintf(out, count, L"%s, %d %s %d", kDays[st.wDayOfWeek % 7], st.wDay,
             kMonths[(st.wMonth - 1) % 12], st.wYear);
}

static void FormatBattery(wchar_t* out, size_t count)
{
    SYSTEM_POWER_STATUS sps;
    if (!GetSystemPowerStatus(&sps) || sps.BatteryLifePercent == 255) {
        wcscpy_s(out, count, L"Battery: --"); return;
    }
    const wchar_t* state = L"";
    if (sps.ACLineStatus == 1) state = (sps.BatteryFlag & 8) ? L"Charging" : L"On AC power";
    else state = L"Discharging";
    swprintf(out, count, L"Battery: %d%%  %s", (int)sps.BatteryLifePercent, state);
}

static void DrawCenteredText(HDC hdc, HFONT font, const wchar_t* text,
                             int x, int y, int w, int h, COLORREF color)
{
    HFONT old = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    RECT rc = { x, y, x + w, y + h };
    DrawTextW(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, old);
}

// v4 reorg: left-aligned section headers (MEDIA / SYSTEM / CALENDAR).
static void DrawLeftText(HDC hdc, HFONT font, const wchar_t* text,
                         int x, int y, int w, int h, COLORREF color)
{
    HFONT old = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    RECT rc = { x, y, x + w, y + h };
    DrawTextW(hdc, text, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, old);
}

// v5-SMTC: same as DrawLeftText, but text longer than the given
// width is truncated with an ellipsis by GDI (DT_END_ELLIPSIS)
// instead of being clipped silently. The width passed in is the
// section width, so truncation is by pixels at the current font
// and DPI, not by a character count; the stored strings are
// additionally capped at 255 characters (see RefreshSmtc).
static void DrawLeftEllipsisText(HDC hdc, HFONT font, const wchar_t* text,
                                 int x, int y, int w, int h, COLORREF color)
{
    HFONT old = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    RECT rc = { x, y, x + w, y + h };
    DrawTextW(hdc, text, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    SelectObject(hdc, old);
}

// ---------------------------------------------------------------------------
// Media keys (plain Win32, no SMTC/WinRT dependency). The active player
// receives the key, exactly as if a media key on the keyboard was pressed.
// ---------------------------------------------------------------------------
static void SendMediaKey(WORD vk)
{
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk;
    SendInput(1, &input, sizeof(input));
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(input));
}

// ---------------------------------------------------------------------------
// v5-SMTC: read-only track title/artist + playback status from the
// local System Media Transport Controls session. Called only from
// the existing 1-second manager timer, and only while Media
// controls are on (off = never called, zero cost). The last good
// title/artist/status are cached in globals; painting only reads
// the cache, never calls WinRT. Every WinRT call is inside
// try/catch, so a failure degrades to "no track" instead of a
// crash. Media keys above stay the only control path.
// Status values: 0 = unknown/none (old combined glyph),
// 1 = playing (show pause bars), 2 = paused (show play triangle).
// Other statuses (Stopped, Changing, Closed, Opened) stay 0, so
// the glyph never claims a state SMTC did not report clearly.
// (The cache globals live with the other settings above, because
// ComputeLayout reads them; see there.)
// Layout key: 0 = no track lines, 1 = title only, 2 = title+artist.
// The manager timer compares this after each refresh and resizes
// expanded notches only when the key changes, so a track change
// that keeps the same line count is just a repaint.

static int SmtcLayoutKey()
{
    if (!g_smtcHasTrack) return 0;
    return (g_smtcArtist[0] != L'\0') ? 2 : 1;
}

static void RefreshSmtc()
{
    if (!g_mediaEnabled) {
        g_smtcHasTrack = false;
        g_smtcTitle[0] = L'\0';
        g_smtcArtist[0] = L'\0';
        g_smtcStatus = 0;
        return;
    }
    try {
        // MTA on purpose: the timer thread is the UI/message thread,
        // and blocking .get() calls below must not wait on STA
        // message pumping to complete. No other COM use exists in
        // this app, so this is the only apartment initialisation.
        if (!g_smtcInitTried) {
            g_smtcInitTried = true;
            try {
                winrt::init_apartment(winrt::apartment_type::multi_threaded);
                g_smtcInitOk = true;
            } catch (...) {
                g_smtcInitOk = false;
            }
        }
        if (!g_smtcInitOk) {
            g_smtcHasTrack = false;
            g_smtcStatus = 0;
            return;
        }
        namespace smtc = winrt::Windows::Media::Control;
        auto manager = smtc::GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        if (!manager) {
            g_smtcHasTrack = false;
            g_smtcStatus = 0;
            return;
        }
        auto session = manager.GetCurrentSession();
        if (!session) {
            g_smtcHasTrack = false;
            g_smtcStatus = 0;
            return;
        }
        // Playback status first: it stays meaningful even if the
        // properties call below fails, so the glyph can still
        // reflect it while the title lines fall back to hidden.
        try {
            auto info = session.GetPlaybackInfo();
            if (info) {
                auto st = info.PlaybackStatus();
                if (st == smtc::GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing) g_smtcStatus = 1;
                else if (st == smtc::GlobalSystemMediaTransportControlsSessionPlaybackStatus::Paused) g_smtcStatus = 2;
                else g_smtcStatus = 0;
            } else {
                g_smtcStatus = 0;
            }
        } catch (...) {
            g_smtcStatus = 0;
        }
        auto props = session.TryGetMediaPropertiesAsync().get();
        if (!props) {
            g_smtcHasTrack = false;
            return;
        }
        winrt::hstring title = props.Title();
        winrt::hstring artist = props.Artist();
        if (title.empty()) {
            // No usable title: hide both lines rather than showing
            // a lone artist or a blank line.
            g_smtcHasTrack = false;
            g_smtcTitle[0] = L'\0';
            g_smtcArtist[0] = L'\0';
            return;
        }
        wcsncpy_s(g_smtcTitle, 256, title.c_str(), _TRUNCATE);
        if (!artist.empty()) wcsncpy_s(g_smtcArtist, 256, artist.c_str(), _TRUNCATE);
        else g_smtcArtist[0] = L'\0';
        g_smtcHasTrack = true;
    } catch (...) {
        // Any WinRT failure (no session broker, player error, COM
        // error): fall back to v4 behaviour, never crash.
        g_smtcHasTrack = false;
        g_smtcTitle[0] = L'\0';
        g_smtcArtist[0] = L'\0';
        g_smtcStatus = 0;
    }
}

// Play/pause glyph mode for DrawMediaGlyph: 0 = old combined
// glyph, 1 = pause bars only, 2 = play triangle only.
static int SmtcPlayPauseGlyphMode()
{
    if (g_smtcStatus == 1) return 1;
    if (g_smtcStatus == 2) return 2;
    return 0;
}

// ---------------------------------------------------------------------------
// System stats (sampled only while the widget is on, on the 1s timer).
// ---------------------------------------------------------------------------
static ULONGLONG FileTimeToU64(const FILETIME& ft)
{
    ULARGE_INTEGER v; v.LowPart = ft.dwLowDateTime; v.HighPart = ft.dwHighDateTime;
    return v.QuadPart;
}

static void SampleStats()
{
    if (!g_widgetStats) return;
    FILETIME idle, kernel, user;
    if (GetSystemTimes(&idle, &kernel, &user)) {
        ULONGLONG idleNow = FileTimeToU64(idle);
        ULONGLONG totalNow = FileTimeToU64(kernel) + FileTimeToU64(user);
        if (g_havePrevTimes) {
            ULONGLONG idleDelta = idleNow - FileTimeToU64(g_prevIdle);
            ULONGLONG totalDelta = totalNow - (FileTimeToU64(g_prevKernel) + FileTimeToU64(g_prevUser));
            if (totalDelta > 0) {
                g_cpuPercent = (int)(100.0 * (1.0 - (double)idleDelta / (double)totalDelta) + 0.5);
                if (g_cpuPercent < 0) g_cpuPercent = 0;
                if (g_cpuPercent > 100) g_cpuPercent = 100;
            }
        }
        g_prevIdle = idle; g_prevKernel = kernel; g_prevUser = user; g_havePrevTimes = true;
    }
    MEMORYSTATUSEX ms = { 0 }; ms.dwLength = sizeof(ms);
    if (GlobalMemoryStatusEx(&ms) && ms.ullTotalPhys > 0) {
        g_memPercent = (int)ms.dwMemoryLoad;
        g_memUsedGB = (double)(ms.ullTotalPhys - ms.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0);
        g_memTotalGB = (double)ms.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
    }
}

// ---------------------------------------------------------------------------
// Fullscreen detection (heuristic, checked once per second).
// A window counts as fullscreen on a monitor when it is visible, not
// minimised, not one of our own windows, not the desktop, and its rect
// covers the whole monitor rect. Captioned/maximised windows that leave
// the taskbar visible do not cover the full rect, so they do not hide
// the notch. Some borderless games keep a caption; those may be missed
// (see README limits).
// ---------------------------------------------------------------------------
struct FullscreenCheck {
    RECT monitorRect;
    bool found = false;
};

static BOOL CALLBACK EnumFullscreenWindow(HWND hwnd, LPARAM lParam)
{
    FullscreenCheck* check = (FullscreenCheck*)lParam;
    if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) return TRUE;
    if (hwnd == g_managerHwnd) return TRUE;
    if (FindNotch(hwnd) != NULL) return TRUE;
    if (hwnd == GetShellWindow()) return TRUE;
    wchar_t cls[64] = { 0 };
    GetClassNameW(hwnd, cls, 64);
    if (wcscmp(cls, L"Progman") == 0 || wcscmp(cls, L"WorkerW") == 0) return TRUE;
    RECT rc;
    if (!GetWindowRect(hwnd, &rc)) return TRUE;
    if (rc.left <= check->monitorRect.left && rc.top <= check->monitorRect.top &&
        rc.right >= check->monitorRect.right && rc.bottom >= check->monitorRect.bottom) {
        check->found = true;
        return FALSE; // stop enumerating
    }
    return TRUE;
}

static bool IsFullscreenOnMonitor(const RECT& monitorRect)
{
    FullscreenCheck check; check.monitorRect = monitorRect; check.found = false;
    EnumWindows(EnumFullscreenWindow, (LPARAM)&check);
    return check.found;
}

static void UpdateFullscreenAndVisibility()
{
    for (size_t i = 0; i < g_notches.size(); i++) {
        Notch* n = g_notches[i];
        bool fs = g_hideOnFullscreen && IsFullscreenOnMonitor(n->monitorRect);
        if (fs && !n->hiddenByFullscreen) {
            ShowWindow(n->hwnd, SW_HIDE);
            n->hiddenByFullscreen = true;
        } else if (!fs && n->hiddenByFullscreen) {
            n->hiddenByFullscreen = false;
            ShowWindow(n->hwnd, SW_SHOWNOACTIVATE);
            ApplyTargetSize(n, n->expanded);
        }
    }
}

static void InvalidateAllNotches()
{
    for (size_t i = 0; i < g_notches.size(); i++) {
        if (g_notches[i]->hwnd && !g_notches[i]->hiddenByFullscreen) {
            InvalidateRect(g_notches[i]->hwnd, NULL, FALSE);
        }
    }
}

static void ApplySizeToAll()
{
    for (size_t i = 0; i < g_notches.size(); i++) {
        if (g_notches[i]->hwnd && !g_notches[i]->hiddenByFullscreen) {
            ApplyTargetSize(g_notches[i], g_notches[i]->expanded);
        }
    }
}

// ---------------------------------------------------------------------------
// Monitor enumeration and rebuild (display change / monitor add-remove).
// ---------------------------------------------------------------------------
struct MonitorCollector {
    std::vector<Notch*> pending;
};

static BOOL CALLBACK EnumMonitorCallback(HMONITOR hMonitor, HDC /*hdc*/,
                                         LPRECT /*rect*/, LPARAM lParam)
{
    MonitorCollector* collector = (MonitorCollector*)lParam;
    MONITORINFO mi = { 0 }; mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW(hMonitor, &mi)) return TRUE;
    Notch* n = new Notch();
    n->hMonitor = hMonitor;
    n->monitorRect = mi.rcMonitor;
    n->workRect = mi.rcWork;
    collector->pending.push_back(n);
    return TRUE;
}

static void RebuildNotches(HINSTANCE hInst)
{
    if (g_rebuilding) return;
    g_rebuilding = true;
    // Destroy existing notch windows first. Each WM_DESTROY erases its
    // Notch from g_notches and deletes it, so the vector empties itself.
    std::vector<HWND> old;
    for (size_t i = 0; i < g_notches.size(); i++) old.push_back(g_notches[i]->hwnd);
    for (size_t i = 0; i < old.size(); i++) {
        if (old[i]) DestroyWindow(old[i]);
    }
    for (size_t i = 0; i < g_notches.size(); i++) delete g_notches[i];
    g_notches.clear();

    MonitorCollector collector;
    EnumDisplayMonitors(NULL, NULL, EnumMonitorCallback, (LPARAM)&collector);
    if (collector.pending.empty()) {
        // Fallback: primary monitor, so the app never ends up with no notch.
        Notch* n = new Notch();
        n->monitorRect.left = 0; n->monitorRect.top = 0;
        n->monitorRect.right = GetSystemMetrics(SM_CXSCREEN);
        n->monitorRect.bottom = GetSystemMetrics(SM_CYSCREEN);
        n->workRect = n->monitorRect;
        collector.pending.push_back(n);
    }
    for (size_t i = 0; i < collector.pending.size(); i++) {
        Notch* n = collector.pending[i];
        int w = CollapsedWidth(96);
        int workW = n->workRect.right - n->workRect.left;
        int x = n->workRect.left + (workW - w) / 2;
        HWND hwnd = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
            kNotchClass, kWindowTitle, WS_POPUP,
            x, n->workRect.top, w, 34,
            NULL, NULL, hInst, (LPVOID)n);
        if (hwnd) ShowWindow(hwnd, SW_SHOWNOACTIVATE);
        else delete n;
    }
    g_lastRebuildTick = GetTickCount64();
    g_rebuilding = false;
}

static void RequestRebuild(HINSTANCE hInst)
{
    ULONGLONG now = GetTickCount64();
    if (now - g_lastRebuildTick < 1000) return; // debounce duplicate forwards
    RebuildNotches(hInst);
}

// ---------------------------------------------------------------------------
// Notch window procedure.
// ---------------------------------------------------------------------------
// Media-button hit test. Returns -1 for none, else 0/1/2.
// usePaint=true matches what is drawn (paintExpanded, so buttons
// still show during a collapse animation); usePaint=false matches
// click semantics (expanded only, so a click during a collapse
// toggles pin instead of firing a media key). Rects are identical
// in both cases: the layout is shared, so hover, pressed and click
// can never disagree about where a button is.
// v4: media buttons are drawn as circles, so the hit test is a
// circle test, not a rectangle test. One shared helper computes the
// drawn circle for a button from the shared Layout rects: the
// circle is centred on the old button rect, with diameter equal to
// the media-row height. PaintNotch and MediaButtonIndexAt both use
// it, so a visible circle is exactly what is clickable, and the
// invisible corners of the old rectangles are no longer clickable.
static void MediaCircleFromRect(const RECT& btn, int buttonY, int buttonH, int& cx, int& cy, int& r)
{
    cx = (btn.left + btn.right) / 2;
    cy = buttonY + buttonH / 2;
    r = buttonH / 2;
}

static void MediaButtonCircle(const Layout& L, int idx, int& cx, int& cy, int& r)
{
    RECT btn = (idx == 0) ? L.btnPrev : (idx == 1) ? L.btnPlay : L.btnNext;
    MediaCircleFromRect(btn, L.mediaButtonY, L.mediaButtonH, cx, cy, r);
}

// v4 tabs: the smaller HOME media buttons use the same circle
// test, scaled down, so a visible mini circle is exactly what
// is clickable on the HOME tab.
static void HomeMediaButtonCircle(const Layout& L, int idx, int& cx, int& cy, int& r)
{
    RECT btn = (idx == 0) ? L.homeBtnPrev : (idx == 1) ? L.homeBtnPlay : L.homeBtnNext;
    MediaCircleFromRect(btn, L.homeMediaButtonY, L.homeMediaButtonH, cx, cy, r);
}

static int MediaButtonIndexAt(Notch* n, int px, int py, bool usePaint)
{
    if (!g_mediaEnabled) return -1;
    if (usePaint ? !n->paintExpanded : !n->expanded) return -1;
    RECT rc; GetClientRect(n->hwnd, &rc);
    Layout L = ComputeLayout(n, rc.right);
    int effTab = EffectiveTab(n);
    if (effTab == kTabHome) {
        if (py < L.homeMediaButtonY || py >= L.homeMediaButtonY + L.homeMediaButtonH) return -1;
        for (int i = 0; i < 3; i++) {
            int cx, cy, r;
            HomeMediaButtonCircle(L, i, cx, cy, r);
            int dx = px - cx, dy = py - cy;
            if (dx * dx + dy * dy <= r * r) return i;
        }
        return -1;
    }
    if (effTab != kTabMedia) return -1;
    if (py < L.mediaButtonY || py >= L.mediaButtonY + L.mediaButtonH) return -1;
    for (int i = 0; i < 3; i++) {
        int cx, cy, r;
        MediaButtonCircle(L, i, cx, cy, r);
        int dx = px - cx, dy = py - cy;
        if (dx * dx + dy * dy <= r * r) return i;
    }
    return -1;
}

static bool MediaButtonHit(Notch* n, int px, int py, WORD& vkOut)
{
    static const WORD kVk[3] = { VK_MEDIA_PREV_TRACK, VK_MEDIA_PLAY_PAUSE, VK_MEDIA_NEXT_TRACK };
    int idx = MediaButtonIndexAt(n, px, py, false);
    if (idx < 0) return false;
    vkOut = kVk[idx];
    return true;
}

// v4 tabs: tab hit test. Returns the tab id (0 = HOME, 1 =
// MEDIA, 2 = SYSTEM, 3 = CALENDAR) or -1. Same paint-vs-click
// split as the media buttons and the gear: hover may use what
// is drawn, a click only counts while truly expanded.
static int TabIndexAt(Notch* n, int px, int py, bool usePaint)
{
    if (usePaint ? !n->paintExpanded : !n->expanded) return -1;
    RECT rc; GetClientRect(n->hwnd, &rc);
    Layout L = ComputeLayout(n, rc.right);
    if (!L.showTabs) return -1;
    if (py < L.tabY || py >= L.tabY + L.tabH) return -1;
    for (int i = 0; i < L.tabCount; i++) {
        RECT tr = L.tabRects[i];
        if (px >= tr.left && px < tr.right) return L.tabIds[i];
    }
    return -1;
}

// v4 reorg: settings-gear hit test. Same paint-vs-click split as
// media buttons: hover may use what is drawn (paintExpanded),
// a click only counts while truly expanded, so a click during a
// collapse cannot open the menu. A 2 px tolerance ring makes the
// small gear forgiving to hit without enlarging its drawing.
static bool GearHitAt(Notch* n, int px, int py, bool usePaint)
{
    if (usePaint ? !n->paintExpanded : !n->expanded) return false;
    RECT rc; GetClientRect(n->hwnd, &rc);
    Layout L = ComputeLayout(n, rc.right);
    int dx = px - L.gearCX, dy = py - L.gearCY;
    int r = L.gearR + 2;
    return dx * dx + dy * dy <= r * r;
}

// v4 reorg: plain-GDI gear — 8 teeth, a solid ring, a dark centre
// hole. No font glyph, no image asset. Colour follows the
// current theme (gear / gearHover / gearPressed, gearHole
// for the centre hole), matching the media buttons' hover
// language in that theme.
static void DrawGear(HDC hdc, int cx, int cy, int r, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
    HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);
    // Teeth: 8 small rectangles rotated around the centre, drawn
    // first so the ring below covers their inner ends.
    const double kPi = 3.141592653589793;
    int toothW = r * 2 / 5; if (toothW < 3) toothW = 3;
    int toothH = r / 3; if (toothH < 2) toothH = 2;
    for (int i = 0; i < 8; i++) {
        double a = (double)i * kPi / 4.0;
        double ca = cos(a), sa = sin(a);
        // Tooth rectangle, centred on the ring, long axis radial.
        double hx = (double)r, hy = 0.0; // centre offset, unrotated
        double hw = (double)toothH / 2.0, hh = (double)toothW / 2.0;
        POINT pts[4];
        double corners[4][2] = { { hx - hh, -hw }, { hx + hh, -hw }, { hx + hh, hw }, { hx - hh, hw } };
        for (int c = 0; c < 4; c++) {
            double x = corners[c][0], y = corners[c][1];
            pts[c].x = (LONG)(cx + x * ca - y * sa + 0.5);
            pts[c].y = (LONG)(cy + x * sa + y * ca + 0.5);
        }
        Polygon(hdc, pts, 4);
    }
    // Ring, then the dark centre hole (dark = notch background tone).
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    HBRUSH holeBrush = CreateSolidBrush(CurrentTheme().gearHole);
    HBRUSH prevBrush = (HBRUSH)SelectObject(hdc, holeBrush);
    int hr = r * 2 / 5; if (hr < 2) hr = 2;
    Ellipse(hdc, cx - hr, cy - hr, cx + hr, cy + hr);
    SelectObject(hdc, prevBrush);
    DeleteObject(holeBrush);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}

// v4 tabs: one media glyph (Prev / Play-Pause / Next), shared
// by the full-size buttons on the MEDIA tab and the smaller
// buttons on the HOME tab. The glyph brush must already be
// selected into the DC by the caller.
// v5-SMTC: playPauseMode (from SmtcPlayPauseGlyphMode) applies
// only to idx 1: 0 = old combined triangle+bars, 1 = pause bars
// only (currently playing, so the button pauses), 2 = play
// triangle only (currently paused, so the button plays).
static void DrawMediaGlyph(HDC hdc, int idx, int cx, int cy, int r, HBRUSH glyphBrush, int playPauseMode)
{
    int h = (2 * r) * 11 / 20; // glyph height, as in v3
    int half = h / 2;
    if (idx == 0 || idx == 2) {
        // Prev: bar + two triangles pointing left.
        // Next: two triangles pointing right + bar (mirrored).
        int dir = (idx == 0) ? -1 : 1;
        int barW = h / 6; if (barW < 2) barW = 2;
        int triW = h / 2;
        int gap = 1;
        int totalW = barW + gap + 2 * triW + gap;
        int x0 = cx - totalW / 2;
        RECT bar = { 0, 0, 0, 0 };
        if (dir < 0) {
            bar = { x0, cy - half, x0 + barW, cy + half };
            POINT t1[3] = { { x0 + barW + gap + triW, cy - half }, { x0 + barW + gap + triW, cy + half }, { x0 + barW + gap, cy } };
            POINT t2[3] = { { x0 + barW + gap + 2 * triW + gap, cy - half }, { x0 + barW + gap + 2 * triW + gap, cy + half }, { x0 + barW + gap + triW + gap, cy } };
            FillRect(hdc, &bar, glyphBrush);
            Polygon(hdc, t1, 3);
            Polygon(hdc, t2, 3);
        } else {
            POINT t1[3] = { { x0, cy - half }, { x0, cy + half }, { x0 + triW, cy } };
            POINT t2[3] = { { x0 + triW + gap, cy - half }, { x0 + triW + gap, cy + half }, { x0 + 2 * triW + gap, cy } };
            bar = { x0 + 2 * triW + 2 * gap, cy - half, x0 + 2 * triW + 2 * gap + barW, cy + half };
            Polygon(hdc, t1, 3);
            Polygon(hdc, t2, 3);
            FillRect(hdc, &bar, glyphBrush);
        }
    } else if (playPauseMode == 1) {
        // v5-SMTC, playing: pause bars only, centred.
        int barW = h / 6; if (barW < 2) barW = 2;
        int gap = h / 8; if (gap < 2) gap = 2;
        int totalW = 2 * barW + gap;
        int x0 = cx - totalW / 2;
        RECT b1 = { x0, cy - half, x0 + barW, cy + half };
        RECT b2 = { x0 + barW + gap, cy - half, x0 + 2 * barW + gap, cy + half };
        FillRect(hdc, &b1, glyphBrush);
        FillRect(hdc, &b2, glyphBrush);
    } else if (playPauseMode == 2) {
        // v5-SMTC, paused: play triangle only, centred.
        int triW = h / 2;
        int x0 = cx - triW / 2;
        POINT tri[3] = { { x0, cy - half }, { x0, cy + half }, { x0 + triW, cy } };
        Polygon(hdc, tri, 3);
    } else {
        // Play/Pause: a play triangle on the left, pause bars
        // on the right, so one button reads as the toggle.
        int triW = h / 2;
        int barW = h / 6; if (barW < 2) barW = 2;
        int gap = h / 8; if (gap < 2) gap = 2;
        int totalW = triW + gap + 2 * barW + gap;
        int x0 = cx - totalW / 2;
        POINT tri[3] = { { x0, cy - half }, { x0, cy + half }, { x0 + triW, cy } };
        Polygon(hdc, tri, 3);
        RECT b1 = { x0 + triW + gap, cy - half, x0 + triW + gap + barW, cy + half };
        RECT b2 = { x0 + triW + gap + barW + gap, cy - half, x0 + triW + gap + 2 * barW + gap, cy + half };
        FillRect(hdc, &b1, glyphBrush);
        FillRect(hdc, &b2, glyphBrush);
    }
}

// v4 premium surface helpers, now theme-driven (v5-themes). No
// new allocations, no new timers: the gradient is drawn row by
// row with plain GDI (the window region already clips it to the
// pill shape), and the border and top highlight are single
// strokes drawn over the content. All colours come from the
// current theme; nothing here is hardcoded.
static void PaintPremiumSurface(HDC hdc, const RECT& rc)
{
    // Vertical theme gradient: top colour at the top, bottom
    // colour at the bottom, interpolated per row. Pure GDI, no
    // GradientFill/msimg32 lib.
    int h = rc.bottom - rc.top;
    if (h < 1) h = 1;
    const Theme& th = CurrentTheme();
    for (int y = 0; y < rc.bottom; y++) {
        int t = y * 100 / h; // 0 at top, 100 at bottom
        int r = GetRValue(th.gradTop) + (GetRValue(th.gradBottom) - GetRValue(th.gradTop)) * t / 100;
        int g = GetGValue(th.gradTop) + (GetGValue(th.gradBottom) - GetGValue(th.gradTop)) * t / 100;
        int b = GetBValue(th.gradTop) + (GetBValue(th.gradBottom) - GetBValue(th.gradTop)) * t / 100;
        HBRUSH rowBrush = CreateSolidBrush(RGB(r, g, b));
        RECT row = { rc.left, y, rc.right, y + 1 };
        FillRect(hdc, &row, rowBrush);
        DeleteObject(rowBrush);
    }
}

static void PaintPremiumBorder(HDC hdc, const RECT& rc, int dpi, bool hover)
{
    // Subtle 1 px light border, a touch brighter on hover (the v3
    // hover outline, restyled as the permanent premium border).
    COLORREF border = hover ? CurrentTheme().borderHover : CurrentTheme().border;
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, nullBrush);
    RoundRect(hdc, 0, 0, rc.right, rc.bottom, 28, 28);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
    // Soft inner top highlight: a 1 px light line, inset so it
    // never touches the rounded corners.
    int inset = Scale(20, dpi);
    HBRUSH hl = CreateSolidBrush(CurrentTheme().highlight);
    RECT line = { inset, Scale(1, dpi), rc.right - inset, Scale(2, dpi) };
    if (line.right > line.left) FillRect(hdc, &line, hl);
    DeleteObject(hl);
}

// Battery state for the v4 glyph. Returns false when there is no
// battery (desktop / unknown), in which case only the outline is
// drawn and the text stays "Battery: --" as before.
static bool BatteryInfo(int& pct, bool& charging)
{
    SYSTEM_POWER_STATUS sps;
    if (!GetSystemPowerStatus(&sps) || sps.BatteryLifePercent == 255) return false;
    pct = (int)sps.BatteryLifePercent;
    charging = (sps.ACLineStatus == 1 && (sps.BatteryFlag & 8) != 0);
    return true;
}

static void DrawBatteryGlyph(HDC hdc, int x, int y, int w, int h, int pct, bool charging, bool known)
{
    HPEN pen = CreatePen(PS_SOLID, 1, CurrentTheme().batteryOutline);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, nullBrush);
    RoundRect(hdc, x, y, x + w, y + h, 4, 4);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
    // Tip nub on the right.
    int tipW = (w / 8 < 2) ? 2 : w / 8;
    int tipH = h / 2;
    HBRUSH tipBrush = CreateSolidBrush(CurrentTheme().batteryOutline);
    RECT tip = { x + w, y + (h - tipH) / 2, x + w + tipW, y + (h + tipH) / 2 };
    FillRect(hdc, &tip, tipBrush);
    DeleteObject(tipBrush);
    if (!known) return;
    // Fill by percentage, inset inside the outline.
    int innerX = x + 2, innerY = y + 2;
    int innerW = w - 4, innerH = h - 4;
    if (innerW > 0 && innerH > 0) {
        int fillW = innerW * pct / 100;
        if (fillW > 0) {
            HBRUSH fillBrush = CreateSolidBrush(CurrentTheme().batteryFill);
            RECT fill = { innerX, innerY, innerX + fillW, innerY + innerH };
            FillRect(hdc, &fill, fillBrush);
            DeleteObject(fillBrush);
        }
    }
    if (charging) {
        // Lightning bolt in dark grey over the fill, centred.
        int cx = x + w / 2, cy = y + h / 2, bh = h * 3 / 4, bw = bh / 2;
        POINT bolt[6] = {
            { cx + bw / 2, cy - bh / 2 }, { cx - bw / 2, cy + 1 },
            { cx, cy + 1 }, { cx - bw / 2, cy + bh / 2 },
            { cx + bw / 2, cy - 1 }, { cx, cy - 1 }
        };
        HBRUSH boltBrush = CreateSolidBrush(CurrentTheme().batteryBolt);
        HBRUSH oldB = (HBRUSH)SelectObject(hdc, boltBrush);
        HPEN nullP = (HPEN)GetStockObject(NULL_PEN);
        HPEN oldP = (HPEN)SelectObject(hdc, nullP);
        Polygon(hdc, bolt, 6);
        SelectObject(hdc, oldP);
        SelectObject(hdc, oldB);
        DeleteObject(boltBrush);
    }
}

static void PaintNotch(Notch* n, HDC hdc)
{
    RECT rc; GetClientRect(n->hwnd, &rc);
    PaintPremiumSurface(hdc, rc);

    wchar_t clockText[32]; FormatClock(clockText, 32);
    int dpi = n->dpi;
    // v3: paint by paintExpanded, not expanded, so a collapse
    // animation keeps drawing expanded content until it finishes
    // instead of popping to the collapsed clock on the first frame.
    if (!n->paintExpanded) {
        DrawCenteredText(hdc, n->hFontClock, clockText, 0, 0, rc.right, rc.bottom, CurrentTheme().primary);
        PaintPremiumBorder(hdc, rc, dpi, n->hoverNotch);
        return;
    }

    Layout L = ComputeLayout(n, rc.right);
    int effTab = EffectiveTab(n);
    DrawCenteredText(hdc, n->hFontBig, clockText, 0, Scale(12, dpi), rc.right, Scale(48, dpi), CurrentTheme().primary);
    wchar_t dateText[64]; FormatDate(dateText, 64);
    DrawCenteredText(hdc, n->hFontSmall, dateText, 0, Scale(64, dpi), rc.right, Scale(22, dpi), CurrentTheme().secondary);
    // v4 battery row: drawn glyph + text, centred as a pair, so the
    // row reads as one unit instead of text alone.
    wchar_t batteryText[64]; FormatBattery(batteryText, 64);
    {
        HFONT oldF = (HFONT)SelectObject(hdc, n->hFontSmall);
        SIZE sz = { 0, 0 };
        GetTextExtentPoint32W(hdc, batteryText, lstrlenW(batteryText), &sz);
        SelectObject(hdc, oldF);
        int glyphW = Scale(24, dpi), glyphH = Scale(13, dpi), gap = Scale(8, dpi);
        int totalW = glyphW + gap + sz.cx;
        int startX = (rc.right - totalW) / 2;
        if (startX < 0) startX = 0;
        int glyphY = Scale(88, dpi) + (Scale(22, dpi) - glyphH) / 2;
        int pct = 0; bool charging = false; bool known = BatteryInfo(pct, charging);
        DrawBatteryGlyph(hdc, startX, glyphY, glyphW, glyphH, pct, charging, known);
        DrawCenteredText(hdc, n->hFontSmall, batteryText,
                         startX + glyphW + gap, Scale(88, dpi), sz.cx, Scale(22, dpi),
                         CurrentTheme().secondary);
    }
    // v4 divider: a thin line between the clock/date/battery block
    // and whatever sections follow (media and/or widgets).
    if (g_mediaEnabled || g_widgetStats || g_widgetCalendar) {
        int margin = Scale(20, dpi);
        HBRUSH divBrush = CreateSolidBrush(CurrentTheme().divider);
        RECT div = { margin, Scale(118, dpi), rc.right - margin, Scale(119, dpi) };
        if (div.right > div.left) FillRect(hdc, &div, divBrush);
        DeleteObject(divBrush);
    }

    // v4 tabs: tab bar below the divider. Active tab: near-white
    // text with a near-white underline across its segment;
    // inactive tabs: muted grey, brighter on hover.
    if (L.showTabs) {
        static const wchar_t* kTabNames[4] = { L"HOME", L"MEDIA", L"SYSTEM", L"CALENDAR" };
        for (int i = 0; i < L.tabCount; i++) {
            int id = L.tabIds[i];
            RECT tr = L.tabRects[i];
            COLORREF tc = (id == effTab) ? CurrentTheme().primary
                : (id == n->hoverTab ? CurrentTheme().tabHover : CurrentTheme().muted);
            DrawCenteredText(hdc, n->hFontTiny, kTabNames[id],
                             tr.left, tr.top, tr.right - tr.left, Scale(20, dpi), tc);
            if (id == effTab) {
                HBRUSH ulBrush = CreateSolidBrush(CurrentTheme().accent);
                int pad = Scale(12, dpi);
                RECT ul = { tr.left + pad, tr.top + Scale(20, dpi), tr.right - pad, tr.top + Scale(22, dpi) };
                if (ul.right > ul.left) FillRect(hdc, &ul, ulBrush);
                DeleteObject(ulBrush);
            }
        }
    }

    // v4 tabs: HOME is the compact summary — smaller media
    // buttons (still functional), one stats line, the current
    // week only. No section headers here: each row is self
    // evident, and headers would cost the height HOME saves.
    if (L.showTabs && effTab == kTabHome) {
        if (g_mediaEnabled) {
            // v5-SMTC: compact title line above the mini buttons,
            // left-aligned in the body, ellipsis-truncated to the
            // body width. Only when a track is known; otherwise
            // this row does not exist and the layout is v4.
            if (g_smtcHasTrack && L.homeMediaTitleY >= 0) {
                int margin = Scale(16, dpi);
                DrawLeftEllipsisText(hdc, n->hFontTiny, g_smtcTitle,
                                     margin, L.homeMediaTitleY,
                                     rc.right - 2 * margin, Scale(20, dpi),
                                     CurrentTheme().primary);
            }
            HBRUSH glyphBrush = CreateSolidBrush(CurrentTheme().glyph);
            HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
            HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);
            HBRUSH oldBrush = (HBRUSH)GetCurrentObject(hdc, OBJ_BRUSH);
            for (int i = 0; i < 3; i++) {
                int cx, cy, r;
                HomeMediaButtonCircle(L, i, cx, cy, r);
                COLORREF bg = CurrentTheme().btnBg;
                if (i == n->pressedBtn) bg = CurrentTheme().btnPressed;
                else if (i == n->hoverBtn) bg = CurrentTheme().btnHover;
                HBRUSH btnBrush = CreateSolidBrush(bg);
                HBRUSH prevBrush = (HBRUSH)SelectObject(hdc, btnBrush);
                Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
                SelectObject(hdc, prevBrush);
                DeleteObject(btnBrush);
            }
            SelectObject(hdc, glyphBrush);
            for (int i = 0; i < 3; i++) {
                int cx, cy, r;
                HomeMediaButtonCircle(L, i, cx, cy, r);
                DrawMediaGlyph(hdc, i, cx, cy, r, glyphBrush, SmtcPlayPauseGlyphMode());
            }
            SelectObject(hdc, oldPen);
            SelectObject(hdc, oldBrush);
            DeleteObject(glyphBrush);
        }
        if (g_widgetStats) {
            wchar_t homeStats[128];
            swprintf(homeStats, 128, L"CPU %d%%  \x00B7  RAM %d%%", g_cpuPercent, g_memPercent);
            DrawCenteredText(hdc, n->hFontTiny, homeStats,
                             Scale(16, dpi), L.homeStatsY, rc.right - Scale(32, dpi), Scale(20, dpi),
                             CurrentTheme().secondary);
        }
        if (g_widgetCalendar) {
            SYSTEMTIME st; GetLocalTime(&st);
            // Current week, Sunday start: step today by whole
            // days in FILETIME (100 ns units), then convert back.
            FILETIME ft; SystemTimeToFileTime(&st, &ft);
            ULARGE_INTEGER base; base.LowPart = ft.dwLowDateTime; base.HighPart = ft.dwHighDateTime;
            int margin = Scale(16, dpi);
            int cellW = (rc.right - 2 * margin) / 7;
            int cellH = Scale(20, dpi);
            for (int c = 0; c < 7; c++) {
                ULARGE_INTEGER v;
                v.QuadPart = base.QuadPart + (ULONGLONG)((LONGLONG)(c - st.wDayOfWeek) * 864000000000LL);
                FILETIME cft; cft.dwLowDateTime = v.LowPart; cft.dwHighDateTime = v.HighPart;
                SYSTEMTIME cst; FileTimeToSystemTime(&cft, &cst);
                int x = margin + c * cellW;
                int y = L.homeWeekY;
                bool isToday = (c == st.wDayOfWeek);
                if (isToday) {
                    HPEN nullP = (HPEN)GetStockObject(NULL_PEN);
                    HPEN oldP = (HPEN)SelectObject(hdc, nullP);
                    HBRUSH pill = CreateSolidBrush(CurrentTheme().accent);
                    HBRUSH oldB = (HBRUSH)SelectObject(hdc, pill);
                    RoundRect(hdc, x + Scale(2, dpi), y + Scale(1, dpi),
                              x + cellW - Scale(2, dpi), y + cellH - Scale(1, dpi),
                              cellH, cellH);
                    SelectObject(hdc, oldP);
                    SelectObject(hdc, oldB);
                    DeleteObject(pill);
                }
                wchar_t dayText[8]; swprintf(dayText, 8, L"%d", cst.wDay);
                DrawCenteredText(hdc, n->hFontTiny, dayText, x, y, cellW, cellH,
                                 isToday ? CurrentTheme().pillText : CurrentTheme().secondary);
            }
        }
    }


    if (effTab == kTabMedia) {
        // v4 reorg: MEDIA section header, left-aligned in its column
        // (the "media keys" explanation lives in the README/About).
        DrawLeftText(hdc, n->hFontTiny, L"MEDIA",
                     L.mediaX, L.mediaLabelY, L.mediaW, Scale(16, dpi), CurrentTheme().muted);
        // v5-SMTC: title on one line, artist on the next, both
        // left-aligned in the media section and ellipsis-truncated
        // to the section width. Artist line only when an artist
        // string exists; with no track, neither line is drawn and
        // the buttons sit at the v4 position.
        if (g_smtcHasTrack && L.mediaTitleY >= 0) {
            DrawLeftEllipsisText(hdc, n->hFontSmall, g_smtcTitle,
                                 L.mediaX, L.mediaTitleY, L.mediaW, Scale(20, dpi),
                                 CurrentTheme().primary);
            if (g_smtcArtist[0] != L'\0' && L.mediaArtistY >= 0) {
                DrawLeftEllipsisText(hdc, n->hFontTiny, g_smtcArtist,
                                     L.mediaX, L.mediaArtistY, L.mediaW, Scale(20, dpi),
                                     CurrentTheme().secondary);
            }
        }
        // v4 buttons: circles, not rectangles. Diameter = media-row
        // height, centred on the old button rects, so the row stays
        // centred and balanced. The hit test (MediaButtonIndexAt)
        // uses these same circles, so drawing and clicking agree.
        // Hover/pressed states are the v3 idea, restrained: a
        // lighter circle on hover, lighter still on press.
        HBRUSH glyphBrush = CreateSolidBrush(CurrentTheme().glyph);
        HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
        HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);
        HBRUSH oldBrush = (HBRUSH)GetCurrentObject(hdc, OBJ_BRUSH);
        for (int i = 0; i < 3; i++) {
            int cx, cy, r;
            MediaButtonCircle(L, i, cx, cy, r);
            COLORREF bg = CurrentTheme().btnBg;
            if (i == n->pressedBtn) bg = CurrentTheme().btnPressed;
            else if (i == n->hoverBtn) bg = CurrentTheme().btnHover;
            HBRUSH btnBrush = CreateSolidBrush(bg);
            HBRUSH prevBrush = (HBRUSH)SelectObject(hdc, btnBrush);
            Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
            SelectObject(hdc, prevBrush);
            DeleteObject(btnBrush);
        }
        SelectObject(hdc, glyphBrush);
        for (int i = 0; i < 3; i++) {
            int cx, cy, r;
            MediaButtonCircle(L, i, cx, cy, r);
            DrawMediaGlyph(hdc, i, cx, cy, r, glyphBrush, SmtcPlayPauseGlyphMode());
        }
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(glyphBrush);
    }

    if (effTab == kTabSystem) {
        // v4 reorg: SYSTEM header, then two labelled rows, each
        // with a thin progress bar underneath (track dark, fill
        // near-white). Same CPU/RAM values as before, just split
        // out of the old single dense line so they scan at a glance.
        DrawLeftText(hdc, n->hFontTiny, L"SYSTEM",
                     L.statsX, L.statsLabelY, L.statsW, Scale(16, dpi), CurrentTheme().muted);
        wchar_t cpuText[32];
        swprintf(cpuText, 32, L"CPU  %d%%", g_cpuPercent);
        DrawLeftText(hdc, n->hFontTiny, cpuText,
                     L.statsX, L.cpuLabelY, L.statsW, Scale(18, dpi), CurrentTheme().secondary);
        wchar_t ramText[64];
        swprintf(ramText, 64, L"RAM  %d%%  \x00B7  %.1f/%.1f GB", g_memPercent, g_memUsedGB, g_memTotalGB);
        DrawLeftText(hdc, n->hFontTiny, ramText,
                     L.statsX, L.ramLabelY, L.statsW, Scale(18, dpi), CurrentTheme().secondary);
        int barH = Scale(6, dpi);
        HBRUSH trackBrush = CreateSolidBrush(CurrentTheme().divider);
        HBRUSH fillBrush = CreateSolidBrush(CurrentTheme().accent);
        RECT cpuTrack = { L.statsX, L.cpuBarY, L.statsX + L.statsW, L.cpuBarY + barH };
        FillRect(hdc, &cpuTrack, trackBrush);
        RECT cpuFill = { L.statsX, L.cpuBarY, L.statsX + L.statsW * g_cpuPercent / 100, L.cpuBarY + barH };
        if (cpuFill.right > cpuFill.left) FillRect(hdc, &cpuFill, fillBrush);
        RECT ramTrack = { L.statsX, L.ramBarY, L.statsX + L.statsW, L.ramBarY + barH };
        FillRect(hdc, &ramTrack, trackBrush);
        RECT ramFill = { L.statsX, L.ramBarY, L.statsX + L.statsW * g_memPercent / 100, L.ramBarY + barH };
        if (ramFill.right > ramFill.left) FillRect(hdc, &ramFill, fillBrush);
        DeleteObject(trackBrush);
        DeleteObject(fillBrush);
    }

    if (effTab == kTabCalendar) {
        SYSTEMTIME st; GetLocalTime(&st);
        static const wchar_t* kMonths[] = { L"January", L"February", L"March", L"April", L"May", L"June",
            L"July", L"August", L"September", L"October", L"November", L"December" };
        wchar_t monthText[64];
        swprintf(monthText, 64, L"%s %d", kMonths[(st.wMonth - 1) % 12], st.wYear);
        // v4 reorg: CALENDAR header shares the month row — label on
        // the left, month centred — so it costs no extra height.
        DrawLeftText(hdc, n->hFontTiny, L"CALENDAR",
                     Scale(16, dpi), L.calendarY, Scale(100, dpi), Scale(24, dpi), CurrentTheme().muted);
        DrawCenteredText(hdc, n->hFontSmall, monthText, 0, L.calendarY, rc.right, Scale(24, dpi), CurrentTheme().primary);
        int margin = Scale(16, dpi);
        int cellW = (rc.right - 2 * margin) / 7;
        int cellH = Scale(20, dpi);
        int weekY = L.calendarY + Scale(24, dpi);
        static const wchar_t* kWeek[] = { L"S", L"M", L"T", L"W", L"T", L"F", L"S" };
        for (int c = 0; c < 7; c++) {
            DrawCenteredText(hdc, n->hFontTiny, kWeek[c], margin + c * cellW, weekY, cellW, cellH, CurrentTheme().muted);
        }
        int firstWeekday = (int)((st.wDayOfWeek - (st.wDay - 1) % 7) % 7);
        if (firstWeekday < 0) firstWeekday += 7;
        int days = DaysInMonth(st.wYear, st.wMonth);
        for (int d = 1; d <= days; d++) {
            int idx = firstWeekday + d - 1;
            int col = idx % 7, row = idx / 7;
            int x = margin + col * cellW;
            int y = weekY + cellH + row * cellH;
            // v4 today highlight: a white pill with dark text, not
            // a plain grey box — it reads as selected at a glance.
            bool isToday = (d == st.wDay);
            if (isToday) {
                HPEN nullP = (HPEN)GetStockObject(NULL_PEN);
                HPEN oldP = (HPEN)SelectObject(hdc, nullP);
                HBRUSH pill = CreateSolidBrush(CurrentTheme().accent);
                HBRUSH oldB = (HBRUSH)SelectObject(hdc, pill);
                RoundRect(hdc, x + Scale(2, dpi), y + Scale(1, dpi),
                          x + cellW - Scale(2, dpi), y + cellH - Scale(1, dpi),
                          cellH, cellH);
                SelectObject(hdc, oldP);
                SelectObject(hdc, oldB);
                DeleteObject(pill);
            }
            wchar_t dayText[8]; swprintf(dayText, 8, L"%d", d);
            DrawCenteredText(hdc, n->hFontTiny, dayText, x, y, cellW, cellH,
                             isToday ? CurrentTheme().pillText : CurrentTheme().secondary);
        }
    }

    // v4 reorg: the footer ("OnyxNotch v4 | Click to pin") is gone
    // — pinning still works by clicking the header, and the pin
    // explanation now lives in the About box and the README.
    // Settings gear, top-right of the header: drawn before the
    // border so the border stroke never covers it.
    {
        COLORREF gearColor = CurrentTheme().gear;
        if (n->pressedGear) gearColor = CurrentTheme().gearPressed;
        else if (n->hoverGear) gearColor = CurrentTheme().gearHover;
        DrawGear(hdc, L.gearCX, L.gearCY, L.gearR, gearColor);
    }
    // v4 border (drawn last, over the content): subtle always, a
    // touch brighter on hover. This replaces the v3 hover-only
    // outline; the hover state is now part of the border itself.
    PaintPremiumBorder(hdc, rc, dpi, n->hoverNotch);
}

static LRESULT CALLBACK NotchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Notch* n = FindNotch(hwnd);
    switch (msg) {
    case WM_TIMER:
        if (wParam == ID_TIMER_ANIM && n) StepAnimation(n);
        return 0;
    case WM_CREATE: {
        LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
        Notch* created = (Notch*)cs->lpCreateParams;
        if (created) {
            created->hwnd = hwnd;
            UINT dpi = GetDpiForWindow(hwnd);
            created->dpi = (dpi != 0) ? (int)dpi : 96;
            RecreateFonts(created);
            bool already = false;
            for (size_t i = 0; i < g_notches.size(); i++) if (g_notches[i] == created) already = true;
            if (!already) g_notches.push_back(created);
            // First show is instant on purpose (v3): no fade/slide-in,
            // which would need a layered-window rewrite (see README).
            ApplyNotchSizeInstant(created, false);
        }
        return 0;
    }
    case WM_DPICHANGED: {
        if (n) {
            n->dpi = HIWORD(wParam);
            RecreateFonts(n);
            ApplyTargetSize(n, n->expanded);
        }
        return 0;
    }
    case WM_DISPLAYCHANGE:
        if (g_managerHwnd) PostMessage(g_managerHwnd, WM_APP_REBUILD, 0, 0);
        return 0;
    case WM_MOUSEMOVE: {
        if (n) {
            n->hoverNotch = true;
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            int idx = MediaButtonIndexAt(n, pt.x, pt.y, true);
            if (idx != n->hoverBtn) {
                n->hoverBtn = idx;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            bool gear = GearHitAt(n, pt.x, pt.y, true);
            if (gear != n->hoverGear) {
                n->hoverGear = gear;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            int tab = TabIndexAt(n, pt.x, pt.y, true);
            if (tab != n->hoverTab) {
                n->hoverTab = tab;
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        if (n && !n->expanded) SetNotchExpanded(n, true);
        if (n && !n->trackingLeave) {
            TRACKMOUSEEVENT tme = { 0 };
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            n->trackingLeave = true;
        }
        return 0;
    }
    case WM_MOUSELEAVE:
        if (n) {
            n->trackingLeave = false;
            n->hoverNotch = false;
            n->hoverBtn = -1;
            n->pressedBtn = -1;
            n->hoverGear = false;
            n->pressedGear = false;
            n->hoverTab = -1;
            if (n->expanded && !n->pinned) SetNotchExpanded(n, false);
        }
        return 0;
    case WM_LBUTTONDOWN: {
        if (!n) return 0;
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        // v4 reorg: gear is hit-tested FIRST, before media buttons
        // and before the pin toggle, so a gear click opens the
        // settings menu and never pins/unpins the notch.
        if (GearHitAt(n, pt.x, pt.y, false)) {
            n->pressedGear = true;
            InvalidateRect(hwnd, NULL, FALSE);
            RECT wr; GetWindowRect(hwnd, &wr);
            Layout L = ComputeLayout(n, wr.right - wr.left);
            POINT menuPt;
            menuPt.x = wr.left + L.gearCX + L.gearR;
            menuPt.y = wr.top + L.gearCY + L.gearR;
            ShowTrayMenuAt(menuPt, TPM_RIGHTALIGN);
            n->pressedGear = false;
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        // v4 tabs: a tab click switches the body — never pins,
        // never fires a media key, never opens the gear menu.
        // Height animates via the existing path (or snaps, if
        // Animations is off); ApplyTargetSize already handles
        // the no-change case, so re-clicking a tab is harmless.
        int tab = TabIndexAt(n, pt.x, pt.y, false);
        if (tab >= 0) {
            if (tab != n->activeTab) {
                n->activeTab = tab;
                ApplyTargetSize(n, true);
            } else {
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        WORD vk = 0;
        if (MediaButtonHit(n, pt.x, pt.y, vk)) {
            n->pressedBtn = MediaButtonIndexAt(n, pt.x, pt.y, false);
            InvalidateRect(hwnd, NULL, FALSE);
            SendMediaKey(vk);
            return 0;
        }
        n->pinned = !n->pinned;
        SetNotchExpanded(n, n->pinned);
        return 0;
    }
    case WM_LBUTTONUP:
        if (n && (n->pressedBtn >= 0 || n->pressedGear)) {
            n->pressedBtn = -1;
            n->pressedGear = false;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (n) PaintNotch(n, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY: {
        if (n) {
            KillTimer(hwnd, ID_TIMER_ANIM);
            if (n->hFontClock) DeleteObject(n->hFontClock);
            if (n->hFontBig) DeleteObject(n->hFontBig);
            if (n->hFontSmall) DeleteObject(n->hFontSmall);
            if (n->hFontTiny) DeleteObject(n->hFontTiny);
            for (size_t i = 0; i < g_notches.size(); i++) {
                if (g_notches[i] == n) { g_notches.erase(g_notches.begin() + i); break; }
            }
            delete n;
        }
        return 0;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ---------------------------------------------------------------------------
// Manager window procedure: tray, timer, toggles, rebuilds.
// ---------------------------------------------------------------------------
static LRESULT CALLBACK ManagerWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        g_managerHwnd = hwnd;
        AddTrayIcon(hwnd, ((LPCREATESTRUCT)lParam)->hInstance);
        SetTimer(hwnd, ID_TIMER_CLOCK, 1000, NULL);
        RebuildNotches(((LPCREATESTRUCT)lParam)->hInstance);
        return 0;
    }
    case WM_TIMER:
        if (wParam == ID_TIMER_CLOCK) {
            // v5-SMTC: refresh the cached track/status on the
            // existing 1-second timer (no new timer). If the line
            // count changed (track appeared/disappeared, artist
            // gained/lost), resize expanded notches through the
            // existing animation path; a title-text-only change
            // keeps the same height and is just a repaint below.
            RefreshSmtc();
            int newKey = SmtcLayoutKey();
            if (newKey != g_smtcLayoutKey) {
                g_smtcLayoutKey = newKey;
                ApplySizeToAll();
            }
            SampleStats();
            UpdateFullscreenAndVisibility();
            InvalidateAllNotches();
        }
        return 0;
    case WM_APP_REBUILD: {
        // The instance handle is recovered from the manager window's
        // class registration via GetWindowLongPtr; simpler: use the
        // module handle, which is the same for this single .exe.
        RequestRebuild((HINSTANCE)GetModuleHandle(NULL));
        return 0;
    }
    case WM_DISPLAYCHANGE:
        RequestRebuild((HINSTANCE)GetModuleHandle(NULL));
        return 0;
    case WM_TRAY:
        if (lParam == WM_RBUTTONUP) ShowTrayMenu(hwnd);
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_QUIT: {
            // v5-settings: save on clean quit, before any window
            // is destroyed (WM_DESTROY below saves again as a
            // belt-and-braces step, so a quit via any path saves).
            SaveSettings();
            // Destroy notch windows first, then the manager (which quits).
            std::vector<HWND> hwnds;
            for (size_t i = 0; i < g_notches.size(); i++) hwnds.push_back(g_notches[i]->hwnd);
            for (size_t i = 0; i < hwnds.size(); i++) if (hwnds[i]) DestroyWindow(hwnds[i]);
            DestroyWindow(hwnd);
            break;
        }
        case ID_TRAY_SIZE_S: g_size = 0; SaveSettings(); ApplySizeToAll(); break;
        case ID_TRAY_SIZE_M: g_size = 1; SaveSettings(); ApplySizeToAll(); break;
        case ID_TRAY_SIZE_L: g_size = 2; SaveSettings(); ApplySizeToAll(); break;
        case ID_TRAY_HIDE_FULLSCREEN:
            g_hideOnFullscreen = !g_hideOnFullscreen;
            SaveSettings();
            UpdateFullscreenAndVisibility();
            break;
        case ID_TRAY_MEDIA:
            g_mediaEnabled = !g_mediaEnabled;
            SaveSettings();
            ApplySizeToAll();
            break;
        case ID_TRAY_WIDGET_STATS:
            g_widgetStats = !g_widgetStats;
            if (!g_widgetStats) g_havePrevTimes = false;
            SaveSettings();
            ApplySizeToAll();
            break;
        case ID_TRAY_WIDGET_CALENDAR:
            g_widgetCalendar = !g_widgetCalendar;
            SaveSettings();
            ApplySizeToAll();
            break;
        case ID_TRAY_AUTOSTART: {
            // v5-settings: registry only, never the settings file.
            // On registry failure the toggle is left unchanged,
            // so the menu never shows a state Windows will not do.
            bool want = !g_startWithWindows;
            if (SetAutoStart(want)) g_startWithWindows = want;
            break;
        }
        case ID_TRAY_ANIMATIONS:
            g_animations = !g_animations;
            SaveSettings();
            if (!g_animations) {
                // Stop every in-progress animation instantly and
                // snap each notch to its final v2 size at once.
                for (size_t i = 0; i < g_notches.size(); i++) {
                    Notch* an = g_notches[i];
                    // Hidden-by-fullscreen notches stay hidden; their
                    // size is snapped on restore instead (which would
                    // otherwise un-hide them here via SHOWWINDOW).
                    if (an && an->hwnd && !an->hiddenByFullscreen) {
                        StopAnimation(an);
                        an->paintExpanded = an->expanded;
                        ApplyNotchSizeInstant(an, an->expanded);
                        InvalidateRect(an->hwnd, NULL, TRUE);
                    }
                }
            }
            break;
        // v5-themes: switch, persist via the existing settings
        // mechanism, and repaint every notch at once. Hidden-by-
        // fullscreen notches are skipped by InvalidateAllNotches
        // (they are not visible) and repaint on restore.
        case ID_TRAY_THEME_ONYX: g_theme = 0; SaveSettings(); InvalidateAllNotches(); break;
        case ID_TRAY_THEME_MIDNIGHT: g_theme = 1; SaveSettings(); InvalidateAllNotches(); break;
        case ID_TRAY_THEME_GRAPHITE: g_theme = 2; SaveSettings(); InvalidateAllNotches(); break;
        case ID_TRAY_MEDIA_PREV: SendMediaKey(VK_MEDIA_PREV_TRACK); break;
        case ID_TRAY_MEDIA_PLAYPAUSE: SendMediaKey(VK_MEDIA_PLAY_PAUSE); break;
        case ID_TRAY_MEDIA_NEXT: SendMediaKey(VK_MEDIA_NEXT_TRACK); break;
        case ID_TRAY_ABOUT:
            MessageBoxW(hwnd,
                L"OnyxNotch v5 (SMTC titles + saved settings + themes)\nMac-style notch for Windows.\nNative Win32 C++, no runtime, low memory.\n\nv4: premium UI — gradient surface, lighter\ntypography, circular media buttons, battery glyph,\ncalendar pill — reorganized in place: compact\ntwo-column body (media beside system stats),\nsection headers, stats rows with bars.\n\nv5-SMTC: with Media controls on, the Media tab\nshows the current track title and artist from\nWindows SMTC (read-only), and Home shows a\ncompact title line. No track, no title — the\nmedia buttons still work via media keys.\n\nv5-settings: size and toggles are saved to\n%APPDATA%\\OnyxNotch\\settings.ini and restored\nat startup. Start with Windows (tray menu) writes\nthe Run registry value for this .exe.\n\nThemes: tray menu Themes submenu — Onyx, Midnight,\nGraphite — applies instantly to all notches and is\nsaved to settings.ini as theme=onyx|midnight|graphite.\n\nClick the notch header to pin it open;\nclick again to unpin. The gear button in the\nexpanded header opens this menu from the notch.\n\nTabs: Home is a compact summary of every\ntab; Media, System and Calendar show the\nfull versions. Click a tab to switch.",
                L"OnyxNotch", MB_OK | MB_ICONINFORMATION);
            break;
        }
        return 0;
    case WM_DESTROY:
        // v5-settings: save on clean quit even if the quit did not
        // come through the tray Quit command. SaveSettings never
        // blocks or shows UI, so this cannot delay shutdown.
        SaveSettings();
        KillTimer(hwnd, ID_TIMER_CLOCK);
        Shell_NotifyIconW(NIM_DELETE, &g_nid);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                    PWSTR /*pCmdLine*/, int /*nCmdShow*/)
{
    // Per-monitor DPI awareness (v2): fall back silently on older Windows.
    // DPI_AWARENESS_CONTEXT is a HANDLE; using HANDLE here avoids
    // depending on the SDK header that declares the named type.
    typedef BOOL (WINAPI* SetDpiCtxFn)(HANDLE);
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        SetDpiCtxFn fn = (SetDpiCtxFn)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
        if (fn) fn((HANDLE)-4); // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
    }

    WNDCLASSEXW wcNotch = { 0 };
    wcNotch.cbSize = sizeof(wcNotch);
    wcNotch.style = CS_HREDRAW | CS_VREDRAW;
    wcNotch.lpfnWndProc = NotchWndProc;
    wcNotch.hInstance = hInstance;
    wcNotch.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcNotch.hIcon = LoadOnyxIcon(hInstance);
    wcNotch.hIconSm = LoadOnyxIcon(hInstance);
    wcNotch.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcNotch.lpszClassName = kNotchClass;
    RegisterClassExW(&wcNotch);

    WNDCLASSEXW wcMgr = { 0 };
    wcMgr.cbSize = sizeof(wcMgr);
    wcMgr.lpfnWndProc = ManagerWndProc;
    wcMgr.hInstance = hInstance;
    wcMgr.lpszClassName = kManagerClass;
    RegisterClassExW(&wcMgr);

    // v5-settings: load BEFORE the first notch is drawn. The
    // manager window below creates the notch windows in its
    // WM_CREATE (via RebuildNotches), so loading here means the
    // first notch already uses the saved size/toggles. Load
    // failures fall back to the defaults silently (see LoadSettings
    // / LoadAutoStart); startup never depends on them.
    LoadSettings();
    LoadAutoStart();

    // Hidden manager window: owns the tray icon and the 1-second timer,
    // and creates one notch window per monitor in WM_CREATE.
    HWND manager = CreateWindowExW(0, kManagerClass, kWindowTitle, WS_POPUP,
                                   0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (!manager) return 1;

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
