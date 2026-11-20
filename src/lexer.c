#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} StrBuf;

static int buf_init(StrBuf *b) {
    b->cap = 32;
    b->len = 0;
    b->data = (char *)malloc(b->cap);
    if (b->data) {
        b->data[0] = '\0';
    }
    return b->data != NULL;
}

static int buf_push(StrBuf *b, char c) {
    if (b->len + 2 > b->cap) {
        if (b->cap > SIZE_MAX / 2) return 0;
        size_t nueva = b->cap * 2;
        char *datos = (char *)realloc(b->data, nueva);
        if (!datos) return 0;
        b->data = datos;
        b->cap = nueva;
    }
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
    return 1;
}

static void buf_free(StrBuf *b) {
    if (b->data) {
        free(b->data);
        b->data = NULL;
    }
    b->len = 0;
    b->cap = 0;
}

void lexer_init(Lexer *l, const char *texto) {
    l->texto = texto ? texto : "";
    l->i = 0;
    l->n = strlen(l->texto);
    l->line = 1;
    l->col = 1;
    l->has_error = 0;
    l->error_msg = NULL;
    l->error_line = 0;
    l->error_col = 0;
}

void lexer_destroy(Lexer *l) {
    if (l->error_msg) {
        free(l->error_msg);
        l->error_msg = NULL;
    }
}

static int char_at(const Lexer *l) {
    if (l->i >= l->n) return -1;
    return (unsigned char)l->texto[l->i];
}

static int avanzar(Lexer *l) {
    int c = char_at(l);
    if (c == -1) {
        l->i++;
        return -1;
    }
    l->i++;
    if (c == '\n') {
        l->line++;
        l->col = 1;
    } else {
        l->col++;
    }
    return c;
}

static int es_letra(int c) {
    if (c == -1) return 0;
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static int es_digito(int c) {
    if (c == -1) return 0;
    return c >= '0' && c <= '9';
}

static Token error_token(Lexer *l, int line, int col, const char *msg) {
    l->has_error = 1;
    l->error_line = line;
    l->error_col = col;
    if (l->error_msg) free(l->error_msg);
    l->error_msg = (char *)malloc(strlen(msg) + 1);
    if (l->error_msg) {
        strcpy(l->error_msg, msg);
    }
    return token_create(TOK_ERROR, msg, line, col);
}

static Token leer_identificador(Lexer *l) {
    int start_line = l->line;
    int start_col = l->col;
    StrBuf buf;
    if (!buf_init(&buf)) return error_token(l, start_line, start_col, "memoria insuficiente");

    while (1) {
        int c = char_at(l);
        if (c != -1 && (es_letra(c) || es_digito(c))) {
            if (!buf_push(&buf, (char)c)) {
                buf_free(&buf);
                return error_token(l, start_line, start_col, "memoria insuficiente");
            }
            avanzar(l);
        } else {
            break;
        }
    }

    TokenKind kind = TOK_ID;
    if (strcmp(buf.data, "sin") == 0) kind = TOK_SIN;
    else if (strcmp(buf.data, "sen") == 0) kind = TOK_SEN;
    else if (strcmp(buf.data, "cos") == 0) kind = TOK_COS;
    else if (strcmp(buf.data, "tan") == 0) kind = TOK_TAN;

    Token tok = token_create(kind, buf.data, start_line, start_col);
    buf_free(&buf);
    return tok;
}

static Token leer_numero(Lexer *l) {
    int start_line = l->line;
    int start_col = l->col;
    StrBuf buf;
    if (!buf_init(&buf)) return error_token(l, start_line, start_col, "memoria insuficiente");

    while (1) {
        int c = char_at(l);
        if (c != -1 && es_digito(c)) {
            if (!buf_push(&buf, (char)c)) {
                buf_free(&buf);
                return error_token(l, start_line, start_col, "memoria insuficiente");
            }
            avanzar(l);
        } else {
            break;
        }
    }

    if (char_at(l) == '.') {
        avanzar(l);
        if (!es_digito(char_at(l))) {
            buf_free(&buf);
            return error_token(l, start_line, start_col, "numero mal formado");
        }
        if (!buf_push(&buf, '.')) {
            buf_free(&buf);
            return error_token(l, start_line, start_col, "memoria insuficiente");
        }
        while (es_digito(char_at(l))) {
            if (!buf_push(&buf, (char)char_at(l))) {
                buf_free(&buf);
                return error_token(l, start_line, start_col, "memoria insuficiente");
            }
            avanzar(l);
        }
    }

    Token tok = token_create(TOK_NUMBER, buf.data, start_line, start_col);
    buf_free(&buf);
    return tok;
}

static Token token_simple(Lexer *l, TokenKind kind, const char *lexema, int line, int col) {
    avanzar(l);
    return token_create(kind, lexema, line, col);
}

Token lexer_siguiente(Lexer *l) {
    while (1) {
        int c = char_at(l);
        if (c == -1) return token_create(TOK_EOF, "", l->line, l->col);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f' || c == '\n') {
            avanzar(l);
            continue;
        }

        int start_line = l->line;
        int start_col = l->col;

        if (es_letra(c)) return leer_identificador(l);
        if (es_digito(c)) return leer_numero(l);

        switch (c) {
            case '+': return token_simple(l, TOK_PLUS, "+", start_line, start_col);
            case '-': return token_simple(l, TOK_MINUS, "-", start_line, start_col);
            case '*': return token_simple(l, TOK_MUL, "*", start_line, start_col);
            case '/': return token_simple(l, TOK_DIV, "/", start_line, start_col);
            case '^': return token_simple(l, TOK_EXP, "^", start_line, start_col);
            case '(': return token_simple(l, TOK_LPAR, "(", start_line, start_col);
            case ')': return token_simple(l, TOK_RPAR, ")", start_line, start_col);
            default: break;
        }

        avanzar(l);
        char err[64];
        snprintf(err, sizeof(err), "caracter '%c' no valido", (char)c);
        return error_token(l, start_line, start_col, err);
    }
}
