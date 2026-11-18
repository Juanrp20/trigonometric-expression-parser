/* Prueba opcional de Linux/GNU ld: enlazar con --wrap=malloc y --wrap=realloc. */
#include "parser_expr.h"
#include "parser.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void *__real_malloc(size_t);
void *__real_realloc(void *, size_t);
static long intento, fallo;
void *__wrap_malloc(size_t n) {
    if (++intento == fallo) return NULL;
    return __real_malloc(n);
}
void *__wrap_realloc(void *p, size_t n) {
    if (++intento == fallo) return NULL;
    return __real_realloc(p, n);
}
int main(void) {
    for (long i = 1; i <= 120; i++) {
        ParserExpr p;
        intento = 0; fallo = i;
        parser_expr_init(&p, "sin(variable_larga_12345678901234567890)+cos(x^2)+3.14");
        ASTExpr *expr = parser_expr_parse(&p);
        assert(expr || p.has_error);
        if (intento >= i) assert(!expr && p.has_error);
        parser_expr_destroy(&p);
    }
    for (long i = 1; i <= 120; i++) {
        Lexer l;
        Parser p;
        intento = 0; fallo = i;
        lexer_init(&l, "func main() local x := 3.14 + 5 ret x end main()");
        parser_init(&p, &l);
        ASTProgram *programa = parser_parse(&p);
        assert(programa || p.has_error);
        if (intento >= i) assert(!programa && p.has_error);
        parser_destroy(&p);
        lexer_destroy(&l);
    }
    puts("OK fallos de malloc/realloc en lexer y arena de ambos parsers");
    return 0;
}
