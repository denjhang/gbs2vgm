@echo off
REM GBS2VGM Batch Converter - Quick Launch Script
REM Drag and drop 7z/M3U files onto this script to convert

setlocal enabledelayedexpansion

if "%~1"=="" (
    echo.
    echo GBS2VGM Batch Converter
    echo =======================
    echo.
    echo Usage: Drag and drop 7z or M3U files onto this script
    echo   Or: %~nx0 input_file [output_dir]
    echo.
    echo Examples:
    echo   %~nx0 "Dragon Warrior III.7z"
    echo   %~nx0 playlist.m3u output_folder
    echo.
    pause
    exit /b 1
)

set "INPUT=%~1"
set "OUTPUT=%~2"

echo.
echo ========================================
echo GBS2VGM Batch Converter
echo ========================================
echo.
echo Input: %INPUT%
if not "%OUTPUT%"=="" echo Output: %OUTPUT%
echo.

REM Check if input file exists
if not exist "%INPUT%" (
    echo Error: Input file not found: %INPUT%
    echo.
    pause
    exit /b 1
)

REM Run converter
if "%OUTPUT%"=="" (
    "%~dp0bin\gbs2vgm_batch.exe" "%INPUT%"
) else (
    "%~dp0bin\gbs2vgm_batch.exe" "%INPUT%" "%OUTPUT%"
)

echo.
echo ========================================
echo Conversion Complete!
echo ========================================
echo.
pause
