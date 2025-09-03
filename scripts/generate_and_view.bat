@echo off
echo =====================================
echo Perlin Noise Generator and Viewer
echo =====================================
echo.

REM Check if Python is available
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python is not installed or not in PATH
    echo Please install Python from https://python.org
    pause
    exit /b 1
)

REM Change to the data directory
cd /d "%~dp0..\data"
echo Current directory: %cd%
echo.

REM Check if the noise file exists
if not exist "noise_200x200_2bpp.bin" (
    echo Generating Perlin noise binary...
    python "..\scripts\pearlian.py" --out "noise_200x200_2bpp.bin" --seed 1337 --scale 25.0
    if errorlevel 1 (
        echo ERROR: Failed to generate noise file
        pause
        exit /b 1
    )
    echo.
) else (
    echo Found existing noise file: noise_200x200_2bpp.bin
    echo.
)

REM Try to install Pillow if not available
python -c "import PIL" >nul 2>&1
if errorlevel 1 (
    echo Installing Pillow for image display...
    python -m pip install Pillow
    echo.
)

REM Display the noise
echo Displaying noise binary...
python "..\scripts\display_noise.py" "noise_200x200_2bpp.bin" --png "noise_preview.png"

echo.
echo Available files in data directory:
dir *.bin *.png

echo.
echo =====================================
echo Commands you can run manually:
echo.
echo Generate new noise:
echo   python ..\scripts\pearlian.py --seed 2024 --scale 30.0
echo.
echo Display noise:
echo   python ..\scripts\display_noise.py noise_200x200_2bpp.bin
echo.
echo Display as ASCII:
echo   python ..\scripts\display_noise.py noise_200x200_2bpp.bin --ascii
echo =====================================
pause
