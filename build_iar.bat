@echo off
setlocal enabledelayedexpansion

echo ================================================================
echo   IAR Embedded Workbench Auto-Build Script for STM32F767
echo   Project: E7502 IO Extension Board - Ezi-SERVO Controller
echo ================================================================

:: 1. Search for IarBuild.exe in common installation paths
set "IAR_EXE="

if exist "C:\Program Files (x86)\IAR Systems\Embedded Workbench 8.4_2\common\bin\IarBuild.exe" (
    set "IAR_EXE=C:\Program Files (x86)\IAR Systems\Embedded Workbench 8.4_2\common\bin\IarBuild.exe"
)

if not defined IAR_EXE (
    for /d %%D in ("C:\Program Files\IAR Systems\Embedded Workbench*") do (
        if exist "%%D\common\bin\IarBuild.exe" set "IAR_EXE=%%D\common\bin\IarBuild.exe"
    )
)

if not defined IAR_EXE (
    for /d %%D in ("C:\Program Files (x86)\IAR Systems\Embedded Workbench*") do (
        if exist "%%D\common\bin\IarBuild.exe" set "IAR_EXE=%%D\common\bin\IarBuild.exe"
    )
)

if not defined IAR_EXE (
    echo [ERROR] Could not find IarBuild.exe!
    echo Please open EWARM\eziservo_ioboard.eww in IAR Embedded Workbench IDE and press F7.
    pause
    exit /b 1
)

echo [INFO] Found IAR Toolchain: "%IAR_EXE%"

:: 2. Define Project Paths
cd /d "%~dp0"
set "PROJ_PATH=EWARM\eziservo_ioboard.ewp"
set "BIN_OUT=EWARM\Release\Exe\eziservo_ioboard.bin"

echo [INFO] Building Project: "%PROJ_PATH%" (Configuration: Release)...
echo.

:: 3. Execute Build Command
"%IAR_EXE%" "%PROJ_PATH%" -build Release -log all

set "BUILD_STATUS=%ERRORLEVEL%"

echo.
if %BUILD_STATUS% equ 0 (
    echo ================================================================
    echo [SUCCESS] Build completed successfully!
    if exist "%BIN_OUT%" (
        echo [OUTPUT] Firmware Binary: "%BIN_OUT%"
    )
    echo ================================================================
) else (
    echo ================================================================
    echo [ERROR] Build failed with exit code: %BUILD_STATUS%
    echo Please check error messages above or open in IAR IDE.
    echo ================================================================
)

pause
