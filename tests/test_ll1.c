#include "ll1.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *RUTA_GRAMATICA = "gramatica/expresiones_matematicas.txt";

static char *leer_gramatica(const char *ruta) {
    FILE *f = fopen(ruta, "rb");
    if (!f) return NULL;

    size_t capacidad = 4096;
    size_t longitud = 0;
    char *texto = (char *)malloc(capacidad);
    if (!texto) { fclose(f); return NULL; }

    while (1) {
        if (longitud + 1 >= capacidad) {
            if (capacidad > SIZE_MAX / 2) { free(texto); fclose(f); return NULL; }
            capacidad *= 2;
            char *mas = (char *)realloc(texto, capacidad);
            if (!mas) { free(texto); fclose(f); return NULL; }
            texto = mas;
        }
        size_t n = fread(texto + longitud, 1, capacidad - longitud - 1, f);
        longitud += n;
        if (n == 0) break;
    }

    if (ferror(f)) { free(texto); fclose(f); return NULL; }
    texto[longitud] = '\0';
    fclose(f);
    return texto;
}

static Rule *buscar_regla(LL1Grammar *g, const char *lhs) {
    for (size_t i = 0; i < g->rule_count; i++) {
        if (strcmp(g->rules[i].lhs, lhs) == 0) return &g->rules[i];
    }
    return NULL;
}

static void probar_gramatica_matematica(void) {
    char *texto = leer_gramatica(RUTA_GRAMATICA);
    if (!texto) {
        fprintf(stderr, "No se pudo leer la gramatica real: %s\n", RUTA_GRAMATICA);
        exit(1);
    }

    LL1Grammar g;
    LL1ConflictList confs;
    ll1_grammar_init(&g);
    ll1_conflicts_init(&confs);
    assert(ll1_grammar_parse(&g, texto) == 1);
    assert(ll1_grammar_calculate(&g) == 1);
    assert(ll1_grammar_find_conflicts(&g, &confs) == 1);
    assert(confs.count == 0);
    Rule *expr = buscar_regla(&g, "<Expr>");
    Rule *funcion = buscar_regla(&g, "<Funcion>");
    assert(expr && expr->first.count > 0 && expr->follow.count > 0);
    assert(funcion && funcion->first.count == 4);
    ll1_conflicts_free(&confs);
    ll1_grammar_free(&g);
    free(texto);
    puts("OK LL(1): gramatica real sin conflictos");
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
