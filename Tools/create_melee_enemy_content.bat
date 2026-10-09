@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0create_melee_enemy_content.ps1" %*
exit /b %errorlevel%
