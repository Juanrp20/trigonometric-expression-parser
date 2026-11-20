#ifndef EXPR_EVALUADOR_EXPR_H
#define EXPR_EVALUADOR_EXPR_H

#include "ast.h"

/* Retorna distinto de cero y escribe el valor si la variable existe. */
typedef int (*ExprResolverVariable)(const char *nombre, void *contexto, double *valor);

typedef struct {
    int exito;
    double valor;             /* Solo es valido cuando exito es distinto de cero. */
    char error[256];
} ExprResultado;

/* No modifica ni posee el AST. El resolver y el contexto pueden ser NULL. */
ExprResultado evaluador_expr(const ASTExpr *expr, ExprResolverVariable resolver, void *contexto);

#endif /* EXPR_EVALUADOR_EXPR_H */
