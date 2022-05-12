#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 入力
char *user_input;

// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_NUM, // 整数トークン
    TK_EOF, // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind; // トークンの型
    Token *next; // 次の入力トークン（連結リスト構造）
    int val; // kindがTK_NUMの場合、その数値
    char *str; // トークン文字列
    int len; // トークンの長さ
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

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) { // 可変長引数
    va_list ap;
    va_start(ap, fmt); // ポインタapを、引数fmtの次の位置にする

    int pos = loc - user_input; // エラー箇所の文字の位置を計算
    fprintf(stderr, "%s\n", user_input); // 入力全体を出力
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 次のトークンが期待している記号の時には、トークンを一つ読み進めて真を返す
// それ以外の場合は偽を返す
bool consume(char *op) {
    if (token->kind != TK_RESERVED || // 記号以外の場合
        strlen(op) != token->len || // 期待する記号とトークンの長さが違う場合
        memcmp(token->str, op, token->len)) // 第一引数と第二引数の先頭アドレスからlen分の文字を比較して違う場合
    {
        return false;
    }

    token = token->next;
    return true;
}

// 次のトークンが期待している記号の時には、トークンを一つ読み進める
// それ以外の場合はエラーを報告する
void expect(char *op) {
    if (token->kind != TK_RESERVED || // 記号以外の場合
        strlen(op) != token->len || // 期待する記号とトークンの長さが違う場合
        memcmp(token->str, op, token->len)) // 第一引数と第二引数の先頭アドレスからlen分の文字を比較して違う場合
    {
        error_at(token->str, "'%s'ではありません", op);
    } else {
        token = token->next;
    }
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す
// それ以外の場合にはエラーを報告する
int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "数ではありません");
    }

    int val = token->val;
    token = token->next;
    return val;
}

// 着目しているトークンが終わりか
bool at_eof() {
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str) {
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

        if (strchr("+-*/()", *p)) {
            cur = new_token(TK_RESERVED, cur, p++); // new_tokenを実行した後に、p++でポインタを進める
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10); // 10進数でポインタから数値に変換できるところを変換して、ポインタを進める
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

// 抽象構文木のノードの種類
typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
    ND_RT,  // >
    ND_RE,  // >=
    ND_NUM, // 整数
    ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs; // 左辺
    Node *rhs; // 右辺
    int val; // kindがND_NUMの場合のみ使う
};

// 新しいノードを作成する
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node)); // ノードはどこまで増えるか分からないため、動的にメモリを確保する必要がある
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 新しい整数ノードを作成する
Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node)); // ノードはどこまで増えるか分からないため、動的にメモリを確保する必要がある
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// 関数のプロトタイプ宣言
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// expr = equality
Node *expr = equality();

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
    Node *node = relational();

    for (;;) { // 無限ループ
        if (consume("==")) {
            node = new_node(ND_EQ, node, equality());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, equality());
        } else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
    Node *node = add();

    for (;;) { // 無限ループ
        if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if (consume(">")) {
            node = new_node(ND_RT, node, add());
        } else if (consume(">=")) {
            node = new_node(ND_RE, node, add());
        } else {
            return node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
    Node *node = mul();

    for (;;) { // 無限ループ
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

// unary = ("+" | "-")? primary
Node *unary() {
    if (consume("+")) {
        return primary();
    } else if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary()); // -xを0-xにしてしまう
    } else {
        return primary();
    }
}

// primary = num | "(" expr ")"
Node *primary() {
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    // そうでなければ数値のはず
    return new_node_num(expect_number());
}

// 抽象構文木をアセンブリにする関数
void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs); // 左の部分木から数値をスタックにpushする
    gen(node->rhs); // 右の部分木から数値をスタックにpushする

    // スタックから数値2つをレジスタにpopする
    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
    }

    // 計算結果の値をスタックにpushする
    printf("  push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];

    // トークナイズする
    token = tokenize(user_input); // tokenはグローバル変数に設定

    // 抽象構文木にする
    Node *node = expr();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // アセンブリを出力
    gen(node);

    // スタックトップに式全体の値が残っているので、それをRAXにロードして関数からの戻り値にする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
