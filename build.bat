@echo off
REM OnyxNotch v1 - build on Windows.
REM Run this from a Visual Studio Developer Command Prompt
REM (or "x64 Native Tools Command Prompt for VS").
REM Requires: Visual Studio with "Desktop development with C++".
REM Output: OnyxNotch.exe in this folder. No installer, no runtime needed.
setlocal
cl /nologo /O2 /MT /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS main.cpp user32.lib gdi32.lib shell32.lib /link /SUBSYSTEM:WINDOWS /OUT:OnyxNotch.exe
if errorlevel 1 (
  echo.
  echo Build failed. Make sure you are in a Developer Command Prompt.
  exit /b 1
)
echo.
echo Built: OnyxNotch.exe
echo Run it by double-clicking OnyxNotch.exe. Right-click its tray icon to quit.
endlocal
