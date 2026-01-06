@echo off
chcp 65001 >nul
cls

REM =====================================
REM   StudyLink Build & Run (Stable v4)
REM   - For large users.jsonl (6000+)
REM =====================================

REM Always run in bat directory
cd /d "%~dp0"

echo =====================================
echo        StudyLink Runner
echo =====================================
echo Current dir: %cd%
echo.

REM ---- Kill old main.exe to avoid Permission denied ----
taskkill /f /im main.exe >nul 2>nul

REM ---- Check g++ exists ----
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERR] g++ not found.
    echo Please install MinGW-w64/MSYS2 and add g++ to PATH.
    pause
    exit /b 1
)

REM ---- Ensure data folder/files exist ----
if not exist data (
    echo [INFO] Creating data folder...
    mkdir data
)

for %%f in (users.jsonl applications.jsonl teams.jsonl messages.jsonl reviews.jsonl tasks.jsonl) do (
    if not exist data\%%f (
        echo [INFO] Creating data\%%f
        type nul > data\%%f
    )
)

if not exist data\config.json (
    echo [INFO] Creating data\config.json
    echo {"strategy":"weighted","w_goal":0.5,"w_location":0.3,"w_time":0.2,"topN":5} > data\config.json
)

REM ---- Check users.jsonl not empty ----
for %%A in (data\users.jsonl) do set size=%%~zA
echo [INFO] users.jsonl size: %size% bytes
if %size% LSS 50 (
    echo [WARN] users.jsonl seems empty!
    echo Please copy your demo users.jsonl into data\users.jsonl
    pause
)

echo.
echo ===== Select Mode =====
echo 1) Build + Run   (after code changes)
echo 2) Run only      (demo mode, no rebuild)
echo 3) Build only    (compile without running)
echo 0) Exit
echo.
set /p mode=Enter choice (0/1/2/3): 

if "%mode%"=="0" exit /b 0
if "%mode%"=="2" goto RUNONLY
if "%mode%"=="3" goto BUILDONLY

REM =====================================
REM Build
REM =====================================
:BUILD
echo.
echo ===== [1/2] Building =====
echo.

g++ -std=c++17 ^
main.cpp ^
auth/auth.cpp ^
storage/storage.cpp ^
storage/meta.cpp ^
storage/migrate.cpp ^
matching/matcher.cpp ^
matching/matching_config.cpp ^
team/application.cpp ^
team/message.cpp ^
team/review.cpp ^
team/team.cpp ^
analytics/analytics.cpp ^
study/task.cpp ^
-o main.exe

if %errorlevel% neq 0 (
    echo.
    echo [ERR] Build failed. See errors above.
    pause
    exit /b %errorlevel%
)

echo.
echo [OK] Build success!
echo.

if "%mode%"=="3" goto END

REM =====================================
REM Run
REM =====================================
:RUNONLY
echo ===== [2/2] Running =====
echo.
echo [TIP] With 6000 users, first load may take a few seconds. Please wait...
echo.

main.exe

echo.
echo =====================================
echo Program exited. Press any key.
echo =====================================
pause >nul
goto END

:BUILDONLY
set mode=3
goto BUILD

:END
