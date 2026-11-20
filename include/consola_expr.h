#ifndef EXPR_CONSOLA_EXPR_H
#define EXPR_CONSOLA_EXPR_H

#include <stdio.h>

/* El llamador debe configurar LC_NUMERIC como "C" antes de ejecutar. */
int consola_expr_ejecutar(FILE *entrada, FILE *salida, FILE *errores);

#endif /* EXPR_CONSOLA_EXPR_H */
