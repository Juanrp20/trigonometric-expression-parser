#include "consola_expr.h"
#include <assert.h>
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { int codigo; char salida[16384]; char errores[16384]; } Ejecucion;

static void capturar(FILE *f, char *buffer, size_t capacidad) {
    rewind(f);
    size_t n = fread(buffer, 1, capacidad - 1, f);
    assert(!ferror(f) && feof(f));
    buffer[n] = '\0';
}

static Ejecucion ejecutar(const char *input) {
    FILE *in = tmpfile(), *out = tmpfile(), *err = tmpfile();
    assert(in && out && err);
    assert(fputs(input, in) >= 0);
    rewind(in);
    Ejecucion r;
    r.codigo = consola_expr_ejecutar(in, out, err);
    capturar(out, r.salida, sizeof(r.salida));
    capturar(err, r.errores, sizeof(r.errores));
    fclose(in); fclose(out); fclose(err);
    return r;
}

static size_t contar(const char *texto, const char *subcadena) {
    size_t n = 0, len = strlen(subcadena);
    while ((texto = strstr(texto, subcadena)) != NULL) { n++; texto += len; }
    return n;
}

static void valido(const char *entrada, const char *resultado) {
    Ejecucion r = ejecutar(entrada);
    if (r.codigo || !strstr(r.salida, resultado))
        fprintf(stderr, "Entrada: %s\nSalida: %s\nError: %s\n", entrada, r.salida, r.errores);
    assert(r.codigo == 0 && strstr(r.salida, resultado) && !r.errores[0]);
}

static void valido_cercano(const char *entrada, double esperado) {
    Ejecucion r = ejecutar(entrada);
    const char *inicio = strstr(r.salida, "Resultado: ");
    assert(r.codigo == 0 && !r.errores[0] && inicio);
    char *fin;
    double valor = strtod(inicio + strlen("Resultado: "), &fin);
    assert(fin != inicio + strlen("Resultado: ") && *fin == '\n');
    assert(fabs(valor - esperado) <= 1e-12 * fmax(1.0, fabs(esperado)));
}

int main(void) {
    assert(setlocale(LC_NUMERIC, "C"));
    Ejecucion r = ejecutar("2+3*4\n");
    assert(r.codigo == 0 && strstr(r.salida, "Resultado: 14") && !strstr(r.salida, "Valor de"));
    valido("sin(0)+cos(0)\n", "Resultado: 1\n");
    valido("sin(x)+cos(y)\n0\n0\n", "Resultado: 1\n");
    r = ejecutar("x+x\n3\n");
    assert(r.codigo == 0 && strstr(r.salida, "Resultado: 6\n") && contar(r.salida, "Valor de x:") == 1);
    valido("x^2+2*x+1\n3\n", "Resultado: 16\n");
    valido_cercano("cos(x)^2+sen(x)^2\n1\n", 1.0);
    valido_cercano("sin(cos(x))+tan(y)/cos(x)\n0\n0\n", sin(1.0));
    r = ejecutar("x+sin()\n3\n");
    assert(r.codigo == 2 && strstr(r.errores, "Línea 1, columna") && !strstr(r.salida, "Valor de"));
    r = ejecutar("x\n\nabc\n1.2junk\nnan\ninf\n1e9999\n -2.5 \n");
    assert(r.codigo == 0 && contar(r.errores, "Valor inválido") == 6 && strstr(r.salida, "Resultado: -2.5\n"));
    r = ejecutar("x+y\n3\n");
    assert(r.codigo == 3 && strstr(r.errores, "Fin de entrada") && contar(r.salida, "Valor de y:") == 1);
    r = ejecutar("_variable_larga_123+_variable_larga_123+X+x\n2\n3\n4\n");
    assert(r.codigo == 0 && strstr(r.salida, "Resultado: 11\n"));
    assert(contar(r.salida, "Valor de _variable_larga_123:") == 1);
    assert(strstr(r.salida, "Valor de X:") && strstr(r.salida, "Valor de x:"));
    assert(strstr(r.salida, "Valor de X:") < strstr(r.salida, "Valor de x:"));
    r = ejecutar("sin(y)+x+y+sin(x)\n0\n0\n");
    assert(r.codigo == 0 && contar(r.salida, "Valor de y:") == 1 && contar(r.salida, "Valor de x:") == 1);
    assert(strstr(r.salida, "Valor de y:") < strstr(r.salida, "Valor de x:"));
    r = ejecutar("1/0\n");
    assert(r.codigo == 4 && strstr(r.errores, "División entre cero"));
    r = ejecutar("0^-1\n");
    assert(r.codigo == 4 && strstr(r.errores, "Potencia indefinida"));
    r = ejecutar("(-1)^0.5\n");
    assert(r.codigo == 4 && strstr(r.errores, "dominio real"));
    r = ejecutar("10^1000\n");
    assert(r.codigo == 4 && strstr(r.errores, "rango"));
    r = ejecutar("");
    assert(r.codigo == 3 && strstr(r.errores, "Fin de entrada"));
    valido("2+3*4", "Resultado: 14\n");
    valido("x\n3.5", "Resultado: 3.5\n");
    {
        char nombre[601], entrada[1300];
        nombre[0] = '_';
        memset(nombre + 1, 'a', sizeof(nombre) - 2);
        nombre[sizeof(nombre) - 1] = '\0';
        snprintf(entrada, sizeof(entrada), "%s+%s\n2\n", nombre, nombre);
        r = ejecutar(entrada);
        assert(r.codigo == 0 && strstr(r.salida, "Resultado: 4\n"));
        assert(contar(r.salida, "Valor de ") == 1);
    }
    {
        char largo[4096];
        memset(largo, ' ', sizeof(largo));
        largo[0] = 'x'; largo[sizeof(largo)-4] = '\n';
        largo[sizeof(largo)-3] = '7'; largo[sizeof(largo)-2] = '\n'; largo[sizeof(largo)-1] = '\0';
        valido(largo, "Resultado: 7\n");
    }
    puts("OK consola: expresiones, variables, reintentos, EOF y errores");
    return 0;
}
