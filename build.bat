@echo off
setlocal enabledelayedexpansion

:: Build script for Shadingway
:: Usage: build.bat [Release|Debug] [x64|Win32]

echo Shadingway Build Script
echo ======================

:: Set default configuration and platform if not specified
set CONFIG=Release
set PLATFORM=x64

:: Parse command line arguments
if not "%1"=="" (
    if /i "%1"=="Release" (
        set CONFIG=Release
    ) else if /i "%1"=="Debug" (
        set CONFIG=Debug
    ) else (
        echo Unknown configuration: %1
        echo Usage: build.bat [Release^|Debug] [x64^|Win32]
        exit /b 1
    )
)

if not "%2"=="" (
    if /i "%2"=="x64" (
        set PLATFORM=x64
    ) else if /i "%2"=="Win32" (
        set PLATFORM=Win32
    ) else if /i "%2"=="x86" (
        set PLATFORM=Win32
    ) else (
        echo Unknown platform: %2
        echo Usage: build.bat [Release^|Debug] [x64^|Win32]
        exit /b 1
    )
)

echo Building Shadingway with configuration: %CONFIG%, platform: %PLATFORM%

:: Detect Visual Studio installation and MSBuild path
set FOUND_MSBUILD=0
set MSBUILD_PATH=

:: Check for VS2022 (17.0)
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set MSBUILD_PATH="%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    set FOUND_MSBUILD=1
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set MSBUILD_PATH="%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    set FOUND_MSBUILD=1
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set MSBUILD_PATH="%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    set FOUND_MSBUILD=1
)

:: Check for VS2019 (16.0) if VS2022 wasn't found
if %FOUND_MSBUILD%==0 (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" (
        set MSBUILD_PATH="%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
        set FOUND_MSBUILD=1
    ) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" (
        set MSBUILD_PATH="%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"
        set FOUND_MSBUILD=1
    ) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
        set MSBUILD_PATH="%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
        set FOUND_MSBUILD=1
    )
)

:: Check for VS2017 (15.0) if VS2019 wasn't found
if %FOUND_MSBUILD%==0 (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe" (
        set MSBUILD_PATH="%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"
        set FOUND_MSBUILD=1
    ) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Professional\MSBuild\15.0\Bin\MSBuild.exe" (
        set MSBUILD_PATH="%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Professional\MSBuild\15.0\Bin\MSBuild.exe"
        set FOUND_MSBUILD=1
    ) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe" (
        set MSBUILD_PATH="%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe"
        set FOUND_MSBUILD=1
    )
)

:: Try to find MSBuild in PATH if it wasn't found in known locations
if %FOUND_MSBUILD%==0 (
    where msbuild.exe >nul 2>&1
    if %ERRORLEVEL%==0 (
        set MSBUILD_PATH=msbuild.exe
        set FOUND_MSBUILD=1
    )
)

:: Exit if MSBuild wasn't found
if %FOUND_MSBUILD%==0 (
    echo Error: Could not find MSBuild.exe. Make sure Visual Studio is installed.
    exit /b 1
)

echo Using MSBuild from: %MSBUILD_PATH%

:: Create dist directory if it doesn't exist
if not exist dist (
    echo Creating dist directory...
    mkdir dist
)

:: Clean the previous build
echo Cleaning previous build...
%MSBUILD_PATH% Shadingway.sln /t:Clean /p:Configuration=%CONFIG% /p:Platform=%PLATFORM%
if %ERRORLEVEL% neq 0 (
    echo Error: Build clean failed!
    exit /b %ERRORLEVEL%
)

:: Build the project
echo Building the project...
%MSBUILD_PATH% Shadingway.sln /t:Build /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /v:minimal
if %ERRORLEVEL% neq 0 (
    echo Error: Build failed!
    exit /b %ERRORLEVEL%
) else (
    echo Build completed successfully!
    echo The addon file has been copied to the dist folder.
    echo:
    if exist dist\version.txt (
        echo Current version:
        type dist\version.txt
    )
)

:: Run publish.bat if it exists
if exist publish.bat (
    echo.
    echo Running publish script...
    call publish.bat
    if %ERRORLEVEL% neq 0 (
        echo Warning: Publish script completed with errors.
    ) else (
        echo Publish completed successfully.
    )
)

exit /b 0