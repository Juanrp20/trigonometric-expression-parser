#include "parser_expr.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int es_binario(const ASTExpr *e, TokenKind op) {
    return e && e->kind == EXPR_BINARY &&
           strcmp(e->as.binary.op_str, token_kind_to_str(op)) == 0;
}

static void probar_validas(void) {
    const char *casos[] = {
        "sin(3.14 / 2) + cos(0) * 2^2",
        "sin(x) + cos(y)",
        "sin(cos(x)) + tan(y) / cos(x)",
        "cos(x)^2 + sin(x)^2",
        "sin(x)", "cos(x + y)", "tan(x^2 + sin(x))",
        "sen(x)", "2+3*4", "8-3-1", "8/4/2", "2^3^2", "2^-2",
        "-2^2", "---x", "1--2", "((x+(y)))", "sen(tan(cos(sin(x))))",
        "variable1+_valor+main", "  \n 42 \t"
    };
    size_t n = sizeof(casos) / sizeof(casos[0]);
    for (size_t i = 0; i < n; i++) {
        ParserExpr p;
        parser_expr_init(&p, casos[i]);
        ASTExpr *e = parser_expr_parse(&p);
        if (!e) fprintf(stderr, "Expresión válida rechazada: %s: %s\n", casos[i], p.error_msg);
        assert(e && !p.has_error);
        parser_expr_destroy(&p);
    }
    printf("OK parser matemático: %zu expresiones válidas\n", n);
}

static void probar_invalidas(void) {
    const char *casos[] = {
        "", "sin()", "sin(x+)", "cos(x", "2+", "2**3", "sin(1,2)",
        "foo(1)", "x(2)", "2 3", "sin(x) texto", "1 @ 2", "5.",
        ".5", "(x))", "sin x", "tan", "+2", "x%2", "1^", "()"
    };
    size_t n = sizeof(casos) / sizeof(casos[0]);
    for (size_t i = 0; i < n; i++) {
        ParserExpr p;
        parser_expr_init(&p, casos[i]);
        ASTExpr *e = parser_expr_parse(&p);
        if (e) fprintf(stderr, "Expresión inválida aceptada: %s\n", casos[i]);
        assert(!e && p.has_error);
        assert(strstr(p.error_msg, "Línea ") && strstr(p.error_msg, "columna "));
        parser_expr_destroy(&p);
    }
    printf("OK parser matemático: %zu expresiones inválidas\n", n);
}

static void probar_estructura(void) {
    ParserExpr p;
    ASTExpr *e;

    parser_expr_init(&p, "2+3*4");
    e = parser_expr_parse(&p);
    assert(es_binario(e, TOK_PLUS) && es_binario(e->as.binary.right, TOK_MUL));
    parser_expr_destroy(&p);

    parser_expr_init(&p, "8-3-1");
    e = parser_expr_parse(&p);
    assert(es_binario(e, TOK_MINUS) && es_binario(e->as.binary.left, TOK_MINUS));
    parser_expr_destroy(&p);

    parser_expr_init(&p, "8/4/2");
    e = parser_expr_parse(&p);
    assert(es_binario(e, TOK_DIV) && es_binario(e->as.binary.left, TOK_DIV));
    parser_expr_destroy(&p);

    parser_expr_init(&p, "2^3^2");
    e = parser_expr_parse(&p);
    assert(es_binario(e, TOK_EXP) && es_binario(e->as.binary.right, TOK_EXP));
    parser_expr_destroy(&p);

    parser_expr_init(&p, "-2^2");
    e = parser_expr_parse(&p);
    assert(e && e->kind == EXPR_UNARY_MINUS && es_binario(e->as.unary_arg, TOK_EXP));
    parser_expr_destroy(&p);

    parser_expr_init(&p, "2^-2");
    e = parser_expr_parse(&p);
    assert(es_binario(e, TOK_EXP) && e->as.binary.right->kind == EXPR_UNARY_MINUS);
    parser_expr_destroy(&p);

    parser_expr_init(&p, "sen(cos(x))");
    e = parser_expr_parse(&p);
    assert(e && e->kind == EXPR_CALL && strcmp(e->as.call.callee, "sen") == 0);
    assert(e->as.call.args.count == 1 && e->as.call.args.items[0]->kind == EXPR_CALL);
    assert(strcmp(e->as.call.args.items[0]->as.call.callee, "cos") == 0);
    parser_expr_destroy(&p);
    puts("OK parser matemático: precedencia y estructura");
}

static void probar_errores(void) {
    struct Caso { const char *input; const char *mensaje; } casos[] = {
        {"", "Línea 1, columna 1: se esperaba"},
        {"sin()", "columna 5: se esperaba"},
        {"cos(x", "columna 6: se esperaba TOK-RPAR"},
        {"2**3", "columna 3: se esperaba"},
        {"2 3", "columna 3: se esperaba fin de entrada, se encontró 3 (TOK-NUMBER)"},
        {"sin(1,2)", "columna 6: error léxico: carácter ',' no válido"},
        {"x+\n @", "Línea 2, columna 2: error léxico: carácter '@' no válido"},
        {"5.", "columna 1: error léxico: número mal formado"}
    };
    for (size_t i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
        ParserExpr p;
        parser_expr_init(&p, casos[i].input);
        assert(!parser_expr_parse(&p));
        if (!strstr(p.error_msg, casos[i].mensaje)) fprintf(stderr, "%s\n", p.error_msg);
        assert(strstr(p.error_msg, casos[i].mensaje));
        parser_expr_destroy(&p);
    }
    puts("OK parser matemático: diagnósticos");
}

int main(void) {
    probar_validas();
    probar_invalidas();
    probar_estructura();
    probar_errores();
    return 0;
}
