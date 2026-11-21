#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/wait.h>
#endif

#ifdef _WIN32
#define BINARIO "calculadora_expr.exe"
#else
#define BINARIO "./calculadora_expr"
#endif

#define ARCHIVO_EXPR "tokens_cli_expr.tmp"
#define ARCHIVO_SALIDA "tokens_cli_out.tmp"
#define ARCHIVO_ERRORES "tokens_cli_err.tmp"
#define ARCHIVO_AUSENTE "tokens_cli_inexistente.tmp"

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

static void normalizar_saltos(char *texto) {
    char *destino = texto;
    for (char *origen = texto; *origen; origen++) {
        if (*origen != '\r') {
            *destino++ = *origen;
        }
    }
    *destino = '\0';
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
    normalizar_saltos(salida);
    normalizar_saltos(errores);
    remove(ARCHIVO_SALIDA);
    remove(ARCHIVO_ERRORES);
    return codigo;
}

static int escribir_archivo(const char *ruta, const char *contenido) {
    FILE *f = fopen(ruta, "wb");
    if (!f) return 0;
    int correcto = fputs(contenido, f) >= 0;
    if (fclose(f) != 0) correcto = 0;
    return correcto;
}

static int contiene(const char *texto, const char *fragmento) {
    return strstr(texto, fragmento) != NULL;
}

static int informar_fallo(const char *grupo, int codigo,
                          const char *salida, const char *errores) {
    fprintf(stderr, "Fallo CLI tokens en %s: codigo %d\n", grupo, codigo);
    fprintf(stderr, "Salida: %s", salida);
    fprintf(stderr, "Errores: %s", errores);
    return 1;
}

static int probar_archivo_valido(void) {
    char comando[512];
    char salida[8192];
    char errores[4096];

    if (!escribir_archivo(ARCHIVO_EXPR, "2+3*4\n")) {
        fprintf(stderr, "No se pudo escribir %s\n", ARCHIVO_EXPR);
        return 1;
    }
    snprintf(comando, sizeof(comando), "%s %s --tokens > %s 2> %s",
             BINARIO, ARCHIVO_EXPR, ARCHIVO_SALIDA, ARCHIVO_ERRORES);
    int codigo = ejecutar(comando, salida, sizeof(salida), errores, sizeof(errores));
    remove(ARCHIVO_EXPR);

    if (codigo != 0 || errores[0] != '\0' ||
        strcmp(salida,
               "TOK-NUMBER 2\n"
               "TOK-PLUS +\n"
               "TOK-NUMBER 3\n"
               "TOK-MUL *\n"
               "TOK-NUMBER 4\n"
               "TOK-EOF\n") != 0) {
        return informar_fallo("archivo valido", codigo, salida, errores);
    }
    puts("OK CLI tokens: archivo valido");
    return 0;
}

static int probar_archivo_ausente(void) {
    char comando[512];
    char salida[8192];
    char errores[4096];

    remove(ARCHIVO_AUSENTE);
    snprintf(comando, sizeof(comando), "%s --tokens %s > %s 2> %s",
             BINARIO, ARCHIVO_AUSENTE, ARCHIVO_SALIDA, ARCHIVO_ERRORES);
    int codigo = ejecutar(comando, salida, sizeof(salida), errores, sizeof(errores));

    if (codigo != 1 || !contiene(errores, "Error al abrir archivo")) {
        return informar_fallo("archivo ausente", codigo, salida, errores);
    }
    puts("OK CLI tokens: archivo ausente");
    return 0;
}

static int probar_error_lexico(void) {
    char comando[512];
    char salida[8192];
    char errores[4096];

    if (!escribir_archivo(ARCHIVO_EXPR, "sin(1,2)\n")) {
        fprintf(stderr, "No se pudo escribir %s\n", ARCHIVO_EXPR);
        return 1;
    }
    snprintf(comando, sizeof(comando), "%s --tokens %s > %s 2> %s",
             BINARIO, ARCHIVO_EXPR, ARCHIVO_SALIDA, ARCHIVO_ERRORES);
    int codigo = ejecutar(comando, salida, sizeof(salida), errores, sizeof(errores));
    remove(ARCHIVO_EXPR);

    if (codigo != 2 || !contiene(salida, "TOK-SIN sin") ||
        !contiene(errores, "Error lexico")) {
        return informar_fallo("error lexico", codigo, salida, errores);
    }
    puts("OK CLI tokens: error lexico");
    return 0;
}

static int probar_sin_archivo(void) {
    char comando[512];
    char salida[8192];
    char errores[4096];

    snprintf(comando, sizeof(comando), "%s --tokens > %s 2> %s",
             BINARIO, ARCHIVO_SALIDA, ARCHIVO_ERRORES);
    int codigo = ejecutar(comando, salida, sizeof(salida), errores, sizeof(errores));

    if (codigo != 1 || !contiene(errores, "Uso:")) {
        return informar_fallo("sin archivo", codigo, salida, errores);
    }
    puts("OK CLI tokens: falta el archivo");
    return 0;
}

int main(void) {
    if (probar_archivo_valido()) return 1;
    if (probar_archivo_ausente()) return 1;
    if (probar_error_lexico()) return 1;
    if (probar_sin_archivo()) return 1;
    remove(ARCHIVO_EXPR);
    remove(ARCHIVO_SALIDA);
    remove(ARCHIVO_ERRORES);
    remove(ARCHIVO_AUSENTE);
    return 0;
}
