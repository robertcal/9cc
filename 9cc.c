#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_NUM, // 整数トークン
    TK_EOF, // 入力の終わりを表すトークン
} Tokenkind;

typedef struct Token Token;

// トークン型
struct Token {
    Tokenkind kind; // トークンの型
    Token *next; // 次の入力トークン（連結リスト構造）
    int val; // kindがTK_NUMの場合、その数値
    char *str; // トークン文字列
};

// 現在着目しているトークン
Token *token;

void error(char *fmt, ...) { // 可変長引数
    va_list ap;
    va_start(ap, fmt); // ポインタapを、引数fmtの次の位置にする
    vfprintf(stderr, fmt, ap); // 標準エラー出力へ出す
    fprintf(stderr, "\n");
    exit(1);
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(Tokenkind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token)); // 連結リストはどこまで増えるか分からないため、動的にメモリを確保する必要がある
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
    Token head; // 最初にダミーの要素を作り、そこから連結リストを作る（head自身はreturnしないためローカル変数で良い）
    head.next = NULL;
    Token *cur = &head;

    while (*p) { // 文字列が終わるまで（文字列の最後はヌル文字が入りwhile文が終了する）
        // 空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++); // new_tokenを実行した後に、p++でポインタを進める
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10); // 10進数でポインタから数値に変換できるところを変換して、ポインタを進める
            continue;
        }

        error("トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    // トークナイズする
    token = tokenize(argv[1]); // tokenはグローバル変数に設定

    // アセンブリの前半部分を出力
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
