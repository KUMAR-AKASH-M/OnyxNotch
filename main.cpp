// OnyxNotch v7 - Direct2D rewrite of v6 (see README v7 section).
// Every v6 feature and behaviour is preserved; only the rendering
// layer changed: GDI painting is replaced by Direct2D/DirectWrite
// on a layered window with per-pixel alpha (see the v7 section
// before PaintNotchD2D), and expand/collapse uses a damped
// spring instead of ease-out cubic. Blur is an OFF-by-default
// opt-in with an opaque fallback. Album-art tinting is out of
// scope (SMTC gives title/artist, not art). NOT compiled or
// tested on Windows in this VM.
// OnyxNotch v6-unified-size - one expanded size for every tab
// (in place, after the v6 flicker/icons/calendar fix).
//  1. Unified height: ComputeLayout returns the MAXIMUM total
//     height over every visible dock tab, so switching tabs never
//     changes the panel width or height, and the header and dock
//     sit at exactly the same Y on every tab.
//  2. Shorter tabs are vertically centred in the unified body
//     area (empty space split evenly above and below), for every
//     tab including Calendar, whose card is already a fixed
//     6-week height. Painting and hit tests share this layout.
//  3. A tab switch only repaints; it does not animate the size.
//     Expand/collapse still animate as before, and a feature
//     toggle still resizes through the animated size path, since
//     the set of visible tabs (and therefore the maximum) changed.
// OnyxNotch v6-fix - flicker, icons, calendar (in place, after v6).
//  1. Double-buffered painting: WM_PAINT renders into a memory DC
//     and BitBlts once; no erase-on-invalidate anywhere, so the
//     16 ms animation frames and the 1-second tick no longer flash.
//  2. Stable heights: the calendar card is always 6 weeks tall and
//     the MEDIA tab always reserves title+artist rows, so a track
//     starting/stopping or a month change never snaps the height.
//  3. Icons redrawn on one thicker, centred GDI grid (see DrawDockGlyph
//     and DrawMediaGlyph). Segoe MDL2 Assets was deliberately NOT
//     used: its glyph codepoints are unverifiable in this VM and a
//     wrong or missing glyph would show as tofu; hand-drawn glyphs
//     match their hit areas exactly and take theme colours.
//  4. Calendar: prev/next month arrows, a Today button, clickable
//     days (selected = outlined pill), faded adjacent-month days,
//     weekend days muted, and an agenda that follows the selection.
//     No events are invented: the agenda says No events.
// OnyxNotch v6 - card + dock dashboard look, existing data only.
// Wide expanded panel: header bar, card grid, bottom icon dock.
// No weather, no notes, no clipboard history, no screen-time
// tracking in v6 - those were deliberately excluded by the user.
// New local data only: Disk percent, Network up/down rates,
// system volume percent, all sampled on the existing 1s timer.
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
#include <iphlpapi.h> // v6: GetIfTable for network rates
#include <mmsystem.h> // v6: waveOutGetVolume for system volume
#include <d2d1.h> // v7: Direct2D rendering
#include <dwrite.h> // v7: DirectWrite text
// v5-SMTC: C++/WinRT headers for the local SMTC session manager.
// These need the Windows SDK with C++/WinRT and /std:c++17 (see
// build.bat). No other WinRT API is used; there is no network use.
// VS 2022/2026 cppwinrt headers still pull in the deprecated
// experimental coroutine header, which fails with C2338 unless
// this silence macro is defined before the first winrt include.
#ifndef _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS
#define _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS
#endif
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Control.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "windowsapp.lib")
#pragma comment(lib, "advapi32.lib") // v5-settings: Reg* calls for auto-start
#pragma comment(lib, "iphlpapi.lib") // v6: network counters
#pragma comment(lib, "winmm.lib") // v6: waveOutGetVolume
#pragma comment(lib, "d2d1.lib") // v7: Direct2D rendering
#pragma comment(lib, "dwrite.lib") // v7: DirectWrite text

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
#define ID_TRAY_BLUR            1018
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
// v7: blur background, OFF by default (see ApplyBlur). Saved as
// blur=0/1 in settings.ini. Never the default: an opaque panel
// is the safe look, and blur on a layered window is untested.
static bool g_blurEnabled = false;

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
                } else if (StrEqualsIgnoreCase(key, "blur")) {
                    if (ParseBoolValue(val, b)) g_blurEnabled = b;
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
        "blur=%d\r\n"
        "theme=%s\r\n",
        g_size,
        g_hideOnFullscreen ? 1 : 0,
        g_mediaEnabled ? 1 : 0,
        g_widgetStats ? 1 : 0,
        g_widgetCalendar ? 1 : 0,
        g_animations ? 1 : 0,
        g_blurEnabled ? 1 : 0,
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
    // v7: no GDI fonts. DirectWrite formats are created per
    // frame in PresentNotch from the notch DPI (see FontSizeFor).
    bool blurActive = false; // v7: accent blur actually applied
    // v7 spring state (see StepAnimation): position 0..~1 and
    // velocity, integrated per timer tick from animLastTick.
    double animSpringPos = 0.0;
    double animSpringVel = 0.0;
    ULONGLONG animLastTick = 0;
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
    int hoverDock = -1; // v6: dock tab id hovered, or -1
    bool hoverClose = false; // v6: close X hovered
    int lastClockMinute = -1; // v6-fix: collapsed repaints only on a minute change
    int calYear = 0, calMonth = 0, calDay = 0;
    int hoverCalArrow = 0; // v6-fix: -1 prev, 0 none, 1 next
    bool hoverCalToday = false;
    int hoverCalCell = -1; // v6-fix: 0..41 cell in the displayed grid, or -1
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

// v6: expanded panel is about 840 px at 96 DPI, clamped to the
// monitor work width minus margins so it never overflows a
// narrow monitor. Collapsed width is unchanged.
static int ExpandedWidth(Notch* n)
{
    int dpi = n->dpi;
    int w = Scale(840, dpi);
    int workW = n->workRect.right - n->workRect.left;
    int maxW = workW - Scale(32, dpi);
    if (w > maxW) w = maxW;
    if (w < Scale(320, dpi)) w = workW > 0 ? workW : Scale(320, dpi);
    if (w < 1) w = 1;
    return w;
}

// ---------------------------------------------------------------------------
// Expanded layout. One shared computation drives both painting and the
// media-button hit test, so the clickable buttons match what is drawn.
// ---------------------------------------------------------------------------
// v6 tab ids: Clock is new and always available, since clock
// data needs no toggle. Dock order is Home, System, Media,
// Clock, Calendar.
static const int kTabHome = 0;
static const int kTabMedia = 1;
static const int kTabSystem = 2;
static const int kTabCalendar = 3;
static const int kTabClock = 4;

struct Layout {
    int totalH = 0;
    int headerH = 0;
    int gearCX = 0, gearCY = 0, gearR = 0;
    int closeCX = 0, closeCY = 0, closeR = 0;
    int bodyY = 0;
    int dockY = 0, dockH = 0, dockCount = 0;
    int dockIds[5] = { 0, 0, 0, 0, 0 };
    RECT dockRects[5] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
    RECT dockPill = { 0, 0, 0, 0 };
    // HOME grid cards. homeCardIds: 0 System, 1 Calendar, 2 Clock,
    // 3 Media, 4 Battery, 5 Network.
    int homeCardCount = 0;
    int homeCardIds[6] = { 0, 0, 0, 0, 0, 0 };
    RECT homeCards[6] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
    int homeMediaTitleY = -1;
    int homeMediaButtonY = 0, homeMediaButtonH = 0;
    RECT homeBtnPrev = { 0, 0, 0, 0 };
    RECT homeBtnPlay = { 0, 0, 0, 0 };
    RECT homeBtnNext = { 0, 0, 0, 0 };
    int homeStatsY = 0;
    int homeWeekY = 0;
    // MEDIA card.
    RECT mediaCard = { 0, 0, 0, 0 };
    int mediaTitleY = -1, mediaArtistY = -1;
    int mediaButtonY = 0, mediaButtonH = 0;
    RECT btnPrev = { 0, 0, 0, 0 };
    RECT btnPlay = { 0, 0, 0, 0 };
    RECT btnNext = { 0, 0, 0, 0 };
    // SYSTEM cards: 0 Battery, 1 Network, 2 Performance, 3 Volume.
    RECT sysCards[4] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
    // CLOCK cards.
    RECT clockCard = { 0, 0, 0, 0 };
    RECT worldCard = { 0, 0, 0, 0 };
    // CALENDAR cards.
    RECT monthCard = { 0, 0, 0, 0 };
    RECT agendaCard = { 0, 0, 0, 0 };
    int calendarY = 0;
    int calendarH = 0;
    RECT calPrevRect = { 0, 0, 0, 0 };
    RECT calNextRect = { 0, 0, 0, 0 };
    RECT calTodayRect = { 0, 0, 0, 0 };
    int calCellW = 0, calCellH = 0, calWeekY = 0, calGridY = 0;
    int calFirstWeekday = 0, calDays = 0, calPrevDays = 0;
};

static int DaysInMonth(int year, int month)
{
    static const int kDays[] = { 31, 28, 31, 30, 31, 30, 31, 30, 31, 30, 31, 30, 31 };
    if (month == 2) {
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return leap ? 29 : 28;
    }
    return kDays[(month - 1) % 12];
}

// v6-fix: the month the calendar tab is showing. 0/0 means the
// current month (the default, and the reset state on leaving the
// tab or collapsing). Displayed month is per-notch state.
static void CalDisplayed(Notch* n, int& year, int& month)
{
    if (n && n->calYear > 0 && n->calMonth >= 1 && n->calMonth <= 12) {
        year = n->calYear; month = n->calMonth; return;
    }
    SYSTEMTIME st; GetLocalTime(&st);
    year = st.wYear; month = st.wMonth;
}

// v6: the tab actually shown. Clock is always valid. A selected
// tab whose feature was toggled off falls back to HOME.
static int EffectiveTab(Notch* n)
{
    int t = n->activeTab;
    if (t == kTabMedia && !g_mediaEnabled) return kTabHome;
    if (t == kTabSystem && !g_widgetStats) return kTabHome;
    if (t == kTabCalendar && !g_widgetCalendar) return kTabHome;
    if (t != kTabHome && t != kTabMedia && t != kTabSystem && t != kTabCalendar && t != kTabClock) return kTabHome;
    return t;
}

// v6 dock ids in dock order, given the toggles.
static int DockIds(int* out)
{
    int c = 0;
    out[c++] = kTabHome;
    if (g_widgetStats) out[c++] = kTabSystem;
    if (g_mediaEnabled) out[c++] = kTabMedia;
    out[c++] = kTabClock;
    if (g_widgetCalendar) out[c++] = kTabCalendar;
    return c;
}

static Layout ComputeLayoutForTab(Notch* n, int clientW, int tab)
{
    Layout L;
    int dpi = n->dpi;
    int margin = Scale(20, dpi);
    int gap = Scale(12, dpi);
    int bodyW = clientW - 2 * margin;
    if (bodyW < 40) bodyW = 40;
    L.headerH = Scale(56, dpi);
    // Header right cluster: gear, then close X at the edge.
    L.closeR = Scale(10, dpi);
    L.closeCX = clientW - margin - L.closeR;
    L.closeCY = L.headerH / 2;
    L.gearR = Scale(11, dpi);
    L.gearCX = L.closeCX - Scale(40, dpi);
    L.gearCY = L.headerH / 2;
    L.bodyY = L.headerH + Scale(8, dpi);
    int effTab = tab;
    int bodyH = 0;
    int btnGap = Scale(8, dpi);
    if (effTab == kTabHome) {
        int ids[6] = { 0, 1, 2, 3, 4, 5 };
        bool on[6] = { g_widgetStats, g_widgetCalendar, true, g_mediaEnabled, g_widgetStats, g_widgetStats };
        int count = 0;
        for (int i = 0; i < 6; i++) if (on[i]) { L.homeCardIds[count] = ids[i]; count++; }
        L.homeCardCount = count;
        int cols = 3;
        int cardW = (bodyW - (cols - 1) * gap) / cols;
        int cardH = Scale(140, dpi);
        int rows = (count + cols - 1) / cols;
        for (int i = 0; i < count; i++) {
            int col = i % cols, row = i / cols;
            int x = margin + col * (cardW + gap);
            int y = L.bodyY + row * (cardH + gap);
            L.homeCards[i] = { x, y, x + cardW, y + cardH };
        }
        bodyH = rows * cardH + (rows > 0 ? (rows - 1) * gap : 0);
        // Media card buttons: bottom of that card, 24 px circles.
        for (int i = 0; i < count; i++) if (L.homeCardIds[i] == 3) {
            RECT c = L.homeCards[i];
            L.homeMediaButtonH = Scale(24, dpi);
            L.homeMediaButtonY = c.bottom - Scale(12, dpi) - L.homeMediaButtonH;
            L.homeMediaTitleY = c.top + Scale(36, dpi);
            int innerW = (c.right - c.left) - 2 * Scale(16, dpi);
            int bw = (innerW - 2 * btnGap) / 3;
            if (bw < 20) bw = 20;
            int x0 = c.left + Scale(16, dpi);
            L.homeBtnPrev = { x0, L.homeMediaButtonY, x0 + bw, L.homeMediaButtonY + L.homeMediaButtonH };
            L.homeBtnPlay = { x0 + bw + btnGap, L.homeMediaButtonY, x0 + 2 * bw + btnGap, L.homeMediaButtonY + L.homeMediaButtonH };
            L.homeBtnNext = { x0 + 2 * bw + 2 * btnGap, L.homeMediaButtonY, x0 + 3 * bw + 2 * btnGap, L.homeMediaButtonY + L.homeMediaButtonH };
        }
    } else if (effTab == kTabSystem) {
        int cardW = (bodyW - gap) / 2;
        int cardH = Scale(160, dpi);
        for (int i = 0; i < 4; i++) {
            int col = i % 2, row = i / 2;
            int x = margin + col * (cardW + gap);
            int y = L.bodyY + row * (cardH + gap);
            L.sysCards[i] = { x, y, x + cardW, y + cardH };
        }
        bodyH = 2 * cardH + gap;
    } else if (effTab == kTabClock) {
        int clockW = bodyW * 55 / 100;
        int worldW = bodyW - clockW - gap;
        int cardH = Scale(190, dpi);
        L.clockCard = { margin, L.bodyY, margin + clockW, L.bodyY + cardH };
        L.worldCard = { margin + clockW + gap, L.bodyY, margin + clockW + gap + worldW, L.bodyY + cardH };
        bodyH = cardH;
    } else if (effTab == kTabMedia) {
        L.mediaCard = { margin, L.bodyY, margin + bodyW, L.bodyY + Scale(120, dpi) };
        int y = L.bodyY + Scale(36, dpi);
        // v6-fix: ALWAYS reserve both rows when media is on, whether
        // or not a track is playing, so a track starting or stopping
        // never snaps the card or panel height.
        L.mediaTitleY = y; y += Scale(22, dpi);
        L.mediaArtistY = y; y += Scale(20, dpi);
        L.mediaButtonH = Scale(32, dpi);
        L.mediaButtonY = y + Scale(6, dpi);
        int bw = (bodyW - 2 * btnGap) / 3;
        if (bw < 20) bw = 20;
        L.btnPrev = { margin, L.mediaButtonY, margin + bw, L.mediaButtonY + L.mediaButtonH };
        L.btnPlay = { margin + bw + btnGap, L.mediaButtonY, margin + 2 * bw + btnGap, L.mediaButtonY + L.mediaButtonH };
        L.btnNext = { margin + 2 * bw + 2 * btnGap, L.mediaButtonY, margin + 3 * bw + 2 * btnGap, L.mediaButtonY + L.mediaButtonH };
        bodyH = L.mediaButtonY + L.mediaButtonH + Scale(12, dpi) - L.bodyY;
        L.mediaCard.bottom = L.mediaButtonY + L.mediaButtonH + Scale(12, dpi);
    } else if (effTab == kTabCalendar) {
        // v6-fix: FIXED 6-week height, whatever the month.
        int dispY = 0, dispM = 0; CalDisplayed(n, dispY, dispM);
        SYSTEMTIME first; first.wYear = (WORD)dispY; first.wMonth = (WORD)dispM; first.wDay = 1;
        first.wHour = 0; first.wMinute = 0; first.wSecond = 0; first.wMilliseconds = 0;
        FILETIME fft; SystemTimeToFileTime(&first, &fft); FileTimeToSystemTime(&fft, &first);
        int firstWeekday = (int)first.wDayOfWeek;
        int days = DaysInMonth(dispY, dispM);
        int prevY = dispM == 1 ? dispY - 1 : dispY;
        int prevM = dispM == 1 ? 12 : dispM - 1;
        int prevDays = DaysInMonth(prevY, prevM);
        int monthW = bodyW * 60 / 100;
        int agendaW = bodyW - monthW - gap;
        int monthH = Scale(262, dpi);
        L.monthCard = { margin, L.bodyY, margin + monthW, L.bodyY + monthH };
        L.agendaCard = { margin + monthW + gap, L.bodyY, margin + monthW + gap + agendaW, L.bodyY + monthH };
        L.calendarY = L.bodyY;
        L.calendarH = monthH;
        L.calFirstWeekday = firstWeekday;
        L.calDays = days;
        L.calPrevDays = prevDays;
        L.calCellW = (monthW - Scale(28, dpi)) / 7;
        L.calCellH = Scale(28, dpi);
        L.calWeekY = L.bodyY + Scale(40, dpi);
        L.calGridY = L.calWeekY + Scale(20, dpi);
        L.calPrevRect = { margin + Scale(14, dpi), L.bodyY + Scale(10, dpi),
                          margin + Scale(42, dpi), L.bodyY + Scale(38, dpi) };
        L.calNextRect = { margin + monthW - Scale(42, dpi), L.bodyY + Scale(10, dpi),
                          margin + monthW - Scale(14, dpi), L.bodyY + Scale(38, dpi) };
        int todayW = Scale(64, dpi);
        L.calTodayRect = { margin + (monthW - todayW) / 2, L.bodyY + Scale(230, dpi),
                           margin + (monthW + todayW) / 2, L.bodyY + Scale(252, dpi) };
        bodyH = monthH;
    }
    // Bottom icon dock: centred pill, one icon per dock tab.
    L.dockH = Scale(56, dpi);
    L.dockY = L.bodyY + bodyH + Scale(8, dpi);
    int dockIds[5];
    int dc = DockIds(dockIds);
    L.dockCount = dc;
    int iconStep = Scale(48, dpi);
    int pillW = dc * iconStep + Scale(16, dpi);
    int pillX = (clientW - pillW) / 2;
    if (pillX < margin) pillX = margin;
    L.dockPill = { pillX, L.dockY, pillX + pillW, L.dockY + L.dockH };
    for (int i = 0; i < dc; i++) {
        L.dockIds[i] = dockIds[i];
        int cx = pillX + Scale(8, dpi) + i * iconStep + iconStep / 2;
        int cy = L.dockY + L.dockH / 2;
        int r = Scale(20, dpi);
        L.dockRects[i] = { cx - r, cy - r, cx + r, cy + r };
    }
    L.totalH = L.dockY + L.dockH + Scale(12, dpi);
    return L;
}

// v6-unified-size: every visible tab shares ONE expanded height.
// ComputeLayoutForTab above builds one tab at its natural height;
// this wrapper takes the maximum total height over every tab in
// the dock (only tabs that are actually visible for the current
// toggles) and vertically centres this tab's body inside that
// unified body area, so the header and the dock sit at exactly
// the same Y on every tab and switching tabs never moves them.
// Centring is used for EVERY tab, including Calendar: its card
// is already a fixed 6-week height, so centring it just splits
// the remaining space evenly instead of leaving it all below.
// Because painting and every hit test both call this wrapper,
// a click always lands where the content is drawn, on every tab.
// Header geometry (gear, close X) is never shifted.
static void ShiftRectY(RECT& r, int dy)
{
    r.top += dy; r.bottom += dy;
}

static Layout ComputeLayout(Notch* n, int clientW)
{
    Layout L = ComputeLayoutForTab(n, clientW, EffectiveTab(n));
    int ids[5];
    int dc = DockIds(ids);
    int maxTotal = L.totalH;
    for (int i = 0; i < dc; i++) {
        Layout t = ComputeLayoutForTab(n, clientW, ids[i]);
        if (t.totalH > maxTotal) maxTotal = t.totalH;
    }
    int delta = maxTotal - L.totalH;
    if (delta <= 0) return L;
    int shift = delta / 2;
    for (int i = 0; i < L.homeCardCount; i++) ShiftRectY(L.homeCards[i], shift);
    if (L.homeMediaTitleY >= 0) L.homeMediaTitleY += shift;
    L.homeMediaButtonY += shift;
    ShiftRectY(L.homeBtnPrev, shift);
    ShiftRectY(L.homeBtnPlay, shift);
    ShiftRectY(L.homeBtnNext, shift);
    L.homeStatsY += shift;
    L.homeWeekY += shift;
    ShiftRectY(L.mediaCard, shift);
    if (L.mediaTitleY >= 0) L.mediaTitleY += shift;
    if (L.mediaArtistY >= 0) L.mediaArtistY += shift;
    L.mediaButtonY += shift;
    ShiftRectY(L.btnPrev, shift);
    ShiftRectY(L.btnPlay, shift);
    ShiftRectY(L.btnNext, shift);
    for (int i = 0; i < 4; i++) ShiftRectY(L.sysCards[i], shift);
    ShiftRectY(L.clockCard, shift);
    ShiftRectY(L.worldCard, shift);
    ShiftRectY(L.monthCard, shift);
    ShiftRectY(L.agendaCard, shift);
    L.calendarY += shift;
    L.calWeekY += shift;
    L.calGridY += shift;
    ShiftRectY(L.calPrevRect, shift);
    ShiftRectY(L.calNextRect, shift);
    ShiftRectY(L.calTodayRect, shift);
    L.dockY += delta;
    ShiftRectY(L.dockPill, delta);
    for (int i = 0; i < L.dockCount; i++) ShiftRectY(L.dockRects[i], delta);
    L.totalH = maxTotal;
    return L;
}

// v7: layered windows have no region. The panel shape comes
// from the alpha channel of the presented bitmap, so this is a
// no-op by design (kept so size paths do not need restructuring).
static void ApplyRegion(HWND hwnd, int w, int h, bool expanded)
{
    (void)hwnd; (void)w; (void)h; (void)expanded;
}

// Forward declarations: the D2D present/blur functions live in
// the v7 rendering section below, but size paths call them.
static void PresentNotch(Notch* n);
static void ApplyBlur(Notch* n);

static void ApplyNotchSizeInstant(Notch* n, bool expanded)
{
    if (!n || !n->hwnd) return;
    int dpi = n->dpi;
    int w = expanded ? ExpandedWidth(n) : CollapsedWidth(dpi);
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
    // v7: layered windows present via UpdateLayeredWindow, not
    // via WM_PAINT alone, so present explicitly after sizing.
    PresentNotch(n);
}

// v7 animation: a damped spring (Windhawk-inspired), integrated
// per timer tick, instead of the v3 fixed 220 ms ease-out cubic.
// Stiffness 170 / damping 20 is underdamped (damping ratio
// ~0.77), so the panel settles with a small, natural overshoot
// and then snaps exactly to target. The timer is still per-notch,
// started only for a size change and killed on finish, so an idle
// notch has no animation timer. Position is elapsed-based, so
// the motion does not depend on the timer firing exactly on
// schedule; the 16 ms cadence itself is still timer-limited
// (no timeBeginPeriod, which would be a global side effect).
// A 700 ms cap guarantees the snap even if the spring stalls.
static const int kAnimIntervalMs = 16;
static const int kAnimCapMs = 700;

static void TargetSizeFor(Notch* n, bool expanded, int& outW, int& outH)
{
    int dpi = n->dpi;
    outW = expanded ? ExpandedWidth(n) : CollapsedWidth(dpi);
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
        InvalidateRect(n->hwnd, NULL, FALSE);
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
        InvalidateRect(n->hwnd, NULL, FALSE);
        return;
    }
    if (targetExpanded) n->paintExpanded = true; // collapsing keeps
    // paintExpanded true until the collapse finishes (see Step).
    n->animFromW = fromW; n->animFromH = fromH;
    n->animToW = toW; n->animToH = toH;
    n->animStartTick = GetTickCount64();
    n->animSpringPos = 0.0; n->animSpringVel = 0.0;
    n->animLastTick = GetTickCount64();
    n->animActive = true;
    SetTimer(n->hwnd, ID_TIMER_ANIM, kAnimIntervalMs, NULL);
}

static void StepAnimation(Notch* n)
{
    if (!n || !n->hwnd || !n->animActive) return;
    ULONGLONG now = GetTickCount64();
    ULONGLONG elapsed = now - n->animStartTick;
    double dt = (double)(now - n->animLastTick) / 1000.0;
    n->animLastTick = now;
    if (dt < 0.001) dt = 0.001;
    if (dt > 0.05) dt = 0.05;
    // Damped spring towards 1.0 (target size). Interrupt-reversing
    // falls out naturally: ApplyTargetSize restarts from the
    // current window size with position reset to 0.
    const double kStiffness = 170.0;
    const double kDamping = 20.0;
    double accel = kStiffness * (1.0 - n->animSpringPos) - kDamping * n->animSpringVel;
    n->animSpringVel += accel * dt;
    n->animSpringPos += n->animSpringVel * dt;
    if (elapsed >= (ULONGLONG)kAnimCapMs ||
        (n->animSpringPos >= 0.98 && fabs(n->animSpringVel) < 0.8)) {
        StopAnimation(n);
        n->paintExpanded = n->expanded;
        ApplyNotchSizeInstant(n, n->expanded);
        InvalidateRect(n->hwnd, NULL, FALSE);
        return;
    }
    double pos = n->animSpringPos;
    int w = (int)(n->animFromW + (n->animToW - n->animFromW) * pos + 0.5);
    int h = (int)(n->animFromH + (n->animToH - n->animFromH) * pos + 0.5);
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    int workW = n->workRect.right - n->workRect.left;
    int x = n->workRect.left + (workW - w) / 2;
    if (x < n->workRect.left) x = n->workRect.left;
    SetWindowPos(n->hwnd, HWND_TOPMOST, x, n->workRect.top, w, h,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    ApplyRegion(n->hwnd, w, h, h > Scale(34, n->dpi) + 2);
    // v7: present the new size immediately (layered windows do
    // not paint through the screen DC), then invalidate so any
    // other repaint path stays consistent.
    PresentNotch(n);
    InvalidateRect(n->hwnd, NULL, FALSE);
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
    // v6-fix: the calendar display resets with it.
    if (expanded) { n->activeTab = kTabHome; n->calYear = 0; n->calMonth = 0; n->calDay = 0; }
    ApplyTargetSize(n, expanded);
}

// v7: no RecreateFonts. DirectWrite formats are created per
// frame in PresentNotch from the current notch DPI, so a DPI
// change needs no font object recreation (see WM_DPICHANGED).

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
    AppendCheck(menu, ID_TRAY_BLUR, L"Blur background (experimental)", g_blurEnabled);
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

// v6-fix: FormatDate for an arbitrary date (the agenda's selected
// day), not just today. Weekday is recovered via FILETIME so it is
// correct for any displayed month, not assumed from today.
static void FormatDateFromSt(SYSTEMTIME st, wchar_t* out, size_t count)
{
    static const wchar_t* kDays[] = { L"Sunday", L"Monday", L"Tuesday", L"Wednesday",
        L"Thursday", L"Friday", L"Saturday" };
    static const wchar_t* kMonths[] = { L"January", L"February", L"March", L"April", L"May", L"June",
        L"July", L"August", L"September", L"October", L"November", L"December" };
    FILETIME ft; SystemTimeToFileTime(&st, &ft); FileTimeToSystemTime(&ft, &st);
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

// v7: GDI text helpers are gone. DirectWrite text lives in the
// v7 rendering section (D2DText / D2DTextEllipsis / D2DMeasure).

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
static void SampleDiskNetVolume();
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
    SampleDiskNetVolume();
}

// ---------------------------------------------------------------------------
// v6 local data: Disk, Network, Volume, sparkline histories.
// All sampled on the existing 1-second timer and only while the
// stats widget is on, exactly like CPU/RAM sampling above, so
// idle cost does not grow when everything is off. No network
// calls: network rates come from local interface byte counters.
// ---------------------------------------------------------------------------
static int g_diskPercent = 0;
static double g_diskUsedGB = 0.0, g_diskTotalGB = 0.0;
static bool g_netOk = false;
static wchar_t g_netName[64] = { 0 };
static unsigned long long g_netDownBps = 0, g_netUpBps = 0;
static bool g_havePrevNet = false;
static unsigned long long g_prevNetIn = 0, g_prevNetOut = 0;
static bool g_volOk = false;
static int g_volPercent = 0;
static int g_cpuHist[30] = { 0 }, g_ramHist[30] = { 0 }, g_diskHist[30] = { 0 };
static int g_histCount = 0;

static void PushHist(int* hist, int v)
{
    if (g_histCount < 30) { hist[g_histCount] = v; }
    else {
        for (int i = 1; i < 30; i++) hist[i - 1] = hist[i];
        hist[29] = v;
    }
}

static void SampleDiskNetVolume()
{
    if (!g_widgetStats) return;
    ULARGE_INTEGER freeB = { 0 }, totalB = { 0 }, freeC = { 0 };
    if (GetDiskFreeSpaceExW(L"C:\\", &freeC, &totalB, &freeB) && totalB.QuadPart > 0) {
        g_diskTotalGB = (double)totalB.QuadPart / (1024.0 * 1024.0 * 1024.0);
        g_diskUsedGB = (double)(totalB.QuadPart - freeB.QuadPart) / (1024.0 * 1024.0 * 1024.0);
        g_diskPercent = (int)(100.0 * (double)(totalB.QuadPart - freeB.QuadPart) / (double)totalB.QuadPart + 0.5);
        if (g_diskPercent < 0) g_diskPercent = 0;
        if (g_diskPercent > 100) g_diskPercent = 100;
    }
    // Network: sum byte counters over operational, non-loopback
    // interfaces. Rates are deltas over the 1-second timer tick.
    {
        DWORD tableSize = 0;
        GetIfTable(NULL, &tableSize, TRUE);
        MIB_IFTABLE* table = (MIB_IFTABLE*)malloc(tableSize > 0 ? tableSize : 1);
        g_netOk = false;
        if (table && GetIfTable(table, &tableSize, TRUE) == NO_ERROR) {
            unsigned long long inTotal = 0, outTotal = 0;
            for (DWORD i = 0; i < table->dwNumEntries; i++) {
                MIB_IFROW& row = table->table[i];
                if (row.dwOperStatus != IF_OPER_STATUS_OPERATIONAL) continue;
                if (row.dwType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
                inTotal += row.dwInOctets;
                outTotal += row.dwOutOctets;
                if (!g_netOk && row.dwDescrLen > 0) {
                    char desc[256] = { 0 };
                    DWORD dl = row.dwDescrLen < 255 ? row.dwDescrLen : 255;
                    for (DWORD k = 0; k < dl; k++) desc[k] = (char)row.bDescr[k];
                    MultiByteToWideChar(CP_ACP, 0, desc, -1, g_netName, 64);
                    g_netOk = true;
                }
            }
            if (g_havePrevNet) {
                g_netDownBps = (inTotal >= g_prevNetIn) ? (inTotal - g_prevNetIn) : 0;
                g_netUpBps = (outTotal >= g_prevNetOut) ? (outTotal - g_prevNetOut) : 0;
            }
            g_prevNetIn = inTotal; g_prevNetOut = outTotal; g_havePrevNet = true;
        }
        if (table) free(table);
    }
    // Volume: default playback device level via waveOut, which is
    // plain Win32 and needs no COM. If it fails, the card shows
    // "Volume unavailable" instead of a fake value.
    {
        DWORD vol = 0;
        if (waveOutGetVolume((HWAVEOUT)(UINT_PTR)WAVE_MAPPER, &vol) == MMSYSERR_NOERROR) {
            int left = (int)(vol & 0xFFFF), right = (int)((vol >> 16) & 0xFFFF);
            g_volPercent = ((left + right) / 2) * 100 / 65535;
            g_volOk = true;
        } else {
            g_volOk = false;
        }
    }
    if (g_histCount < 30) g_histCount++;
    PushHist(g_cpuHist, g_cpuPercent);
    PushHist(g_ramHist, g_memPercent);
    PushHist(g_diskHist, g_diskPercent);
}

// v6 world clocks: resolve a Windows timezone key, convert UTC
// now into it, and return the offset from UTC in minutes. If the
// key cannot be found, returns false and the city is omitted.
static bool GetCityTime(const wchar_t* tzKey, SYSTEMTIME* out, int* offsetMin)
{
    SYSTEMTIME utc; GetSystemTime(&utc);
    for (DWORD i = 0; i < 300; i++) {
        DYNAMIC_TIME_ZONE_INFORMATION dtzi = { 0 };
        if (EnumDynamicTimeZoneInformation(i, &dtzi) != ERROR_SUCCESS) break;
        if (wcscmp(dtzi.TimeZoneKeyName, tzKey) != 0) continue;
        SYSTEMTIME local;
        if (!SystemTimeToTzSpecificLocalTimeEx(&dtzi, &utc, &local)) return false;
        FILETIME fu, fl;
        if (!SystemTimeToFileTime(&utc, &fu)) return false;
        if (!SystemTimeToFileTime(&local, &fl)) return false;
        ULARGE_INTEGER u, l;
        u.LowPart = fu.dwLowDateTime; u.HighPart = fu.dwHighDateTime;
        l.LowPart = fl.dwLowDateTime; l.HighPart = fl.dwHighDateTime;
        *offsetMin = (int)(((long long)l.QuadPart - (long long)u.QuadPart) / 600000000LL);
        *out = local;
        return true;
    }
    return false;
}

static void FormatTime12(const SYSTEMTIME& st, wchar_t* out, size_t count)
{
    int h = st.wHour % 12;
    if (h == 0) h = 12;
    swprintf(out, count, L"%d:%02d %s", h, st.wMinute, st.wHour < 12 ? L"AM" : L"PM");
}

static void FormatOffset(int offsetMin, wchar_t* out, size_t count)
{
    int sign = offsetMin < 0 ? -1 : 1;
    int a = offsetMin < 0 ? -offsetMin : offsetMin;
    swprintf(out, count, L"UTC%c%02d:%02d", sign < 0 ? L'-' : L'+', a / 60, a % 60);
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
        Notch* nn = g_notches[i];
        if (!nn->hwnd || nn->hiddenByFullscreen) continue;
        // v6-fix, provably safe dirty-region cut: a collapsed notch
        // shows only the clock (HH:MM), so a 1-second tick that does
        // not change the minute paints nothing new. Expanded notches
        // still repaint every tick (stats, sparklines, SMTC, clock
        // card all move per second) — now into the double buffer,
        // so that repaint no longer flashes.
        if (!nn->expanded) {
            SYSTEMTIME st; GetLocalTime(&st);
            int minute = st.wHour * 60 + st.wMinute;
            if (nn->lastClockMinute == minute) continue;
            nn->lastClockMinute = minute;
        }
        InvalidateRect(nn->hwnd, NULL, FALSE);
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
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED,
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

// v6: dock hit test. Returns the tab id or -1. Same
// paint-vs-click split as media buttons and gear.
static int DockIndexAt(Notch* n, int px, int py, bool usePaint)
{
    if (usePaint ? !n->paintExpanded : !n->expanded) return -1;
    RECT rc; GetClientRect(n->hwnd, &rc);
    Layout L = ComputeLayout(n, rc.right);
    for (int i = 0; i < L.dockCount; i++) {
        RECT dr = L.dockRects[i];
        int cx = (dr.left + dr.right) / 2, cy = (dr.top + dr.bottom) / 2, r = (dr.right - dr.left) / 2 + 2;
        int dx = px - cx, dy = py - cy;
        if (dx * dx + dy * dy <= r * r) return L.dockIds[i];
    }
    return -1;
}

// v6: close X hit test (header right, next to the gear).
static bool CloseHitAt(Notch* n, int px, int py, bool usePaint)
{
    if (usePaint ? !n->paintExpanded : !n->expanded) return false;
    RECT rc; GetClientRect(n->hwnd, &rc);
    Layout L = ComputeLayout(n, rc.right);
    int dx = px - L.closeCX, dy = py - L.closeCY;
    int r = L.closeR + 4;
    return dx * dx + dy * dy <= r * r;
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

// v6-fix calendar helpers. CalSelected: the day the agenda follows.
// 0 (default) means today when the displayed month is the current
// month, else day 1; an explicit day is clamped to the displayed
// month's length. CalCellDate maps a 0..41 grid cell to a real date,
// including faded adjacent-month days.
static int CalSelectedDay(Notch* n, int dispY, int dispM)
{
    SYSTEMTIME now; GetLocalTime(&now);
    if (n && n->calDay > 0) {
        int d = n->calDay;
        int maxD = DaysInMonth(dispY, dispM);
        return d > maxD ? maxD : d;
    }
    if (dispY == now.wYear && dispM == now.wMonth) return now.wDay;
    return 1;
}

static void CalCellDate(const Layout& L, int dispY, int dispM, int cell,
                        int& outY, int& outM, int& outD)
{
    int idx = cell - L.calFirstWeekday;
    if (idx < 0) {
        int py = dispM == 1 ? dispY - 1 : dispY;
        int pm = dispM == 1 ? 12 : dispM - 1;
        outY = py; outM = pm; outD = L.calPrevDays + idx + 1;
    } else if (idx >= L.calDays) {
        int ny = dispM == 12 ? dispY + 1 : dispY;
        int nm = dispM == 12 ? 1 : dispM + 1;
        outY = ny; outM = nm; outD = idx - L.calDays + 1;
    } else {
        outY = dispY; outM = dispM; outD = idx + 1;
    }
}

static bool CalPrevHitAt(Notch* n, int px, int py, bool usePaint)
{
    if (usePaint ? !n->paintExpanded : !n->expanded) return false;
    if (EffectiveTab(n) != kTabCalendar) return false;
    RECT rc; GetClientRect(n->hwnd, &rc);
    Layout L = ComputeLayout(n, rc.right);
    return px >= L.calPrevRect.left && px < L.calPrevRect.right &&
           py >= L.calPrevRect.top && py < L.calPrevRect.bottom;
}

static bool CalNextHitAt(Notch* n, int px, int py, bool usePaint)
{
    if (usePaint ? !n->paintExpanded : !n->expanded) return false;
    if (EffectiveTab(n) != kTabCalendar) return false;
    RECT rc; GetClientRect(n->hwnd, &rc);
    Layout L = ComputeLayout(n, rc.right);
    return px >= L.calNextRect.left && px < L.calNextRect.right &&
           py >= L.calNextRect.top && py < L.calNextRect.bottom;
}

static bool CalTodayHitAt(Notch* n, int px, int py, bool usePaint)
{
    if (usePaint ? !n->paintExpanded : !n->expanded) return false;
    if (EffectiveTab(n) != kTabCalendar) return false;
    RECT rc; GetClientRect(n->hwnd, &rc);
    Layout L = ComputeLayout(n, rc.right);
    return px >= L.calTodayRect.left && px < L.calTodayRect.right &&
           py >= L.calTodayRect.top && py < L.calTodayRect.bottom;
}

static int CalDayHitAt(Notch* n, int px, int py, bool usePaint)
{
    if (usePaint ? !n->paintExpanded : !n->expanded) return -1;
    if (EffectiveTab(n) != kTabCalendar) return -1;
    RECT rc; GetClientRect(n->hwnd, &rc);
    Layout L = ComputeLayout(n, rc.right);
    if (L.calCellW <= 0 || L.calCellH <= 0) return -1;
    int x0 = L.monthCard.left + Scale(14, n->dpi);
    int col = (px - x0) / L.calCellW;
    int row = (py - L.calGridY) / L.calCellH;
    if (col < 0 || col > 6 || row < 0 || row > 5) return -1;
    return row * 7 + col;
}

// ===========================================================================
// v7 Direct2D rendering layer. This section REPLACES the v6 GDI painting
// (memory DC + BitBlt, GDI brushes/pens/fonts, hand-drawn GDI glyphs).
// What changed, and what did not:
//  - The window is now a LAYERED window (WS_EX_LAYERED, see the
//    CreateWindowExW call). Its shape comes from the alpha channel
//    of the bitmap we present with UpdateLayeredWindow, NOT from a
//    window region: ApplyRegion is a no-op in v7, and corners are
//    truly anti-aliased instead of region-stepped. The window is
//    still topmost, toolwindow, and NOACTIVATE (no focus steal).
//  - Each frame is rendered OFFSCREEN into a 32-bit premultiplied
//    DIB with an ID2D1DCRenderTarget (Direct2D), then presented
//    once with UpdateLayeredWindow. Nothing partial is ever shown:
//    this is the v6 double-buffer idea, in D2D terms.
//  - Text is DirectWrite (Segoe UI, same sizes/weights as v6).
//    The render target DPI is fixed at 96, so D2D units are pixels
//    and every Layout coordinate (already DPI-scaled by Scale())
//    maps 1:1, exactly as in v6. Per-monitor DPI still works
//    because Layout and font sizes are scaled by the notch DPI.
//  - Glyphs are Direct2D geometry (paths/arcs), centred in the
//    same hit areas, same hit order gear > X > dock > media > pin.
//  - Cards get a soft shadow: a dark rounded rect offset 2 px
//    below each card, alpha-blended, drawn before the card.
//    The PANEL itself has no external shadow: the window bounds
//    are exactly the panel bounds, so an outside shadow would be
//    clipped by the bitmap. That is a deliberate omission, not
//    a bug to paper over (see README v7 section).
//  - Blur is OFF by default (see ApplyBlur). When on, the panel
//    gradient is drawn at 0.88 alpha so the blur can show
//    through; when off, or if the accent call fails, the panel
//    is fully opaque exactly as in v6. Blur can never produce a
//    black/transparent window here, because the layered bitmap
//    always carries our own drawn content.
//  - Album-art accent tinting is OUT of scope: SMTC gives
//    title/artist, not album art, in our path. Nothing is faked.
//  - This code is ORIGINAL to OnyxNotch, inspired by (not copied
//    from) the Windhawk Dynamic Island mod's approach: native
//    C++, Direct2D, layered/alpha presentation, spring animation.
//    Windhawk code is MIT licensed, but no Windhawk code is in
//    this file; the techniques were reimplemented for our
//    layout, themes, and window manager.
// ===========================================================================
static ID2D1Factory* g_d2dFactory = NULL;
static IDWriteFactory* g_dwFactory = NULL;

static bool InitD2D()
{
    if (!g_d2dFactory) {
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_d2dFactory);
        if (FAILED(hr)) return false;
    }
    if (!g_dwFactory) {
        HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g_dwFactory));
        if (FAILED(hr)) return false;
    }
    return true;
}

static D2D1_COLOR_F D2DColor(COLORREF c, float a)
{
    return D2D1::ColorF((float)GetRValue(c) / 255.0f, (float)GetGValue(c) / 255.0f,
        (float)GetBValue(c) / 255.0f, a);
}

static D2D1_RECT_F RectF(const RECT& r)
{
    return D2D1::RectF((float)r.left, (float)r.top, (float)r.right, (float)r.bottom);
}

static D2D1_RECT_F RectFF(float x, float y, float w, float h)
{
    return D2D1::RectF(x, y, x + w, y + h);
}

// Font kinds, matching the v6 GDI fonts (sizes in 96-DPI points,
// scaled by the notch DPI at format creation, as v6 did).
enum { kFontClock = 0, kFontBig = 1, kFontSmall = 2, kFontTiny = 3 };

struct Canvas {
    ID2D1DCRenderTarget* rt = NULL;
    IDWriteFactory* dw = NULL;
    int dpi = 96;
    IDWriteTextFormat* fmt[4] = { NULL, NULL, NULL, NULL };
    // Small per-frame solid-brush cache (colour+alpha -> brush),
    // released at the end of every frame in ReleaseAll().
    struct BrushEnt { UINT32 key; ID2D1SolidColorBrush* brush; };
    BrushEnt brushes[64];
    int brushCount = 0;

    ID2D1SolidColorBrush* Brush(COLORREF c, float a)
    {
        UINT32 key = (UINT32)c | ((UINT32)(a * 255.0f) << 24);
        for (int i = 0; i < brushCount; i++) if (brushes[i].key == key) return brushes[i].brush;
        if (brushCount >= 64 || !rt) return NULL;
        ID2D1SolidColorBrush* b = NULL;
        if (FAILED(rt->CreateSolidColorBrush(D2DColor(c, a), &b))) return NULL;
        brushes[brushCount].key = key;
        brushes[brushCount].brush = b;
        brushCount++;
        return b;
    }
    void ReleaseAll()
    {
        for (int i = 0; i < brushCount; i++) if (brushes[i].brush) brushes[i].brush->Release();
        brushCount = 0;
        for (int i = 0; i < 4; i++) if (fmt[i]) { fmt[i]->Release(); fmt[i] = NULL; }
    }
    void FillRect(const RECT& r, COLORREF c, float a = 1.0f)
    {
        ID2D1SolidColorBrush* b = Brush(c, a);
        if (b) rt->FillRectangle(RectF(r), b);
    }
    void FillRectF(float x, float y, float w, float h, COLORREF c, float a = 1.0f)
    {
        ID2D1SolidColorBrush* b = Brush(c, a);
        if (b) rt->FillRectangle(RectFF(x, y, w, h), b);
    }
    void FillRounded(const RECT& r, COLORREF c, float radius, float a = 1.0f)
    {
        ID2D1SolidColorBrush* b = Brush(c, a);
        if (b) rt->FillRoundedRectangle(D2D1::RoundedRect(RectF(r), radius, radius), b);
    }
    void StrokeRounded(const RECT& r, COLORREF c, float width, float radius)
    {
        ID2D1SolidColorBrush* b = Brush(c, 1.0f);
        if (b) rt->DrawRoundedRectangle(D2D1::RoundedRect(RectF(r), radius, radius), b, width);
    }
    void FillEllipse(float cx, float cy, float rx, float ry, COLORREF c, float a = 1.0f)
    {
        ID2D1SolidColorBrush* b = Brush(c, a);
        if (b) rt->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), rx, ry), b);
    }
    void StrokeEllipse(float cx, float cy, float rx, float ry, COLORREF c, float width)
    {
        ID2D1SolidColorBrush* b = Brush(c, 1.0f);
        if (b) rt->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), rx, ry), b, width);
    }
    void Line(float x0, float y0, float x1, float y1, COLORREF c, float width)
    {
        ID2D1SolidColorBrush* b = Brush(c, 1.0f);
        if (b) rt->DrawLine(D2D1::Point2F(x0, y0), D2D1::Point2F(x1, y1), b, width);
    }
    void FillPoly(const POINT* pts, int n, COLORREF c)
    {
        if (n < 3 || !g_d2dFactory) return;
        ID2D1PathGeometry* geo = NULL;
        if (FAILED(g_d2dFactory->CreatePathGeometry(&geo))) return;
        ID2D1GeometrySink* sink = NULL;
        if (SUCCEEDED(geo->Open(&sink))) {
            sink->BeginFigure(D2D1::Point2F((float)pts[0].x, (float)pts[0].y), D2D1_FIGURE_BEGIN_FILLED);
            for (int i = 1; i < n; i++) sink->AddLine(D2D1::Point2F((float)pts[i].x, (float)pts[i].y));
            sink->EndFigure(D2D1_FIGURE_END_CLOSED);
            sink->Close();
            sink->Release();
            ID2D1SolidColorBrush* b = Brush(c, 1.0f);
            if (b) rt->FillGeometry(geo, b);
        }
        geo->Release();
    }
    // Stroke an arc of a circle, clockwise from the top, covering
    // pct percent (0 = nothing, 100 = full circle). Used by rings.
    void StrokeArc(float cx, float cy, float r, int pct, COLORREF c, float width)
    {
        if (pct <= 0) return;
        ID2D1SolidColorBrush* b = Brush(c, 1.0f);
        if (!b) return;
        if (pct >= 100) { rt->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), r, r), b, width); return; }
        if (!g_d2dFactory) return;
        const double kPi = 3.141592653589793;
        double endA = -kPi / 2.0 + (double)pct * 2.0 * kPi / 100.0;
        D2D1_POINT_2F start = D2D1::Point2F(cx, cy - r);
        D2D1_POINT_2F end = D2D1::Point2F((float)(cx + r * cos(endA)), (float)(cy + r * sin(endA)));
        ID2D1PathGeometry* geo = NULL;
        if (FAILED(g_d2dFactory->CreatePathGeometry(&geo))) return;
        ID2D1GeometrySink* sink = NULL;
        if (SUCCEEDED(geo->Open(&sink))) {
            sink->BeginFigure(start, D2D1_FIGURE_BEGIN_HOLLOW);
            sink->AddArc(D2D1::ArcSegment(end, D2D1::SizeF(r, r), 0.0f,
                pct > 50 ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_CLOCKWISE,
                pct > 50 ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL));
            sink->EndFigure(D2D1_FIGURE_END_OPEN);
            sink->Close();
            sink->Release();
            rt->DrawGeometry(geo, b, width);
        }
        geo->Release();
    }
};

// --- DirectWrite text helpers (replace GDI DrawText) -----------------------
static int FontSizeFor(int kind, int dpi)
{
    int base = (kind == kFontClock) ? 20 : (kind == kFontBig) ? 36 : (kind == kFontSmall) ? 17 : 14;
    return Scale(base, dpi);
}

static void D2DText(Canvas* cv, int kind, const wchar_t* text,
                    float x, float y, float w, float h, COLORREF color, bool centred)
{
    if (!cv || !cv->rt || !cv->fmt[kind] || !text) return;
    ID2D1SolidColorBrush* b = cv->Brush(color, 1.0f);
    if (!b) return;
    cv->fmt[kind]->SetTextAlignment(centred ? DWRITE_TEXT_ALIGNMENT_CENTER : DWRITE_TEXT_ALIGNMENT_LEADING);
    cv->fmt[kind]->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    cv->rt->DrawText(text, (UINT32)wcslen(text), cv->fmt[kind],
        D2D1::RectF(x, y, x + w, y + h), b);
}

// Ellipsis by measurement, not by an obscure trimming API: shorten
// the string until it plus an ellipsis fits the given width.
static void D2DTextEllipsis(Canvas* cv, int kind, const wchar_t* text,
                            float x, float y, float w, float h, COLORREF color)
{
    if (!cv || !cv->dw || !cv->fmt[kind] || !text) return;
    // Measure, and while too wide, drop the last non-ellipsis
    // character and append an ellipsis.
    {
        wchar_t out[256];
        wcsncpy_s(out, 256, text, _TRUNCATE);
        for (int guard = 0; guard < 300; guard++) {
            IDWriteTextLayout* lay = NULL;
            if (FAILED(cv->dw->CreateTextLayout(out, (UINT32)wcslen(out), cv->fmt[kind], 10000.0f, 10000.0f, &lay))) break;
            DWRITE_TEXT_METRICS m = { 0 };
            lay->GetMetrics(&m);
            lay->Release();
            if (m.width <= w) break;
            size_t len = wcslen(out);
            if (len <= 1) { out[0] = L'\x2026'; out[1] = L'\0'; break; }
            if (out[len - 1] == L'\x2026') out[len - 2] = L'\x2026', out[len - 1] = L'\0';
            else { out[len - 1] = L'\x2026'; out[len] = L'\0'; }
        }
        D2DText(cv, kind, out, x, y, w, h, color, false);
    }
}

static float D2DMeasure(Canvas* cv, int kind, const wchar_t* text)
{
    if (!cv || !cv->dw || !cv->fmt[kind] || !text) return 0.0f;
    IDWriteTextLayout* lay = NULL;
    if (FAILED(cv->dw->CreateTextLayout(text, (UINT32)wcslen(text), cv->fmt[kind], 10000.0f, 10000.0f, &lay))) return 0.0f;
    DWRITE_TEXT_METRICS m = { 0 };
    lay->GetMetrics(&m);
    lay->Release();
    return m.width;
}

// --- Glyphs, redrawn as Direct2D geometry ----------------------------------
static void DrawGearD2D(Canvas* cv, int cx, int cy, int r, COLORREF color)
{
    const double kPi = 3.141592653589793;
    int toothW = r * 11 / 20; if (toothW < 3) toothW = 3;
    int toothH = r * 9 / 20; if (toothH < 2) toothH = 2;
    for (int i = 0; i < 8; i++) {
        double a = (double)i * kPi / 4.0;
        double ca = cos(a), sa = sin(a);
        double hx = (double)r, hw = (double)toothH / 2.0, hh = (double)toothW / 2.0;
        double corners[4][2] = { { hx - hh, -hw }, { hx + hh, -hw }, { hx + hh, hw }, { hx - hh, hw } };
        POINT pts[4];
        for (int c = 0; c < 4; c++) {
            double x = corners[c][0], y = corners[c][1];
            pts[c].x = (LONG)(cx + x * ca - y * sa + 0.5);
            pts[c].y = (LONG)(cy + x * sa + y * ca + 0.5);
        }
        cv->FillPoly(pts, 4, color);
    }
    cv->FillEllipse((float)cx, (float)cy, (float)r, (float)r, color);
    int hr = r * 2 / 5; if (hr < 2) hr = 2;
    cv->FillEllipse((float)cx, (float)cy, (float)hr, (float)hr, CurrentTheme().gearHole);
}

static void DrawMediaGlyphD2D(Canvas* cv, int idx, int cx, int cy, int r, COLORREF color, int playPauseMode)
{
    int h = (2 * r) * 14 / 20; // 70% of the diameter, as in v6-fix
    int half = h / 2;
    if (idx == 0 || idx == 2) {
        int dir = (idx == 0) ? -1 : 1;
        int barW = h / 5; if (barW < 3) barW = 3;
        int triW = h / 2;
        int gap = 1;
        int totalW = barW + gap + 2 * triW + gap;
        int x0 = cx - totalW / 2;
        if (dir < 0) {
            RECT bar = { x0, cy - half, x0 + barW, cy + half };
            cv->FillRect(bar, color);
            POINT t1[3] = { { x0 + barW + gap + triW, cy - half }, { x0 + barW + gap + triW, cy + half }, { x0 + barW + gap, cy } };
            POINT t2[3] = { { x0 + barW + gap + 2 * triW + gap, cy - half }, { x0 + barW + gap + 2 * triW + gap, cy + half }, { x0 + barW + gap + triW + gap, cy } };
            cv->FillPoly(t1, 3, color);
            cv->FillPoly(t2, 3, color);
        } else {
            POINT t1[3] = { { x0, cy - half }, { x0, cy + half }, { x0 + triW, cy } };
            POINT t2[3] = { { x0 + triW + gap, cy - half }, { x0 + triW + gap, cy + half }, { x0 + 2 * triW + gap, cy } };
            RECT bar = { x0 + 2 * triW + 2 * gap, cy - half, x0 + 2 * triW + 2 * gap + barW, cy + half };
            cv->FillPoly(t1, 3, color);
            cv->FillPoly(t2, 3, color);
            cv->FillRect(bar, color);
        }
    } else if (playPauseMode == 1) {
        int barW = h / 5; if (barW < 3) barW = 3;
        int gap = h / 6; if (gap < 2) gap = 2;
        int totalW = 2 * barW + gap;
        int x0 = cx - totalW / 2;
        cv->FillRect(RECT{ x0, cy - half, x0 + barW, cy + half }, color);
        cv->FillRect(RECT{ x0 + barW + gap, cy - half, x0 + 2 * barW + gap, cy + half }, color);
    } else if (playPauseMode == 2) {
        int triW = h / 2;
        int x0 = cx - triW / 2;
        POINT tri[3] = { { x0, cy - half }, { x0, cy + half }, { x0 + triW, cy } };
        cv->FillPoly(tri, 3, color);
    } else {
        int triW = h / 2;
        int barW = h / 5; if (barW < 3) barW = 3;
        int gap = h / 6; if (gap < 2) gap = 2;
        int totalW = triW + gap + 2 * barW + gap;
        int x0 = cx - totalW / 2;
        POINT tri[3] = { { x0, cy - half }, { x0, cy + half }, { x0 + triW, cy } };
        cv->FillPoly(tri, 3, color);
        cv->FillRect(RECT{ x0 + triW + gap, cy - half, x0 + triW + gap + barW, cy + half }, color);
        cv->FillRect(RECT{ x0 + triW + gap + barW + gap, cy - half, x0 + triW + gap + 2 * barW + gap, cy + half }, color);
    }
}

// --- Surface, border, battery, cards ---------------------------------------
static void PaintSurfaceD2D(Canvas* cv, Notch* n, const RECT& rc, float panelAlpha)
{
    if (!cv || !cv->rt) return;
    const Theme& th = CurrentTheme();
    ID2D1GradientStopCollection* stops = NULL;
    D2D1_GRADIENT_STOP gs[2] = {
        { 0.0f, D2DColor(th.gradTop, panelAlpha) },
        { 1.0f, D2DColor(th.gradBottom, panelAlpha) }
    };
    bool painted = false;
    if (SUCCEEDED(cv->rt->CreateGradientStopCollection(gs, 2, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &stops))) {
        ID2D1LinearGradientBrush* gb = NULL;
        D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES bp = D2D1::LinearGradientBrushProperties(
            D2D1::Point2F((float)rc.left, (float)rc.top),
            D2D1::Point2F((float)rc.left, (float)rc.bottom));
        if (SUCCEEDED(cv->rt->CreateLinearGradientBrush(&bp, stops, &gb))) {
            cv->rt->FillRoundedRectangle(D2D1::RoundedRect(RectF(rc), 14.0f, 14.0f), gb);
            gb->Release();
            painted = true;
        }
        stops->Release();
    }
    // Fallback: if gradient creation ever fails, the panel must
    // still be opaque, never transparent or black.
    if (!painted) cv->FillRounded(rc, th.gradTop, 14.0f, panelAlpha);
    (void)n;
}

static void PaintBorderD2D(Canvas* cv, const RECT& rc, int dpi, bool hover, Notch* n)
{
    COLORREF border = hover ? CurrentTheme().borderHover : CurrentTheme().border;
    cv->StrokeRounded(rc, border, 1.0f, 14.0f);
    int inset = Scale(20, dpi);
    int y0 = Scale(1, dpi), y1 = Scale(2, dpi);
    if (rc.right - inset > inset) {
        cv->FillRectF((float)inset, (float)y0, (float)(rc.right - 2 * inset), (float)(y1 - y0), CurrentTheme().highlight);
    }
    (void)n;
}

static void DrawBatteryGlyphD2D(Canvas* cv, int x, int y, int w, int h, int pct, bool charging, bool known)
{
    const Theme& th = CurrentTheme();
    cv->StrokeRounded(RECT{ x, y, x + w, y + h }, th.batteryOutline, 1.0f, 2.0f);
    int tipW = (w / 8 < 2) ? 2 : w / 8;
    int tipH = h / 2;
    cv->FillRect(RECT{ x + w, y + (h - tipH) / 2, x + w + tipW, y + (h + tipH) / 2 }, th.batteryOutline);
    if (!known) return;
    int innerX = x + 2, innerY = y + 2, innerW = w - 4, innerH = h - 4;
    if (innerW > 0 && innerH > 0) {
        int fillW = innerW * pct / 100;
        if (fillW > 0) cv->FillRect(RECT{ innerX, innerY, innerX + fillW, innerY + innerH }, th.batteryFill);
    }
    if (charging) {
        int cx = x + w / 2, cy = y + h / 2, bh = h * 3 / 4, bw = bh / 2;
        POINT bolt[6] = {
            { cx + bw / 2, cy - bh / 2 }, { cx - bw / 2, cy + 1 },
            { cx, cy + 1 }, { cx - bw / 2, cy + bh / 2 },
            { cx + bw / 2, cy - 1 }, { cx, cy - 1 }
        };
        cv->FillPoly(bolt, 6, th.batteryBolt);
    }
}

static COLORREF CardColorD2D()
{
    const Theme& th = CurrentTheme();
    int r = GetRValue(th.gradTop) + 14; if (r > 255) r = 255;
    int g = GetGValue(th.gradTop) + 14; if (g > 255) g = 255;
    int b = GetBValue(th.gradTop) + 14; if (b > 255) b = 255;
    return RGB(r, g, b);
}

static void DrawCardD2D(Canvas* cv, const RECT& rc, const wchar_t* label)
{
    // Soft shadow first: dark rounded rect 2 px below the card.
    RECT shadow = rc; shadow.top += 2; shadow.bottom += 2;
    cv->FillRounded(shadow, RGB(0, 0, 0), 9.0f, 0.30f);
    cv->FillRounded(rc, CardColorD2D(), 9.0f, 1.0f);
    if (label) D2DText(cv, kFontTiny, label, (float)rc.left + 14.0f, (float)rc.top + 10.0f,
        (float)(rc.right - rc.left - 28), 16.0f, CurrentTheme().muted, false);
}

static void DrawRingD2D(Canvas* cv, int cx, int cy, int r, int pct, int dpi)
{
    const Theme& th = CurrentTheme();
    cv->StrokeEllipse((float)cx, (float)cy, (float)r, (float)r, th.divider, (float)Scale(6, dpi));
    if (pct > 0) cv->StrokeArc((float)cx, (float)cy, (float)r, pct, th.accent, (float)Scale(6, dpi));
}

static void DrawSparklineD2D(Canvas* cv, const RECT& rc, const int* hist, int count, COLORREF color)
{
    if (count < 2) return;
    int w = rc.right - rc.left, h = rc.bottom - rc.top;
    for (int i = 1; i < count; i++) {
        float x0 = (float)rc.left + (float)(i - 1) * w / (count - 1);
        float x1 = (float)rc.left + (float)i * w / (count - 1);
        float y0 = (float)rc.bottom - (float)hist[i - 1] * h / 100.0f;
        float y1 = (float)rc.bottom - (float)hist[i] * h / 100.0f;
        cv->Line(x0, y0, x1, y1, color, 1.0f);
    }
}

static void DrawBarD2D(Canvas* cv, const RECT& rc, int pct)
{
    cv->FillRect(rc, CurrentTheme().divider);
    if (pct > 0) {
        RECT fill = rc;
        fill.right = rc.left + (rc.right - rc.left) * pct / 100;
        if (fill.right > fill.left) cv->FillRect(fill, CurrentTheme().accent);
    }
}

static void DrawDockGlyphD2D(Canvas* cv, int tabId, int cx, int cy, int r, COLORREF color)
{
    int ir = r * 3 / 4; if (ir < 4) ir = 4;
    int stroke = ir / 5; if (stroke < 2) stroke = 2;
    if (tabId == kTabHome) {
        POINT roof[3] = { { cx, cy - ir }, { cx - ir, cy - ir / 4 }, { cx + ir, cy - ir / 4 } };
        cv->FillPoly(roof, 3, color);
        cv->FillRect(RECT{ cx - ir * 2 / 3, cy - ir / 4, cx + ir * 2 / 3, cy + ir }, color);
    } else if (tabId == kTabSystem) {
        DrawGearD2D(cv, cx, cy, ir, color);
    } else if (tabId == kTabMedia) {
        POINT tri[3] = { { cx - ir / 2, cy - ir * 2 / 3 }, { cx - ir / 2, cy + ir * 2 / 3 }, { cx + ir * 2 / 3, cy } };
        cv->FillPoly(tri, 3, color);
    } else if (tabId == kTabClock) {
        cv->StrokeEllipse((float)cx, (float)cy, (float)ir, (float)ir, color, (float)stroke);
        cv->Line((float)cx, (float)cy, (float)cx, (float)(cy - ir / 2), color, (float)stroke);
        cv->Line((float)cx, (float)cy, (float)(cx + ir / 2), (float)cy, color, (float)stroke);
    } else if (tabId == kTabCalendar) {
        RECT cal = { cx - ir, cy - ir * 3 / 4, cx + ir, cy + ir };
        cv->StrokeRounded(cal, color, 1.0f, 2.0f);
        cv->FillRect(RECT{ cal.left, cal.top, cal.right, cal.top + ir / 3 }, color);
    }
}

static void DrawMediaButtonsRowD2D(Canvas* cv, Notch* n, const Layout& L, bool home)
{
    for (int i = 0; i < 3; i++) {
        int cx, cy, r;
        if (home) HomeMediaButtonCircle(L, i, cx, cy, r);
        else MediaButtonCircle(L, i, cx, cy, r);
        COLORREF bg = CurrentTheme().btnBg;
        if (i == n->pressedBtn) bg = CurrentTheme().btnPressed;
        else if (i == n->hoverBtn) bg = CurrentTheme().btnHover;
        cv->FillEllipse((float)cx, (float)cy, (float)r, (float)r, bg);
    }
    for (int i = 0; i < 3; i++) {
        int cx, cy, r;
        if (home) HomeMediaButtonCircle(L, i, cx, cy, r);
        else MediaButtonCircle(L, i, cx, cy, r);
        DrawMediaGlyphD2D(cv, i, cx, cy, r, CurrentTheme().glyph, SmtcPlayPauseGlyphMode());
    }
}

// --- PaintNotch, Direct2D version -------------------------------------------
// Layout, tab, calendar, SMTC, and hit-test logic are UNCHANGED from
// v6: only the drawing calls differ. Panel alpha is 1.0 normally,
// 0.88 when blur is active (see ApplyBlur), so blur can show
// through the panel instead of being hidden behind opaque paint.
static void PaintNotchD2D(Notch* n, Canvas* cv)
{
    RECT rc; GetClientRect(n->hwnd, &rc);
    float panelAlpha = n->blurActive ? 0.88f : 1.0f;
    PaintSurfaceD2D(cv, n, rc, panelAlpha);
    wchar_t clockText[32]; FormatClock(clockText, 32);
    int dpi = n->dpi;
    const Theme& th = CurrentTheme();
    if (!n->paintExpanded) {
        D2DText(cv, kFontClock, clockText, 0, 0, (float)rc.right, (float)rc.bottom, th.primary, true);
        PaintBorderD2D(cv, rc, dpi, n->hoverNotch, n);
        return;
    }
    Layout L = ComputeLayout(n, rc.right);
    int effTab = EffectiveTab(n);
    static const wchar_t* kTabTitles[5] = { L"Home", L"Media", L"System", L"Calendar", L"Clock" };
    wchar_t subtitle[128] = { 0 };
    if (effTab == kTabHome) { FormatDate(subtitle, 128); }
    else if (effTab == kTabSystem) { wcscpy_s(subtitle, 128, L"Power \x00B7 Network \x00B7 Audio"); }
    else if (effTab == kTabClock) { wcscpy_s(subtitle, 128, L"Local Time & Date"); }
    else if (effTab == kTabCalendar) {
        int dy = 0, dm = 0; CalDisplayed(n, dy, dm);
        static const wchar_t* kMonths[] = { L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December" };
        swprintf(subtitle, 128, L"%s %d", kMonths[(dm - 1) % 12], dy);
    } else if (effTab == kTabMedia) {
        if (g_smtcHasTrack && g_smtcArtist[0] != L'\0') wcsncpy_s(subtitle, 128, g_smtcArtist, _TRUNCATE);
        else wcscpy_s(subtitle, 128, L"Media controls");
    }
    int margin = Scale(20, dpi);
    D2DText(cv, kFontSmall, kTabTitles[effTab], (float)margin, (float)Scale(8, dpi), (float)rc.right / 2.0f, (float)Scale(24, dpi), th.primary, false);
    D2DText(cv, kFontTiny, subtitle, (float)margin, (float)Scale(30, dpi), (float)rc.right / 2.0f, (float)Scale(20, dpi), th.secondary, false);
    {
        int pct = 0; bool charging = false; bool known = BatteryInfo(pct, charging);
        wchar_t batt[32];
        if (known) swprintf(batt, 32, L"%d%%", pct); else wcscpy_s(batt, 32, L"--");
        float tw = D2DMeasure(cv, kFontTiny, batt);
        int glyphW = Scale(24, dpi), glyphH = Scale(13, dpi), gap = Scale(6, dpi);
        int textX = (int)(L.gearCX - L.gearR - Scale(12, dpi) - gap - glyphW - gap - tw);
        if (textX < rc.right / 2) textX = rc.right / 2;
        D2DText(cv, kFontTiny, batt, (float)textX, (float)(L.headerH / 2 - Scale(10, dpi)), tw, (float)Scale(20, dpi), th.secondary, false);
        DrawBatteryGlyphD2D(cv, (int)(textX + tw) + gap, L.headerH / 2 - glyphH / 2, glyphW, glyphH, pct, charging, known);
    }
    {
        COLORREF gearColor = th.gear;
        if (n->pressedGear) gearColor = th.gearPressed;
        else if (n->hoverGear) gearColor = th.gearHover;
        DrawGearD2D(cv, L.gearCX, L.gearCY, L.gearR, gearColor);
        int xw = L.closeR / 4; if (xw < 2) xw = 2;
        int r = L.closeR;
        cv->Line((float)(L.closeCX - r), (float)(L.closeCY - r), (float)(L.closeCX + r), (float)(L.closeCY + r), n->hoverClose ? th.primary : th.secondary, (float)xw);
        cv->Line((float)(L.closeCX + r), (float)(L.closeCY - r), (float)(L.closeCX - r), (float)(L.closeCY + r), n->hoverClose ? th.primary : th.secondary, (float)xw);
    }

    SYSTEMTIME now; GetLocalTime(&now);
    if (effTab == kTabHome) {
        for (int i = 0; i < L.homeCardCount; i++) {
            RECT c = L.homeCards[i];
            int id = L.homeCardIds[i];
            if (id == 0) {
                DrawCardD2D(cv, c, L"SYSTEM");
                int vals[3] = { g_cpuPercent, g_memPercent, g_diskPercent };
                static const wchar_t* names[3] = { L"CPU", L"RAM", L"DISK" };
                int r = Scale(26, dpi);
                int totalW = 3 * (2 * r) + 2 * Scale(16, dpi);
                int x0 = c.left + (c.right - c.left - totalW) / 2;
                int cy = c.top + Scale(62, dpi);
                for (int k = 0; k < 3; k++) {
                    int cx = x0 + k * (2 * r + Scale(16, dpi)) + r;
                    DrawRingD2D(cv, cx, cy, r, vals[k], dpi);
                    wchar_t pv[16]; swprintf(pv, 16, L"%d%%", vals[k]);
                    D2DText(cv, kFontTiny, pv, (float)(cx - r), (float)(cy - Scale(10, dpi)), (float)(2 * r), (float)Scale(20, dpi), th.primary, true);
                    D2DText(cv, kFontTiny, names[k], (float)(cx - r), (float)(cy + r + Scale(4, dpi)), (float)(2 * r), (float)Scale(16, dpi), th.muted, true);
                }
            } else if (id == 1) {
                DrawCardD2D(cv, c, L"CALENDAR");
                FILETIME ft; SystemTimeToFileTime(&now, &ft);
                ULARGE_INTEGER base; base.LowPart = ft.dwLowDateTime; base.HighPart = ft.dwHighDateTime;
                int cellW = (c.right - c.left - Scale(28, dpi)) / 7;
                int cellH = Scale(20, dpi);
                int y = c.top + Scale(56, dpi);
                for (int cc = 0; cc < 7; cc++) {
                    ULARGE_INTEGER v; v.QuadPart = base.QuadPart + (ULONGLONG)((LONGLONG)(cc - now.wDayOfWeek) * 864000000000LL);
                    FILETIME cft; cft.dwLowDateTime = v.LowPart; cft.dwHighDateTime = v.HighPart;
                    SYSTEMTIME cst; FileTimeToSystemTime(&cft, &cst);
                    int x = c.left + Scale(14, dpi) + cc * cellW;
                    bool isToday = (cc == now.wDayOfWeek);
                    if (isToday) {
                        cv->FillRounded(RECT{ x + 2, y + 1, x + cellW - 2, y + cellH - 1 }, th.accent, (float)cellH / 2.0f);
                    }
                    wchar_t dayText[8]; swprintf(dayText, 8, L"%d", cst.wDay);
                    D2DText(cv, kFontTiny, dayText, (float)x, (float)y, (float)cellW, (float)cellH, isToday ? th.pillText : th.secondary, true);
                }
                wchar_t weekLabel[64]; FormatDate(weekLabel, 64);
                D2DText(cv, kFontTiny, weekLabel, (float)(c.left + 14), (float)(c.top + Scale(84, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, true);
            } else if (id == 2) {
                DrawCardD2D(cv, c, L"CLOCK");
                wchar_t t12[32]; FormatTime12(now, t12, 32);
                D2DText(cv, kFontBig, t12, (float)c.left, (float)(c.top + Scale(36, dpi)), (float)(c.right - c.left), (float)Scale(48, dpi), th.primary, true);
                wchar_t dt[64]; FormatDate(dt, 64);
                D2DText(cv, kFontTiny, dt, (float)(c.left + 14), (float)(c.top + Scale(92, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, true);
            } else if (id == 3) {
                DrawCardD2D(cv, c, L"MEDIA");
                if (g_smtcHasTrack) {
                    D2DTextEllipsis(cv, kFontTiny, g_smtcTitle, (float)(c.left + 14), (float)L.homeMediaTitleY, (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.primary);
                    if (g_smtcArtist[0] != L'\0') D2DTextEllipsis(cv, kFontTiny, g_smtcArtist, (float)(c.left + 14), (float)(L.homeMediaTitleY + Scale(20, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary);
                } else {
                    D2DText(cv, kFontTiny, L"No track", (float)(c.left + 14), (float)L.homeMediaTitleY, (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, false);
                }
                DrawMediaButtonsRowD2D(cv, n, L, true);
            } else if (id == 4) {
                DrawCardD2D(cv, c, L"BATTERY");
                wchar_t bt[64]; FormatBattery(bt, 64);
                int pct = 0; bool ch = false; bool known = BatteryInfo(pct, ch);
                wchar_t big[16]; if (known) swprintf(big, 16, L"%d%%", pct); else wcscpy_s(big, 16, L"--");
                D2DText(cv, kFontBig, big, (float)c.left, (float)(c.top + Scale(36, dpi)), (float)(c.right - c.left), (float)Scale(48, dpi), th.primary, true);
                D2DText(cv, kFontTiny, bt, (float)(c.left + 14), (float)(c.top + Scale(92, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, true);
                if (known) { RECT bar = { c.left + 14, c.bottom - Scale(18, dpi), c.right - 14, c.bottom - Scale(12, dpi) }; DrawBarD2D(cv, bar, pct); }
            } else if (id == 5) {
                DrawCardD2D(cv, c, L"NETWORK");
                if (g_netOk) {
                    D2DTextEllipsis(cv, kFontTiny, g_netName, (float)(c.left + 14), (float)(c.top + Scale(36, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.primary);
                    wchar_t rates[96];
                    swprintf(rates, 96, L"\x2193 %llu B/s  \x2191 %llu B/s", g_netDownBps, g_netUpBps);
                    D2DText(cv, kFontTiny, rates, (float)(c.left + 14), (float)(c.top + Scale(62, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, false);
                } else {
                    D2DText(cv, kFontTiny, L"Network unavailable", (float)(c.left + 14), (float)(c.top + Scale(36, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, false);
                }
            }
        }
    } else if (effTab == kTabSystem) {
        DrawCardD2D(cv, L.sysCards[0], L"BATTERY");
        {
            RECT c = L.sysCards[0];
            int pct = 0; bool ch = false; bool known = BatteryInfo(pct, ch);
            wchar_t big[16]; if (known) swprintf(big, 16, L"%d%%", pct); else wcscpy_s(big, 16, L"--");
            wchar_t bt[64]; FormatBattery(bt, 64);
            D2DText(cv, kFontBig, big, (float)(c.left + 14), (float)(c.top + Scale(36, dpi)), (float)(c.right - c.left - 28), (float)Scale(48, dpi), th.primary, false);
            D2DText(cv, kFontTiny, bt, (float)(c.left + 14), (float)(c.top + Scale(92, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, false);
            if (known) { RECT bar = { c.left + 14, c.bottom - Scale(20, dpi), c.right - 14, c.bottom - Scale(12, dpi) }; DrawBarD2D(cv, bar, pct); }
        }
        DrawCardD2D(cv, L.sysCards[1], L"NETWORK");
        {
            RECT c = L.sysCards[1];
            if (g_netOk) {
                D2DTextEllipsis(cv, kFontSmall, g_netName, (float)(c.left + 14), (float)(c.top + Scale(36, dpi)), (float)(c.right - c.left - 28), (float)Scale(24, dpi), th.primary);
                wchar_t rates[96];
                swprintf(rates, 96, L"\x2193 %llu B/s", g_netDownBps);
                D2DText(cv, kFontTiny, rates, (float)(c.left + 14), (float)(c.top + Scale(68, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, false);
                swprintf(rates, 96, L"\x2191 %llu B/s", g_netUpBps);
                D2DText(cv, kFontTiny, rates, (float)(c.left + 14), (float)(c.top + Scale(92, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, false);
            } else {
                D2DText(cv, kFontTiny, L"Network unavailable", (float)(c.left + 14), (float)(c.top + Scale(36, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, false);
            }
        }
        DrawCardD2D(cv, L.sysCards[2], L"PERFORMANCE");
        {
            RECT c = L.sysCards[2];
            int vals[3] = { g_cpuPercent, g_memPercent, g_diskPercent };
            const int* hists[3] = { g_cpuHist, g_ramHist, g_diskHist };
            static const wchar_t* names[3] = { L"CPU", L"RAM", L"DISK" };
            for (int k = 0; k < 3; k++) {
                int y = c.top + Scale(36, dpi) + k * Scale(38, dpi);
                wchar_t row[48]; swprintf(row, 48, L"%s  %d%%", names[k], vals[k]);
                D2DText(cv, kFontTiny, row, (float)(c.left + 14), (float)y, (float)Scale(90, dpi), (float)Scale(20, dpi), th.secondary, false);
                RECT sp = { c.left + Scale(110, dpi), y, c.right - 14, y + Scale(18, dpi) };
                DrawSparklineD2D(cv, sp, hists[k], g_histCount, th.accent);
            }
        }
        DrawCardD2D(cv, L.sysCards[3], L"VOLUME");
        {
            RECT c = L.sysCards[3];
            if (g_volOk) {
                wchar_t big[16]; swprintf(big, 16, L"%d%%", g_volPercent);
                D2DText(cv, kFontBig, big, (float)(c.left + 14), (float)(c.top + Scale(36, dpi)), (float)(c.right - c.left - 28), (float)Scale(48, dpi), th.primary, false);
                RECT track = { c.left + 14, c.top + Scale(100, dpi), c.right - 14, c.top + Scale(108, dpi) };
                DrawBarD2D(cv, track, g_volPercent);
                int knobX = track.left + (track.right - track.left) * g_volPercent / 100;
                int knobY = (track.top + track.bottom) / 2;
                int kr = Scale(8, dpi);
                cv->FillEllipse((float)knobX, (float)knobY, (float)kr, (float)kr, th.primary);
            } else {
                D2DText(cv, kFontTiny, L"Volume unavailable", (float)(c.left + 14), (float)(c.top + Scale(36, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, false);
            }
        }
    } else if (effTab == kTabClock) {
        DrawCardD2D(cv, L.clockCard, L"CLOCK");
        {
            RECT c = L.clockCard;
            wchar_t t12[32]; FormatTime12(now, t12, 32);
            D2DText(cv, kFontBig, t12, (float)c.left, (float)(c.top + Scale(40, dpi)), (float)(c.right - c.left), (float)Scale(48, dpi), th.primary, true);
            wchar_t dt[64]; FormatDate(dt, 64);
            D2DText(cv, kFontTiny, dt, (float)(c.left + 14), (float)(c.top + Scale(100, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.secondary, true);
            D2DText(cv, kFontTiny, L"Local", (float)(c.left + 14), (float)(c.top + Scale(124, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.muted, true);
        }
        DrawCardD2D(cv, L.worldCard, L"WORLD CLOCKS");
        {
            RECT c = L.worldCard;
            struct City { const wchar_t* name; const wchar_t* key; };
            static const City cities[3] = { { L"New York", L"Eastern Standard Time" }, { L"London", L"GMT Standard Time" }, { L"Tokyo", L"Tokyo Standard Time" } };
            int y = c.top + Scale(36, dpi);
            wchar_t t12[32]; FormatTime12(now, t12, 32);
            D2DText(cv, kFontTiny, L"Local", (float)(c.left + 14), (float)y, (float)Scale(120, dpi), (float)Scale(20, dpi), th.secondary, false);
            D2DText(cv, kFontTiny, t12, (float)(c.left + Scale(140, dpi)), (float)y, (float)(c.right - c.left - Scale(154, dpi)), (float)Scale(20, dpi), th.primary, false);
            y += Scale(28, dpi);
            for (int k = 0; k < 3; k++) {
                SYSTEMTIME ct; int off = 0;
                if (!GetCityTime(cities[k].key, &ct, &off)) continue;
                wchar_t ct12[32]; FormatTime12(ct, ct12, 32);
                wchar_t offText[32]; FormatOffset(off, offText, 32);
                D2DText(cv, kFontTiny, cities[k].name, (float)(c.left + 14), (float)y, (float)Scale(120, dpi), (float)Scale(20, dpi), th.secondary, false);
                D2DText(cv, kFontTiny, ct12, (float)(c.left + Scale(140, dpi)), (float)y, (float)Scale(110, dpi), (float)Scale(20, dpi), th.primary, false);
                D2DText(cv, kFontTiny, offText, (float)(c.right - Scale(90, dpi)), (float)y, (float)Scale(76, dpi), (float)Scale(20, dpi), th.muted, false);
                y += Scale(28, dpi);
            }
        }
    } else if (effTab == kTabCalendar) {
        DrawCardD2D(cv, L.monthCard, L"CALENDAR");
        {
            RECT c = L.monthCard;
            int dispY = 0, dispM = 0; CalDisplayed(n, dispY, dispM);
            int selD = CalSelectedDay(n, dispY, dispM);
            static const wchar_t* kMonths[] = { L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December" };
            wchar_t monthText[64]; swprintf(monthText, 64, L"%s %d", kMonths[(dispM - 1) % 12], dispY);
            D2DText(cv, kFontSmall, monthText, (float)c.left, (float)(c.top + Scale(10, dpi)), (float)(c.right - c.left), (float)Scale(28, dpi), th.primary, true);
            COLORREF prevC = (n->hoverCalArrow == -1) ? th.primary : th.secondary;
            COLORREF nextC = (n->hoverCalArrow == 1) ? th.primary : th.secondary;
            D2DText(cv, kFontSmall, L"<", (float)L.calPrevRect.left, (float)L.calPrevRect.top, (float)(L.calPrevRect.right - L.calPrevRect.left), (float)(L.calPrevRect.bottom - L.calPrevRect.top), prevC, true);
            D2DText(cv, kFontSmall, L">", (float)L.calNextRect.left, (float)L.calNextRect.top, (float)(L.calNextRect.right - L.calNextRect.left), (float)(L.calNextRect.bottom - L.calNextRect.top), nextC, true);
            static const wchar_t* kWeek[] = { L"S", L"M", L"T", L"W", L"T", L"F", L"S" };
            for (int cc = 0; cc < 7; cc++) D2DText(cv, kFontTiny, kWeek[cc], (float)(c.left + Scale(14, dpi) + cc * L.calCellW), (float)L.calWeekY, (float)L.calCellW, (float)Scale(20, dpi), th.muted, true);
            for (int cell = 0; cell < 42; cell++) {
                int cy2 = 0, cm2 = 0, cd2 = 0;
                CalCellDate(L, dispY, dispM, cell, cy2, cm2, cd2);
                int col = cell % 7, row = cell / 7;
                int x = c.left + Scale(14, dpi) + col * L.calCellW;
                int y = L.calGridY + row * L.calCellH;
                bool inMonth = (cy2 == dispY && cm2 == dispM);
                bool isWeekend = (col == 0 || col == 6);
                bool isToday = (cy2 == now.wYear && cm2 == now.wMonth && cd2 == now.wDay);
                bool isSelected = (cy2 == dispY && cm2 == dispM && cd2 == selD);
                RECT cellRc = { x + Scale(4, dpi), y + Scale(2, dpi), x + L.calCellW - Scale(4, dpi), y + L.calCellH - Scale(2, dpi) };
                if (isToday) {
                    cv->FillRounded(cellRc, th.accent, (float)L.calCellH / 2.0f);
                } else if (isSelected) {
                    cv->StrokeRounded(cellRc, th.accent, 2.0f, (float)L.calCellH / 2.0f);
                } else if (cell == n->hoverCalCell) {
                    cv->StrokeRounded(cellRc, th.muted, 1.0f, (float)L.calCellH / 2.0f);
                }
                wchar_t dayText[8]; swprintf(dayText, 8, L"%d", cd2);
                COLORREF tc = th.secondary;
                if (isToday) tc = th.pillText;
                else if (isSelected) tc = th.primary;
                else if (!inMonth || isWeekend) tc = th.muted;
                D2DText(cv, kFontTiny, dayText, (float)x, (float)y, (float)L.calCellW, (float)L.calCellH, tc, true);
            }
            COLORREF todayC = n->hoverCalToday ? th.primary : th.secondary;
            D2DText(cv, kFontTiny, L"Today", (float)L.calTodayRect.left, (float)L.calTodayRect.top, (float)(L.calTodayRect.right - L.calTodayRect.left), (float)(L.calTodayRect.bottom - L.calTodayRect.top), todayC, true);
        }
        DrawCardD2D(cv, L.agendaCard, L"AGENDA");
        {
            RECT c = L.agendaCard;
            int dispY = 0, dispM = 0; CalDisplayed(n, dispY, dispM);
            int selD = CalSelectedDay(n, dispY, dispM);
            SYSTEMTIME sel; sel.wYear = (WORD)dispY; sel.wMonth = (WORD)dispM; sel.wDay = (WORD)selD;
            sel.wHour = 0; sel.wMinute = 0; sel.wSecond = 0; sel.wMilliseconds = 0;
            wchar_t dt[64]; FormatDateFromSt(sel, dt, 64);
            D2DText(cv, kFontSmall, dt, (float)(c.left + 14), (float)(c.top + Scale(30, dpi)), (float)(c.right - c.left - 28), (float)Scale(24, dpi), th.primary, false);
            bool selIsToday = (dispY == now.wYear && dispM == now.wMonth && selD == now.wDay);
            D2DText(cv, kFontTiny, selIsToday ? L"Today" : L"Selected day", (float)(c.left + 14), (float)(c.top + Scale(58, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.muted, false);
            FILETIME sft; SystemTimeToFileTime(&sel, &sft);
            ULARGE_INTEGER base; base.LowPart = sft.dwLowDateTime; base.HighPart = sft.dwHighDateTime;
            SYSTEMTIME withDow; FileTimeToSystemTime(&sft, &withDow);
            int y = c.top + Scale(84, dpi);
            static const wchar_t* kDays[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
            for (int cc = 0; cc < 7; cc++) {
                ULONGLONG off = (ULONGLONG)((LONGLONG)(cc - withDow.wDayOfWeek) * 864000000000LL);
                ULARGE_INTEGER v; v.QuadPart = base.QuadPart + off;
                FILETIME cft; cft.dwLowDateTime = v.LowPart; cft.dwHighDateTime = v.HighPart;
                SYSTEMTIME cst; FileTimeToSystemTime(&cft, &cst);
                wchar_t row[32]; swprintf(row, 32, L"%s  %d", kDays[cst.wDayOfWeek % 7], cst.wDay);
                bool isSel = (cst.wYear == dispY && cst.wMonth == dispM && cst.wDay == selD);
                D2DText(cv, kFontTiny, row, (float)(c.left + 14), (float)y, (float)(c.right - c.left - 28), (float)Scale(20, dpi), isSel ? th.primary : th.secondary, false);
                y += Scale(20, dpi);
            }
            D2DText(cv, kFontTiny, L"No events", (float)(c.left + 14), (float)(y + Scale(4, dpi)), (float)(c.right - c.left - 28), (float)Scale(20, dpi), th.muted, false);
        }
    } else if (effTab == kTabMedia) {
        DrawCardD2D(cv, L.mediaCard, L"MEDIA");
        if (g_smtcHasTrack) {
            D2DTextEllipsis(cv, kFontSmall, g_smtcTitle, (float)(L.mediaCard.left + 14), (float)L.mediaTitleY, (float)(L.mediaCard.right - L.mediaCard.left - 28), (float)Scale(22, dpi), th.primary);
            if (g_smtcArtist[0] != L'\0') D2DTextEllipsis(cv, kFontTiny, g_smtcArtist, (float)(L.mediaCard.left + 14), (float)L.mediaArtistY, (float)(L.mediaCard.right - L.mediaCard.left - 28), (float)Scale(20, dpi), th.secondary);
        } else {
            D2DText(cv, kFontTiny, L"No track", (float)(L.mediaCard.left + 14), (float)L.mediaTitleY, (float)(L.mediaCard.right - L.mediaCard.left - 28), (float)Scale(20, dpi), th.secondary, false);
        }
        DrawMediaButtonsRowD2D(cv, n, L, false);
    }
    {
        RECT shadow = L.dockPill; shadow.top += 2; shadow.bottom += 2;
        cv->FillRounded(shadow, RGB(0, 0, 0), 14.0f, 0.30f);
        cv->FillRounded(L.dockPill, CardColorD2D(), 14.0f, 1.0f);
        for (int i = 0; i < L.dockCount; i++) {
            RECT dr = L.dockRects[i];
            int cx = (dr.left + dr.right) / 2, cy = (dr.top + dr.bottom) / 2, r = (dr.right - dr.left) / 2;
            int id = L.dockIds[i];
            if (id == effTab) {
                cv->FillEllipse((float)cx, (float)cy, (float)r, (float)r, th.primary);
                DrawDockGlyphD2D(cv, id, cx, cy, r, th.pillText);
            } else {
                COLORREF gc = (id == n->hoverDock) ? th.tabHover : th.muted;
                DrawDockGlyphD2D(cv, id, cx, cy, r, gc);
            }
        }
    }
    PaintBorderD2D(cv, rc, dpi, n->hoverNotch, n);
}

// --- Blur opt-in (OFF by default) and layered presentation ------------------
// Blur uses the undocumented-but-widely-used SetWindowCompositionAttribute
// accent API, resolved at runtime (no link-time dependency, no header
// dependency). It is deliberately NOT the default, and it can never
// black out the window: the layered bitmap always carries our own
// drawn panel. If the accent call fails, or Windows ignores it on a
// layered window (an untested interaction, see README), blurActive
// stays false and the panel is fully opaque, exactly as in v6.
// When blur IS active, the panel gradient is drawn at 0.88 alpha
// (see PaintNotchD2D) so the blurred desktop can show through;
// cards stay opaque. Blur also needs Windows Transparency effects
// on; with them off, Windows shows a flat tint instead of blur,
// and the panel still draws correctly (it is just translucent).
// WCA_ACCENT_POLICY = 19. ACCENT_ENABLE_BLURBEHIND = 3.
static void ApplyBlur(Notch* n)
{
    if (!n || !n->hwnd) return;
    n->blurActive = false;
    if (!g_blurEnabled) return;
    typedef BOOL (WINAPI* SetWindowCompositionAttributeFn)(HWND, void*);
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (!user32) return;
    SetWindowCompositionAttributeFn fn =
        (SetWindowCompositionAttributeFn)GetProcAddress(user32, "SetWindowCompositionAttribute");
    if (!fn) return;
    struct AccentPolicy { int AccentState; int AccentFlags; int GradientColor; int AnimationId; };
    struct CompositionData { int Attrib; void* Data; SIZE_T SizeOfData; };
    AccentPolicy pol = { 3, 0, 0, 0 };
    CompositionData data = { 19, &pol, sizeof(pol) };
    BOOL ok = fn(n->hwnd, &data);
    n->blurActive = (ok != FALSE);
}

static void ApplyBlurToAll()
{
    for (size_t i = 0; i < g_notches.size(); i++) {
        Notch* an = g_notches[i];
        if (an && an->hwnd) { ApplyBlur(an); InvalidateRect(an->hwnd, NULL, FALSE); }
    }
}

// Present one frame: render offscreen with Direct2D into a 32-bit
// premultiplied DIB, then UpdateLayeredWindow once. The window's
// position/size are taken from GetWindowRect, so presentation
// never moves or resizes the window on its own.
static void PresentNotch(Notch* n)
{
    if (!n || !n->hwnd) return;
    if (!InitD2D()) return;
    RECT rc; GetClientRect(n->hwnd, &rc);
    int w = rc.right, h = rc.bottom;
    if (w < 1 || h < 1) return;
    BITMAPINFO bi = { 0 };
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h; // top-down DIB
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void* bits = NULL;
    HBITMAP bmp = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!bmp) return;
    HDC mem = CreateCompatibleDC(NULL);
    if (!mem) { DeleteObject(bmp); return; }
    HBITMAP oldBmp = (HBITMAP)SelectObject(mem, bmp);
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        96.0f, 96.0f);
    ID2D1DCRenderTarget* rt = NULL;
    HRESULT hr = g_d2dFactory->CreateDCRenderTarget(&props, &rt);
    bool frameOk = false;
    if (SUCCEEDED(hr) && rt) {
        RECT bind = { 0, 0, w, h };
        hr = rt->BindDC(mem, &bind);
        if (SUCCEEDED(hr)) {
            frameOk = true;
            Canvas cv;
            cv.rt = rt;
            cv.dw = g_dwFactory;
            cv.dpi = n->dpi;
            static const DWRITE_FONT_WEIGHT kWeights[4] = {
                DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_LIGHT,
                DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_WEIGHT_NORMAL };
            for (int i = 0; i < 4; i++) {
                g_dwFactory->CreateTextFormat(L"Segoe UI", NULL, kWeights[i],
                    DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                    (FLOAT)FontSizeFor(i, n->dpi), L"en-us", &cv.fmt[i]);
            }
            rt->BeginDraw();
            rt->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
            rt->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            PaintNotchD2D(n, &cv);
            // A failed EndDraw means the frame is not trustworthy:
            // do not present it (see the frameOk guard below).
            frameOk = SUCCEEDED(rt->EndDraw());
            cv.ReleaseAll();
        }
        rt->Release();
    }
    // Never present a blank bitmap: if the frame failed, the
    // window keeps its previous content instead of going empty.
    if (frameOk) {
        RECT wr; GetWindowRect(n->hwnd, &wr);
        POINT pos = { wr.left, wr.top };
        SIZE sz = { w, h };
        POINT src = { 0, 0 };
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
        HDC screen = GetDC(NULL);
        UpdateLayeredWindow(n->hwnd, screen, &pos, &sz, mem, &src, 0, &bf, ULW_ALPHA);
        if (screen) ReleaseDC(NULL, screen);
    }
    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);
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
            // v7: no font recreation (DirectWrite formats are
            // per-frame); apply the saved blur setting to the
            // new window so its first frame matches the others.
            ApplyBlur(created);
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
            int dock = DockIndexAt(n, pt.x, pt.y, true);
            if (dock != n->hoverDock) {
                n->hoverDock = dock;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            bool close = CloseHitAt(n, pt.x, pt.y, true);
            if (close != n->hoverClose) {
                n->hoverClose = close;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            // v6-fix calendar hover (arrows, Today, day cells).
            if (n->paintExpanded && EffectiveTab(n) == kTabCalendar) {
                int arrow = CalPrevHitAt(n, pt.x, pt.y, true) ? -1 : (CalNextHitAt(n, pt.x, pt.y, true) ? 1 : 0);
                bool today = CalTodayHitAt(n, pt.x, pt.y, true);
                int cell = CalDayHitAt(n, pt.x, pt.y, true);
                if (arrow != n->hoverCalArrow || today != n->hoverCalToday || cell != n->hoverCalCell) {
                    n->hoverCalArrow = arrow; n->hoverCalToday = today; n->hoverCalCell = cell;
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (n->hoverCalArrow != 0 || n->hoverCalToday || n->hoverCalCell != -1) {
                n->hoverCalArrow = 0; n->hoverCalToday = false; n->hoverCalCell = -1;
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
            n->hoverDock = -1;
            n->hoverClose = false;
            n->hoverCalArrow = 0; n->hoverCalToday = false; n->hoverCalCell = -1;
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
        // v6: close X collapses the panel to the clock pill.
        // It never quits the app; Quit stays in the tray menu.
        if (CloseHitAt(n, pt.x, pt.y, false)) {
            n->pinned = false;
            SetNotchExpanded(n, false);
            return 0;
        }
        // v6 dock: a dock click switches the tab - never pins,
        // never fires a media key, never opens the gear menu.
        int dockTab = DockIndexAt(n, pt.x, pt.y, false);
        if (dockTab >= 0) {
            if (dockTab != n->activeTab) {
                // v6-fix: leaving the calendar tab resets its
                // displayed month, so returning to it always starts
                // at the current month (documented choice).
                if (n->activeTab == kTabCalendar) { n->calYear = 0; n->calMonth = 0; n->calDay = 0; }
                n->activeTab = dockTab;
                // v6-unified-size: the panel size is identical on
                // every tab, so a switch only repaints content
                // (into the double buffer). It must NOT run the
                // size-animation path, or the panel would jump.
                InvalidateRect(hwnd, NULL, FALSE);
            } else {
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        // v6-fix calendar: prev/next month, Today, and day
        // selection. Inserted after dock and before media/pin, so
        // the documented order (gear, X, dock, media, pin) is
        // unchanged for those controls; calendar clicks simply
        // never reach the pin fallback.
        if (EffectiveTab(n) == kTabCalendar) {
            if (CalPrevHitAt(n, pt.x, pt.y, false) || CalNextHitAt(n, pt.x, pt.y, false)) {
                int delta = CalPrevHitAt(n, pt.x, pt.y, false) ? -1 : 1;
                int dy = 0, dm = 0; CalDisplayed(n, dy, dm);
                int oldSel = CalSelectedDay(n, dy, dm);
                int ny = dy, nm = dm + delta;
                if (nm < 1) { nm = 12; ny--; }
                if (nm > 12) { nm = 1; ny++; }
                n->calYear = ny; n->calMonth = nm;
                int maxD = DaysInMonth(ny, nm);
                n->calDay = oldSel > maxD ? maxD : oldSel;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            if (CalTodayHitAt(n, pt.x, pt.y, false)) {
                n->calYear = 0; n->calMonth = 0; n->calDay = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            int cell = CalDayHitAt(n, pt.x, pt.y, false);
            if (cell >= 0) {
                RECT rc2; GetClientRect(hwnd, &rc2);
                Layout Lc = ComputeLayout(n, rc2.right);
                int dy = 0, dm = 0; CalDisplayed(n, dy, dm);
                int cy2 = 0, cm2 = 0, cd2 = 0;
                CalCellDate(Lc, dy, dm, cell, cy2, cm2, cd2);
                n->calYear = cy2; n->calMonth = cm2; n->calDay = cd2;
                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
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
        // v7: a layered window is presented with
        // UpdateLayeredWindow, not by painting into the screen
        // DC. BeginPaint/EndPaint only clear the paint queue;
        // PresentNotch renders the whole frame offscreen with
        // Direct2D and presents it once.
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        if (n) PresentNotch(n);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY: {
        if (n) {
            KillTimer(hwnd, ID_TIMER_ANIM);
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
            // v6-fix: no resize here any more. The MEDIA layout
            // always reserves title+artist rows, so a track
            // appearing or disappearing is just a repaint (below),
            // never a height snap.
            g_smtcLayoutKey = SmtcLayoutKey();
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
                        InvalidateRect(an->hwnd, NULL, FALSE);
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
        case ID_TRAY_BLUR:
            g_blurEnabled = !g_blurEnabled;
            SaveSettings();
            ApplyBlurToAll();
            break;
        case ID_TRAY_MEDIA_PREV: SendMediaKey(VK_MEDIA_PREV_TRACK); break;
        case ID_TRAY_MEDIA_PLAYPAUSE: SendMediaKey(VK_MEDIA_PLAY_PAUSE); break;
        case ID_TRAY_MEDIA_NEXT: SendMediaKey(VK_MEDIA_NEXT_TRACK); break;
        case ID_TRAY_ABOUT:
            MessageBoxW(hwnd,
                L"OnyxNotch v7 (Direct2D rewrite of v6)\nMac-style notch for Windows.\nNative Win32 C++, no runtime, low memory.\nDirect2D/DirectWrite rendering on a layered\nwindow with per-pixel alpha; spring animation;\nblur is an OFF-by-default experimental tray\ntoggle with an opaque fallback.\n\nv4: premium UI — gradient surface, lighter\ntypography, circular media buttons, battery glyph,\ncalendar pill — reorganized in place: compact\ntwo-column body (media beside system stats),\nsection headers, stats rows with bars.\n\nv5-SMTC: with Media controls on, the Media tab\nshows the current track title and artist from\nWindows SMTC (read-only), and Home shows a\ncompact title line. No track, no title — the\nmedia buttons still work via media keys.\n\nv5-settings: size and toggles are saved to\n%APPDATA%\\OnyxNotch\\settings.ini and restored\nat startup. Start with Windows (tray menu) writes\nthe Run registry value for this .exe.\n\nThemes: tray menu Themes submenu — Onyx, Midnight,\nGraphite — applies instantly to all notches and is\nsaved to settings.ini as theme=onyx|midnight|graphite.\n\nClick the notch header to pin it open;\nclick again to unpin. The gear button in the\nexpanded header opens this menu from the notch.\n\nv6: wide card dashboard with a header bar,\ncard grid and a bottom icon dock. Home is a\ncompact summary; System, Clock, Calendar\nand Media show full card versions. Click a\ndock icon to switch. Close X collapses only.",
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
