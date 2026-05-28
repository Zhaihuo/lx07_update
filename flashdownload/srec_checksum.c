#include <stdio.h>
#include <stdlib.h>  // 关键：strtol 在这里声明
#include <string.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }

    char* s = argv[1];
    int sum = 0;

    // 从第 2 个字符开始（跳过 'Sx'），每两个字符算一个字节
    for (int i = 2; s[i] && s[i+1]; i += 2) {
        char b[3] = {s[i], s[i+1], 0};
        sum += strtol(b, NULL, 16);
    }

    // SREC 校验和 = (~总和) & 0xFF
    sum = (~sum) & 0xFF;
    printf("%02X", sum);
    return 0;
}