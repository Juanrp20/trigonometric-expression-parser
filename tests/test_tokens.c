#include "consola_expr.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int codigo;
    char salida[4096];
    char errores[4096];
} Ejecucion;

#define ARCHIVO_TMP "tokens_test.tmp"
#define ARCHIVO_AUSENTE "tokens_test_inexistente.tmp"

static void capturar(FILE *f, char *buffer, size_t capacidad) {
    rewind(f);
    size_t n = fread(buffer, 1, capacidad - 1, f);
    assert(!ferror(f) && feof(f));
    buffer[n] = '\0';
}

static Ejecucion ejecutar_texto(const char *texto) {
    FILE *salida = tmpfile();
    FILE *errores = tmpfile();
    assert(salida && errores);

    Ejecucion r;
    r.codigo = consola_expr_mostrar_tokens(texto, salida, errores);
    capturar(salida, r.salida, sizeof(r.salida));
    capturar(errores, r.errores, sizeof(r.errores));

    fclose(salida);
    fclose(errores);
    return r;
}

static Ejecucion ejecutar_archivo(const char *ruta) {
    FILE *salida = tmpfile();
    FILE *errores = tmpfile();
    assert(salida && errores);

    Ejecucion r;
    r.codigo = consola_expr_tokens_archivo(ruta, salida, errores);
    capturar(salida, r.salida, sizeof(r.salida));
    capturar(errores, r.errores, sizeof(r.errores));

    fclose(salida);
    fclose(errores);
    return r;
}

static void escribir_archivo(const char *contenido) {
    FILE *f = fopen(ARCHIVO_TMP, "wb");
    assert(f);
    assert(fputs(contenido, f) >= 0);
    assert(fclose(f) == 0);
}

static void probar_expresion_trigonometrica(void) {
    Ejecucion r = ejecutar_texto("sin(x)+1.5");
    assert(r.codigo == 0 && !r.errores[0]);
    assert(strcmp(r.salida,
                  "TOK-SIN sin\n"
                  "TOK-LPAR (\n"
                  "TOK-ID x\n"
                  "TOK-RPAR )\n"
                  "TOK-PLUS +\n"
                  "TOK-NUMBER 1.5\n"
                  "TOK-EOF\n") == 0);
    puts("OK tokens: expresion trigonometrica");
}

static void probar_signos_negativos(void) {
    Ejecucion r = ejecutar_texto("1--2");
    assert(r.codigo == 0 && !r.errores[0]);
    assert(strcmp(r.salida,
                  "TOK-NUMBER 1\n"
                  "TOK-MINUS -\n"
                  "TOK-MINUS -\n"
                  "TOK-NUMBER 2\n"
                  "TOK-EOF\n") == 0);
    puts("OK tokens: signos negativos consecutivos");
}

static void probar_potencia(void) {
    Ejecucion r = ejecutar_texto("2^3");
    assert(r.codigo == 0 && !r.errores[0]);
    assert(strcmp(r.salida,
                  "TOK-NUMBER 2\n"
                  "TOK-EXP ^\n"
                  "TOK-NUMBER 3\n"
                  "TOK-EOF\n") == 0);
    puts("OK tokens: operador de potencia");
}

static void probar_texto_vacio(void) {
    Ejecucion r = ejecutar_texto("");
    assert(r.codigo == 0 && !r.errores[0]);
    assert(strcmp(r.salida, "TOK-EOF\n") == 0);
    puts("OK tokens: entrada vacia");
}

static void probar_error_lexico(void) {
    Ejecucion r = ejecutar_texto("sin(1,2)");
    assert(r.codigo == 2);
    assert(strstr(r.salida, "TOK-SIN sin\n") != NULL);
    assert(strstr(r.salida, "TOK-NUMBER 1\n") != NULL);
    assert(!strstr(r.salida, "TOK-EOF"));
    assert(strstr(r.errores, "Linea 1, columna 6") != NULL);
    assert(strstr(r.errores, "caracter ',' no valido") != NULL);
    puts("OK tokens: error lexico");
}

static void probar_archivo(void) {
    escribir_archivo("cos(y)\n");
    Ejecucion r = ejecutar_archivo(ARCHIVO_TMP);
    remove(ARCHIVO_TMP);
    assert(r.codigo == 0 && !r.errores[0]);
    assert(strcmp(r.salida,
                  "TOK-COS cos\n"
                  "TOK-LPAR (\n"
                  "TOK-ID y\n"
                  "TOK-RPAR )\n"
                  "TOK-EOF\n") == 0);
    puts("OK tokens: lectura de archivo");
}

static void probar_archivo_ausente(void) {
    remove(ARCHIVO_AUSENTE);
    Ejecucion r = ejecutar_archivo(ARCHIVO_AUSENTE);
    assert(r.codigo == 1 && !r.salida[0]);
    assert(strstr(r.errores, "Error al abrir archivo") != NULL);
    puts("OK tokens: archivo ausente");
}

static void probar_byte_nulo(void) {
    FILE *f = fopen(ARCHIVO_TMP, "wb");
    assert(f);
    assert(fwrite("x\0y", 1, 3, f) == 3);
    assert(fclose(f) == 0);

    Ejecucion r = ejecutar_archivo(ARCHIVO_TMP);
    remove(ARCHIVO_TMP);
    assert(r.codigo == 1 && !r.salida[0]);
    assert(strstr(r.errores, "byte nulo") != NULL);
    puts("OK tokens: byte nulo rechazado");
}

int main(void) {
    probar_expresion_trigonometrica();
    probar_signos_negativos();
    probar_potencia();
    probar_texto_vacio();
    probar_error_lexico();
    probar_archivo();
    probar_archivo_ausente();
    probar_byte_nulo();
    return 0;
}
