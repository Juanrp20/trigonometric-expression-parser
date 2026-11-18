#include "consola_expr.h"
#include <locale.h>
#include <stdio.h>

int main(void) {
    if (!setlocale(LC_NUMERIC, "C")) {
        fputs("No se pudo configurar el separador decimal.\n", stderr);
        return 1;
    }
    return consola_expr_ejecutar(stdin, stdout, stderr);
}
