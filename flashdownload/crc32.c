#include <stdio.h>
#include <stdint.h>

static uint32_t crc32_table[256];
static int table_initialized = 0;

// 初始化 CRC32 表
void init_crc32_table() {
    if (table_initialized) return;
    for (int i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            if (c & 1)
                c = 0xEDB88320 ^ (c >> 1);
            else
                c >>= 1;
        }
        crc32_table[i] = c;
    }
    table_initialized = 1;
}

// 计算 CRC32
uint32_t crc32(const void *buf, size_t len, uint32_t crc) {
    const uint8_t *p = (const uint8_t *)buf;
    init_crc32_table();
    crc ^= 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc = crc32_table[(crc ^ p[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }

    FILE* f = fopen(argv[1], "rb");
    if (!f) {
        return 1;
    }

    unsigned char buf[4096];
    uint32_t crc = 0;
    size_t n;

    while ((n = fread(buf, 1, 4096, f)) > 0) {
        crc = crc32(buf, n, crc);
    }
    fclose(f);

    printf("%08X", crc & 0xFFFFFFFF);
    return 0;
}