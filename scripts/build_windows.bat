@echo off
setlocal
REM Build YMYVerb VST3, run DSP tests, and create the Windows installer.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build_windows.ps1"
if errorlevel 1 (
  echo.
  echo ERROR: YMYVerb build failed. Review the output above.
  exit /b 1
)
exit /b 0
