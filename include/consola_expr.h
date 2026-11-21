#ifndef EXPR_CONSOLA_EXPR_H
#define EXPR_CONSOLA_EXPR_H

#include <stdio.h>

/* El llamador debe configurar LC_NUMERIC como "C" antes de ejecutar. */
int consola_expr_ejecutar(FILE *entrada, FILE *salida, FILE *errores);

/* Modo --tokens: solo analisis lexico, sin parser ni evaluacion.
   Devuelve 0 por exito, 1 por error de archivo/memoria y 2 por error lexico. */
int consola_expr_mostrar_tokens(const char *texto, FILE *salida, FILE *errores);
int consola_expr_tokens_archivo(const char *ruta, FILE *salida, FILE *errores);

#endif /* EXPR_CONSOLA_EXPR_H */
