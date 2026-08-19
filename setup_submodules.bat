@echo off
echo Initializing Nexus addon dependencies...
git submodule update --init --recursive
if errorlevel 1 (
    echo.
    echo FAILED: Git could not initialize the submodules.
    pause
    exit /b 1
)
echo.
echo Done. Open FoodReminder.sln in Visual Studio.
pause
