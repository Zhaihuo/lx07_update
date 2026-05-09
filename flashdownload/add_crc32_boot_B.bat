@echo off
REM 批量处理工具脚本
REM 用途：将HEX文件转换为BIN文件，然后生成带CRC32校验的SREC文件

copy "..\boot\Lx07_Project\KeilProject\Objects\Lx07_Boot_B.hex" "./Lx07_Boot_B.hex"

set HEXVIEW_PATH="Hexview\hexview.exe"
set INPUT_HEX=./Lx07_Boot_B.hex
set OUTPUT_BIN=./Lx07_Boot_B.bin
set OUTPUT_SREC=./Lx07_Boot_B_crc32.srec

REM 第一步：将HEX文件转换为BIN文件
%HEXVIEW_PATH% %INPUT_HEX% /XN -o /s %OUTPUT_BIN%

REM 第二步：为BIN文件添加CRC32校验并生成SREC格式文件
%HEXVIEW_PATH% /IN:%OUTPUT_BIN%;0xE000 /CS9:@append /XS:32:2 /AL:32 /s -o %OUTPUT_SREC%

echo Lx07_Boot_B hex Convert srec_crc32 Successful!
REM pause