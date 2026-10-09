@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0create_squad_assets.ps1" %*
exit /b %errorlevel%
