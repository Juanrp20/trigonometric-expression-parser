#ifndef TOY_CONSOLA_EXPR_H
#define TOY_CONSOLA_EXPR_H

#include <stdio.h>

/* El llamador debe configurar LC_NUMERIC como "C" antes de ejecutar. */
int consola_expr_ejecutar(FILE *entrada, FILE *salida, FILE *errores);

#endif /* TOY_CONSOLA_EXPR_H */
