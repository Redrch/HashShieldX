@echo off
powershell -WindowStyle Hidden -ExecutionPolicy Bypass -File "%~dp0decrypt.ps1" "%1"