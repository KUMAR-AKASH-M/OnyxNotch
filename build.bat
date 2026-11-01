@echo off
REM OnyxNotch v5 - SMTC titles plus saved settings and auto-start - build on Windows.
REM Run this from a Visual Studio Developer Command Prompt, or from the x64 Native Tools Command Prompt for VS.
REM Requires Visual Studio with Desktop development with C++ and the Windows SDK with C++/WinRT headers.
REM The SMTC code needs C++17 and windowsapp.lib. Those are the only new build requirements over v4.
REM v5-settings adds advapi32.lib only, for the Reg auto-start calls.
REM File I/O uses the already-linked Win32 APIs, so no other library is new.
REM MinGW CANNOT build the C++/WinRT SMTC part, so there is no MinGW command for v5 - MSVC is the supported path.
REM Output is OnyxNotch.exe in this folder. No installer, no runtime needed.
REM The rc step compiles onyxnotch.ico into onyxnotch.res, so the custom icon is baked into the single exe.
REM Note on cmd parsing: this script deliberately uses no parenthesised if blocks and no parenthesis in any echo or REM line, because a parenthesis inside a block breaks cmd parsing with a was unexpected error.
setlocal
rc /nologo onyxnotch.rc
if errorlevel 1 goto rc_failed
cl /nologo /std:c++17 /EHsc /O2 /MT /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS /D_SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS main.cpp onyxnotch.res user32.lib gdi32.lib shell32.lib windowsapp.lib advapi32.lib /link /SUBSYSTEM:WINDOWS /OUT:OnyxNotch.exe
if errorlevel 1 goto cl_failed
echo.
echo Built: OnyxNotch.exe
echo Run it by double-clicking OnyxNotch.exe. Right-click its tray icon to quit.
endlocal
exit /b 0
:rc_failed
echo.
echo Resource compile failed. Make sure rc.exe is on PATH.
echo It ships with the Windows SDK and Visual Studio.
endlocal
exit /b 1
:cl_failed
echo.
echo Build failed. Make sure you are in a Developer Command Prompt.
echo If the failure mentions winrt headers, install the Windows SDK with C++/WinRT headers.
endlocal
exit /b 1
