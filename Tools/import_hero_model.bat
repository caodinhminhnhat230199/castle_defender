@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0import_hero_model.ps1" %*
exit /b %errorlevel%
