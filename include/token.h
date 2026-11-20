#ifndef EXPR_TOKEN_H
#define EXPR_TOKEN_H

#include <stddef.h>

typedef enum {
    TOK_EOF = 0,
    TOK_ERROR,

    /* Operadores y puntuacion */
    TOK_MINUS,
    TOK_PLUS,
    TOK_MUL,
    TOK_DIV,
    TOK_EXP,
    TOK_LPAR,
    TOK_RPAR,

    /* Operandos y funciones */
    TOK_ID,
    TOK_NUMBER,
    TOK_SIN,
    TOK_SEN,
    TOK_COS,
    TOK_TAN
} TokenKind;

typedef struct {
    TokenKind kind;
    char *lexeme;
    int line;
    int col;
} Token;

const char *token_kind_to_str(TokenKind kind);
Token token_create(TokenKind kind, const char *lexeme, int line, int col);
void token_free(Token *tok);

#endif /* EXPR_TOKEN_H */
