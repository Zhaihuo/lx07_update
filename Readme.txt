这是lx07项目的升级软件，分为preboot、boot、app

Z20k118M芯片

文件说明：
1）在flashdownload/appupdate目录中（与bootupdate一致）
11）add_crc32_app_A.bat、add_crc32_app_B.bat将hex文件转换成bin和增加crc32的srec文件
12）update_crc32.bat将生成带有crc32的srec文件中的校验值替换成重新计算的校验值，其中srec_checksum.exe与crc32.exe
是执行update_crc32.bat的必要文件
13）merge_app_srec.bat是将生成后的srec文件合成一个srec文件
14）在flashdownload目录中，getappsrec.bat执行flashdownload/bat/目录下所有.bat文件
2）在flashdownload/files目录中
21）upgrate_app_srec.py是串口升级脚本，使用的升级文件是App.srec
3）在flashdownload/Autoburn目录中
31）preboot_boot_app_flashburn.bat用于烧录preboot、boot、app文件


升级其他app：
1）doc文件夹中记录了使用其他app时需要更改的部分，更新完后，将编译的hex文件复制到 app\Lx07_Project\KeilProject\Objects路径下
使用串口升级流程：


1）接通uart2，Jlink
2）进入flashdownload/Autoburn，运行preboot_boot_app_flashburn.bat首次烧录preboot、boot、app程序
3）升级boot：
31）进入flashdownload/bootupdate，运行getbootsrec.bat，生成的所有文件在files文件夹中
32）进入flashdownload/bootupdate/files，在终端运行upgrate_boot_srec.py调用boot.srec文件
3）升级app：
31）进入flashdownload/appupdate，运行getappsrec.bat，生成的所有文件在files文件夹中
32）进入flashdownload/appupdate/files，在终端运行upgrate_app_srec.py调用App.srec文件
