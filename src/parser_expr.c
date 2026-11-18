#include "parser_expr.h"
#include "expr_limites.h"
#include <stdio.h>

static void error(ParserExpr *p, const char *esperado) {
    p->has_error = 1;
    if (p->la.kind == TOK_ERROR) {
        snprintf(p->error_msg, sizeof(p->error_msg),
                 "Línea %d, columna %d: error léxico: %s",
                 p->la.line, p->la.col, p->la.lexeme ? p->la.lexeme : "memoria insuficiente");
    } else {
        const char *encontrado = p->la.lexeme && p->la.lexeme[0]
                                     ? p->la.lexeme : token_kind_to_str(p->la.kind);
        snprintf(p->error_msg, sizeof(p->error_msg),
                 "Línea %d, columna %d: se esperaba %s, se encontró %s (%s)",
                 p->la.line, p->la.col, esperado, encontrado,
                 token_kind_to_str(p->la.kind));
    }
    longjmp(p->jmp_env, 1);
}

static void comer(ParserExpr *p, TokenKind kind) {
    if (p->la.kind != kind) error(p, token_kind_to_str(kind));
    token_free(&p->la);
    p->la = lexer_siguiente(&p->lexer);
}

static ASTExpr *expr(ParserExpr *p);
static ASTExpr *termino(ParserExpr *p);
static ASTExpr *unario(ParserExpr *p);
static ASTExpr *potencia(ParserExpr *p);
static ASTExpr *primario(ParserExpr *p);

static int es_funcion(TokenKind k) {
    return k == TOK_SIN || k == TOK_SEN || k == TOK_COS || k == TOK_TAN;
}

/* <Funcion>: los cuatro nombres se conservan para la futura evaluación. */
static const char *funcion(ParserExpr *p) {
    if (!es_funcion(p->la.kind)) error(p, "función trigonométrica");
    const char *nombre = arena_strdup(&p->arena, p->la.lexeme);
    comer(p, p->la.kind);
    return nombre;
}

/* <Primario> */
static ASTExpr *primario(ParserExpr *p) {
    if (p->la.kind == TOK_NUMBER || p->la.kind == TOK_ID) {
        TokenKind kind = p->la.kind;
        ASTExpr *nodo = kind == TOK_NUMBER
                            ? ast_expr_number(&p->arena, p->la.lexeme)
                            : ast_expr_id(&p->arena, p->la.lexeme);
        comer(p, kind);
        return nodo;
    }
    if (p->la.kind == TOK_LPAR) {
        comer(p, TOK_LPAR);
        ASTExpr *interior = expr(p);
        comer(p, TOK_RPAR);
        return ast_expr_paren(&p->arena, interior);
    }
    if (es_funcion(p->la.kind)) {
        const char *nombre = funcion(p);
        comer(p, TOK_LPAR);
        ASTExpr *argumento = expr(p);
        comer(p, TOK_RPAR);
        ASTExprList args;
        ast_expr_list_init(&p->arena, &args);
        ast_expr_list_push(&p->arena, &args, argumento);
        return ast_expr_call(&p->arena, nombre, args);
    }
    error(p, "número, identificador, '(' o función");
    return NULL;
}

/* <Potencia>: el exponente vuelve a <Unario>, con asociación derecha. */
static ASTExpr *potencia(ParserExpr *p) {
    ASTExpr *nodo = primario(p);
    if (p->la.kind == TOK_EXP) {
        comer(p, TOK_EXP);
        ASTExpr *derecha = unario(p);
        nodo = ast_expr_binary(&p->arena, token_kind_to_str(TOK_EXP), nodo, derecha);
    }
    return nodo;
}

/* <Unario> */
static ASTExpr *unario(ParserExpr *p) {
    if (++p->profundidad > EXPR_PROFUNDIDAD_MAX) {
        p->has_error = 1;
        snprintf(p->error_msg, sizeof(p->error_msg), "Línea %d, columna %d: profundidad sintáctica máxima (%d) excedida", p->la.line, p->la.col, EXPR_PROFUNDIDAD_MAX);
        longjmp(p->jmp_env, 1);
    }
    ASTExpr *resultado;
    if (p->la.kind == TOK_MINUS) {
        comer(p, TOK_MINUS);
        resultado = ast_expr_unary(&p->arena, EXPR_UNARY_MINUS, unario(p));
    } else {
        resultado = potencia(p);
    }
    p->profundidad--;
    return resultado;
}

/* <Termino> y <Expr> acumulan los operadores por la izquierda. */
static ASTExpr *termino(ParserExpr *p) {
    ASTExpr *nodo = unario(p);
    while (p->la.kind == TOK_MUL || p->la.kind == TOK_DIV) {
        TokenKind op = p->la.kind;
        comer(p, op);
        ASTExpr *derecha = unario(p);
        nodo = ast_expr_binary(&p->arena, token_kind_to_str(op), nodo, derecha);
    }
    return nodo;
}

static ASTExpr *expr(ParserExpr *p) {
    ASTExpr *nodo = termino(p);
    while (p->la.kind == TOK_PLUS || p->la.kind == TOK_MINUS) {
        TokenKind op = p->la.kind;
        comer(p, op);
        ASTExpr *derecha = termino(p);
        nodo = ast_expr_binary(&p->arena, token_kind_to_str(op), nodo, derecha);
    }
    return nodo;
}

void parser_expr_init(ParserExpr *p, const char *texto) {
    lexer_init_matematico(&p->lexer, texto);
    arena_init(&p->arena);
    p->la.kind = TOK_EOF;
    p->la.lexeme = NULL;
    p->la.line = 1;
    p->la.col = 1;
    p->has_error = 0;
    p->profundidad = 0;
    p->error_msg[0] = '\0';
}

/* <Inicio>: además de <Expr>, exige que no quede entrada sin consumir. */
ASTExpr *parser_expr_parse(ParserExpr *p) {
    if (setjmp(p->jmp_env) != 0) {
        arena_set_error_destino(&p->arena, NULL);
        if (p->arena.sin_memoria) {
            p->has_error = 1;
            snprintf(p->error_msg, sizeof(p->error_msg), "memoria insuficiente");
        }
        return NULL;
    }
    arena_set_error_destino(&p->arena, &p->jmp_env);
    p->la = lexer_siguiente(&p->lexer);
    ASTExpr *resultado = expr(p);
    if (p->la.kind != TOK_EOF) error(p, "fin de entrada");
    arena_set_error_destino(&p->arena, NULL);
    return resultado;
}

void parser_expr_destroy(ParserExpr *p) {
    token_free(&p->la);
    arena_free(&p->arena);
    lexer_destroy(&p->lexer);
}
