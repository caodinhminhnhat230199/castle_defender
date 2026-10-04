@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0create_hero_assets.ps1" %*
exit /b %errorlevel%
