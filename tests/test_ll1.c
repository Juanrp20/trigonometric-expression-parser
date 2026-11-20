#include "ll1.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *GRAMATICA_MATEMATICA =
    "<Inicio> -> <Expr>\n"
    "<Expr> -> <Termino> { ( \"+\" | \"-\" ) <Termino> }\n"
    "<Termino> -> <Unario> { ( \"*\" | \"/\" ) <Unario> }\n"
    "<Unario> -> \"-\" <Unario> | <Potencia>\n"
    "<Potencia> -> <Primario> [ \"^\" <Unario> ]\n"
    "<Primario> -> NUMBER | ID | \"(\" <Expr> \")\" | <Funcion> \"(\" <Expr> \")\"\n"
    "<Funcion> -> \"sin\" | \"sen\" | \"cos\" | \"tan\"\n";

static Rule *buscar_regla(LL1Grammar *g, const char *lhs) {
    for (size_t i = 0; i < g->rule_count; i++) {
        if (strcmp(g->rules[i].lhs, lhs) == 0) return &g->rules[i];
    }
    return NULL;
}

static void probar_gramatica_matematica(void) {
    LL1Grammar g;
    LL1ConflictList confs;
    ll1_grammar_init(&g);
    ll1_conflicts_init(&confs);
    assert(ll1_grammar_parse(&g, GRAMATICA_MATEMATICA) == 1);
    assert(ll1_grammar_calculate(&g) == 1);
    assert(ll1_grammar_find_conflicts(&g, &confs) == 1);
    assert(confs.count == 0);
    Rule *expr = buscar_regla(&g, "<Expr>");
    Rule *funcion = buscar_regla(&g, "<Funcion>");
    assert(expr && expr->first.count > 0 && expr->follow.count > 0);
    assert(funcion && funcion->first.count == 4);
    ll1_conflicts_free(&confs);
    ll1_grammar_free(&g);
    puts("OK LL(1): gramatica matematica sin conflictos");
}

static void probar_gramaticas_invalidas(void) {
    const char *casos[] = {"", "<S> -> [ ID", "<S> -> <N>"};
    for (size_t i = 0; i < sizeof(casos)/sizeof(casos[0]); i++) {
        LL1Grammar g;
        ll1_grammar_init(&g);
        assert(ll1_grammar_parse(&g, casos[i]) == 0);
        ll1_grammar_free(&g);
    }
    puts("OK LL(1): gramaticas vacias y mal formadas");
}

static void probar_conflicto(void) {
    LL1Grammar g;
    LL1ConflictList confs;
    ll1_grammar_init(&g);
    ll1_conflicts_init(&confs);
    assert(ll1_grammar_parse(&g, "<S> -> \"a\" | \"a\"\n") == 1);
    assert(ll1_grammar_calculate(&g) == 1);
    assert(ll1_grammar_find_conflicts(&g, &confs) == 1);
    assert(confs.count > 0 && confs.items[0].intersection.count > 0);
    ll1_conflicts_free(&confs);
    ll1_grammar_free(&g);
    puts("OK LL(1): deteccion de conflictos");
}

int main(void) {
    probar_gramatica_matematica();
    probar_gramaticas_invalidas();
    probar_conflicto();
    return 0;
}
