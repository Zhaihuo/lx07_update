@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

:: 出错立即停止
set "errorstatus=0"

:: 复制 A
copy "..\..\boot\Lx07_Project\KeilProject\Objects\Lx07_Boot_A.bin" .
if errorlevel 1 goto fail

:: 复制 B
copy "..\..\boot\Lx07_Project\KeilProject\Objects\Lx07_Boot_B.bin" .
if errorlevel 1 goto fail

:: 生成正确布局 bin
powershell -Command "$out=New-Object byte[] 0x18000; for($i=0;$i -lt 0x18000;$i++){$out[$i]=0xFF}; $a=[System.IO.File]::ReadAllBytes('Lx07_Boot_A.bin'); $b=[System.IO.File]::ReadAllBytes('Lx07_Boot_B.bin'); $a.CopyTo($out, 0x4000); $b.CopyTo($out, 0xE000); [System.IO.File]::WriteAllBytes('boot.bin',$out)"
if errorlevel 1 goto fail

:: 清理临时文件
del Lx07_Boot_A.bin
del Lx07_Boot_B.bin

echo.
echo get boot.bin successful!
exit /b 0

:fail
echo.
echo ERROR: Build failed!
pause
exit /b 1