#include "consola_expr.h"
#include <locale.h>
#include <stdio.h>
#include <string.h>

static const char *nombre_programa(const char *ruta) {
    const char *nombre = ruta;
    for (const char *p = ruta; *p; p++) {
        if (*p == '\\' || *p == '/') {
            nombre = p + 1;
        }
    }
    return nombre;
}

static void mostrar_uso(FILE *flujo, const char *programa) {
    const char *nombre = nombre_programa(programa);
    fprintf(flujo,
            "Uso: %s\n"
            "     %s --tokens archivo.txt\n",
            nombre, nombre);
}

int main(int argc, char **argv) {
    if (!setlocale(LC_NUMERIC, "C")) {
        fputs("No se pudo configurar el separador decimal.\n", stderr);
        return 1;
    }

    int opt_tokens = 0;
    const char *archivo = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0) {
            opt_tokens = 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            mostrar_uso(stdout, argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            mostrar_uso(stderr, argv[0]);
            return 1;
        } else if (!archivo) {
            archivo = argv[i];
        } else {
            mostrar_uso(stderr, argv[0]);
            return 1;
        }
    }

    if (opt_tokens) {
        if (!archivo) {
            mostrar_uso(stderr, argv[0]);
            return 1;
        }
        return consola_expr_tokens_archivo(archivo, stdout, stderr);
    }

    if (archivo) {
        mostrar_uso(stderr, argv[0]);
        return 1;
    }

    return consola_expr_ejecutar(stdin, stdout, stderr);
}
