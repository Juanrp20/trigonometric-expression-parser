#ifndef TOY_LEXER_H
#define TOY_LEXER_H

#include "token.h"
#include <stddef.h>

typedef enum {
    LEXER_TOY,
    LEXER_MATEMATICO
} LexerModo;

typedef struct {
    const char *texto;
    size_t i;
    size_t n;
    int line;
    int col;
    int has_error;
    char *error_msg;
    int error_line;
    int error_col;
    LexerModo modo;
} Lexer;

/* lexer_init conserva el modo Toy; el modo matemático se selecciona explícitamente. */
void lexer_init(Lexer *l, const char *texto);
void lexer_init_matematico(Lexer *l, const char *texto);
Token lexer_siguiente(Lexer *l);
void lexer_destroy(Lexer *l);

#endif /* TOY_LEXER_H */
