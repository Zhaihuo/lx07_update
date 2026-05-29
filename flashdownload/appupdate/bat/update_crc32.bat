@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion
echo === Lx07 App A/B 分区 CRC32 更新工具（BAT版） ===
echo.

:: 依赖工具：必须同目录放这两个小工具
:: 1. crc32.exe    计算 bin 文件 CRC32（输出大写十六进制）
:: 2. srec_cat.exe 处理 SREC 行、计算校验和
:: 我会一并给你用法

set "success=0"

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: 处理 A 分区
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
call :PROCESS "A 分区" "Lx07_App_A.bin" "Lx07_App_A_crc32.srec"
if !errorlevel! equ 0 set /a success+=1

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: 处理 B 分区
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
call :PROCESS "B 分区" "Lx07_App_B.bin" "Lx07_App_B_crc32.srec"
if !errorlevel! equ 0 set /a success+=1

echo.
echo === 完成！共成功处理 %success%/2 个分区 ===
@REM pause >nul
if !success! equ 2 (
    exit /b
) else (
    echo 存在处理失败项，按任意键退出...
    pause >nul
    exit /b
)
exit /b

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: 处理函数：%1=名称  %2=bin  %3=srec
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:PROCESS
echo 📄 处理 %~1: %~2
if not exist "%~2" (
    echo ⚠️ BIN 文件不存在：%~2
    exit /b 1
)
if not exist "%~3" (
    echo ⚠️ SREC 文件不存在：%~3
    exit /b 1
)

:: ===================== 计算 CRC32 =====================
for /f "delims=" %%c in ('crc32 "%~2"') do set "CRC32=%%c"
echo 🔢 CRC32 = 0x%CRC32%

:: ===================== 读取所有行 =====================
set "total_lines=0"
for /f "delims=" %%a in ('type "%~3" ^| find /c /v ""') do set total_lines=%%a
set /a target_line=total_lines - 1
if !target_line! lss 1 set target_line=1

:: ===================== 逐行处理 =====================
set "line_num=0"
set "updated=0"
set "out_file=%~n3_updated%~x3"
del /q "%out_file%" 2>nul

for /f "tokens=* delims=" %%L in ('findstr /n "^" "%~3"') do (
    set "line=%%L"
    set "line=!line:*:=!"
    set /a line_num+=1

    :: 目标行：倒数第二行
    if !line_num! equ !target_line! (
        echo !line! | findstr /b "S3" >nul
        if !errorlevel! equ 0 (
            set "pre=!line:~0,12!"
            set "end=!line:~20,-2!"
            set "new_base=!pre!!CRC32!!end!"

            :: 计算 SREC 校验和
            for /f "delims=" %%s in ('srec_checksum "!new_base!"') do set "sum=%%s"
            set "new_line=!new_base!!sum!"

            echo    ✅ 替换成功：!line:~12,8! → !CRC32! 校验和=!sum!
            echo !new_line!>>"%out_file%"
            set updated=1
        ) else (
            echo !line!>>"%out_file%"
        )
    ) else (
        if not "!line!"=="" echo !line!>>"%out_file%"
    )
)

if !updated! equ 1 (
    echo    🎉 保存成功：%out_file%
    exit /b 0
) else (
    echo    ❌ 未找到倒数第二行 S3 记录
    exit /b 1
)