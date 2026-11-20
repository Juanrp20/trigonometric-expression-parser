#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/wait.h>
#endif

#ifdef _WIN32
#define BINARIO "verificar_ll1.exe"
#define RUTA_GRAMATICA "gramatica\\expresiones_matematicas.txt"
#else
#define BINARIO "./verificar_ll1"
#define RUTA_GRAMATICA "gramatica/expresiones_matematicas.txt"
#endif

#define ARCHIVO_SALIDA "ll1_cli_out.tmp"
#define ARCHIVO_ERRORES "ll1_cli_err.tmp"
#define ARCHIVO_GRAMATICA "ll1_cli_gram.tmp"
#define ARCHIVO_AUSENTE "ll1_cli_inexistente.tmp"

#define MENSAJE_INVALIDA "Error: gramatica vacia o mal formada"

static int leer_archivo(const char *ruta, char *buffer, size_t capacidad) {
    FILE *f = fopen(ruta, "rb");
    if (!f) {
        buffer[0] = '\0';
        return 0;
    }
    size_t n = fread(buffer, 1, capacidad - 1, f);
    int fallo = ferror(f);
    fclose(f);
    if (fallo) {
        buffer[0] = '\0';
        return 0;
    }
    buffer[n] = '\0';
    return 1;
}

static int codigo_de_salida(int estado) {
#ifdef _WIN32
    return estado;
#else
    if (!WIFEXITED(estado)) return -1;
    return WEXITSTATUS(estado);
#endif
}

static int ejecutar(const char *comando, char *salida, size_t cap_salida,
                    char *errores, size_t cap_errores) {
    remove(ARCHIVO_SALIDA);
    remove(ARCHIVO_ERRORES);

    int estado = system(comando);
    if (estado == -1) {
        fprintf(stderr, "Fallo al ejecutar: %s\n", comando);
        return -1;
    }
    int codigo = codigo_de_salida(estado);
    if (codigo < 0) {
        fprintf(stderr, "El proceso no termino normalmente: %s\n", comando);
        return -1;
    }

    if (!leer_archivo(ARCHIVO_SALIDA, salida, cap_salida)) {
        fprintf(stderr, "No se pudo leer %s\n", ARCHIVO_SALIDA);
        return -1;
    }
    if (!leer_archivo(ARCHIVO_ERRORES, errores, cap_errores)) {
        fprintf(stderr, "No se pudo leer %s\n", ARCHIVO_ERRORES);
        return -1;
    }
    remove(ARCHIVO_SALIDA);
    remove(ARCHIVO_ERRORES);
    return codigo;
}

static int escribir_gramatica(const char *contenido) {
    FILE *f = fopen(ARCHIVO_GRAMATICA, "wb");
    if (!f) return 0;
    int correcto = fputs(contenido, f) != EOF;
    if (fclose(f) != 0) correcto = 0;
    return correcto;
}

static int contiene(const char *texto, const char *fragmento) {
    return strstr(texto, fragmento) != NULL;
}

static int informar_fallo(const char *grupo, int codigo,
                          const char *salida, const char *errores) {
    fprintf(stderr, "Fallo CLI LL(1) en %s: codigo %d\n", grupo, codigo);
    fprintf(stderr, "Salida: %s", salida);
    fprintf(stderr, "Errores: %s", errores);
    return 1;
}

static int probar_gramatica_real(void) {
    char comando[512];
    char salida[16384];
    char errores[8192];

    snprintf(comando, sizeof(comando),
             "%s %s --sets > %s 2> %s",
             BINARIO, RUTA_GRAMATICA, ARCHIVO_SALIDA, ARCHIVO_ERRORES);
    int codigo = ejecutar(comando, salida, sizeof(salida), errores, sizeof(errores));
    if (codigo != 0 || !contiene(salida, "ninguno") ||
        !contiene(salida, "PRIMERO(") || !contiene(salida, "SIGUIENTE(")) {
        return informar_fallo("gramatica real", codigo, salida, errores);
    }
    puts("OK CLI LL(1): gramatica real sin conflictos y con conjuntos");
    return 0;
}

static int probar_conflicto(void) {
    char comando[512];
    char salida[16384];
    char errores[8192];

    if (!escribir_gramatica("<S> -> \"a\" | \"a\"\n")) {
        fprintf(stderr, "No se pudo escribir %s\n", ARCHIVO_GRAMATICA);
        return 1;
    }
    snprintf(comando, sizeof(comando), "%s %s > %s 2> %s",
             BINARIO, ARCHIVO_GRAMATICA, ARCHIVO_SALIDA, ARCHIVO_ERRORES);
    int codigo = ejecutar(comando, salida, sizeof(salida), errores, sizeof(errores));
    remove(ARCHIVO_GRAMATICA);

    if (codigo != 1 || !contiene(salida, "Conflicto en")) {
        return informar_fallo("conflicto", codigo, salida, errores);
    }
    puts("OK CLI LL(1): gramatica con conflicto");
    return 0;
}

static int probar_gramaticas_invalidas(void) {
    const char *casos[] = {"", "<S> -> [ ID\n", "<S> -> <N>\n"};
    char comando[512];
    char salida[16384];
    char errores[8192];

    for (size_t i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
        if (!escribir_gramatica(casos[i])) {
            fprintf(stderr, "No se pudo escribir %s\n", ARCHIVO_GRAMATICA);
            return 1;
        }
        snprintf(comando, sizeof(comando), "%s %s > %s 2> %s",
                 BINARIO, ARCHIVO_GRAMATICA, ARCHIVO_SALIDA, ARCHIVO_ERRORES);
        int codigo = ejecutar(comando, salida, sizeof(salida), errores, sizeof(errores));
        remove(ARCHIVO_GRAMATICA);

        if (codigo != 2 || !contiene(errores, MENSAJE_INVALIDA)) {
            return informar_fallo("gramatica invalida", codigo, salida, errores);
        }
    }
    puts("OK CLI LL(1): gramaticas vacias y mal formadas");
    return 0;
}

static int probar_archivo_ausente(void) {
    char comando[512];
    char salida[16384];
    char errores[8192];

    remove(ARCHIVO_AUSENTE);
    snprintf(comando, sizeof(comando), "%s %s > %s 2> %s",
             BINARIO, ARCHIVO_AUSENTE, ARCHIVO_SALIDA, ARCHIVO_ERRORES);
    int codigo = ejecutar(comando, salida, sizeof(salida), errores, sizeof(errores));
    if (codigo != 1) {
        return informar_fallo("archivo inexistente", codigo, salida, errores);
    }
    puts("OK CLI LL(1): archivo inexistente");
    return 0;
}

int main(void) {
    if (probar_gramatica_real()) return 1;
    if (probar_conflicto()) return 1;
    if (probar_gramaticas_invalidas()) return 1;
    if (probar_archivo_ausente()) return 1;
    return 0;
}
