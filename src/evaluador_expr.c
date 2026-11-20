#include "evaluador_expr.h"
#include "expr_limites.h"
#include "token.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    ExprResolverVariable resolver;
    void *contexto;
    char error[256];
} Evaluador;

static int fallo(Evaluador *e, const char *mensaje, const char *detalle) {
    if (detalle) snprintf(e->error, sizeof(e->error), "%s: %s", mensaje, detalle);
    else snprintf(e->error, sizeof(e->error), "%s", mensaje);
    return 0;
}

static int evaluar(Evaluador *e, const ASTExpr *expr, double *valor, size_t profundidad) {
    double izquierda, derecha, resultado;
    if (profundidad > EXPR_PROFUNDIDAD_MAX) return fallo(e, "Profundidad maxima del AST excedida", NULL);
    if (!expr) return fallo(e, "Nodo de expresion ausente", NULL);

    switch (expr->kind) {
        case EXPR_NUMBER: {
            char *fin;
            if (!expr->as.str_val) return fallo(e, "Literal numerico ausente", NULL);
            errno = 0;
            resultado = strtod(expr->as.str_val, &fin);
            if (fin == expr->as.str_val || *fin != '\0')
                return fallo(e, "Literal numerico invalido", expr->as.str_val);
            if (errno == ERANGE || !isfinite(resultado))
                return fallo(e, "Literal numerico fuera de rango", expr->as.str_val);
            *valor = resultado;
            return 1;
        }
        case EXPR_ID:
            if (!expr->as.str_val) return fallo(e, "Nombre de variable ausente", NULL);
            if (!e->resolver || !e->resolver(expr->as.str_val, e->contexto, &resultado))
                return fallo(e, "Variable sin valor", expr->as.str_val);
            if (!isfinite(resultado)) return fallo(e, "Valor no finito de variable", expr->as.str_val);
            *valor = resultado;
            return 1;
        case EXPR_PAREN:
            return evaluar(e, expr->as.unary_arg, valor, profundidad + 1);
        case EXPR_UNARY_MINUS:
            if (!evaluar(e, expr->as.unary_arg, &resultado, profundidad + 1)) return 0;
            *valor = -resultado;
            return 1;
        case EXPR_BINARY: {
            const char *op = expr->as.binary.op_str;
            if (!op) return fallo(e, "Operador ausente", NULL);
            if (strcmp(op, token_kind_to_str(TOK_PLUS)) != 0 &&
                strcmp(op, token_kind_to_str(TOK_MINUS)) != 0 &&
                strcmp(op, token_kind_to_str(TOK_MUL)) != 0 &&
                strcmp(op, token_kind_to_str(TOK_DIV)) != 0 &&
                strcmp(op, token_kind_to_str(TOK_EXP)) != 0)
                return fallo(e, "Operador no admitido", op);
            if (!evaluar(e, expr->as.binary.left, &izquierda, profundidad + 1) ||
                !evaluar(e, expr->as.binary.right, &derecha, profundidad + 1)) return 0;
            if (strcmp(op, token_kind_to_str(TOK_PLUS)) == 0) resultado = izquierda + derecha;
            else if (strcmp(op, token_kind_to_str(TOK_MINUS)) == 0) resultado = izquierda - derecha;
            else if (strcmp(op, token_kind_to_str(TOK_MUL)) == 0) resultado = izquierda * derecha;
            else if (strcmp(op, token_kind_to_str(TOK_DIV)) == 0) {
                if (derecha == 0.0) return fallo(e, "Division entre cero", NULL);
                resultado = izquierda / derecha;
            } else {
                if (izquierda == 0.0 && derecha < 0.0)
                    return fallo(e, "Potencia indefinida: cero con exponente negativo", NULL);
                errno = 0;
                resultado = pow(izquierda, derecha);
                if (errno == EDOM) return fallo(e, "Potencia fuera del dominio real", NULL);
                if (errno == ERANGE) return fallo(e, "Potencia fuera de rango", NULL);
            }
            if (!isfinite(resultado)) return fallo(e, "Resultado no finito o fuera de rango", NULL);
            *valor = resultado;
            return 1;
        }
        case EXPR_CALL: {
            const char *nombre = expr->as.call.callee;
            if (!nombre) return fallo(e, "Nombre de funcion ausente", NULL);
            if (strcmp(nombre, "sin") != 0 && strcmp(nombre, "sen") != 0 &&
                strcmp(nombre, "cos") != 0 && strcmp(nombre, "tan") != 0)
                return fallo(e, "Funcion no admitida", nombre);
            if (expr->as.call.args.count != 1 || !expr->as.call.args.items)
                return fallo(e, "La funcion requiere un argumento", nombre);
            if (!evaluar(e, expr->as.call.args.items[0], &izquierda, profundidad + 1)) return 0;
            errno = 0;
            if (strcmp(nombre, "sin") == 0 || strcmp(nombre, "sen") == 0) resultado = sin(izquierda);
            else if (strcmp(nombre, "cos") == 0) resultado = cos(izquierda);
            else resultado = tan(izquierda);
            if (errno == EDOM || errno == ERANGE || !isfinite(resultado))
                return fallo(e, "Resultado trigonometrico invalido o fuera de rango", nombre);
            *valor = resultado;
            return 1;
        }
        default:
            return fallo(e, "Tipo de nodo no admitido", NULL);
    }
}

ExprResultado evaluador_expr(const ASTExpr *expr, ExprResolverVariable resolver, void *contexto) {
    Evaluador e = {resolver, contexto, ""};
    ExprResultado r = {0, 0.0, ""};
    double valor;
    if (evaluar(&e, expr, &valor, 1)) {
        r.exito = 1;
        r.valor = valor;
    } else {
        snprintf(r.error, sizeof(r.error), "%s", e.error);
    }
    return r;
}
