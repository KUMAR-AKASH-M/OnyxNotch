@echo off
REM OnyxNotch v2 - build on Windows.
REM Run this from a Visual Studio Developer Command Prompt
REM (or "x64 Native Tools Command Prompt for VS").
REM Requires: Visual Studio with "Desktop development with C++".
REM Output: OnyxNotch.exe in this folder. No installer, no runtime needed.
REM The .rc step compiles onyxnotch.ico into onyxnotch.res, so the
REM custom icon is baked into the single .exe (no separate icon file
REM needed at run time).
setlocal
rc /nologo onyxnotch.rc
if errorlevel 1 (
  echo.
  echo Resource compile failed. Make sure rc.exe is on PATH.
  echo It ships with the Windows SDK / Visual Studio.
  exit /b 1
)
cl /nologo /O2 /MT /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS main.cpp onyxnotch.res user32.lib gdi32.lib shell32.lib /link /SUBSYSTEM:WINDOWS /OUT:OnyxNotch.exe
if errorlevel 1 (
  echo.
  echo Build failed. Make sure you are in a Developer Command Prompt.
  exit /b 1
)
echo.
echo Built: OnyxNotch.exe
echo Run it by double-clicking OnyxNotch.exe. Right-click its tray icon to quit.
endlocal
