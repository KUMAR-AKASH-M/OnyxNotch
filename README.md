# OnyxNotch v7 — Direct2D rewrite of v6

Mac-style notch for Windows, in native C++ Win32. One source file, one
`.exe`, no Electron, no Python, no .NET runtime.

**Scope of this folder:** v7 keeps every v6 feature and behaviour
and replaces ONLY the rendering layer: GDI painting is replaced by
Direct2D/DirectWrite rendering on a layered window with per-pixel
alpha, and expand/collapse animation uses a damped spring instead
of ease-out cubic. Nothing else was added: no weather, no notes, no
clipboard history, no screen-time tracking, no new data sources.
Album-art accent tinting is deliberately OUT of scope (SMTC gives
title/artist, not album art, in our path) and is not faked.

## What's new in v7

### 1. Direct2D rendering on a layered window (the headline)
- The notch window is now `WS_EX_LAYERED` (still topmost,
  toolwindow, and `WS_EX_NOACTIVATE`, so it still never steals
  focus). Its shape comes from the alpha channel of the bitmap
  presented with `UpdateLayeredWindow`, NOT from a window region:
  the old `SetWindowRgn` path is gone, and corners are truly
  anti-aliased instead of region-stepped.
- Each frame is rendered offscreen into a 32-bit premultiplied
  DIB with an `ID2D1DCRenderTarget`, then presented once with
  `UpdateLayeredWindow`. This is the v6 double-buffer idea in
  Direct2D terms: no partial frames, no erase flicker. If a frame
  fails to render (BindDC or EndDraw error), the previous frame
  is kept instead of presenting a blank bitmap.
- The render target DPI is fixed at 96, so Direct2D units are
  pixels and every Layout coordinate (already DPI-scaled) maps
  1:1, exactly as in v6. Per-monitor DPI still works because
  Layout and font sizes are scaled by the notch DPI.
- Text is DirectWrite (Segoe UI, same sizes and weights as v6:
  clock 20 normal, big 36 light, small 17 normal, tiny 14
  normal, all DPI-scaled). Track titles and other long text are
  truncated with an ellipsis by measurement, as in v6.
- All glyphs (dock icons, gear, close X, media buttons, battery)
  are redrawn as Direct2D geometry (paths and arcs), centred in
  the same hit areas, with the same hit order: gear, then close
  X, then dock, then media buttons, then pin.
- Cards now have a soft shadow: a dark rounded rect offset 2 px
  below each card (and under the dock pill), alpha-blended,
  drawn before the card. The PANEL itself has no external shadow:
  the window bounds are exactly the panel bounds, so an outside
  shadow would be clipped by the bitmap. That omission is
  deliberate and documented here, not a bug to paper over; a
  full external shadow would need a transparent margin window
  (window larger than the panel), which would shift every hit
  test and is left as future work.
- This rendering code is original to OnyxNotch, inspired by (not
  copied from) the Windhawk Dynamic Island mod's approach:
  native C++, Direct2D, layered/alpha presentation, spring
  animation. Windhawk code is MIT licensed, but no Windhawk code
  is in this file; the techniques were reimplemented for our
  layout, themes, and window manager. A Windhawk mod also differs
  in kind: it runs inside windhawk.exe with Windhawk's settings
  panel, while OnyxNotch is a standalone .exe with its own tray
  menu, settings file, auto-start, and multi-monitor windows.

### 2. Spring animation (replaces ease-out cubic)
Expand/collapse now uses a damped spring integrated per timer
tick: stiffness 170, damping 20 (damping ratio about 0.77), so
the panel settles with a small natural overshoot and then snaps
exactly to target. Starting a new animation mid-flight restarts
from the current window size, so reversing direction (expand,
then collapse immediately) is smooth, not a jump. The timer is
still per-notch, 16 ms, started only for a size change and
killed on finish, with a 700 ms cap that guarantees the snap
even if the spring stalls. Position is elapsed-based, so motion
does not depend on the timer firing exactly on schedule; the
16 ms cadence itself is still timer-limited (no `timeBeginPeriod`,
which would be a global side effect), so on a high-refresh
display the animation is smoother than v6's but is not yet
locked to the display refresh rate. Tab switch still only
repaints (no size animation). Animations OFF still restores
instant behaviour everywhere.

### 3. Blur background — OFF by default, experimental
A new tray toggle, "Blur background (experimental)", saved as
`blur=0/1` in settings.ini (default 0). When on, it calls the
runtime-resolved `SetWindowCompositionAttribute` accent API
(blur-behind) for each notch window. Safety rules:
- When off, or if the accent call fails, the panel is fully
  opaque exactly as in v6. Blur can never black out the window,
  because the layered bitmap always carries our own drawn panel.
- When blur is active, the panel gradient is drawn at 0.88
  alpha so the blurred desktop can show through; cards stay
  opaque. With an opaque panel, blur would be invisible, which
  is why the alpha change is part of the toggle.
- Blur needs Windows Transparency effects on; with them off,
  Windows shows a flat tint instead of blur, and the panel
  still draws correctly (it is just translucent).
- Layered window + accent blur together is UNTESTED on real
  Windows: Windows may ignore the accent on a layered window,
  in which case the panel will simply look like a translucent
  dark panel. That is an acceptable fallback, not a failure
  mode to hide: please report what you actually see.
- Blur is in the tray menu (and therefore the gear menu, which
  opens the same menu), like every other toggle.

### 4. What was preserved, exactly
Multi-monitor (one notch per monitor, per-monitor DPI, rebuild
on display change); hide-on-fullscreen OFF opt-in; custom tray
icon; media controls OFF opt-in with SMTC titles (header-only
C++/WinRT, media-keys-only control, exact v4 fallback on any
SMTC failure); widgets OFF opt-in (stats, calendar); saved
settings in `%APPDATA%\OnyxNotch\settings.ini` (now also
`blur`); auto-start OFF via HKCU Run (registry is source of
truth); themes Onyx/Midnight/Graphite (colour-only, through
Direct2D brushes, no layout change per theme); card+dock look
(about 840 px at 96 DPI, clamped to work width minus 32, 320
floor); HOME 3-column grid; SYSTEM (Battery, Network,
Performance sparklines, Volume read-only); CLOCK (big clock +
Local/New York/London/Tokyo, unresolvable city omitted);
CALENDAR (fixed 6-week grid, prev/next arrows, Today button,
28 px cells, faded adjacent-month days, muted weekends, accent
today pill, outlined selected day, honest "No events",
selection resets to today on tab leave/collapse, not saved);
unified tab size (max over visible tabs, header/dock at the
same Y every tab, shorter tabs centred, tab switch repaints
only, feature toggle may change the unified height via the
animated path); tray/gear menus; no taskbar; no focus steal;
1-second timer; Animations ON by default.

### 5. Build on Windows (MSVC only)
From a Visual Studio Developer Command Prompt, in this folder,
run `build.bat`:

```bat
rc /nologo onyxnotch.rc
cl /nologo /std:c++17 /EHsc /O2 /MT /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS main.cpp onyxnotch.res user32.lib gdi32.lib shell32.lib windowsapp.lib advapi32.lib iphlpapi.lib winmm.lib d2d1.lib dwrite.lib /link /SUBSYSTEM:WINDOWS /OUT:OnyxNotch.exe
```

New over v6: `d2d1.lib` (Direct2D) and `dwrite.lib`
(DirectWrite). Both are also listed as `#pragma comment(lib)`
in main.cpp, so the build works from the cl line alone as
well. `gdi32.lib` is still linked: the offscreen bitmap is a
GDI DIB selected into a memory DC for the Direct2D DC render
target, and `UpdateLayeredWindow` takes that DC. MinGW cannot
build v7 (C++/WinRT SMTC part, as in v5/v6). `build.bat` keeps
its safety properties: goto error labels, zero parenthesis
characters anywhere, and the coroutine silence macro guarded
with `#ifndef` in code (never also on the cl line). Quit any
running OnyxNotch first (tray Quit, or
`taskkill /F /IM OnyxNotch.exe`) or the link will fail with
LNK1104 because it cannot overwrite the running .exe.

### 6. Honest list of what was NOT verified on Windows
- NOT compiled on Windows. This Linux VM has no Windows
  toolchain, so `cl`/`rc` were never run on v7. Static checks
  passed (braces/parens/brackets balance; no GDI paint calls
  remain except the DIB memory DC and the screen DC for
  `UpdateLayeredWindow`; `build.bat` has zero parenthesis
  characters), but a compile error is still possible until
  the first Windows build. The most likely risk areas, in
  order, are: Direct2D/DirectWrite API signatures, the
  `SetWindowCompositionAttribute` runtime resolution, and the
  layered-window presentation path.
- Whether Direct2D + layered window + NOACTIVATE actually work
  together on a real display is untested: the notch could in
  principle show nothing, show without input, or flicker on
  first show if `WM_PAINT` semantics for layered windows differ
  from what was assumed. Mitigations already in the code:
  explicit presentation after every size change (not only on
  `WM_PAINT`), and never presenting a failed frame.
- Blur safety and appearance are untested (see section 3):
  please test with the toggle on and off, with Transparency
  effects on and off, and report whether blur appears, the
  panel looks translucent-dark, or anything goes black.
- Animation smoothness, spring overshoot feel, and the 16 ms
  cadence on a high-refresh display are untested, as are DPI
  behaviour (per-monitor scaling of the Direct2D frame) and
  the under-10 MB idle target (a target, not a measurement).
- Icon legibility in Direct2D (thicker geometry vs the old
  GDI glyphs) and calendar/day hit alignment are untested on
  screen, although painting and hit tests still share the
  same Layout computation.
- If v7 does not run at all, v6 (commit ce42cbf, unified tab
  size, GDI painting) remains the known-good fallback: it is
  untouched in `~/workspace/onyxnotch-v6/`.

# OnyxNotch v6 — Card + dock dashboard look, existing data only

Mac-style notch for Windows, in native C++ Win32. One source file, one
`.exe`, no Electron, no Python, no .NET runtime.

**Scope of this folder:** v6 is a visual redesign to the reference
dashboard look the user chose: a wide expanded panel with a header
bar, a card grid, and a bottom icon dock. It uses only data already
available locally. Everything from v5 (SMTC titles, saved settings,
auto-start, themes, animations, tabs) is otherwise unchanged in
behaviour, except that text tabs are replaced by the dock and the
old clock/date/battery header block is replaced by the header bar
plus cards.

## What's new in v6

### 1. Wide expanded panel — about 840 px at 96 DPI
The expanded panel is about 840 px wide at 96 DPI, DPI-scaled, and
clamped to the monitor work width minus margins so it never
overflows a narrow monitor. The collapsed notch is unchanged: a
clock pill only. Height is computed from the card layout per tab
and animates through the existing 220 ms animation path. The Size
menu still controls only the collapsed width, as before.

### 2. Header bar
The top of the expanded view is now a header bar, not the old big
clock/date/battery block. Left: the tab title (Home, System, Clock,
Calendar, Media) with a subtitle under it — Home shows the current
date, System shows "Power · Network · Audio", Clock shows
"Local Time & Date", Calendar shows the current month and year,
Media shows the SMTC artist or "Media controls". Right: battery %
with the existing battery glyph, the settings gear, and a close X.
The close X collapses the panel back to the clock pill only; it
does NOT quit the app. Quit stays in the tray menu only. The big
clock, date, and battery content now lives in cards below, not
duplicated in the header.

### 3. Bottom icon dock — replaces the text tab bar
A centred pill-shaped dock at the bottom of the expanded view, with
plain-GDI icon glyphs (no image or font glyphs) in this order:
Home, System, Media, Clock, Calendar. Home and Clock are always
present; System appears only if the stats widget is on, Media only
if media controls are on, Calendar only if the calendar widget is
on. The active icon is a white circle with a dark glyph; inactive
icons are muted grey and brighten on hover. Clicking a dock icon
switches tabs exactly as the old text tabs did. Hit order is gear
first, then close X, then dock, then media buttons, then pin, so a
dock click never pins and never fires a media key. Tab selection
still resets to HOME on collapse and falls back to HOME if a
feature is toggled off.

### 4. Cards
Every body section is now a dark rounded-rectangle card, slightly
lighter than the panel background, with a small uppercase muted-grey
label, a big near-white number, and a muted secondary line — the
reference's visual language. Card colours come from the current
theme, so the accent is the theme accent (Onyx near-white, Midnight
blue, Graphite light grey), not always blue. All cards are drawn in
plain GDI only.

### 5. HOME tab — 3-column card grid, summaries only
Home is a 3-column grid (2 rows when everything is on) of summary
cards, one per enabled feature, and it stays a summary — never the
full detailed sections. Cards: System (CPU/RAM/Disk rings with %
in the centre), Calendar (current week only, today pill), Clock
(current time in 12-hour format + date), Media (SMTC title/artist
or "No track" + functional mini media buttons), Battery (%,
state, accent bar), Network (adapter name + up/down speeds).
System, Battery, and Network cards appear only if the stats widget
is on; the Media card only if media controls are on; Clock and
Calendar cards follow their own availability (Clock always,
Calendar only if the calendar widget is on). The grid reflows when
cards are absent.

### 6. Detailed tabs — existing/local data only
- **SYSTEM:** Battery card (%, state, accent bar), Network card
  (adapter name + up/down speeds), Performance card (CPU/RAM/Disk
  % with small sparkline graphs from a 30-sample history buffer),
  Volume card (system volume % with an accent slider bar).
  The volume bar is read-only display: the notch is no-activate,
  so it is not a draggable slider in v6.
- **CLOCK:** a big digital clock card (12-hour time, date, Local
  label) plus a World clocks card (Local, New York, London, Tokyo
  with times and UTC offsets, computed from Windows timezone data).
  A city whose timezone cannot be resolved is omitted, never shown
  with a wrong time.
- **CALENDAR:** a month grid card (as before, today pill) plus an
  agenda side card (today's date, the current week's days, and an
  honest "No events" line). No holidays or events are invented.
- **MEDIA:** SMTC title/artist (large) + full-size media buttons,
  as in v5.

### 7. New local data and their fallbacks
- **Disk %:** `GetDiskFreeSpaceEx` on the system drive (C:),
  % used plus used/total GB, sampled on the existing 1-second
  timer only while the stats widget is on.
- **Network up/down:** local interface byte counters via
  `GetIfTable`, rates from 1-second deltas, only while the stats
  widget is on. If counters are unavailable, the cards show
  "Network unavailable" rather than fake speeds.
- **Volume %:** default playback device level via `waveOutGetVolume`
  (plain Win32, no COM). If it cannot be read, the Volume card
  shows "Volume unavailable" rather than a fake value.
- **Sparklines:** at most 30 samples, sampled only while the stats
  widget is on, so idle cost does not grow when everything is off.
  No new timers, no polling beyond the existing 1-second timer,
  no network calls.

### 8. What was deliberately NOT added in v6
No weather (needs network and a weather API), no notes (needs a
local store and text input in a no-activate window), no clipboard
history, no screen-time tracking. Those were the other cards in
the reference screenshots and were explicitly excluded by the
user's "existing data only" choice. Tray menu, gear menu, Themes
submenu, saved settings, and auto-start are unchanged from v5.
The Clock tab needs no toggle: it is always available when
expanded.

### 9. Build
From a Visual Studio Developer Command Prompt, in this folder, run
`build.bat`. It is MSVC only (C++17, `/EHsc`, and the coroutine
silence macro from the v5 build fix, all preserved). New libraries
over v5: `iphlpapi.lib` (network counters) and `winmm.lib`
(volume). `build.bat` keeps the goto-label, zero-parenthesis
property from the v5 build fix: there are no parenthesis
characters anywhere in the file, so a build failure can no longer
trigger a "was unexpected" cmd parsing error that masks the real
error.

### Honest limits — v6 additions (not verified on Windows)
- Not compiled on Windows: this Linux VM has no Windows toolchain,
  so `cl`/`rc` were never run on the v6 code, especially the new
  Win32 APIs (`GetIfTable`, `waveOutGetVolume`,
  `EnumDynamicTimeZoneInformation` /
  `SystemTimeToTzSpecificLocalTimeEx`) and the new `iphlpapi.lib` /
  `winmm.lib` links. A compile or link error is still possible
  until the first Windows build.
- Card/dock balance at 840 px, icon glyph legibility at small
  size, ring/sparkline rendering, and the header/close-X layout
  have not been seen on a real display and may need tuning.
- World-clock correctness (especially daylight-saving offsets),
  network up/down readings, and volume readings have not been
  verified against a real machine; a city that cannot be resolved
  is omitted by design, which may look like a missing row.
- Sparklines, DPI behaviour, and animation interplay (height
  changes on dock switching) are untested on screen; the
  Animations tray tick remains the instant fallback.
- Under-10 MB idle remains a target, not a measurement.

# OnyxNotch v5 — SMTC titles + saved settings / auto-start + themes (features 1, 2 and 3 of v5)

Mac-style notch for Windows, in native C++ Win32. One source file, one
`.exe`, no Electron, no Python, no .NET runtime.

**Scope of this folder:** v5 features 1 (SMTC track titles, below),
2 (saved settings + auto-start, below) and 3 (themes, below) are
implemented here. Everything from tabbed v4 is otherwise unchanged.

**Superseded note:** the v2 section below says no track title/artist
is shown. That was true for v2–v4 and is superseded by the v5-SMTC
section below: with Media controls on, titles/artists are now shown
when SMTC reports them.

## What's new in v6 — Flicker, icons, and calendar fix (in place, after the v6 push)

This is a fix release inside v6, not a v7. It answers the report
"it's ui fluctuate and make icons ui and calendar ui better".

### 1. What caused the fluctuation, and what changed

Three causes were found in code, not in the user's machine:

- **Painting went straight to the screen, in pieces.** Every
  `WM_PAINT` drew the gradient row by row, then cards, text, and
  icons directly into the screen DC, so a 16 ms animation frame
  (and the 1-second tick) was visible half-drawn. Fix: `WM_PAINT`
  now paints the whole notch into a memory DC with a compatible
  bitmap sized to the client area, then copies it to the screen in
  a single `BitBlt`. Nothing partial is ever shown.
- **Erase-on-invalidate was still on in the size path.**
  `ApplyTargetSize`, the animation finish step, and one manager
  toggle path passed `TRUE` to `InvalidateRect`, marking the
  window for background erase between frames. `WM_ERASEBKGND`
  already returned 1, but the flag is now `FALSE` everywhere, so
  no erase step runs at all.
- **Heights snapped.** The calendar card was 4, 5, or 6 weeks tall
  depending on the month, and the MEDIA card grew or shrank when
  a track appeared or disappeared. Both are now fixed-height:
  the calendar card is always 6 weeks (blank rows for short
  months), and the MEDIA layout always reserves title + artist
  rows whenever media is on, showing "No track" when nothing
  plays. The manager timer therefore no longer resizes anything
  on an SMTC change; it just repaints. Sparklines, history, and
  network speeds were already inside fixed-height cards and are
  unchanged in that respect.

One cheap, provably safe dirty-region cut was also made: a
collapsed notch shows only the clock (HH:MM), so the 1-second
timer now skips repainting a collapsed notch unless the minute
changed. Expanded notches still repaint every tick (stats,
sparklines, SMTC, and the clock card all move per second) — into
the double buffer, so that repaint no longer flashes. Nothing was
skipped where content could go stale.

The window region is still updated per animation frame, because
the window itself changes size; with painting offscreen, that
update no longer shows partial frames.

### 2. Icons — redrawn GDI on one grid, not MDL2

Segoe MDL2 Assets was considered and deliberately NOT used: its
glyph codepoints cannot be verified in the Linux VM, and a wrong
or missing glyph would render as a tofu box on a user's machine.
Hand-drawn GDI glyphs were redrawn instead, on one system: every
glyph lives in a circle of radius 3/4 of its hit radius, strokes
are at least 1/5 of that radius thick, and every glyph is centred
on its hit centre. That covers the dock icons, the header gear
(chunkier teeth: radial 0.55r, width 0.45r), the close X (stroke
now scales with its radius instead of a fixed 2 px), and the
media glyphs (now 70% of the button diameter, was 55%, with
thicker bars). Hit areas are unchanged in size and still match
the drawn glyphs exactly: gear first, then close X, then dock,
then media buttons, then pin. Dock active is still a white
circle with a dark glyph; inactive icons are muted and brighten
on hover.

### 3. Calendar — navigation, selection, fixed height

- **Header:** month + year centred, with clickable `<` and `>`
  arrows at the sides (hover brightens them) that move the
  displayed month, per notch. A centred **Today** button below
  the grid returns to the current month, as does clicking the
  today pill when it is visible.
- **Grid:** fixed 6 weeks, 28 px cells (was 20 px), weekday header
  (S M T W T F S) in muted grey, larger cells with more breathing
  room. Days of adjacent months are shown faded (muted) instead
  of blank cells, weekend days are muted, today is a strong
  theme-accent pill with dark text, the selected day is an
  accent outline pill, and a hovered day gets a muted outline.
- **Selection:** clicking any day selects it, including an
  adjacent-month day, which also moves the display to that month.
  The agenda side card follows the selection: it shows the
  selected date and weekday, "Today" or "Selected day", that
  week's days with the selected day highlighted, and an honest
  "No events" line. No holidays or events are invented.
- **Reset choice (documented):** leaving the calendar tab, or
  collapsing the panel, resets the displayed month to the current
  month and the selection to today, so returning to the calendar
  always starts at today. The selection is not saved to
  settings.ini.

### 4. What did not change

Themes, tabs/dock order and visibility, the Home grid, the
System/Clock/Media cards, the tray/gear menus, saved settings,
auto-start, the SMTC fallback, and `build.bat` (goto labels, zero
parenthesis characters, C++17, `/EHsc`, coroutine silence macro)
are all unchanged. Only `main.cpp` and this README changed in
this fix.

### 5. Build

Unchanged from v6: from a Visual Studio Developer Command Prompt,
in this folder, run `build.bat` (MSVC only; MinGW cannot build
the WinRT part). It links `windowsapp.lib`, `advapi32.lib`,
`iphlpapi.lib`, and `winmm.lib`.

### Honest limits — v6-fix (not verified on Windows)

- **Not compiled on Windows.** This Linux VM has no Windows
  toolchain, so `cl`/`rc` were never run on the fixed code.
  Static checks passed (braces, parentheses, and brackets all
  balance; no `InvalidateRect` with `TRUE` remains; `build.bat`
  still has zero parenthesis characters), but a compile error is
  still possible until the first Windows build.
- **Whether the flicker is actually gone on a real display is
  untested.** Double buffering plus no-erase should remove the
  visible flashing, but animation-frame tearing, region-update
  interaction, and high-DPI behaviour have not been observed.
  If any flicker remains, the Animations tray tick is still the
  instant fallback (it stops the animation path entirely).
- **Icon legibility and calendar clicks are untested on screen:**
  the thicker glyphs, the 28 px calendar cells, and the arrow /
  Today / day hit areas have not been clicked through on a real
  display, and the world-clock, network, and volume readings are
  unchanged from v6 in that respect.
- **Heights are arithmetic, not measurements**, and under-10 MB
  idle remains a target, not a measurement.

## What's new in v6 — Unified tab size (in place, after the v6-fix)

This is a fix inside v6, not a v7. It answers the report "every
tab have it's own size that looks odd".

### 1. One expanded size for every tab

Previously each tab computed its own total height (Home, System,
Clock, Calendar, and Media all differed), so switching tabs
jumped the panel size and moved the dock. Now the expanded panel
has exactly one size:

- `ComputeLayout` builds each visible dock tab's layout at its
  natural height (Home, System, Media, Clock, Calendar — only
  the tabs that are actually visible for the current toggles:
  System only if stats are on, Media only if media controls are
  on, Calendar only if the calendar widget is on; Home and Clock
  are always present) and takes the **maximum** total height
  over all of them.
- The panel width never changed between tabs and still does not
  (about 840 px at 96 DPI, clamped to the monitor work width).
- The header (title, subtitle, battery, gear, close X) and the
  bottom icon dock therefore sit at exactly the same Y on every
  tab, because they are placed from the unified height, not from
  a per-tab height. The tallest tab has no extra space; every
  other tab is centred inside the same body area (see below).

### 2. How shorter tabs are placed — centred, all of them

A shorter tab's body content is vertically centred within the
fixed body area between the header and the dock: the leftover
space is split evenly above and below the content, so a short
tab never looks like a small island at the top with a huge empty
gap above the dock. This centring is used for **every** tab,
including Calendar — its card is already a fixed 6-week height,
so centring it just splits the remaining space evenly instead of
leaving it all below. No tab is deliberately top-aligned.

Centring is not a paint-only trick: painting and every hit test
(media buttons, calendar arrows/days/Today, dock, gear, close X)
all call the same `ComputeLayout`, so a click lands where the
content is drawn, on every tab. Header geometry is never
shifted, so the gear and close X keep their exact positions.

### 3. What changes the height now

- **Tab switch:** only repaints content (into the double
  buffer). It deliberately does **not** run the size-animation
  path, since the size does not change. A switch therefore
  cannot jump, resize, or move the dock.
- **Expand / collapse:** still animate (about 220 ms, ease-out)
  when Animations is on, and snap instantly when it is off —
  between the collapsed clock pill and the unified expanded
  size.
- **Feature toggle on/off:** may change the unified height,
  because the set of visible tabs changed and therefore the
  maximum changed. That change goes through the existing
  animated size path, and it is the only time the expanded
  height may change besides expand/collapse.
- Data changes never change the height: the calendar card is
  already fixed at 6 weeks, the MEDIA layout already reserves
  title + artist rows, and sparklines, history, and network
  speeds already live inside fixed-height cards.

### 4. What did not change

Card contents, the Home grid, the System/Clock/Calendar/Media
cards, the calendar's fixed 6-week height and reserved MEDIA
rows, themes, the tray/gear menus, saved settings, auto-start,
the SMTC fallback, double-buffered painting, and `build.bat`
(goto labels, zero parenthesis characters, C++17, `/EHsc`,
coroutine silence macro) are all unchanged. Only `main.cpp`
and this README changed in this fix.

### 5. Build

Unchanged from v6: from a Visual Studio Developer Command Prompt,
in this folder, run `build.bat` (MSVC only; MinGW cannot build
the WinRT part). It links `windowsapp.lib`, `advapi32.lib`,
`iphlpapi.lib`, and `winmm.lib`.

### Honest limits — unified tab size (not verified on Windows)

- **Not compiled on Windows.** This Linux VM has no Windows
  toolchain, so `cl`/`rc` were never run on the changed code.
  Static checks passed (braces, parentheses, and brackets all
  balance once comment/string contents are excluded; the raw
  bracket count differs by one in both the old and the new code
  because of a `[` inside a comment or string literal), but a
  compile error is still possible until the first Windows build.
- **Whether all tabs really are the same size on a real display
  is untested**, as is whether the centred content looks
  balanced on the shortest tabs (Media and Clock, which get the
  most centring when System is the tallest tab). Hit-test
  alignment on centred shorter tabs, per-monitor DPI behaviour,
  and animation interplay (expand/collapse to the unified size)
  have likewise not been observed. If anything still jumps, the
  Animations tray tick remains the instant fallback for the
  expand/collapse path (it does not affect tab switches, which
  no longer animate at all).
- **Heights are arithmetic, not measurements**, and under-10 MB
  idle remains a target, not a measurement.

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

## What's new in v4 — Premium UI only

UI-only redesign. No new features, no behaviour changes, no
changed defaults, no new tray toggles. Everything from v3
(multi-monitor, animations, hide-on-fullscreen, media controls,
widgets, toggles) works exactly as before — only how it looks
changed.

### 1. Premium surface
Flat black is gone. The notch now paints a vertical near-black
gradient (slightly lighter at the top, darker at the bottom),
with a subtle 1 px light border and a soft inner top highlight
line, inset so it never touches the rounded corners. The pill
shape is unchanged: the same rounded region as v3, applied frame
by frame during animation, so the gradient moves with the notch
as it expands and collapses. The gradient is drawn row by row
with plain GDI — no new library, no image asset.

### 2. Premium typography and spacing
Segoe UI throughout (as in v3), with lighter weights: the big
clock drops from semibold to light, the collapsed clock from
medium to normal. Primary text (clocks, calendar month) is
near-white; secondary text (date, battery, labels, stats) is a
muted grey. Spacing is more generous: the date and battery rows
sit slightly lower with more air, and the expanded sections
start a few pixels further down. What text is shown is
unchanged — only weight, colour, and spacing changed.

### 3. Premium media buttons — circles
The rectangular media buttons are now circles: diameter equals
the media-row height, centred on the old button positions, so
the row stays centred and balanced. Hover and pressed states
are the same restrained idea as v3 (lighter circle on hover,
lighter still on press, on slightly raised greys). The hit test
was updated to match: it now tests distance from each circle's
centre, so a visible circle is exactly what is clickable and
the invisible corners of the old rectangles no longer respond.
The glyphs themselves (Prev bar + double triangles, Play/Pause
triangle + bars, Next double triangles + bar) are unchanged.

### 4. Premium details
- **Divider:** a thin dark line now separates the
  clock/date/battery block from the media/widgets sections
  below it. It only appears when a section follows; the
  header-only expanded view has no stray line.
- **Battery glyph:** next to the battery text, a small drawn
  battery outline with a fill proportional to the percentage
  and a lightning bolt when charging. On a desktop / unknown
  battery, only the outline is drawn and the text stays
  "Battery: --" as before. The text itself is unchanged.
- **Calendar today:** today is now a white pill with dark text
  instead of a plain grey box, so it reads as selected at a
  glance. Other days are muted grey as before.

### 5. Glass / acrylic — deliberately skipped, not faked
Windows acrylic/blur (SetWindowCompositionAttribute) was
evaluated and deliberately NOT included. The notch is a
region-shaped, non-layered, topmost, no-activate window, and
acrylic's interplay with that shape cannot be verified without
a Windows test; a failed or half-applied attempt risks a
black, transparent, or flickering notch. The shipped look is
therefore the solid gradient version above, which renders
identically with or without composition. If acrylic is wanted
later, it should be done as its own change with a
layered-window test on Windows, not bundled into a UI pass.

### 6. Performance — unchanged
No new timers, no polling, no new threads, no per-frame
allocations that grow idle memory. The gradient is drawn only
during a paint (the same paints v3 already did), the animation
timer still runs only during a ~220 ms size change, and the
clock timer is still 1 second. Idle cost should be identical
to v3 — measure it in Task Manager to confirm.

## What's new in v4 — Reorganization (in place, after the v4 push)

No new version number, no new features, no changed defaults,
no settings file. The v4 premium look (gradient, border,
typography, circular media buttons, battery glyph, calendar
pill) is unchanged; only how the expanded view is organized
changed, plus one new button.

### 1. Compact two-column body
The expanded header stays full-width: big clock, date, battery
with glyph, then the divider. Below the divider, media and
system-stats now sit **side by side** when both are on — each
about half the notch width (at the standard 340 px expanded
width: 16 px outer margins, 12 px column gap, 148 px columns).
If only one of them is on, that section spans the full body
width (media buttons spread across it, as before; stats rows
stretch across it). Calendar, when on, stays full-width below
the two-column row.

Height effect (standard size, 5-row month where a calendar is
shown; measured from the layout arithmetic, not from a run):
- Media + stats, no calendar: 210 px, down from 236 px.
- Media + stats + calendar: 364 px, down from 388 px.
- Media only: 188 px, down from 212 px. Header only: 138 px,
  down from 156 px (the footer row is gone, see below).
- Stats only is the one exception that grows: 210 px, up from
  180 px — because stats is now two labelled rows with bars
  instead of one dense line (see section 2). That is a
  deliberate trade for readability; the both-on case, which is
  the tall one users actually hit, still gets shorter.

### 2. Grouped section headers, and stats split into rows
Each body section now has a small uppercase muted-grey header,
left-aligned in its column: **MEDIA**, **SYSTEM**,
**CALENDAR**. The calendar header shares the month row (label
on the left, month centred), so it costs no extra height.
Stats is no longer one dense line (`CPU: x%   RAM: y%
(used / total GB)`). It is now two labelled rows, each with a
thin progress bar underneath (dark track, near-white fill,
same CPU/RAM values as before):
- `CPU  x%` with a CPU % bar.
- `RAM  y%  ·  used/total GB` with a RAM % bar.
The media header replaces the old "Media controls (media
keys)" centred label; the media-keys explanation is in the
About box and in the v2 media section above.

### 3. Settings gear button in the expanded header
A small gear button now sits in the expanded header's
top-right corner (centre about 24 px down, 16 px + gear radius
in from the right edge; gear radius 11 px at 96 DPI, DPI-scaled).
It is drawn in plain GDI — 8 teeth, a solid ring, a dark
centre hole — with no font glyph and no image asset. Colours
follow the media buttons' hover language: muted grey normally
(165), brighter on hover (210), brightest on press (240).
Clicking it opens the **existing tray menu** (Size, toggles,
About, Quit) at the gear's screen position, right-aligned
under the gear, so settings are reachable from the notch
without going to the tray. No new settings window or panel
was built, and no settings file was added: toggles still reset
at restart, as before.

### 4. Footer removed, pinning unchanged
The always-visible expanded footer ("OnyxNotch v4 | Click to
pin") is gone — it cost a full row in every expanded view
for information you only need once. Pinning works exactly as
before: click the notch header to pin it open, click again to
unpin, move away to collapse when unpinned. The pin
explanation now lives in the About box text and in this file
only. The gear click is hit-tested **before** the pin click,
so clicking the gear opens the menu and never pins or unpins.
Media-button hit testing is unchanged in spirit: circles are
still computed from the shared layout, now from the column
rects, so a visible circle is exactly what is clickable.
Collapsed behaviour is unchanged (clock only), and the
animation path is unchanged: animated sizes interpolate to
the new total height, and expanded content still paints until
a collapse finishes, so nothing pops mid-collapse.

## What's new in v4 — Tabs (in place, after the reorganization)

No new version number, no new features, no changed defaults,
no settings file. The v4 premium look and the reorganized
layout are unchanged in spirit; the expanded body is now
organized into tabs instead of showing every section at once.

### 1. Tab bar and visibility rules
The tab bar sits in the expanded notch below the header
(clock, date, battery) and its divider, above the body
content. Tabs, in order: **HOME**, then **MEDIA** (only if
media controls are enabled), **SYSTEM** (only if the
system-stats widget is enabled), **CALENDAR** (only if the
calendar widget is enabled). Tabs are uppercase, small, and
evenly spaced across the notch width. The active tab is
near-white text with a near-white underline across its
segment; inactive tabs are muted grey, brighter on hover.
If no media/widget is enabled, there is no tab bar at all:
the expanded view is just the header, exactly as before.
The settings gear stays in the expanded header, top-right,
and works as before; the collapsed notch is unchanged
(clock only).

### 2. HOME — a smaller version of every tab
HOME stacks a compact summary of each enabled feature,
vertically, and is deliberately short:
- **Media summary:** the three media buttons at a smaller
  size (20 px circles instead of 28 px), still fully
  functional — same media keys, same circular hit test
  scaled down, same hover/pressed states.
- **System summary:** one line, `CPU x% · RAM y%` — no
  bars, no GB figures (those live on the SYSTEM tab).
- **Calendar summary:** the current week only — 7 day
  cells, Sunday start, today as the white pill — no month
  grid, no month header, no weekday header row.
With all three features on, HOME is 246 px tall at the
standard size (96 DPI), versus 364 px for the old view
that showed every detailed section at once.

### 3. Detailed tabs — the full versions, unchanged
Clicking a tab shows that feature's full version, with the
same data and behaviour as before the tabs, only moved
behind its tab:
- **MEDIA:** the MEDIA header plus the full-size circular
  media buttons (28 px), as in the reorganization.
- **SYSTEM:** the SYSTEM header, then the CPU row with its
  bar and the RAM row (with GB used/total) with its bar.
- **CALENDAR:** the CALENDAR header sharing the month row,
  the weekday row, and the full month grid with the today
  pill. No track title/artist is added (see the SMTC limit
  in the v2 section); no acrylic is added.

### 4. Switching, height, and reset rules
Clicking a tab switches the body content to that tab. The
hit-test order is: gear first, then tabs, then media
buttons, then pin — so a tab click never pins or unpins,
never fires a media key, and never opens the gear menu.
Switching changes the notch height to the new tab's height
through the existing animation path: it animates (~220 ms,
ease-out) when Animations is on and snaps instantly when
off. The selected tab is per-notch state (each monitor's
notch remembers its own tab). When the notch collapses
(unpinned, mouse leaves), the tab resets to HOME on the
next expand — so every fresh expand starts on HOME. If a
tab's feature is toggled off while that tab is selected
(from the tray menu or the gear menu), the notch falls
back to HOME rather than showing a dead tab.

## What's new in v5 — SMTC track titles (feature 1 only)

No new tray toggle, no new timer, no new thread, no network. SMTC
here is read-only and local: it reads the current Windows System
Media Transport Controls session; it never plays, pauses, seeks,
or sends anything anywhere. Media keys remain the only control
path, exactly as in v2–v4.

### 1. Where the title and artist appear
Media controls must be on (tray **Media controls**, off by default,
as before). Nothing changes when it is off — the SMTC code is not
even called then.

- **MEDIA tab:** below the MEDIA header, the track **title on one
  line** (near-white, small font) and the **artist on the next line**
  (muted grey, tiny font), then the three circular media buttons
  below them. The artist line appears only when SMTC actually
  reports an artist string; a track with no artist shows the title
  line only, never a blank second line.
- **HOME tab:** one compact **title line only** (near-white, tiny
  font) above the small media buttons in the HOME media summary.
  No artist line on HOME, by design — the full title + artist
  live on the MEDIA tab.
- No title/artist anywhere else: not in the collapsed notch
  (clock only, unchanged), not in the header, not on the SYSTEM
  or CALENDAR tabs, and not in the tray menu.

### 2. Truncation rule
Long titles/artists are truncated **by pixels, not by character
count**: GDI `DrawTextW` with `DT_END_ELLIPSIS` cuts the text to
the section width at the current font and DPI and appends an
ellipsis (…). The width is the full body width (MEDIA tab) or the
body width (HOME title line). Separately, the cached strings are
capped at 255 characters, so a pathological player string cannot
grow memory. If a title fits, it is shown whole — no truncation
is faked for short titles.

### 3. Play/pause glyph now reflects playback status
When SMTC reports a clear status, the middle media button's glyph
changes (on both HOME and MEDIA, same glyph code):
- **Playing → pause bars** (two bars: clicking pauses).
- **Paused → play triangle** (clicking plays).
- **Anything else** (Stopped, Changing, Closed, Opened, unknown,
  or no session) → the old combined play-triangle + pause-bars
  glyph, unchanged. The button still always sends the same
  `VK_MEDIA_PLAY_PAUSE` key; only what is drawn changes, and
  only when the status is known.

### 4. Fallback — exactly v4 when SMTC has nothing
If there is no SMTC session, no properties, an empty title, or
any WinRT call fails (all wrapped in try/catch, so a failure
cannot crash the app), the title/artist lines are simply not
drawn and the glyph stays combined — the expanded heights shrink
back to the v4 sizes and the media buttons work via media keys
as before. There is no blank hole: a missing artist costs no
line, a missing track costs no lines at all. Track changes are
picked up on the existing 1-second clock timer, so a new track
(or a track ending) can take up to a second to appear, and a
height change animates through the existing ~220 ms animation
path (or snaps instantly when Animations is off).

### 5. Heights (standard size, 96 DPI, arithmetic, not measured)
Tab bar / header geometry is unchanged; only the body grows:
- MEDIA tab: 220 px with no track (v4), 242 px title only,
  262 px title + artist.
- HOME with all features on: 246 px with no track (v4),
  270 px with a track title line.
- SYSTEM, CALENDAR, header-only, and collapsed heights are
  unchanged. Hit-test order is unchanged (gear first, then
  tabs, then media buttons, then pin), and the media-button
  circles move with the layout because hit tests and painting
  share `ComputeLayout` — a title line never becomes clickable
  and never shifts a button away from its drawn circle.

## What's new in v5 — Saved settings + auto-start (feature 2 only)

No new window, no new timer, no network, no theme system. This is
only persistence for the toggles that already existed, plus one
new tray toggle for starting at login. SMTC titles, tabs, the
gear menu, animations, and every default below are unchanged —
except that the toggles below now survive a restart instead of
resetting, which is the whole point of this feature.

### 1. Settings file — where it is, what is in it
On startup the app loads, before the first notch is drawn:

`%APPDATA%\OnyxNotch\settings.ini`

That is a plain `key=value` text file (no JSON library, no
registry for these keys). The app creates the `OnyxNotch`
folder under `%APPDATA%` if it is missing. A typical file the
app writes looks like this (values shown are the defaults):

```ini
; OnyxNotch settings - rewritten on every change and on clean quit.
; Unknown keys are ignored on load; values are 0 = off, 1 = on.
size=1
hide_on_fullscreen=0
media=0
widget_stats=0
widget_calendar=0
animations=1
theme=onyx
```

Exact keys and defaults:

- `size` — `0` = Small, `1` = Medium (default), `2` = Large.
- `hide_on_fullscreen` — `0` = off (default), `1` = on.
- `media` — Media controls, `0` = off (default), `1` = on.
- `widget_stats` — System stats widget, `0` = off (default),
  `1` = on.
- `widget_calendar` — Calendar widget, `0` = off (default),
  `1` = on.
- `animations` — `1` = on (default), `0` = off.
- `theme` — `onyx` (default), `midnight`, or `graphite` (see the
  themes section below). The value is matched case-insensitively;
  an unknown or corrupt value (for example `theme=blue`) is
  ignored and Onyx stands, exactly like a bad boolean above.

### 2. When it is saved
The file is saved whenever a size or toggle changes (tray menu
or gear menu — they are the same menu, so either path saves)
and again on clean quit (tray Quit, and the manager-window
shutdown path as a second save, so a quit by any clean path
saves). Saving is a **full rewrite of the known keys only**:
that is the documented trade-off — any unrelated lines you
hand-added to the file are NOT preserved across a save. If you
edit the file by hand, edit the values of the known keys only,
while the app is not running, or your edits may be overwritten
by the next toggle change or quit. A failed save is ignored
silently: it never shows an error, never blocks a toggle, and
never blocks quitting.

### 3. Corrupt or missing file — old defaults, never a failure
A missing file is normal on first run: the app just uses the
defaults above and writes the file on the first change or quit.
A corrupt file also never stops the app: blank lines, comment
lines (`;` or `#`), lines without `=`, unknown keys, and values
that do not parse (for example `size=9`, `size=big`, or
`media=maybe`) are all ignored line by line, and only the
keys that parse cleanly override their defaults. Boolean values
accept `1`/`0`, `true`/`false`, `yes`/`no`, `on`/`off`
(case-insensitive). Nothing in this path can crash the app or
refuse to start — if the file cannot be read at all, every
default stands.

### 4. Start with Windows — tray toggle, registry, default OFF
Right-click the tray icon (or click the gear in the expanded
notch) and tick **Start with Windows**. It is **OFF by
default**. Ticking it on writes this registry value:

- Key: `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run`
- Value name: `OnyxNotch`
- Value type: `REG_SZ`
- Value: the current `.exe` path, quoted, for example
  `"C:\Path\To\OnyxNotch.exe"`

Unticking it removes that value. At startup the app reads that
registry value to set the toggle: a present, non-empty value
means ON, an absent value means OFF. **The registry is the
source of truth for auto-start; the settings file is the source
of truth for everything else** — auto-start is never written
to `settings.ini`, and the settings file never overrides the
registry for auto-start. If the registry write fails (for
example, policy blocks it), the toggle is left unchanged, so
the menu never shows a state Windows will not actually do.
Two consequences worth knowing: if you move the `.exe` after
enabling auto-start, Windows will still try the old path —
toggle Start with Windows off and on again to rewrite it to
the current path; and if you delete the `.exe`, remove the
Run value by unticking (or delete it in `regedit`) rather than
assuming the toggle cleans itself up.

### 5. What did not change
No settings window (the gear still opens the existing tray
menu), no new tray items except Start with Windows and the
Themes submenu (feature 3, see the themes section), no change
to SMTC title/artist/status logic, tabs,
hit-test order (gear, then tabs, then media buttons, then
pin), animation timing, or any default above. The v3 note that
toggles "reset at each start" is superseded by this section:
they no longer do.

## What's new in v5 — Themes (feature 3 only)

No new window, no new timer, no network, no position/size
customisation beyond the existing Size menu, no weather, no
calendar events, no installer. This is only a colour system for
the notch surface, plus a submenu to switch it. SMTC titles,
saved settings, auto-start, tabs, hit-test order (gear, then
tabs, then media buttons, then pin), animation timing, and
every default below are unchanged — except that the theme now
survives a restart like the other toggles, which is the point
of saving it.

### 1. The three themes — what each changes
Every theme defines the same slots: gradient top/bottom,
border (normal and hover), top highlight, primary text,
secondary text, muted text (section headers, inactive tabs),
tab hover, accent, divider/bar track, media-button
backgrounds (normal/hover/pressed), media glyph, gear colours,
today-pill text, and battery glyph colours. Painting reads
the current theme for every element — no element keeps a
hardcoded v4 colour — so switching themes recolours the
whole notch at once: collapsed clock, expanded header
(clock, date, battery + glyph), divider, tab bar, HOME
summaries, MEDIA / SYSTEM / CALENDAR tabs, media buttons,
gear, and the calendar today pill.

- **Onyx (default)** — the v4 look, exactly. Gradient
  `32,32,36` (top) → `10,10,12` (bottom); border `58,58,62`
  (`104,104,110` on hover), highlight `78,78,84`; primary
  `248,248,250`, secondary `165,165,170`, muted `140,140,140`;
  accent `248,248,250`; divider/track `52,52,56`; media
  buttons `44,44,48` / hover `66,66,70` / pressed `94,94,98`;
  gear `165,165,170` / hover `210,210,214` / pressed
  `240,240,242`. Choosing Onyx after another theme restores
  the v4 appearance exactly.
- **Midnight** — deep blue gradient. Gradient `28,38,66` →
  `10,16,32`; border `64,84,128` (`110,140,190` on hover),
  highlight `88,110,160`; primary `240,244,252`, secondary
  `165,178,205`, muted `130,145,175`; accent `96,165,250`
  (blue — so the today pill, the CPU/RAM bar fills, and the
  active tab underline all turn blue, with dark-navy pill
  text `10,16,32` for contrast); divider/track `45,58,88`;
  media buttons `38,52,84` / hover `58,74,112` / pressed
  `84,102,142`; gear `165,178,205` / hover `205,218,240` /
  pressed `240,244,252`.
- **Graphite** — lighter grey gradient, still dark.
  Gradient `72,72,78` → `38,38,42`; border `110,110,116`
  (`150,150,158` on hover), highlight `130,130,138`;
  primary `250,250,252`, secondary `200,200,205`, muted
  `170,170,176`; accent `230,230,235` (light grey — pill,
  bar fills, and active tab underline); divider/track
  `88,88,94`; media buttons `84,84,90` / hover `106,106,112`
  / pressed `134,134,142`; gear `200,200,205` / hover
  `225,225,230` / pressed `250,250,252`.

What a theme does NOT change: notch size, position, layout,
fonts, tab order, or any behaviour. A theme is colours only.

### 2. How to switch
Right-click the tray icon, open the **Themes** submenu, and
pick Onyx, Midnight, or Graphite. The current theme has a
tick. The gear button in the expanded notch opens the same
tray menu, so the Themes submenu is there too — no separate
settings panel was built. Switching applies instantly to
every visible notch (all monitors repaint at once); a notch
hidden by fullscreen repaints when it returns. There is no
restart, no rebuild, and no animation for the colour change
itself — the next expand/collapse still animates as before
when Animations is on.

### 3. Settings key, load order, and fallback
The theme is saved through the existing settings mechanism
in `%APPDATA%\OnyxNotch\settings.ini` as:

```ini
theme=onyx
```

(`midnight` or `graphite` for the other themes.) It is
written whenever the theme is switched, like any other
toggle, and again on clean quit as part of the full rewrite
of the known keys — so the same hand-edit warning applies:
edit only the values of known keys, while the app is not
running. It is loaded at startup, in `LoadSettings`, before
the first notch is drawn, so the first notch already opens
in the saved theme. An unknown or corrupt value
(`theme=blue`, `theme=`, a missing key) is ignored line by
line like any other bad value, and Onyx — the default —
stands; a bad theme line never stops startup and never
changes any other setting. Auto-start is unaffected: it
still lives only in the Run registry value, never in the
settings file.

### 4. What did not change
No new library, so `build.bat` is unchanged (same command as
v5-settings). SMTC title/artist/status logic, the settings
file format for the other keys, the registry auto-start
behaviour, and the tray-menu structure outside the new
Themes submenu are all untouched.

## Build (Windows) — v5-SMTC changes the command

1. Install Visual Studio with "Desktop development with C++"
   (the free Community edition is fine).
2. Open "x64 Native Tools Command Prompt for VS".
3. `cd` to this folder and run:

```bat
build.bat
```

That compiles `onyxnotch.rc` (icon) and then `main.cpp`, and
produces `OnyxNotch.exe` in this folder.

New build requirement for v5-SMTC: MSVC with **C++17** and the
Windows SDK **C++/WinRT headers**, plus **`windowsapp.lib`**.
v5-settings adds only **`advapi32.lib`**, for the `Reg*`
auto-start calls; the settings file uses Win32 file APIs that
need no new library.
The only changes to the v2–v4 command are `/std:c++17` and
`windowsapp.lib`; everything else is identical.

Exact command `build.bat` runs (MSVC):

```bat
rc /nologo onyxnotch.rc
cl /nologo /std:c++17 /O2 /MT /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS main.cpp onyxnotch.res user32.lib gdi32.lib shell32.lib windowsapp.lib advapi32.lib /link /SUBSYSTEM:WINDOWS /OUT:OnyxNotch.exe
```

MinGW alternative: **none for v5-SMTC.** MinGW cannot build the
C++/WinRT (SMTC) part — its headers/toolchain do not provide
C++/WinRT — so the old MinGW command from v2–v4 is removed for
this version and MSVC is the only supported path. If the build
fails mentioning `winrt` headers, install a current Windows SDK
with C++/WinRT headers; that is a toolchain install, not a code
change.

## Run

Double-click `OnyxNotch.exe`. To stop it, right-click its tray icon
and choose Quit. There is no installer. For auto-start, use the
tray **Start with Windows** toggle (v5 feature 2, off by default;
see the saved-settings section) — it supersedes the old v2 advice
of putting a shortcut in the Startup folder (`Win + R`, type
`shell:startup`), which still works if you prefer it but is no
longer the documented path.

## Tray menu (v2, also opened by the v4 gear button)

Click the gear button in the expanded notch header to open
this same menu from the notch — it is not a separate settings
panel. Right-clicking the tray icon opens it at the cursor, as
before.

- Size: Small / Medium / Large
- Hide on fullscreen (off by default)
- Media controls (off by default)
- Widget: System stats (off by default)
- Widget: Calendar (off by default)
- Animations (ON by default)
- Start with Windows (OFF by default; writes the Run registry
  value — see the saved-settings section)
- Themes submenu: Onyx / Midnight / Graphite, tick on the
  current theme (see the themes section; also opened by the
  gear button, since it opens this same menu)
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

## Honest limits — v4 reorganization (not verified on Windows)

Everything under "v4 additions" above still applies, plus:
- **Not compiled on Windows.** The reorganization uses only
  the same Win32/GDI APIs as before (`Ellipse`, `Polygon`,
  `FillRect`, `TrackPopupMenu`, `GetClientRect`), plus
  `cos`/`sin` from `<cmath>` for the gear teeth, but `cl`/`rc`
  were never run on it here, so a compile error is still
  possible until the first Windows build.
- **Two-column balance untested on screen.** Column widths
  (148 px each at 96 DPI), the media circles inside a narrow
  column, and the stats bars at that width have not been seen
  on a real display; they may look cramped or unbalanced and
  need tuning.
- **Gear legibility untested at small size.** The gear is
  22 px across at 96 DPI (11 px radius, 8 teeth); on a real
  display its teeth and centre hole may read as a blob, and
  the 2 px hit tolerance may feel too tight or too loose.
  Check it at 100% and at higher DPI before judging it.
- **Menu-from-gear untested.** The tray menu is now opened
  with the manager window as owner while the click came from
  a notch window; the menu items, toggle behaviour, and
  dismissal (including clicking away) have not been exercised
  on Windows, and the gear-must-not-pin rule has not been
  clicked through on a real screen.
- **Animation interplay untested.** Animated sizes now
  interpolate to the new (shorter) total heights, and the gear
  moves with the notch width during animation; flicker,
  clipping, or a gear drawn outside the region mid-animation
  has not been observed — the Animations tray tick remains
  the instant fallback.
- **Stats-only height grows** (180 px to 210 px, see the
  reorganization section) — that is arithmetic from the new
  layout, not a measurement, and the both-on saving (236 px
  to 210 px) is arithmetic too until a Windows run confirms it.

## Honest limits — v4 tabs (not verified on Windows)

Everything under "v4 additions" above still applies, plus:
- **Not compiled on Windows.** The tabs use only the same
  Win32/GDI APIs as before (`Ellipse`, `Polygon`,
  `FillRect`, `RoundRect`, `SystemTimeToFileTime` /
  `FileTimeToSystemTime` for the HOME week), plus the
  existing animation path, but `cl`/`rc` were never run on
  it here, so a compile error is still possible until the
  first Windows build.
- **Tab legibility and balance untested on screen.** Tab
  labels at the tiny font size, the active underline, and
  the even spacing across 2–4 tabs have not been seen on a
  real display; with 4 tabs each segment is about 77 px
  wide at the standard size, which may feel cramped, and
  the HOME week numbers without a weekday header may need
  a label after all.
- **Mini media button hit areas untested.** The HOME
  buttons are 20 px circles (10 px radius); the circular
  hit test is scaled from the same layout as the drawing,
  but whether 20 px targets feel too small to click
  reliably has not been tried on a real screen.
- **Height animation on tab switch untested.** Tab
  switches reuse the expand/collapse animation, but the
  animation from one expanded height to another (rather
  than collapsed-to-expanded) has not been observed;
  flicker, or content from the old tab showing during the
  switch, is possible — the Animations tray tick remains
  the instant fallback.
- **DPI behaviour untested**, as in v3/v4: tab segments,
  mini buttons, and week cells all scale with DPI via
  `Scale()`, but per-monitor DPI tab layout has not been
  run.
- **Heights are arithmetic, not measurements** (standard
  size, 96 DPI): HOME with all features 246 px, MEDIA
  220 px, SYSTEM 242 px, CALENDAR 314 px for a 5-row
  month (294 px for 4 rows, 334 px for 6 rows), header
  only 138 px — until a Windows run confirms them.

## Honest limits — v5-SMTC (not verified on Windows)

Everything under the earlier honest-limits sections still applies,
plus the SMTC-specific unknowns. This Linux VM has no Windows
toolchain, so **none of the v5-SMTC code has been compiled or run
on Windows** — treat every behaviour below as untested until
Kumar builds it with `build.bat` on Windows and runs it:

- **Not compiled on Windows — especially the WinRT code.** The
  C++/WinRT calls (`GlobalSystemMediaTransportControlsSessionManager::RequestAsync`,
  `GetCurrentSession`, `GetPlaybackInfo`, `TryGetMediaPropertiesAsync`),
  the `winrt::init_apartment(multi_threaded)` call, the
  `/std:c++17` flag, and the `windowsapp.lib` link are all written
  from the C++/WinRT API shape, not from a successful build. A
  header, namespace, enum-name, or link error is possible and
  would only show up at that first Windows build. The brace/paren
  balance and symbol ordering were checked here, but that is not
  a compile.
- **SMTC against real players untested.** Whether Spotify, a
  browser, or another player actually exposes a session, title,
  artist, and a reliable Playing/Paused status to this code has
  not been tried. Some players expose no SMTC session, report a
  session with an empty title, or lag status changes — all of
  those fall back to the v4 look by design, but which players do
  what is unknown until tried. The status glyph is therefore
  "reflects SMTC when it reports clearly", not "always correct":
  if a player reports status late or wrong, the glyph will too.
- **Blocking `.get()` on the 1-second timer untested.** The refresh
  uses synchronous `.get()` on the WinRT async calls, on the
  manager's timer tick, in a multi-threaded apartment chosen so
  it does not need STA message pumping. If a player's SMTC
  broker is slow or hangs, that tick could stall the UI thread
  for that second; no timeout is applied, because adding one
  would need a thread, which this design deliberately avoids.
  Watch for a one-second UI hitch when a track changes.
- **DPI behaviour of the new lines untested.** Title/artist rows
  scale with `Scale()` like everything else, and ellipsis
  truncation is measured at the current font/DPI, but per-monitor
  DPI title layout, very long titles at high DPI, and non-Latin
  scripts have not been seen on a real display.
- **Animation interplay untested.** A track appearing or ending
  while the notch is expanded resizes it through the existing
  animation path, and a track change mid-expand/collapse has not
  been observed; flicker or a clipped title during the ~220 ms
  resize is possible — the Animations tray tick remains the
  instant fallback, and collapsing hides the lines entirely.
- **Heights are arithmetic, not measurements** (standard size,
  96 DPI): MEDIA 242 px title-only / 262 px title+artist (220 px
  with no track), HOME all-features 270 px with a title (246 px
  without) — until a Windows run confirms them.
- **Idle memory under 10 MB is still a target, not a measurement.**
  The SMTC cache is two 256-character buffers plus a few globals;
  WinRT initialisation happens once, only while Media controls
  are on, but the runtime cost of the WinRT/SMTC broker itself
  has not been measured in Task Manager.

## Honest limits — v5-settings (not verified on Windows)

Everything under the earlier honest-limits sections still applies,
plus the settings-specific unknowns. This Linux VM has no Windows
toolchain, so **none of the v5-settings code has been compiled or
run on Windows** — treat every behaviour below as untested until
Kumar builds it with `build.bat` on Windows and runs it:

- **Not compiled on Windows.** The new code uses only Win32
  file APIs (`CreateFileW`, `ReadFile`, `WriteFile`,
  `CreateDirectoryW`, `GetEnvironmentVariableW`) and registry
  APIs (`RegOpenKeyExW`, `RegCreateKeyExW`, `RegQueryValueExW`,
  `RegSetValueExW`, `RegDeleteValueW`), plus `sprintf_s`,
  `strlen`/`strcmp` and `wcscpy_s`/`wcscat_s` — but `cl` was
  never run on it here, so a declaration, calling-convention,
  or link error (including a missing `advapi32.lib`) is still
  possible until the first Windows build. Brace/paren balance
  was checked here; that is not a compile.
- **Settings load/save round-trip untested.** That the first
  notch really opens at the saved size with the saved toggles,
  that a toggle change really lands in `settings.ini`, and
  that a quit really saves, have not been observed. In
  particular, whether the file is written with the expected
  line endings/values and re-read identically has not been
  tried — check the file contents after one toggle change
  and one restart before trusting persistence.
- **Auto-start registry behaviour untested.** That ticking
  Start with Windows really creates the quoted-path Run value,
  that unticking really removes it, that the toggle really
  reflects an existing value at startup, and that Windows
  really launches the app at login from that value, have not
  been tried. A stale path after moving the `.exe` is a
  known case (see the saved-settings section), not a tested
  one. Check the value in `regedit` after toggling, and test
  one real login, before relying on it.
- **Interaction with SMTC/tabs untested.** The settings code
  does not touch SMTC title/artist/status logic, tab layout,
  or hit-test order, and it loads before the first notch is
  created — but the combination (saved Media on + SMTC title
  on first expand, saved tab state is NOT persisted so every
  expand still starts on HOME, saved Animations off + saved
  size) has not been run together on Windows.
- **DPI and animation interplay untested.** Settings load
  happens before DPI-aware notch creation, and saving happens
  on toggle regardless of DPI, but per-monitor DPI restores
  and animated resizes after a settings restore have not been
  observed; the Animations toggle itself is persisted, so a
  saved Animations-off state should snap on first expand —
  that too is untested until run.
- **Failure paths untested by observation.** Missing file,
  corrupt file, unknown keys, out-of-range values, a missing
  `%APPDATA%`, and a denied registry write are all handled
  in code by falling back or leaving the toggle unchanged —
  but none of those paths has been exercised on a real
  machine, so treat "never crashes on a bad file" as a design
  claim, not a test result, until a deliberately corrupt
  `settings.ini` is tried.

## Honest limits — v5-themes (not verified on Windows)

Everything under the earlier honest-limits sections still applies,
plus the theme-specific unknowns. This Linux VM has no Windows
toolchain, so **none of the v5-themes code has been compiled or
run on Windows** — treat every behaviour below as untested until
Kumar builds it with `build.bat` on Windows and runs it:

- **Not compiled on Windows.** The theme code uses only the
  same Win32/GDI APIs as before (`CreateSolidBrush`,
  `CreatePen`, `GetRValue`/`GetGValue`/`GetBValue`, `AppendMenuW`
  with `MF_POPUP`, `InvalidateRect`), plus a plain struct table
  — but `cl` was never run on it here, so a declaration or
  initialiser error is still possible until the first Windows
  build. Brace/paren balance was checked here; that is not a
  compile.
- **Theme appearance on a real display untested.** The
  Midnight blue and Graphite grey values are chosen on paper
  (see the themes section); whether Midnight reads as deep blue
  rather than black, whether Graphite still reads as dark, and
  whether the blue accent pill keeps dark-navy text readable
  have not been seen on a real display and may need tuning.
  Onyx is the v4 values exactly, so it should match v4 — but
  even that "should" is untested until run, since the painting
  code paths changed from literals to table lookups.
- **Instant-switch repaint untested.** Switching calls the
  existing `InvalidateAllNotches`, which skips notches hidden
  by fullscreen (they repaint on restore) — whether every
  visible notch on every monitor actually repaints at once,
  with no stale-colour frame, has not been observed. The
  submenu itself (tick position, dismissal, opening from the
  gear at the gear's screen position) is likewise untested on
  Windows, as the gear menu path was already untested.
- **Settings round-trip for theme untested.** That a switch
  really lands as `theme=midnight` (or `graphite`) in
  `settings.ini`, that the next start really opens in that
  theme, and that a corrupt value really falls back to Onyx
  without touching other keys, have not been tried — check
  the file contents after one switch and one restart before
  trusting persistence.
- **DPI behaviour untested**, as in v3/v4: theme colours do not
  scale with DPI (only geometry does), so no DPI-specific
  theme failure is expected — but per-monitor DPI repaint
  after a theme switch has not been run.
- **Animation interplay untested.** A theme switch is a
  repaint, not a resize, so it does not run the ~220 ms
  animation path — but a switch mid-expand/collapse has not
  been observed; a frame painted with mixed theme colours
  during animation is possible, and the Animations tray tick
  remains the instant fallback for animation issues generally.
- **Idle memory under 10 MB is still a target, not a
  measurement.** The theme table is three small structs and
  switching allocates nothing retained, so no idle-cost change
  is expected — measure it in Task Manager to confirm.

## Honest limits — v4 additions (not verified on Windows)

- **Not compiled on Windows.** This Linux VM has no Windows
  toolchain, so `cl`/`rc` were never run on the v4 code. The
  changes use only standard Win32 GDI APIs already in use
  (`FillRect`, `RoundRect`, `Ellipse`, `Polygon`,
  `GetTextExtentPoint32W`, `CreateFontW`), but a compile error
  is still possible until the first Windows build.
- **The premium look is untested on a real display.** The
  gradient tones, border, highlight line, circle size, glyph
  size inside the circles, battery glyph, and calendar pill
  have not been seen on screen. They are chosen to be
  restrained on black, but may need tuning — in particular,
  the gradient step (top 32,32,36 to bottom 10,10,12) and the
  circle diameter (media-row height) are the first things to
  check visually.
- **DPI behaviour during animation is untested**, as in v3:
  gradient rows, glyph, and pill insets all scale with DPI via
  `Scale()`, but per-monitor DPI animation has not been run.
- **No acrylic/blur** (see v4 section 5) — the notch is
  opaque gradient, by design, not by omission in the build.
- **Idle memory under 10 MB is still a target, not a
  measurement**, as in v3. The v4 paints add no new timers or
  retained objects, but measure it in Task Manager to confirm.

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
- No installer. (Auto-start exists since v5 feature 2 — see the
  saved-settings section — so the old "no auto-start" line no
  longer applies.)

## Files

- `main.cpp` — the whole app: Win32 windows (one per monitor plus a
  hidden manager window for tray/timer), rounded notch region,
  1-second timer, tray icon, battery via `GetSystemPowerStatus`,
  media keys, stats via `GetSystemTimes`/`GlobalMemoryStatusEx`,
  calendar grid.
  v3 adds: per-notch animation timer (~220 ms, ease-out),
  media-button hover/pressed states, notch hover outline,
  ClearType fonts, and the Animations tray tick.
  v4 adds (UI only): vertical gradient surface, light border
  and top highlight, lighter Segoe UI weights, circular media
  buttons with a matching circular hit test, a section
  divider, a drawn battery glyph, and a white-pill calendar
  today highlight. No acrylic, no new timers or dependencies.
  v4 reorganization (in place): compact two-column body
  (media beside system stats, calendar full-width below),
  MEDIA / SYSTEM / CALENDAR section headers, stats split
  into two labelled rows with progress bars, footer removed,
  and a GDI settings gear in the expanded header that opens
  the existing tray menu (hit-tested before the pin click).
  No new settings window, no settings file.
  Tabs (in place): HOME summary plus MEDIA / SYSTEM /
  CALENDAR detailed tabs, shown only for enabled features;
  tab clicks switch the body (hit-tested after the gear,
  before media buttons and pin), height animates via the
  existing path, and the tab resets to HOME on the next
  expand after a collapse.
  v5-SMTC (feature 1 only): C++/WinRT read of the local SMTC
  session on the existing 1-second timer (cached title/artist/
  status, no new timer/thread/network), MEDIA title+artist lines
  and a HOME compact title line with GDI ellipsis truncation,
  a status-aware play/pause glyph, exact v4 fallback on any
  failure, and heights grown via the shared `ComputeLayout`.
  No themes.
  v5-settings (feature 2 only): plain key=value settings file
  at `%APPDATA%\OnyxNotch\settings.ini`, loaded before the
  first notch is drawn and saved on every size/toggle change
  and on clean quit (full rewrite of the known keys, corrupt
  or missing file falls back to defaults), plus a Start with
  Windows tray toggle backed by the HKCU Run registry value
  (registry is the source of truth for auto-start, the file
  for everything else). SMTC code is untouched by this change.
  v5-themes (feature 3 only): theme table (Onyx / Midnight /
  Graphite) with every painted colour read from the current
  theme, a Themes submenu in the shared tray/gear menu with a
  tick on the current theme, instant repaint of all notches on
  switch, and a `theme` key in `settings.ini` loaded before the
  first notch is drawn (unknown values fall back to Onyx). No
  new library, so `build.bat` is unchanged.
  `settings.ini` is a runtime file created on the user's
  machine, not a source file in this folder.
- `build.bat` — MSVC build script (resource compile + compile/link).
- `onyxnotch.rc` — resource script binding the icon (ID 100).
- `onyxnotch.ico` — custom black-pill icon, 16/32/48 px, 32-bit.
- `.gitignore` — build outputs (`OnyxNotch.exe`, `.obj`, `.res`,
  etc.) are not meant to be committed.
- `README.md` — this file.
