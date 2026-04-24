@echo off
chcp 65001 >nul
title J-Link 3文件自动烧录（带重试+固定地址）

:: ====================== 【用户配置区域】 ======================
set JLINK_PATH=C:\Program Files\SEGGER\JLink_V844\JLink.exe
set DEVICE=Z20K118MC

:: 三个文件及其烧录地址
set preboot_hex=..\..\preboot\Lx07_Project\KeilProject\Objects\Lx07_Test.hex
set preboot_addr=0x00000000
set boot_hex=..\..\boot\Lx07_Project\KeilProject\Objects\Lx07_Test.hex
set boot_addr=0x00004000
set app_hex=..\..\app\Lx07_Project\KeilProject\Objects\Lx07_Test.hex
set app_addr=0x00018000

set SPEED=4000
set INTERFACE=SWD
::重试烧录次数
set RETRY_TIMES=3
:: ==============================================================

set RETRY_COUNT=0
:RETRY_LOOP
set /a RETRY_COUNT+=1
echo.
echo ======================================================
echo          第 %RETRY_COUNT% / %RETRY_TIMES% 次烧录尝试
echo ======================================================
echo 芯片：%DEVICE%
echo 文件1：%preboot_hex% 地址：%preboot_addr%
echo 文件2：%boot_hex% 地址：%boot_addr%
echo 文件3：%app_hex% 地址：%app_addr%
echo ======================================================
echo.

:: 检查关键文件是否存在
if not exist "%JLINK_PATH%" (
    echo 错误：找不到 JLink.exe，请检查路径！
    pause
    exit /b 1
)
if not exist "%preboot_hex%" (echo 错误：找不到文件 %preboot_hex% & pause & exit /b 1)
if not exist "%boot_hex%" (echo 错误：找不到文件 %boot_hex% & pause & exit /b 1)
if not exist "%app_hex%" (echo 错误：找不到文件 %app_hex% & pause & exit /b 1)

:: 1. 生成干净的J-Link脚本
echo 正在生成烧录脚本...
(
echo connect
echo loadfile "%preboot_hex%" %preboot_addr%
echo loadfile "%boot_hex%" %boot_addr%
echo loadfile "%app_hex%" %app_addr%
echo reset
echo go
echo exit
) > jlink_script.jlink

:: 2. 执行烧录
echo 正在烧录，请不要断开仿真器...
"%JLINK_PATH%" -Device %DEVICE% -If %INTERFACE% -Speed %SPEED% -CommandFile "jlink_script.jlink" > jlink_log.txt 2>&1

:: 3. 检查烧录结果
findstr /i /c:"O.K." /c:"Verified" /c:"Downloading completed" /c:"Successfully" jlink_log.txt >nul 2>&1
if %errorlevel% equ 0 (
    echo.
    echo ==============================================
    echo              烧录成功！✅
    echo ==============================================
    goto :SUCCESS
)

:: 4. 失败重试
echo 烧录失败！
if %RETRY_COUNT% lss %RETRY_TIMES% (
    echo 等待1秒后重试...
    timeout /t 1 /nobreak >nul
    goto RETRY_LOOP
)

:: 5. 最终失败 → 显示日志 + 暂停
echo.
echo ==============================================
echo              烧录失败！❌
echo ==============================================
echo 错误日志：
type jlink_log.txt
echo.
echo 按任意键关闭窗口...
pause >nul
goto :END

:SUCCESS
:: 成功：直接清理 + 退出，不暂停
echo 烧录完成，窗口即将自动关闭...
if exist jlink_log.txt del jlink_log.txt >nul
if exist jlink_script.jlink del jlink_script.jlink >nul
exit /b

:END
:: 失败后清理临时文件
if exist jlink_log.txt del jlink_log.txt >nul
if exist jlink_script.jlink del jlink_script.jlink >nul
exit /b