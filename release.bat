@echo off
setlocal enabledelayedexpansion

rem One-click release script for Wallpaper 24h
rem Function: Compile project and prepare release files

rem ===============================================
rem Configuration Section - Modify these variables
rem ===============================================
rem
rem Set your MSBuild path here (replace the path after the = sign)
rem
rem Option 1: Visual Studio 2026 BuildTools
rem set "MSBUILDPATH=C:\Program Files (x86)\Microsoft Visual Studio\2026\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
rem
rem Option 2: Visual Studio 2022 BuildTools (Default)
set "MSBUILDPATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
rem
rem Option 3: Visual Studio 2026 Community
rem set "MSBUILDPATH=C:\Program Files\Microsoft Visual Studio\2026\Community\MSBuild\Current\Bin\MSBuild.exe"
rem
rem Option 4: Visual Studio 2022 Community
rem set "MSBUILDPATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
rem
rem ===============================================
rem End of Configuration Section
rem ===============================================

echo =====================================
echo Wallpaper 24h Release Script
echo =====================================
echo.

rem Check if MSBuild exists
if not exist "!MSBUILDPATH!" (
    echo Error: MSBuild not found at specified path:
    echo   !MSBUILDPATH!
    echo.
    echo Please modify the MSBUILDPATH variable in this script
    echo to match your actual Visual Studio installation path.
    echo.
    echo Common MSBuild paths:
    echo   - VS 2026 BuildTools: C:\Program Files ^(x86^)\Microsoft Visual Studio\2026\BuildTools\MSBuild\Current\Bin\MSBuild.exe
    echo   - VS 2022 BuildTools: C:\Program Files ^(x86^)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe
    echo   - VS 2026 Community: C:\Program Files\Microsoft Visual Studio\2026\Community\MSBuild\Current\Bin\MSBuild.exe
    echo   - VS 2022 Community: C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe
    pause
    exit /b 1
)

echo Found MSBuild: !MSBUILDPATH!
echo.

rem Compile project
echo Compiling project...
"!MSBUILDPATH!" "Wallpaper 24h.sln" /p:Configuration=Release

if %ERRORLEVEL% neq 0 (
    echo.
    echo Compilation failed. Please check error messages.
    pause
    exit /b 1
)

echo.
echo Compilation successful!
echo.

rem Use release directory directly
set RELEASEDIR=x64\Release

echo Using release directory: !RELEASEDIR!
echo.

rem Create wallpapers folder if not exists
if not exist "!RELEASEDIR!\wallpapers" (
    mkdir "!RELEASEDIR!\wallpapers"
    echo Created wallpapers directory: !RELEASEDIR!\wallpapers
) else (
    echo Wallpapers directory already exists: !RELEASEDIR!\wallpapers
)

rem Create README file
echo # Wallpaper 24h Release > "!RELEASEDIR!\README.txt"
echo. >> "!RELEASEDIR!\README.txt"
echo ## Instructions >> "!RELEASEDIR!\README.txt"
echo 1. Extract this folder to any location >> "!RELEASEDIR!\README.txt"
echo 2. Ensure the `wallpapers` folder exists >> "!RELEASEDIR!\README.txt"
echo 3. Put your wallpaper images into the `wallpapers` folder >> "!RELEASEDIR!\README.txt"
echo 4. Edit the `config.txt` file to set the corresponding wallpaper paths >> "!RELEASEDIR!\README.txt"
echo 5. Run `Wallpaper 24h.exe` to start the program >> "!RELEASEDIR!\README.txt"
echo. >> "!RELEASEDIR!\README.txt"
echo ## Configuration Example >> "!RELEASEDIR!\README.txt"
echo Please refer to the example format in the `config.txt` file to ensure correct image paths. >> "!RELEASEDIR!\README.txt"
echo. >> "!RELEASEDIR!\README.txt"
echo ## Supported Image Formats >> "!RELEASEDIR!\README.txt"
echo - PNG >> "!RELEASEDIR!\README.txt"
echo - JPG >> "!RELEASEDIR!\README.txt"
echo - BMP >> "!RELEASEDIR!\README.txt"
echo. >> "!RELEASEDIR!\README.txt"
echo ## System Requirements >> "!RELEASEDIR!\README.txt"
echo - Windows 7 or later >> "!RELEASEDIR!\README.txt"
echo - .NET Framework 4.0 or later >> "!RELEASEDIR!\README.txt"
echo Created README file: !RELEASEDIR!\README.txt

echo.
echo =====================================
echo Release preparation completed!
echo Release directory: !RELEASEDIR!
echo.
echo Next steps:
echo 1. Put wallpaper images into !RELEASEDIR!\wallpapers folder
echo 2. Edit !RELEASEDIR!\config.txt file
echo 3. Run !RELEASEDIR!\Wallpaper 24h.exe
echo.
echo =====================================

pause
