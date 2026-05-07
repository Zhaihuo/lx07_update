@echo off
chcp 65001 > nul
setlocal enabledelayedexpansion

:: 以脚本所在目录为基准
cd /d "%~dp0"

:: 关键：用 \ 代替 /
set "BIN_A=..\..\boot\Lx07_Project\KeilProject\Objects\Lx07_Boot_A.bin"
set "BIN_B=..\..\boot\Lx07_Project\KeilProject\Objects\Lx07_Boot_B.bin"

echo 正在合成...
echo 路径A: !BIN_A!
echo 路径B: !BIN_B!

:: 合成命令
copy /b "!BIN_A!" + "!BIN_B!" "boot.bin"

echo.
if exist "boot.bin" (
    echo ✅ 合成成功！
) else (
    echo ❌ 合成失败
)
pause