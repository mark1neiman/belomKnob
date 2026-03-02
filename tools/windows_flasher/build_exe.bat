@echo off
setlocal

cd /d "%~dp0"

if not exist ".venv\Scripts\activate.bat" (
  echo [ERROR] Virtual env not found. Create it first:
  echo   python -m venv .venv
  exit /b 1
)

call ".venv\Scripts\activate.bat"
if errorlevel 1 exit /b 1

python -m pip install --upgrade pip >nul
python -m pip install -r requirements.txt >nul
python -m pip install pyinstaller >nul

pyinstaller --noconfirm --clean BelomFlasher.spec
if errorlevel 1 (
  echo [ERROR] Build failed.
  exit /b 1
)

echo.
echo Build complete:
echo   %cd%\dist\BelomFlasher.exe
exit /b 0
