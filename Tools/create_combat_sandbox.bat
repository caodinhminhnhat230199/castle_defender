@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0create_combat_sandbox.ps1" %*
exit /b %errorlevel%
