#define _POSIX_C_SOURCE 200809L
#include "ll1.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* GNU ld intercepta solo las llamadas de los módulos enlazados. */
void *__real_malloc(size_t);
void *__real_realloc(void *, size_t);
void __real_free(void *);
static void *vivos[32768];
static size_t n_vivos, llamadas, fallar;
static void registrar(void *p) {
    if (p) { assert(n_vivos < sizeof(vivos)/sizeof(vivos[0])); vivos[n_vivos++] = p; }
}
static size_t buscar(void *p) {
    for (size_t i = 0; i < n_vivos; i++) if (vivos[i] == p) return i;
    return n_vivos;
}
void *__wrap_malloc(size_t n) {
    if (++llamadas == fallar) return NULL;
    void *p = __real_malloc(n);
    registrar(p);
    return p;
}
void *__wrap_realloc(void *p, size_t n) {
    size_t i = p ? buscar(p) : n_vivos;
    if (p) assert(i < n_vivos);
    if (++llamadas == fallar) return NULL;
    void *q = __real_realloc(p, n);
    if (q) { if (p) vivos[i] = q; else registrar(q); }
    return q;
}
void __wrap_free(void *p) {
    if (p) {
        size_t i = buscar(p);
        assert(i < n_vivos);
        vivos[i] = vivos[--n_vivos];
    }
    __real_free(p);
}

#define main ll1_cli_main
#include "../src/main_ll1.c"
#undef main

static const char *grammar =
    "<S> -> <A> [ \"x\" ] { ( \"a\" | \"b\" ) } | ID\n"
    "<A> -> ID | ID \"x\"\n";

static size_t ejecutar_etapa(int etapa, size_t punto) {
    LL1Grammar g;
    LL1ConflictList c;
    int r;
    assert(n_vivos == 0);
    fallar = etapa == 0 ? punto : 0; llamadas = 0;
    r = ll1_grammar_parse(&g, grammar);
    size_t total = llamadas;
    if (etapa == 0 && punto) assert(r == -1 && llamadas >= punto);
    if (r != 1) { ll1_grammar_free(&g); assert(n_vivos == 0); return total; }
    fallar = etapa == 1 ? punto : 0; llamadas = 0;
    r = ll1_grammar_calculate(&g);
    if (etapa == 1) {
        total = llamadas;
        if (punto) assert(!r && llamadas >= punto);
    }
    if (!r) {
        LL1ConflictList invalida;
        assert(!ll1_grammar_find_conflicts(&g, &invalida));
        ll1_conflicts_free(&invalida);
        ll1_grammar_free(&g); assert(n_vivos == 0); return total;
    }
    fallar = etapa == 2 ? punto : 0; llamadas = 0;
    r = ll1_grammar_find_conflicts(&g, &c);
    if (etapa == 2) {
        total = llamadas;
        if (punto) assert(!r && llamadas >= punto);
    }
    if (r) { assert(c.count > 0); ll1_conflicts_free(&c); }
    ll1_grammar_free(&g);
    assert(n_vivos == 0);
    return total;
}

static void prueba_set(void) {
    StringSet a, b, inter;
    set_init(&a); set_init(&b);
    fallar = 0; llamadas = 0;
    assert(set_add(&a, "x") == SET_CAMBIO);
    assert(set_add(&a, "x") == SET_SIN_CAMBIOS);
    assert(set_add(&b, "y") == SET_CAMBIO);
    fallar = llamadas + 1;
    assert(set_add(&a, "z") == SET_ERROR);
    assert(a.count == 1 && set_contains(&a, "x"));
    fallar = llamadas + 1;
    assert(set_union(&a, &b) == SET_ERROR);
    fallar = 0;
    assert(set_union(&a, &b) == SET_CAMBIO);
    fallar = llamadas + 1;
    assert(set_intersect(&inter, &a, &b) == SET_ERROR);
    set_free(&inter); set_free(&a); set_free(&b);
    assert(n_vivos == 0);
    set_init(&a);
    fallar = 0;
    for (int i = 0; i < 8; i++) {
        char nombre[16];
        snprintf(nombre, sizeof(nombre), "v%d", i);
        assert(set_add(&a, nombre) == SET_CAMBIO);
    }
    fallar = llamadas + 1; /* Fallo de realloc con elementos existentes. */
    assert(set_add(&a, "otro") == SET_ERROR);
    assert(a.count == 8 && set_contains(&a, "v0") && set_contains(&a, "v7"));
    set_free(&a);
    assert(n_vivos == 0);
}

static size_t prueba_parse_grande(size_t punto) {
    char texto[8192] = "<S> -> ID ID ID ID ID ID ID ID ID ID { ( \"a\" | \"b\" ) } | \"c\" | \"d\" | \"e\" | \"f\" | \"g\" | \"h\" | \"i\"\n";
    for (int i = 0; i < 25; i++) {
        char linea[80];
        snprintf(linea, sizeof(linea), "<R%d> -> ID | NUMBER\n", i);
        assert(strlen(texto) + strlen(linea) < sizeof(texto));
        strcat(texto, linea);
    }
    LL1Grammar g;
    llamadas = 0; fallar = punto;
    int r = ll1_grammar_parse(&g, texto);
    size_t total = llamadas;
    if (punto) assert(r == -1 && total >= punto);
    else assert(r == 1);
    ll1_grammar_free(&g);
    assert(n_vivos == 0);
    return total;
}

static size_t prueba_cli(size_t punto, FILE *out, FILE *err, const char *nombre, int esperado) {
    char *argv[] = {"toy_ll1", (char *)nombre, "--sets", NULL};
    assert(n_vivos == 0);
    fflush(stdout); fflush(stderr);
    assert(ftruncate(fileno(out), 0) == 0 && ftruncate(fileno(err), 0) == 0);
    rewind(out); rewind(err);
    llamadas = 0; fallar = punto;
    int code = ll1_cli_main(3, argv);
    size_t total = llamadas;
    fflush(stdout); fflush(stderr);
    assert(n_vivos == 0);
    if (punto) {
        char buffer[512] = {0};
        assert(code == 3 && total >= punto);
        rewind(out);
        assert(fread(buffer, 1, sizeof(buffer)-1, out) == 0);
        rewind(err);
        size_t n = fread(buffer, 1, sizeof(buffer)-1, err);
        buffer[n] = '\0';
        assert(strstr(buffer, "memoria insuficiente"));
    } else assert(code == esperado);
    return total;
}

int main(void) {
    prueba_set();
    size_t counts[3];
    for (int etapa = 0; etapa < 3; etapa++) {
        counts[etapa] = ejecutar_etapa(etapa, 0);
        for (size_t i = 1; i <= counts[etapa]; i++) ejecutar_etapa(etapa, i);
    }
    size_t grande = prueba_parse_grande(0);
    for (size_t i = 1; i <= grande; i++) prueba_parse_grande(i);
    int oldout = dup(STDOUT_FILENO), olderr = dup(STDERR_FILENO);
    FILE *out = tmpfile(), *err = tmpfile();
    assert(oldout >= 0 && olderr >= 0 && out && err);
    assert(dup2(fileno(out), STDOUT_FILENO) >= 0 && dup2(fileno(err), STDERR_FILENO) >= 0);
    const char *con = "tests/ll1_memoria_gramatica.txt";
    const char *sin = "tests/ll1_memoria_sin_conflictos.txt";
    size_t cli_count = prueba_cli(0, out, err, con, 1);
    for (size_t i = 1; i <= cli_count; i++) prueba_cli(i, out, err, con, 1);
    size_t cli_sin = prueba_cli(0, out, err, sin, 0);
    for (size_t i = 1; i <= cli_sin; i++) prueba_cli(i, out, err, sin, 0);
    fflush(stdout); fflush(stderr);
    assert(dup2(oldout, STDOUT_FILENO) >= 0 && dup2(olderr, STDERR_FILENO) >= 0);
    close(oldout); close(olderr); fclose(out); fclose(err);
    printf("OK fallos LL(1): parse=%zu parse grande=%zu cálculo=%zu conflictos=%zu CLI conflictos=%zu CLI válida=%zu\n", counts[0], grande, counts[1], counts[2], cli_count, cli_sin);
    return 0;
}
