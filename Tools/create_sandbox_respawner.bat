@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0create_sandbox_respawner.ps1" %*
exit /b %errorlevel%
