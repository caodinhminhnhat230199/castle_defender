@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0create_enemy_combat_tests.ps1" %*
exit /b %ERRORLEVEL%
