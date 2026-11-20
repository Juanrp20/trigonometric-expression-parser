#ifndef EXPR_PARSER_EXPR_H
#define EXPR_PARSER_EXPR_H

#include "ast.h"
#include "lexer.h"
#include <setjmp.h>

/* El parser posee el lexer, el token de preanalisis y la arena del AST. */
typedef struct {
    Lexer lexer;
    Token la;
    Arena arena;
    jmp_buf jmp_env;
    int has_error;
    char error_msg[512];
    size_t profundidad;
} ParserExpr;

/* El texto debe permanecer valido hasta llamar a parser_expr_destroy. */
void parser_expr_init(ParserExpr *p, const char *texto);
/* Se llama una vez por inicializacion. El AST pertenece al parser. */
ASTExpr *parser_expr_parse(ParserExpr *p);
void parser_expr_destroy(ParserExpr *p);

#endif /* EXPR_PARSER_EXPR_H */
