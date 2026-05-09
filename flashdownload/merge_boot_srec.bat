@echo off
chcp 65001 >nul
echo ==============================================
echo      Merge S3-format SREC Files (v1.65.0)
echo ==============================================

:: --------------------------
:: 请在这里修改你的文件名
:: --------------------------
set FILE1=Lx07_Boot_A_crc32.srec
set FILE2=Lx07_Boot_B_crc32.srec
set OUTPUT=boot.srec
:: --------------------------

echo.
echo Merging: %FILE1% + %FILE2%
echo Output:  %OUTPUT%
echo.

:: 正确的命令，强制输出S3格式，不添加额外S0头
srec_cat.exe %FILE1% %FILE2% -o %OUTPUT% -address-length=4 -disable-sequence-warning

if %errorlevel% equ 0 (
    echo.
    echo SUCCESS: Merge completed.
    echo Output file: %OUTPUT%
    echo.
    echo --- Checking output file info ---
    srec_info.exe %OUTPUT%
) else (
    echo.
    echo ERROR: Merge failed.
    echo Possible reasons:
    echo 1. Files not found in the same folder
    echo 2. Incorrect command syntax
)

echo.
pause