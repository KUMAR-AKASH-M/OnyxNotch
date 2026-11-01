// OnyxNotch v1 - Mac-style notch for Windows, native Win32 C++.
// No Electron, no Python, no .NET runtime. One .exe, one source file.
// Build on Windows: run build.bat in a Visual Studio Developer Command Prompt.
//   cl /O2 /MT main.cpp user32.lib gdi32.lib shell32.lib
// Or with MinGW: x86_64-w64-mingw32-g++ -O2 -mwindows -municode main.cpp -o OnyxNotch.exe
//
// v1 behaviour:
//  - Black notch, top-centre of the primary monitor, always on top.
//  - Collapsed: clock only. Hover (or click) expands it.
//  - Expanded: large clock, date, battery status.
//  - Click again to pin/unpin. Moving the mouse away collapses it (unless pinned).
//  - Tray icon (right-click): size Small / Medium / Large, Quit.
//  - No taskbar button, no focus stealing (WS_EX_NOACTIVATE + SW_SHOWNOACTIVATE).
//  - Clock refreshes once per second. Nothing polls in between, so idle CPU ~0%.

#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")

#define ID_TRAY_ICON      1
#define WM_TRAY           (WM_APP + 1)
#define ID_TIMER_CLOCK    1
#define ID_TRAY_QUIT      1001
#define ID_TRAY_SIZE_S    1002
#define ID_TRAY_SIZE_M    1003
#define ID_TRAY_SIZE_L    1004
#define ID_TRAY_ABOUT     1005

static const wchar_t kClassName[] = L"OnyxNotch";
static const wchar_t kWindowTitle[] = L"OnyxNotch";

static HWND     g_hwnd = NULL;
static HFONT    g_hFontClock = NULL;   // collapsed clock
static HFONT    g_hFontBig = NULL;     // expanded clock
static HFONT    g_hFontSmall = NULL;   // expanded detail lines
static bool     g_expanded = false;
static bool     g_pinned = false;
static bool     g_trackingLeave = false;
static int      g_size = 1;            // 0 = Small, 1 = Medium, 2 = Large
static NOTIFYICONDATAW g_nid = { 0 };

static int CollapsedWidth()
{
    switch (g_size) {
    case 0: return 200;
    case 2: return 300;
    default: return 240;
    }
}

static void ApplyRegion(HWND hwnd, int w, int h, bool expanded)
{
    HRGN rgn = CreateRoundRectRgn(0, 0, w, h, 28, 28);
    if (!expanded) {
        // Mac-style notch: square top corners (it touches the top edge),
        // rounded bottom corners only.
        HRGN top = CreateRectRgn(0, 0, w, 18);
        CombineRgn(rgn, rgn, top, RGN_OR);
        DeleteObject(top);
    }
    // SetWindowRgn takes ownership of rgn; do not delete it here.
    SetWindowRgn(hwnd, rgn, TRUE);
}

static void ApplySize(HWND hwnd, bool expanded)
{
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int w = expanded ? 340 : CollapsedWidth();
    int h = expanded ? 150 : 34;
    int x = (screenW - w) / 2;
    if (x < 0) x = 0;
    SetWindowPos(hwnd, HWND_TOPMOST, x, 0, w, h,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    ApplyRegion(hwnd, w, h, expanded);
}

static void SetExpanded(HWND hwnd, bool expanded)
{
    if (g_expanded == expanded) return;
    g_expanded = expanded;
    ApplySize(hwnd, expanded);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void AddTrayIcon(HWND hwnd, HINSTANCE hInst)
{
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = hwnd;
    g_nid.uID = ID_TRAY_ICON;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAY;
    g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy_s(g_nid.szTip, L"OnyxNotch");
    Shell_NotifyIconW(NIM_ADD, &g_nid);
    (void)hInst;
}

static void ShowTrayMenu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, ID_TRAY_SIZE_S, L"Size: Small");
    AppendMenuW(menu, MF_STRING, ID_TRAY_SIZE_M, L"Size: Medium");
    AppendMenuW(menu, MF_STRING, ID_TRAY_SIZE_L, L"Size: Large");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, ID_TRAY_ABOUT, L"About OnyxNotch");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, ID_TRAY_QUIT, L"Quit");
    SetForegroundWindow(hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

static void FormatClock(wchar_t* out, size_t count)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    swprintf(out, count, L"%02d:%02d", st.wHour, st.wMinute);
}

static void FormatDate(wchar_t* out, size_t count)
{
    static const wchar_t* kDays[] =
        { L"Sunday", L"Monday", L"Tuesday", L"Wednesday",
          L"Thursday", L"Friday", L"Saturday" };
    static const wchar_t* kMonths[] =
        { L"January", L"February", L"March", L"April", L"May", L"June",
          L"July", L"August", L"September", L"October", L"November", L"December" };
    SYSTEMTIME st;
    GetLocalTime(&st);
    swprintf(out, count, L"%s, %d %s %d",
             kDays[st.wDayOfWeek % 7],
             st.wDay,
             kMonths[(st.wMonth - 1) % 12],
             st.wYear);
}

static void FormatBattery(wchar_t* out, size_t count)
{
    SYSTEM_POWER_STATUS sps;
    if (!GetSystemPowerStatus(&sps) || sps.BatteryLifePercent == 255) {
        wcscpy_s(out, count, L"Battery: --");
        return;
    }
    const wchar_t* state = L"";
    if (sps.ACLineStatus == 1) {
        state = (sps.BatteryFlag & 8) ? L"Charging" : L"On AC power";
    } else {
        state = L"Discharging";
    }
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

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE: {
        g_hFontClock = CreateFontW(20, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                   CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                   DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_hFontBig = CreateFontW(36, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_hFontSmall = CreateFontW(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                   CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                   DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        AddTrayIcon(hwnd, ((LPCREATESTRUCT)lParam)->hInstance);
        SetTimer(hwnd, ID_TIMER_CLOCK, 1000, NULL);
        ApplySize(hwnd, false);
        return 0;
    }

    case WM_TIMER:
        if (wParam == ID_TIMER_CLOCK) {
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;

    case WM_DISPLAYCHANGE:
        ApplySize(hwnd, g_expanded);
        return 0;

    case WM_MOUSEMOVE: {
        if (!g_expanded) {
            SetExpanded(hwnd, true);
        }
        if (!g_trackingLeave) {
            TRACKMOUSEEVENT tme = { 0 };
            tme.cbSize = sizeof(tme);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hwnd;
            TrackMouseEvent(&tme);
            g_trackingLeave = true;
        }
        return 0;
    }

    case WM_MOUSELEAVE:
        g_trackingLeave = false;
        if (g_expanded && !g_pinned) {
            SetExpanded(hwnd, false);
        }
        return 0;

    case WM_LBUTTONDOWN:
        // Click toggles pin: pinned stays expanded until clicked again.
        g_pinned = !g_pinned;
        SetExpanded(hwnd, g_pinned);
        return 0;

    case WM_TRAY:
        if (lParam == WM_RBUTTONUP) {
            ShowTrayMenu(hwnd);
        } else if (lParam == WM_LBUTTONUP) {
            g_pinned = !g_pinned;
            SetExpanded(hwnd, g_pinned);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_QUIT:
            DestroyWindow(hwnd);
            break;
        case ID_TRAY_SIZE_S:
            g_size = 0;
            ApplySize(hwnd, g_expanded);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_TRAY_SIZE_M:
            g_size = 1;
            ApplySize(hwnd, g_expanded);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_TRAY_SIZE_L:
            g_size = 2;
            ApplySize(hwnd, g_expanded);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case ID_TRAY_ABOUT:
            MessageBoxW(hwnd,
                        L"OnyxNotch v1\nMac-style notch for Windows.\nNative Win32 C++, no runtime, low memory.",
                        L"OnyxNotch", MB_OK | MB_ICONINFORMATION);
            break;
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &rc, brush);
        DeleteObject(brush);

        wchar_t clockText[32];
        FormatClock(clockText, 32);

        if (!g_expanded) {
            DrawCenteredText(hdc, g_hFontClock, clockText,
                             0, 0, rc.right, rc.bottom, RGB(255, 255, 255));
        } else {
            DrawCenteredText(hdc, g_hFontBig, clockText,
                             0, 12, rc.right, 48, RGB(255, 255, 255));
            wchar_t dateText[64];
            FormatDate(dateText, 64);
            DrawCenteredText(hdc, g_hFontSmall, dateText,
                             0, 66, rc.right, 24, RGB(200, 200, 200));
            wchar_t batteryText[64];
            FormatBattery(batteryText, 64);
            DrawCenteredText(hdc, g_hFontSmall, batteryText,
                             0, 92, rc.right, 24, RGB(200, 200, 200));
            DrawCenteredText(hdc, g_hFontSmall, L"OnyxNotch  |  Click to pin",
                             0, 118, rc.right, 20, RGB(140, 140, 140));
        }
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;  // We paint the whole client area ourselves; avoids flicker.

    case WM_DESTROY:
        KillTimer(hwnd, ID_TIMER_CLOCK);
        Shell_NotifyIconW(NIM_DELETE, &g_nid);
        if (g_hFontClock) DeleteObject(g_hFontClock);
        if (g_hFontBig) DeleteObject(g_hFontBig);
        if (g_hFontSmall) DeleteObject(g_hFontSmall);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/,
                    PWSTR /*pCmdLine*/, int /*nCmdShow*/)
{
    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = kClassName;
    RegisterClassExW(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int w = CollapsedWidth();
    g_hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        kClassName, kWindowTitle, WS_POPUP,
        (screenW - w) / 2, 0, w, 34,
        NULL, NULL, hInstance, NULL);
    if (!g_hwnd) return 1;

    // ApplySize (in WM_CREATE) already positioned it; just show without focus.
    ShowWindow(g_hwnd, SW_SHOWNOACTIVATE);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
