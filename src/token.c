#include "token.h"
#include <stdlib.h>
#include <string.h>

const char *token_kind_to_str(TokenKind kind) {
    switch (kind) {
        case TOK_EOF:     return "TOK-EOF";
        case TOK_ERROR:   return "TOK-ERROR";
        case TOK_MINUS:   return "TOK-MINUS";
        case TOK_PLUS:    return "TOK-PLUS";
        case TOK_MUL:     return "TOK-MUL";
        case TOK_DIV:     return "TOK-DIV";
        case TOK_EXP:     return "TOK-EXP";
        case TOK_LPAR:    return "TOK-LPAR";
        case TOK_RPAR:    return "TOK-RPAR";
        case TOK_ID:      return "TOK-ID";
        case TOK_NUMBER:  return "TOK-NUMBER";
        case TOK_SIN:     return "TOK-SIN";
        case TOK_SEN:     return "TOK-SEN";
        case TOK_COS:     return "TOK-COS";
        case TOK_TAN:     return "TOK-TAN";
        default:          return "TOK-UNKNOWN";
    }
}

Token token_create(TokenKind kind, const char *lexeme, int line, int col) {
    Token tok;
    tok.kind = kind;
    tok.line = line;
    tok.col = col;
    if (lexeme) {
        size_t len = strlen(lexeme);
        tok.lexeme = (char *)malloc(len + 1);
        if (tok.lexeme) {
            memcpy(tok.lexeme, lexeme, len + 1);
        }
    } else {
        tok.lexeme = (char *)malloc(1);
        if (tok.lexeme) {
            tok.lexeme[0] = '\0';
        }
    }
    if (!tok.lexeme) tok.kind = TOK_ERROR;
    return tok;
}

void token_free(Token *tok) {
    if (tok && tok->lexeme) {
        free(tok->lexeme);
        tok->lexeme = NULL;
    }
}
