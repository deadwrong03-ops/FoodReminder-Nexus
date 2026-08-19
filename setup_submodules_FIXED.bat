@echo off
setlocal
cd /d "%~dp0"

echo ============================================
echo Food Reminder - Nexus dependency setup
echo ============================================
echo.

where git >nul 2>nul
if errorlevel 1 (
    echo ERROR: Git was not found.
    echo Install Git for Windows, then run this file again.
    pause
    exit /b 1
)

if not exist "src" (
    echo ERROR: The src folder was not found.
    echo Put this BAT in the same folder as FoodReminder.sln.
    pause
    exit /b 1
)

if not exist ".git" (
    echo Initializing FoodReminder as a Git repository...
    git init
    if errorlevel 1 goto :failed

    if exist ".gitmodules" del /q ".gitmodules"
)

echo.
echo Adding Nexus API...
if not exist "src\nexus\Nexus.h" (
    git submodule add https://github.com/RaidcoreGG/Nexus-API.git src/nexus
    if errorlevel 1 goto :failed
) else (
    echo Nexus API already present - skipping.
)

echo.
echo Adding ImGui...
if not exist "src\imgui\imgui.h" (
    git submodule add https://github.com/RaidcoreGG/imgui.git src/imgui
    if errorlevel 1 goto :failed
) else (
    echo ImGui already present - skipping.
)

echo.
echo Adding Mumble API...
if not exist "src\mumble" (
    git submodule add https://github.com/RaidcoreGG/RCGG-lib-mumble-api.git src/mumble
    if errorlevel 1 goto :failed
) else (
    echo Mumble API already present - skipping.
)

echo.
echo Updating all submodules...
git submodule update --init --recursive
if errorlevel 1 goto :failed

echo.
echo ============================================
echo SUCCESS
echo Nexus dependencies are installed.
echo ============================================
echo.
echo Return to Visual Studio and use:
echo Build ^> Rebuild Solution
echo.
pause
exit /b 0

:failed
echo.
echo ============================================
echo SETUP FAILED
echo ============================================
echo Leave this window open and send the error
echo shown above to ChatGPT.
echo.
pause
exit /b 1
