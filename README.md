# OnyxNotch v1

Mac-style notch for Windows, in native C++ Win32. One source file, one
`.exe`, no Electron, no Python, no .NET runtime.

## What it does (v1)

- Black notch, top-centre of the primary monitor, always on top.
- Collapsed: clock only (HH:MM).
- Hover over it, or click it, to expand: large clock, date, battery
  percentage and charging state.
- Click again to pin it open. Move the mouse away and it collapses
  (unless pinned).
- No taskbar button, and it never steals focus.
- Tray icon (right-click): Size Small / Medium / Large, About, Quit.
- Clock refreshes once per second. Nothing polls in between, so idle
  CPU should be near 0%. Target idle memory: under 10 MB.
  Measure it in Task Manager after running it — that number is the
  real test, not a claim in this file.

## Build (Windows)

1. Install Visual Studio with "Desktop development with C++"
   (the free Community edition is fine).
2. Open "x64 Native Tools Command Prompt for VS".
3. `cd` to this folder and run:

```bat
build.bat
```

That produces `OnyxNotch.exe` in this folder.

MinGW alternative (also on Windows, if you have it):

```bat
x86_64-w64-mingw32-g++ -O2 -mwindows -municode main.cpp -o OnyxNotch.exe
```

## Run

Double-click `OnyxNotch.exe`. To stop it, right-click its tray icon
and choose Quit. There is no installer and no auto-start in v1 — if
you want it at login, put a shortcut to the `.exe` in the Startup
folder (`Win + R`, type `shell:startup`).

## Limitations in v1 (honest list)

- Primary monitor only. Multi-monitor support is a v2 item.
- No "hide on fullscreen" yet — the notch stays on top in fullscreen
  apps. If that bothers you in games, quit it from the tray for now.
- Tray icon is the default Windows application icon. A custom
  `.ico` is a small v1.1 polish item.
- No music, notifications, calendar, or widgets yet. Those are v2,
  and each one costs memory — they will be opt-in, not always on.
- The `.exe` built here is unsigned, so Windows SmartScreen may
  warn on first run. That is normal for a self-built unsigned app;
  code-signing is a paid certificate, not a code change.

## Files

- `main.cpp` — the whole app. Win32 window, region for the rounded
  notch shape, 1-second timer, tray icon, battery via
  `GetSystemPowerStatus`.
- `build.bat` — MSVC build script.
- `README.md` — this file.

## Roadmap (not in v1)

- Multi-monitor + per-monitor position
- Hide on fullscreen (opt-in)
- Custom tray / app icon
- Media controls (Windows System Media Transport Controls)
- Optional widgets, each toggleable so idle memory stays low
