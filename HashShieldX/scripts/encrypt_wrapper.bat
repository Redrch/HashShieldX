@echo off
powershell -WindowStyle Hidden -ExecutionPolicy Bypass -File "%~dp0encrypt.ps1" "%1"