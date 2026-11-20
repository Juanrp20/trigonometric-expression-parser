#include "ll1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static char *read_entire_file(const char *path, int *estado) {
    *estado = 1;
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("Error al abrir archivo");
        return NULL;
    }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NULL; }
    long size = ftell(f);
    if (size < 0 || fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NULL; }
    if ((uintmax_t)size >= SIZE_MAX) { *estado = 3; fclose(f); return NULL; }

    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        *estado = 3;
        fclose(f);
        return NULL;
    }
    size_t read_bytes = fread(buf, 1, size, f);
    if (ferror(f)) { free(buf); fclose(f); return NULL; }
    buf[read_bytes] = '\0';
    fclose(f);
    return buf;
}

static void imprimir_conjunto(const StringSet *s) {
    printf("[");
    for (size_t i = 0; i < s->count; i++) {
        if (i > 0) printf(", ");
        printf("'%s'", s->items[i]);
    }
    printf("]");
}

static int compare_strings(const void *a, const void *b) {
    const char *sa = *(const char **)a;
    const char *sb = *(const char **)b;
    return strcmp(sa, sb);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Uso: %s gramatica.txt [--sets]\n", argv[0]);
        return 1;
    }

    const char *filename = NULL;
    int opt_sets = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--sets") == 0) {
            opt_sets = 1;
        } else if (argv[i][0] != '-') {
            filename = argv[i];
        }
    }

    if (!filename) {
        printf("Uso: %s gramatica.txt [--sets]\n", argv[0]);
        return 1;
    }

    int estado_lectura;
    char *text = read_entire_file(filename, &estado_lectura);
    if (!text) {
        if (estado_lectura == 3) fprintf(stderr, "Error: memoria insuficiente\n");
        else fprintf(stderr, "Error al leer la gramatica\n");
        return estado_lectura;
    }

    LL1Grammar g;
    int parse = ll1_grammar_parse(&g, text);
    if (parse != 1) {
        fprintf(stderr, parse < 0 ? "Error: memoria insuficiente\n" : "Error: gramatica vacia o mal formada\n");
        ll1_grammar_free(&g);
        free(text);
        return parse < 0 ? 3 : 2;
    }
    if (!ll1_grammar_calculate(&g)) {
        fprintf(stderr, "Error: memoria insuficiente\n");
        ll1_grammar_free(&g);
        free(text);
        return 3;
    }

    LL1ConflictList confs;
    if (!ll1_grammar_find_conflicts(&g, &confs)) {
        fprintf(stderr, "Error: memoria insuficiente\n");
        ll1_grammar_free(&g);
        free(text);
        return 3;
    }

    /* Reservar antes de imprimir para no comunicar exito si falla la salida. */
    char **sorted_nt = NULL;
    if (opt_sets) {
        if (g.rule_count <= SIZE_MAX / sizeof(*sorted_nt))
            sorted_nt = (char **)malloc(g.rule_count * sizeof(*sorted_nt));
        if (!sorted_nt) {
            fprintf(stderr, "Error: memoria insuficiente\n");
            ll1_conflicts_free(&confs);
            ll1_grammar_free(&g);
            free(text);
            return 3;
        }
        for (size_t i = 0; i < g.rule_count; i++) sorted_nt[i] = g.rules[i].lhs;
        qsort(sorted_nt, g.rule_count, sizeof(*sorted_nt), compare_strings);
    }

    if (confs.count > 0) {
        for (size_t i = 0; i < confs.count; i++) {
            printf("Conflicto en %s: alternativa %zu y %zu comparten ",
                   confs.items[i].non_terminal,
                   confs.items[i].alt1 + 1,
                   confs.items[i].alt2 + 1);
            imprimir_conjunto(&confs.items[i].intersection);
            printf("\n");
        }
    } else {
        printf("ninguno\n");
    }

    if (opt_sets) {
        for (size_t i = 0; i < g.rule_count; i++) {
            const char *nt = sorted_nt[i];
            if (nt[0] == '<') {
                /* Buscar regla */
                for (size_t r = 0; r < g.rule_count; r++) {
                    if (strcmp(g.rules[r].lhs, nt) == 0) {
                        printf("PRIMERO(%s) = ", nt);
                        imprimir_conjunto(&g.rules[r].first);
                        printf("\n");
                        printf("SIGUIENTE(%s) = ", nt);
                        imprimir_conjunto(&g.rules[r].follow);
                        printf("\n");
                        break;
                    }
                }
            }
        }
        free(sorted_nt);
    }

    int estado = confs.count ? 1 : 0;
    ll1_conflicts_free(&confs);
    ll1_grammar_free(&g);
    free(text);
    return estado;
}
