#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void error(char *fmt, ...) { // 可変長引数
    va_list ap;
    va_start(ap, fmt); // ポインタapを、引数fmtの次の位置にする
    vfprintf(stderr, fmt, ap); // 標準エラー出力へ出す
    fprintf(stderr, "\n");
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    char *p = argv[1]; // 引数の文字列の先頭ポインタ

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    printf("  mov rax, %ld\n", strtol(p, &p, 10)); // 10進数でポインタから数値に変換できるところを変換して、ポインタを進める

    while (*p) { // 文字列が終わるまで（文字列の最後はヌル文字が入りwhile文が終了する）
        if (*p == '+') {
            p++;
            printf("  add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        if (*p == '-') {
            p++;
            printf("  sub rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        fprintf(stderr, "予期しない文字です： '%c'\n", *p);
        return 1;
    }

    printf("  ret\n");
    return 0;
}
