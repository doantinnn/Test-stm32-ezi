@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ====================================================================
echo      KHOI TAO GIT VA TAI LEN GITHUB CHO DU AN STM32-SERVO
echo ====================================================================
echo.

:: 1. Kiem tra Git
where git >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [LOI] Khong tim thay lenh 'git' trong PATH.
    echo Vui long cai dat Git hoac kiem tra lai bien moi truong: https://git-scm.com/
    pause
    exit /b 1
)

cd /d "%~dp0"
echo [1/5] Thu muc lam viec: "%CD%"

:: 2. Khoi tao Git repo neu chua co
if not exist ".git" (
    echo [2/5] Dang khoi tao Git repository...
    git init
    git branch -M main
) else (
    echo [2/5] Git repository da ton tai san.
)

:: 3. Them file vao staging
echo.
echo [3/5] Dang them cac file vao Git (theo bo loc .gitignore)...
git add .
git status --short

:: 4. Commit lan dau neu chua co commit
git rev-parse --verify HEAD >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo.
    echo [4/5] Dang tao commit dau tien...
    git commit -m "Initial commit: E7502 IO Extension Board - Ezi-SERVO Controller for STM32F767"
) else (
    echo.
    echo [4/5] Da co commit truoc do. Kiem tra thay doi moi...
    git commit -m "Update project files" 2>nul
)

:: 5. Lien ket voi GitHub
echo.
echo ====================================================================
echo [5/5] TAI LEN GITHUB
echo ====================================================================
echo Hay tao 1 repository moi tren GitHub (Public hoac Private) tai:
echo      https://github.com/new
echo (Luu y: KHONG tick chon 'Add a README file' hoac .gitignore vi da co san)
echo.
echo Sau khi tao xong, copy link repository (vi du: https://github.com/<user>/<repo>.git)
echo.
set /p REPO_URL="Nhap link GitHub repository cua ban (hoac Enter de bo qua): "

if "%REPO_URL%"=="" (
    echo.
    echo Da hoan tat khoi tao Git local!
    echo Khi nao can day len GitHub, ban chi can chay lai file nay hoac go cac lenh:
    echo    git remote add origin ^<link-github^>
    echo    git push -u origin main
    echo.
    pause
    exit /b 0
)

:: Kiem tra xem origin da co chua
git remote | findstr "origin" >nul 2>nul
if %ERRORLEVEL% equ 0 (
    git remote set-url origin "%REPO_URL%"
) else (
    git remote add origin "%REPO_URL%"
)

echo.
echo Dang day ma nguon len GitHub (branch: main)...
git push -u origin main

if %ERRORLEVEL% equ 0 (
    echo.
    echo ====================================================================
    echo [THANH CONG] Du an da duoc tai len GitHub thanh cong!
    echo Repository: %REPO_URL%
    echo ====================================================================
) else (
    echo.
    echo [CANH BAO] Push that bai hoac bi yeu cau dang nhap.
    echo Neu trinh duyet bat len yeu cau dang nhap GitHub, vui long xac thuc.
    echo Hoac kiem tra lai quyen truy cap va link repository.
)

echo.
pause
