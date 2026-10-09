@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0create_synergy_assets.ps1" %*
exit /b %ERRORLEVEL%
