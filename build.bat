@echo off
setlocal EnableDelayedExpansion

echo ============================================================
echo   RDNA Cast - Build ^& Package Script
echo   Target: AMD Radeon (Zen 2+ / RDNA+)
echo ============================================================
echo.

set "APP_VERSION=0.7.20"
set "INSTALLER_NAME=RDNA_Cast_Setup_%APP_VERSION%.exe"

:: Step 0: Find Visual Studio
echo [1/5] Locating Visual Studio 2022...
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo ERROR: Visual Studio Installer not found.
    echo Please install Visual Studio 2022 with "Desktop development with C++" workload.
    pause
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -products * -latest -property installationPath`) do set "VS_PATH=%%i"
if not defined VS_PATH (
    echo ERROR: Visual Studio 2022 not found.
    pause
    exit /b 1
)
echo   Found: %VS_PATH%

:: Initialize VS environment
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo ERROR: Failed to initialize Visual Studio environment.
    pause
    exit /b 1
)
echo   Environment initialized.
echo.

:: Step 1: Configure with CMake
echo [2/5] Configuring CMake (amd-lite-x64 preset, RDNA Cast)...
cmake --preset amd-lite-x64
if errorlevel 1 (
    echo ERROR: CMake configuration failed.
    echo This usually means dependencies are missing. Check the output above.
    pause
    exit /b 1
)
echo   Configuration complete.
echo.

:: Step 2: Build
echo [3/5] Building RDNA Cast (RelWithDebInfo)...
cmake --build build_amd_lite --config RelWithDebInfo --parallel
if errorlevel 1 (
    echo ERROR: Build failed. Check the output above for errors.
    pause
    exit /b 1
)
echo   Build complete.
echo.

:: Step 3: Create installer output directory
echo [4/5] Packaging installer with Inno Setup...
if not exist "dist-installer" mkdir dist-installer

:: Find Inno Setup
set "ISCC="
where iscc.exe >nul 2>&1 && set "ISCC=iscc.exe"
if not defined ISCC (
    if exist "%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe" (
        set "ISCC=%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe"
    ) else if exist "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" (
        set "ISCC=%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
    )
)

if not defined ISCC (
    echo WARNING: Inno Setup not found. Skipping installer creation.
    echo   The build output is in: build_amd_lite\rundir\RelWithDebInfo\
    pause
    exit /b 0
)

"%ISCC%" installer.iss
if errorlevel 1 (
    echo ERROR: Installer packaging failed.
    pause
    exit /b 1
)
echo   Installer created.
echo.

:: Step 4: Copy to desktop
echo [5/5] Copying installer to Desktop...
copy /Y "dist-installer\%INSTALLER_NAME%" "%USERPROFILE%\Desktop\%INSTALLER_NAME%" >nul
if errorlevel 1 (
    echo WARNING: Could not copy to Desktop. Installer is at:
    echo   dist-installer\%INSTALLER_NAME%
) else (
    echo   Installer copied to Desktop!
)

echo.
echo ============================================================
echo   BUILD COMPLETE!
echo   Installer: dist-installer\%INSTALLER_NAME%
echo   Also on your Desktop.
echo ============================================================
echo.

:: Generate SHA256 hash
echo SHA256 Hash:
certutil -hashfile "dist-installer\%INSTALLER_NAME%" SHA256 2>nul | findstr /v "hash"
echo.

pause
