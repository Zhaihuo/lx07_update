@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

:: 切换到当前脚本所在目录
cd /d "%~dp0.."
echo 当前工作目录: %cd%

echo ======================================
echo 开始执行 getbootsrec 流程
echo ======================================
echo.

:: 1. 调用 add_crc32_boot_A.bat
echo [1/4] 正在执行 add_crc32_boot_A.bat
call "add_crc32_boot_A.bat"
if !errorlevel! neq 0 (
    echo ❌ add_crc32_boot_A.bat 执行失败！
    pause
    exit /b 1
)
echo ✅ add_crc32_boot_A.bat 执行成功
echo.

:: 2. 调用 add_crc32_boot_B.bat
echo [2/4] 正在执行 add_crc32_boot_B.bat
call "add_crc32_boot_B.bat"
if !errorlevel! neq 0 (
    echo ❌ add_crc32_boot_B.bat 执行失败！
    pause
    exit /b 1
)
echo ✅ add_crc32_boot_B.bat 执行成功
echo.

:: 3. 调用 update_crc32.bat
echo [3/4] 正在执行 update_crc32.bat
call "update_crc32.bat"
if !errorlevel! neq 0 (
    echo ❌ update_crc32.bat 执行失败！
    pause
    exit /b 1
)
echo ✅ update_crc32.bat 执行成功
echo.

:: 4. 调用 merge_boot_srec.bat
echo [4/4] 正在执行 merge_boot_srec.bat
call "merge_boot_srec.bat"
if !errorlevel! neq 0 (
    echo ❌ merge_boot_srec.bat 执行失败！
    pause
    exit /b 1
)
echo ✅ merge_boot_srec.bat 执行成功
echo.

echo ======================================
echo 🎉 全部执行成功，窗口自动关闭...
echo ======================================

:: 成功：强制立即退出，关闭窗口
exit /b 0