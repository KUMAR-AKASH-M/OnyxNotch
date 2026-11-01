# OnyxNotch v3

Mac-style notch for Windows, in native C++ Win32. One source file, one
`.exe`, no Electron, no Python, no .NET runtime.

## What it does (v1, unchanged)

- Black notch, top-centre of the screen, always on top.
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

## What's new in v2

### 1. Multi-monitor + per-monitor position — ON by default
One notch on each monitor, top-centre of that monitor's work area
(work area, so it sits clear of the taskbar). Monitors added or
removed while the app runs are picked up via display-change handling,
which rebuilds the notch set. Per-monitor DPI is honoured: each notch
scales its size and fonts to its own monitor's DPI, and reacts to
DPI changes. Each monitor's notch expands/pins independently.

### 2. Hide on fullscreen — OFF by default (opt-in)
Right-click the tray icon and tick **Hide on fullscreen** to turn it
on. While it is on, a notch hides on a monitor when a fullscreen or
borderless window covers that whole monitor, and returns (without
stealing focus) when fullscreen ends. It is checked once per second
on the existing clock timer, so hiding can lag up to a second.
Turning the toggle off restores all notches immediately.
With the toggle off (the default), v2 behaves exactly like v1: the
notch stays on top in fullscreen apps.

### 3. Custom tray / app icon — ON by default
The default Windows application icon is replaced by a black-pill
OnyxNotch icon (`onyxnotch.ico`, compiled in via `onyxnotch.rc`).
It is baked into the `.exe` at build time, so no icon file is needed
at run time and the build is still a single `.exe`. If the icon
resource is missing for any reason, the code falls back to the
default Windows icon rather than failing to start.

### 4. Media controls — OFF by default (opt-in)
Right-click the tray icon and tick **Media controls** to turn it on.
While on, the expanded notch shows **Prev / Play / Pause / Next**
buttons, and the tray menu gains matching media items. Pressing one
sends a plain Win32 media key (`VK_MEDIA_PREV_TRACK`,
`VK_MEDIA_PLAY_PAUSE`, `VK_MEDIA_NEXT_TRACK`) to the currently
active media player — the same as pressing a media key on a keyboard.
It works with players that respond to media keys (Spotify, browsers,
most music/video apps).

**Honest SMTC limit:** v2 does NOT show a track title or artist.
Retrieving the current track from Windows System Media Transport
Controls (SMTC) is not available in plain Win32 — it needs C++/WinRT
and the Windows Runtime, which would add a new dependency and
complicate this single-file build. That was deliberately not added.
So the media row shows controls only, and no title is faked. If a
player does not respond to media keys, its buttons will appear to do
nothing for that player; that is a property of the player, not a bug
in the button.

### 5. Optional widgets — each OFF by default (opt-in)
Right-click the tray icon to toggle each widget separately. Turning
a widget on grows the expanded notch to fit it; turning it off
shrinks it back. Nothing for a widget is sampled or drawn while it
is off, so idle memory stays low when they are off.

- **Widget: System stats** — shows `CPU: x%   RAM: y%  (used / total GB)`.
  CPU % is computed from `GetSystemTimes` deltas on the 1-second
  timer (so the first second after enabling it may show 0% while a
  baseline is taken). RAM comes from `GlobalMemoryStatusEx`.
  No network, no telemetry.
- **Widget: Calendar** — a small month grid for the current month,
  with today highlighted. Week starts Sunday, matching the v1 date
  text. No network, no events/sync — it is a calendar view only.

The v1 expanded view (large clock, date, battery) stays always on,
whether or not any widget is enabled.


## What's new in v3 — Animations and polish only

No new features, no changed defaults except the Animations tick
itself. Everything from v2 (multi-monitor, hide-on-fullscreen,
custom icon, media controls, widgets) works exactly as before.

### 1. Smooth expand/collapse — 220 ms, ease-out
Expanding and collapsing now animates the notch height (and width,
when it changes) over about 220 ms with an ease-out cubic curve,
at roughly 16 ms per frame. The animation timer is per-notch,
starts only for a size change, and is killed the moment the
animation finishes — an idle notch has no animation timer, so
idle cost is unchanged from v2.

Interrupt behaviour, by design:
- Hovering away mid-expand reverses smoothly from the current
  size back to collapsed — it does not snap or restart from
  full size.
- Clicking to pin mid-expand settles at the expanded size.
- Unpinning (or moving away when unpinned) animates back to
  collapsed, and the expanded content keeps drawing until the
  collapse finishes, so it does not pop to the clock on the
  first frame.
- Tray size changes and widget/media toggles animate the same
  way, from the current size to the new size.

### 2. Button hover / pressed states
Media buttons now highlight on hover (lighter grey) and again on
press, so it is obvious what you are about to click. The whole
notch (which is itself clickable to pin) also gets a subtle
1 px outline while the mouse is over it. Clickable areas are
unchanged: hover, pressed and click all use the same shared
layout rects as v2, so a button never looks clickable somewhere
it is not.

### 3. Visual polish, low-cost only
- Text is now anti-aliased (ClearType font quality on all four
  notch fonts) instead of the default quality.
- Rounded corners are the same v2 region shape, applied frame by
  frame during animation so the shape stays rounded while tall
  and matches the v2 collapsed shape once short.
- No fade/slide-in on first show, deliberately: that would need
  a layered-window rewrite (alpha blending on top of the custom
  region), which risks breaking the region and focus behaviour
  the notch depends on. First show stays instant, exactly as
  in v2. If a fade is wanted later, it should be done as its
  own change with a layered-window test, not bundled here.

### 4. Animations toggle — ON by default
Right-click the tray icon and tick/untick **Animations**. Turning
it off stops every in-progress animation instantly, snaps each
notch to its final size at once, and restores the instant v2
expand/collapse behaviour everywhere — including tray size
changes and widget/media toggles. Turning it back on re-enables
animation for the next size change. There is no setting file:
like the other v2 toggles, it resets to ON at each start.

## Build (Windows) — unchanged from v2

## Build (Windows)

1. Install Visual Studio with "Desktop development with C++"
   (the free Community edition is fine).
2. Open "x64 Native Tools Command Prompt for VS".
3. `cd` to this folder and run:

```bat
build.bat
```

That compiles `onyxnotch.rc` (icon) and then `main.cpp`, and
produces `OnyxNotch.exe` in this folder.

Exact command `build.bat` runs (MSVC):

```bat
rc /nologo onyxnotch.rc
cl /nologo /O2 /MT /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS main.cpp onyxnotch.res user32.lib gdi32.lib shell32.lib /link /SUBSYSTEM:WINDOWS /OUT:OnyxNotch.exe
```

MinGW alternative (also on Windows, if you have it):

```bat
windres onyxnotch.rc -O coff -o onyxnotch.o
x86_64-w64-mingw32-g++ -O2 -mwindows -municode main.cpp onyxnotch.o -o OnyxNotch.exe
```

## Run

Double-click `OnyxNotch.exe`. To stop it, right-click its tray icon
and choose Quit. There is no installer and no auto-start in v2 — if
you want it at login, put a shortcut to the `.exe` in the Startup
folder (`Win + R`, type `shell:startup`).

## Tray menu (v2)

- Size: Small / Medium / Large
- Hide on fullscreen (off by default)
- Media controls (off by default)
- Widget: System stats (off by default)
- Widget: Calendar (off by default)
- Animations (ON by default)
- When media controls are on: Media: Previous track / Play / Pause /
  Next track
- About, Quit

## Honest limits — what was NOT verified on Windows

This v2 code was written and reviewed in a Linux VM with no Windows
toolchain, so **none of it has been compiled or run on Windows yet**.
Do not treat any behaviour below as tested until Kumar builds it
with `build.bat` on Windows and runs it:

- **Not compiled on Windows.** MSVC `cl`/`rc` and MinGW `windres`
  steps are written from the v1 build pattern plus the standard
  resource-compile step, but have not been run. A compile error in
  the new code (e.g. a Win32 API declaration difference) is possible
  and would only show up at that first Windows build.
- **Not run on Windows.** Multi-monitor positioning, DPI scaling,
  monitor add/remove rebuild, fullscreen hide/restore, media keys,
  stats, and calendar rendering are all untested on real hardware.
  In particular: the tray menu is owned by a hidden manager window
  (the standard tray-icon pattern, but untested here), and the
  fullscreen check is a heuristic (see below).
- **Fullscreen detection is a heuristic.** It counts a window as
  fullscreen when it is visible, not minimised, not one of
  OnyxNotch's own windows, not the desktop, and its rect covers the
  whole monitor rect. Borderless games that keep a caption, or
  fullscreen apps that do not quite cover the full monitor rect,
  may be missed, and the check only runs once per second.
- **Media title/artist are not shown** (see section 4). Media keys
  depend on the active player honouring them.
- **Stats CPU % is a 1-second delta**, not an instantaneous value,
  and shows 0% for the first second after the widget is enabled.
- **Idle memory under 10 MB is still a target, not a measurement.**
  Measure it in Task Manager after running v2, with widgets off and
  then on, and compare.
- The `.exe` built here is unsigned, so Windows SmartScreen may
  warn on first run. That is normal for a self-built unsigned app;
  code-signing is a paid certificate, not a code change.

## Honest limits — v3 additions (not verified on Windows)

- **Not compiled on Windows.** This Linux VM has no Windows
  toolchain, so `cl`/`rc` were never run on the v3 code. The
  changes are small and use only standard Win32 APIs
  (`SetTimer`/`KillTimer`, `GetTickCount64`, `RoundRect`,
  ClearType font quality), but a compile error is still
  possible until the first Windows build.
- **Animation smoothness is untested.** 220 ms / 16 ms frames
  should feel smooth on a normal machine, but frame pacing,
  flicker during animation, and behaviour on very high-DPI or
  very slow machines have not been seen. If animation
  flickers, turn **Animations** off in the tray and report it
  — that restores v2 behaviour exactly.
- **Hover/pressed states are untested on screen.** The colour
  steps (normal 38, hover 62, pressed 90 grey) are chosen to be
  visible against the black notch, but have not been seen on a
  real display; they may need tuning.
- **First-show fade is intentionally absent** (see v3 section 3).
- **Idle memory under 10 MB is still a target, not a
  measurement**, as in v2. The animation timer runs only
  during a ~220 ms size change, so it should not change idle
  memory, but measure it in Task Manager to confirm.

## Limitations carried over from v1

- No notifications, calendar sync, or other networked widgets.
  The calendar widget is a local month view only.
- No auto-start, no installer.

## Files

- `main.cpp` — the whole app: Win32 windows (one per monitor plus a
  hidden manager window for tray/timer), rounded notch region,
  1-second timer, tray icon, battery via `GetSystemPowerStatus`,
  media keys, stats via `GetSystemTimes`/`GlobalMemoryStatusEx`,
  calendar grid.
  v3 adds: per-notch animation timer (~220 ms, ease-out),
  media-button hover/pressed states, notch hover outline,
  ClearType fonts, and the Animations tray tick.
- `build.bat` — MSVC build script (resource compile + compile/link).
- `onyxnotch.rc` — resource script binding the icon (ID 100).
- `onyxnotch.ico` — custom black-pill icon, 16/32/48 px, 32-bit.
- `.gitignore` — build outputs (`OnyxNotch.exe`, `.obj`, `.res`,
  etc.) are not meant to be committed.
- `README.md` — this file.
