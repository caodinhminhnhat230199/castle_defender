@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0create_hud_assets.ps1" %*
exit /b %errorlevel%
