@echo off
echo ============================================================
echo   RDNA Cast — YouTube OAuth Credential Setup
echo ============================================================
echo.
echo This sets up your Google OAuth credentials as permanent
echo environment variables so the YouTube Connect button works.
echo You only need to do this ONCE.
echo.
echo To get your credentials:
echo   1. Go to https://console.cloud.google.com/
echo   2. Create a new project (e.g. "OBS Lite AMD")
echo   3. Go to APIs ^& Services ^> Library
echo   4. Search for "YouTube Data API v3" and ENABLE it
echo   5. Go to APIs ^& Services ^> Credentials
echo   6. Click "Create Credentials" ^> "OAuth client ID"
echo   7. Application type: "Desktop app"
echo   8. Copy the Client ID and Client Secret below
echo.
echo ============================================================
echo.

set /p "YT_CLIENT_ID=Paste your YouTube Client ID: "
set /p "YT_SECRET=Paste your YouTube Client Secret: "

if "%YT_CLIENT_ID%"=="" (
    echo ERROR: Client ID cannot be empty.
    pause
    exit /b 1
)
if "%YT_SECRET%"=="" (
    echo ERROR: Client Secret cannot be empty.
    pause
    exit /b 1
)

echo.
echo Setting permanent environment variables...

:: Set as permanent user environment variables
setx YOUTUBE_CLIENTID "%YT_CLIENT_ID%" >nul
setx YOUTUBE_SECRET "%YT_SECRET%" >nul
setx YOUTUBE_CLIENTID_HASH "0" >nul
setx YOUTUBE_SECRET_HASH "0" >nul

echo.
echo Done! YouTube credentials saved permanently.
echo.
echo IMPORTANT: Close and reopen your terminal/IDE before building
echo so the new environment variables take effect.
echo.
pause
