@echo off
echo =======================================
echo   asynk C++ Build Script (Windows)
echo =======================================
echo.

echo [1/3] Configuring CMake...
cmake -B build -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] CMake configuration failed.
    echo Make sure Qt6 and CMake are installed and on PATH.
    pause
    exit /b 1
)

echo.
echo [2/3] Building...
cmake --build build --config Release
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed. Check output above.
    pause
    exit /b 1
)

echo.
echo [3/3] Deploying Qt DLLs...
mkdir deploy 2>nul
copy build\Release\asynk.exe deploy\ >nul
windeployqt deploy\asynk.exe

echo.
echo =======================================
echo   Build successful!
echo   Output: deploy\asynk.exe
echo =======================================
echo.
echo   FFmpeg must be installed on PATH
echo   for audio extraction to work.
echo.
pause
