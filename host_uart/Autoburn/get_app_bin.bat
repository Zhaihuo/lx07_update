@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

:: 出错立即停止
set "errorstatus=0"

:: 复制 A
copy "..\..\app\Lx07_Project\KeilProject\Objects\Lx07_App_A.bin" .
if errorlevel 1 goto fail

:: 复制 B
copy "..\..\app\Lx07_Project\KeilProject\Objects\Lx07_App_B.bin" .
if errorlevel 1 goto fail

:: 生成正确布局 bin
powershell -Command "$out=New-Object byte[] 0x3F000; for($i=0;$i -lt 0x3F000;$i++){$out[$i]=0xFF}; $a=[System.IO.File]::ReadAllBytes('Lx07_App_A.bin'); $b=[System.IO.File]::ReadAllBytes('Lx07_App_B.bin'); $a.CopyTo($out, 0x18000); $b.CopyTo($out, 0x2B800); [System.IO.File]::WriteAllBytes('app.bin',$out)"
if errorlevel 1 goto fail

:: 清理临时文件
del Lx07_App_A.bin
del Lx07_App_B.bin

echo.
echo get app.bin successful!
exit /b 0

:fail
echo.
echo ERROR: Build failed!
pause
exit /b 1