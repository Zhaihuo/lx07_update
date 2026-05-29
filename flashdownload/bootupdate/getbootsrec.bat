@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

:: 切换到当前脚本所在目录
cd /d "%~dp0"
echo 当前工作目录: %cd%

:: 定义目录
set "BAT_DIR=bat"
set "FILES_DIR=files"

echo ======================================
echo 开始执行 getbootsrec 流程
echo ======================================
echo.

:: 1. 调用 bat\add_crc32_boot_A.bat
echo [1/4] 正在执行 %BAT_DIR%\add_crc32_boot_A.bat
pushd "%BAT_DIR%"
call "add_crc32_boot_A.bat"
popd
if !errorlevel! neq 0 (
    echo ❌ add_crc32_boot_A.bat 执行失败！
    pause
    exit /b 1
)
echo ✅ add_crc32_boot_A.bat 执行成功
echo.

:: 2. 调用 bat\add_crc32_boot_B.bat
echo [2/4] 正在执行 %BAT_DIR%\add_crc32_boot_B.bat
pushd "%BAT_DIR%"
call "add_crc32_boot_B.bat"
popd
if !errorlevel! neq 0 (
    echo ❌ add_crc32_boot_B.bat 执行失败！
    pause
    exit /b 1
)
echo ✅ add_crc32_boot_B.bat 执行成功
echo.

:: 3. 调用 bat\update_crc32.bat
echo [3/4] 正在执行 %BAT_DIR%\update_crc32.bat
pushd "%BAT_DIR%"
call "update_crc32.bat"
popd
if !errorlevel! neq 0 (
    echo ❌ update_crc32.bat 执行失败！
    pause
    exit /b 1
)
echo ✅ update_crc32.bat 执行成功
echo.

:: 4. 调用 bat\merge_boot_srec.bat
echo [4/4] 正在执行 %BAT_DIR%\merge_boot_srec.bat
pushd "%BAT_DIR%"
call "merge_boot_srec.bat"
popd
if !errorlevel! neq 0 (
    echo ❌ merge_boot_srec.bat 执行失败！
    pause
    exit /b 1
)
echo ✅ merge_boot_srec.bat 执行成功
echo.

:: ========== 全部执行完后：清理 + 移动文件 ==========
echo [收尾] 清理 %FILES_DIR% 目录中的 .hex .bin .srec 文件
if not exist "%FILES_DIR%" mkdir "%FILES_DIR%"
del /f /q "%FILES_DIR%\*.hex" 2>nul
del /f /q "%FILES_DIR%\*.bin" 2>nul
del /f /q "%FILES_DIR%\*.srec" 2>nul
echo ✅ 清理完成
echo.

echo [收尾] 将 %BAT_DIR% 下的 .hex .bin .srec 移动到 %FILES_DIR%
move /y "%BAT_DIR%\*.hex" "%FILES_DIR%\" 2>nul
move /y "%BAT_DIR%\*.bin" "%FILES_DIR%\" 2>nul
move /y "%BAT_DIR%\*.srec" "%FILES_DIR%\" 2>nul
echo ✅ 移动完成
echo.
:: =====================================================

echo ======================================
echo 🎉 全部执行成功，窗口自动关闭...
echo ======================================

exit /b 0