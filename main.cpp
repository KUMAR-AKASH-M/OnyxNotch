// OnyxNotch v2 - Mac-style notch for Windows, native Win32 C++.
// No Electron, no Python, no .NET runtime. One .exe, one source file
// (plus a small .rc/.ico for the custom tray/app icon).
// Build on Windows: run build.bat in a Visual Studio Developer Command Prompt.
//   cl /O2 /MT main.cpp onyxnotch.res user32.lib gdi32.lib shell32.lib
// Or with MinGW: windres onyxnotch.rc -O coff -o onyxnotch.o, then
//   x86_64-w64-mingw32-g++ -O2 -mwindows -municode main.cpp onyxnotch.o -o OnyxNotch.exe
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

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")

#define ID_TRAY_ICON            1
#define WM_TRAY                 (WM_APP + 1)
#define WM_APP_REBUILD          (WM_APP + 2)
#define ID_TIMER_CLOCK          1
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
    int mediaLabelY = 0;
    int mediaButtonY = 0;
    int mediaButtonH = 0;
    RECT btnPrev = { 0, 0, 0, 0 };
    RECT btnPlay = { 0, 0, 0, 0 };
    RECT btnNext = { 0, 0, 0, 0 };
    int statsY = 0;
    int calendarY = 0;
    int calendarH = 0;
    int footerY = 0;
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

static Layout ComputeLayout(Notch* n, int clientW)
{
    Layout L;
    int dpi = n->dpi;
    int y = Scale(118, dpi); // after big clock, date, battery (v1 positions)
    if (g_mediaEnabled) {
        L.mediaLabelY = y; y += Scale(20, dpi);
        L.mediaButtonY = y; L.mediaButtonH = Scale(28, dpi); y += L.mediaButtonH + Scale(8, dpi);
        int margin = Scale(16, dpi), gap = Scale(8, dpi);
        int bw = (clientW - 2 * margin - 2 * gap) / 3;
        if (bw < 20) bw = 20;
        L.btnPrev = { margin, L.mediaButtonY, margin + bw, L.mediaButtonY + L.mediaButtonH };
        L.btnPlay = { margin + bw + gap, L.mediaButtonY, margin + 2 * bw + gap, L.mediaButtonY + L.mediaButtonH };
        L.btnNext = { margin + 2 * bw + 2 * gap, L.mediaButtonY, margin + 3 * bw + 2 * gap, L.mediaButtonY + L.mediaButtonH };
    }
    if (g_widgetStats) {
        L.statsY = y; y += Scale(24, dpi);
    }
    if (g_widgetCalendar) {
        SYSTEMTIME st; GetLocalTime(&st);
        int firstWeekday = (int)((st.wDayOfWeek - (st.wDay - 1) % 7) % 7);
        if (firstWeekday < 0) firstWeekday += 7;
        int cells = firstWeekday + DaysInMonth(st.wYear, st.wMonth);
        int rows = (cells + 6) / 7;
        L.calendarY = y;
        L.calendarH = Scale(24 + 20 + rows * 20, dpi);
        y += L.calendarH + Scale(8, dpi);
    }
    L.footerY = y; y += Scale(20 + 10, dpi);
    L.totalH = y;
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

static void ApplyNotchSize(Notch* n, bool expanded)
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

static void SetNotchExpanded(Notch* n, bool expanded)
{
    if (!n || n->expanded == expanded) return;
    n->expanded = expanded;
    ApplyNotchSize(n, expanded);
    InvalidateRect(n->hwnd, NULL, TRUE);
}

static void RecreateFonts(Notch* n)
{
    if (n->hFontClock) DeleteObject(n->hFontClock);
    if (n->hFontBig) DeleteObject(n->hFontBig);
    if (n->hFontSmall) DeleteObject(n->hFontSmall);
    if (n->hFontTiny) DeleteObject(n->hFontTiny);
    int dpi = n->dpi;
    n->hFontClock = CreateFontW(Scale(20, dpi), 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    n->hFontBig = CreateFontW(Scale(36, dpi), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    n->hFontSmall = CreateFontW(Scale(17, dpi), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    n->hFontTiny = CreateFontW(Scale(14, dpi), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
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

static void ShowTrayMenu(HWND hwnd)
{
    POINT pt; GetCursorPos(&pt);
    HMENU menu = CreatePopupMenu();
    AppendCheck(menu, ID_TRAY_SIZE_S, L"Size: Small", g_size == 0);
    AppendCheck(menu, ID_TRAY_SIZE_M, L"Size: Medium", g_size == 1);
    AppendCheck(menu, ID_TRAY_SIZE_L, L"Size: Large", g_size == 2);
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendCheck(menu, ID_TRAY_HIDE_FULLSCREEN, L"Hide on fullscreen", g_hideOnFullscreen);
    AppendCheck(menu, ID_TRAY_MEDIA, L"Media controls", g_mediaEnabled);
    AppendCheck(menu, ID_TRAY_WIDGET_STATS, L"Widget: System stats", g_widgetStats);
    AppendCheck(menu, ID_TRAY_WIDGET_CALENDAR, L"Widget: Calendar", g_widgetCalendar);
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
    SetForegroundWindow(hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);
    DestroyMenu(menu);
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
            ApplyNotchSize(n, n->expanded);
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
            ApplyNotchSize(g_notches[i], g_notches[i]->expanded);
            InvalidateRect(g_notches[i]->hwnd, NULL, TRUE);
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
static bool MediaButtonHit(Notch* n, int px, int py, WORD& vkOut)
{
    if (!g_mediaEnabled || !n->expanded) return false;
    RECT rc; GetClientRect(n->hwnd, &rc);
    Layout L = ComputeLayout(n, rc.right);
    if (py < L.mediaButtonY || py >= L.mediaButtonY + L.mediaButtonH) return false;
    if (px >= L.btnPrev.left && px < L.btnPrev.right) { vkOut = VK_MEDIA_PREV_TRACK; return true; }
    if (px >= L.btnPlay.left && px < L.btnPlay.right) { vkOut = VK_MEDIA_PLAY_PAUSE; return true; }
    if (px >= L.btnNext.left && px < L.btnNext.right) { vkOut = VK_MEDIA_NEXT_TRACK; return true; }
    return false;
}

static void PaintNotch(Notch* n, HDC hdc)
{
    RECT rc; GetClientRect(n->hwnd, &rc);
    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);

    wchar_t clockText[32]; FormatClock(clockText, 32);
    int dpi = n->dpi;
    if (!n->expanded) {
        DrawCenteredText(hdc, n->hFontClock, clockText, 0, 0, rc.right, rc.bottom, RGB(255, 255, 255));
        return;
    }

    Layout L = ComputeLayout(n, rc.right);
    DrawCenteredText(hdc, n->hFontBig, clockText, 0, Scale(12, dpi), rc.right, Scale(48, dpi), RGB(255, 255, 255));
    wchar_t dateText[64]; FormatDate(dateText, 64);
    DrawCenteredText(hdc, n->hFontSmall, dateText, 0, Scale(66, dpi), rc.right, Scale(24, dpi), RGB(200, 200, 200));
    wchar_t batteryText[64]; FormatBattery(batteryText, 64);
    DrawCenteredText(hdc, n->hFontSmall, batteryText, 0, Scale(92, dpi), rc.right, Scale(24, dpi), RGB(200, 200, 200));

    if (g_mediaEnabled) {
        DrawCenteredText(hdc, n->hFontSmall, L"Media controls (media keys)",
                         0, L.mediaLabelY, rc.right, Scale(20, dpi), RGB(170, 170, 170));
        // Buttons: rounded dark-grey boxes with white vector glyphs
        // (no text labels, no font dependence, no image files).
        // The hit test in MediaButtonHit uses exactly these rects,
        // so only the drawing changes here, not the clickable area.
        HBRUSH btnBrush = CreateSolidBrush(RGB(38, 38, 38));
        HBRUSH glyphBrush = CreateSolidBrush(RGB(255, 255, 255));
        HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
        HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, btnBrush);
        RECT btns[3] = { L.btnPrev, L.btnPlay, L.btnNext };
        for (int i = 0; i < 3; i++) {
            RoundRect(hdc, btns[i].left, btns[i].top, btns[i].right, btns[i].bottom, 8, 8);
        }
        SelectObject(hdc, glyphBrush);
        for (int i = 0; i < 3; i++) {
            int cx = (btns[i].left + btns[i].right) / 2;
            int cy = (btns[i].top + btns[i].bottom) / 2;
            int h = (btns[i].bottom - btns[i].top) * 11 / 20; // glyph height
            int half = h / 2;
            if (i == 0 || i == 2) {
                // Prev: bar + two triangles pointing left.
                // Next: two triangles pointing right + bar (mirrored).
                int dir = (i == 0) ? -1 : 1;
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
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(btnBrush);
        DeleteObject(glyphBrush);
    }

    if (g_widgetStats) {
        wchar_t statsText[128];
        swprintf(statsText, 128, L"CPU: %d%%   RAM: %d%%  (%.1f / %.1f GB)",
                 g_cpuPercent, g_memPercent, g_memUsedGB, g_memTotalGB);
        DrawCenteredText(hdc, n->hFontSmall, statsText, 0, L.statsY, rc.right, Scale(24, dpi), RGB(200, 200, 200));
    }

    if (g_widgetCalendar) {
        SYSTEMTIME st; GetLocalTime(&st);
        static const wchar_t* kMonths[] = { L"January", L"February", L"March", L"April", L"May", L"June",
            L"July", L"August", L"September", L"October", L"November", L"December" };
        wchar_t monthText[64];
        swprintf(monthText, 64, L"%s %d", kMonths[(st.wMonth - 1) % 12], st.wYear);
        DrawCenteredText(hdc, n->hFontSmall, monthText, 0, L.calendarY, rc.right, Scale(24, dpi), RGB(255, 255, 255));
        int margin = Scale(16, dpi);
        int cellW = (rc.right - 2 * margin) / 7;
        int cellH = Scale(20, dpi);
        int weekY = L.calendarY + Scale(24, dpi);
        static const wchar_t* kWeek[] = { L"S", L"M", L"T", L"W", L"T", L"F", L"S" };
        for (int c = 0; c < 7; c++) {
            DrawCenteredText(hdc, n->hFontTiny, kWeek[c], margin + c * cellW, weekY, cellW, cellH, RGB(140, 140, 140));
        }
        int firstWeekday = (int)((st.wDayOfWeek - (st.wDay - 1) % 7) % 7);
        if (firstWeekday < 0) firstWeekday += 7;
        int days = DaysInMonth(st.wYear, st.wMonth);
        for (int d = 1; d <= days; d++) {
            int idx = firstWeekday + d - 1;
            int col = idx % 7, row = idx / 7;
            int x = margin + col * cellW;
            int y = weekY + cellH + row * cellH;
            if (d == st.wDay) {
                HBRUSH hl = CreateSolidBrush(RGB(48, 48, 48));
                RECT hlRc = { x, y, x + cellW, y + cellH };
                FillRect(hdc, &hlRc, hl);
                DeleteObject(hl);
            }
            wchar_t dayText[8]; swprintf(dayText, 8, L"%d", d);
            DrawCenteredText(hdc, n->hFontTiny, dayText, x, y, cellW, cellH,
                             (d == st.wDay) ? RGB(255, 255, 255) : RGB(200, 200, 200));
        }
    }

    DrawCenteredText(hdc, n->hFontSmall, L"OnyxNotch v2  |  Click to pin",
                     0, L.footerY, rc.right, Scale(20, dpi), RGB(140, 140, 140));
}

static LRESULT CALLBACK NotchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Notch* n = FindNotch(hwnd);
    switch (msg) {
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
            ApplyNotchSize(created, false);
        }
        return 0;
    }
    case WM_DPICHANGED: {
        if (n) {
            n->dpi = HIWORD(wParam);
            RecreateFonts(n);
            ApplyNotchSize(n, n->expanded);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    case WM_DISPLAYCHANGE:
        if (g_managerHwnd) PostMessage(g_managerHwnd, WM_APP_REBUILD, 0, 0);
        return 0;
    case WM_MOUSEMOVE: {
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
            if (n->expanded && !n->pinned) SetNotchExpanded(n, false);
        }
        return 0;
    case WM_LBUTTONDOWN: {
        if (!n) return 0;
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        WORD vk = 0;
        if (MediaButtonHit(n, pt.x, pt.y, vk)) {
            SendMediaKey(vk);
            return 0;
        }
        n->pinned = !n->pinned;
        SetNotchExpanded(n, n->pinned);
        return 0;
    }
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
            // Destroy notch windows first, then the manager (which quits).
            std::vector<HWND> hwnds;
            for (size_t i = 0; i < g_notches.size(); i++) hwnds.push_back(g_notches[i]->hwnd);
            for (size_t i = 0; i < hwnds.size(); i++) if (hwnds[i]) DestroyWindow(hwnds[i]);
            DestroyWindow(hwnd);
            break;
        }
        case ID_TRAY_SIZE_S: g_size = 0; ApplySizeToAll(); break;
        case ID_TRAY_SIZE_M: g_size = 1; ApplySizeToAll(); break;
        case ID_TRAY_SIZE_L: g_size = 2; ApplySizeToAll(); break;
        case ID_TRAY_HIDE_FULLSCREEN:
            g_hideOnFullscreen = !g_hideOnFullscreen;
            UpdateFullscreenAndVisibility();
            break;
        case ID_TRAY_MEDIA:
            g_mediaEnabled = !g_mediaEnabled;
            ApplySizeToAll();
            break;
        case ID_TRAY_WIDGET_STATS:
            g_widgetStats = !g_widgetStats;
            if (!g_widgetStats) g_havePrevTimes = false;
            ApplySizeToAll();
            break;
        case ID_TRAY_WIDGET_CALENDAR:
            g_widgetCalendar = !g_widgetCalendar;
            ApplySizeToAll();
            break;
        case ID_TRAY_MEDIA_PREV: SendMediaKey(VK_MEDIA_PREV_TRACK); break;
        case ID_TRAY_MEDIA_PLAYPAUSE: SendMediaKey(VK_MEDIA_PLAY_PAUSE); break;
        case ID_TRAY_MEDIA_NEXT: SendMediaKey(VK_MEDIA_NEXT_TRACK); break;
        case ID_TRAY_ABOUT:
            MessageBoxW(hwnd,
                L"OnyxNotch v2\nMac-style notch for Windows.\nNative Win32 C++, no runtime, low memory.\n\nv2: one notch per monitor, hide on fullscreen (opt-in),\ncustom icon, media controls (opt-in), system-stats and\ncalendar widgets (each opt-in).",
                L"OnyxNotch", MB_OK | MB_ICONINFORMATION);
            break;
        }
        return 0;
    case WM_DESTROY:
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
