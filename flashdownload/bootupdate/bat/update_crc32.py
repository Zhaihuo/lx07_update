#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import zlib
import sys
import os

def calculate_ieee_crc32(bin_file):
    with open(bin_file, 'rb') as f:
        data = f.read()
    crc = zlib.crc32(data) & 0xFFFFFFFF
    return crc


def calc_srec_checksum(line_without_checksum):
    data = bytes.fromhex(line_without_checksum[2:])
    checksum = sum(data) & 0xFF
    return (~checksum) & 0xFF


def update_single_partition(srec_file, bin_file, partition_name):
    if not os.path.exists(srec_file):
        print(f"⚠️ SREC 文件不存在: {srec_file}")
        return False
    if not os.path.exists(bin_file):
        print(f"⚠️ BIN 文件不存在: {bin_file}")
        return False

    crc = calculate_ieee_crc32(bin_file)
    crc_bytes = crc.to_bytes(4, byteorder='big')
    crc_hex = crc_bytes.hex().upper()

    print(f"\n📄 处理 {partition_name}: {bin_file}")
    print(f"🔢 CRC32 = 0x{crc_hex}")

    updated = False
    new_lines = []

    with open(srec_file, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()                     # 保留原始换行

    for i, line in enumerate(lines):
        stripped = line.strip()
        if not stripped:
            new_lines.append(line)                # 保留空行
            continue

        # 统一替换倒数第二行
        if i == len(lines) - 2 and stripped.startswith('S3'):
            old_data = stripped[12:20] if len(stripped) >= 20 else ""
            new_line_base = stripped[:12] + crc_hex + stripped[20:-2] if len(stripped) >= 22 else stripped[:12] + crc_hex
            
            checksum = calc_srec_checksum(new_line_base)
            new_line = new_line_base + f"{checksum:02X}\n"
            
            print(f"   ✅ 倒数第二行替换: {old_data} → {crc_hex} | checksum: {checksum:02X}")
            new_lines.append(new_line)
            updated = True
        else:
            new_lines.append(line)                # 保留原始行（含换行）

    if not updated:
        print(f"   ❌ 未找到倒数第二行的 S3 记录")
        return False

    name, ext = os.path.splitext(srec_file)
    output_file = f"{name}_updated{ext}"

    with open(output_file, 'w', encoding='utf-8', newline='\r\n') as f:   # Windows 换行
        f.writelines(new_lines)

    print(f"   🎉 保存成功: {output_file}")
    return True


# ====================== 主程序 ======================
if __name__ == "__main__":
    print("=== Lx07 Boot A/B 分区 CRC32 更新工具（修复格式版） ===\n")

    partitions = [
        {"name": "A 分区", "bin": "Lx07_Boot_A.bin", "srec": "Lx07_Boot_A_crc32.srec"},
        {"name": "B 分区", "bin": "Lx07_Boot_B.bin", "srec": "Lx07_Boot_B_crc32.srec"}
    ]

    success_count = 0
    for part in partitions:
        if update_single_partition(part["srec"], part["bin"], part["name"]):
            success_count += 1

    print(f"\n=== 完成！共成功处理 {success_count}/2 个分区 ===")