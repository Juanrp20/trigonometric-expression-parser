#include "consola_expr.h"
#include "evaluador_expr.h"
#include "parser_expr.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *nombre; /* Pertenece a la arena del parser. */
    double valor;
} Variable;

typedef struct {
    Variable *items;
    size_t count;
    size_t capacity;
} Variables;

/* 1: línea leída, 0: EOF sin datos, -1: error de lectura o memoria. */
static int leer_linea(FILE *entrada, char **linea) {
    size_t capacidad = 128, longitud = 0;
    char *buffer = malloc(capacidad);
    int c;
    if (!buffer) return -1;
    while ((c = fgetc(entrada)) != EOF && c != '\n') {
        if (c == '\0') { free(buffer); return -1; }
        if (longitud + 1 >= capacidad) {
            if (capacidad > SIZE_MAX / 2) { free(buffer); return -1; }
            capacidad *= 2;
            char *nuevo = realloc(buffer, capacidad);
            if (!nuevo) { free(buffer); return -1; }
            buffer = nuevo;
        }
        buffer[longitud++] = (char)c;
    }
    if (ferror(entrada)) { free(buffer); return -1; }
    if (c == EOF && longitud == 0) { free(buffer); return 0; }
    buffer[longitud] = '\0';
    *linea = buffer;
    return 1;
}

static int agregar_variable(Variables *vars, const char *nombre) {
    for (size_t i = 0; i < vars->count; i++)
        if (strcmp(vars->items[i].nombre, nombre) == 0) return 1;
    if (vars->count == vars->capacity) {
        if (vars->capacity > SIZE_MAX / 2 / sizeof(*vars->items)) return 0;
        size_t capacidad = vars->capacity ? vars->capacity * 2 : 8;
        Variable *nuevos = realloc(vars->items, capacidad * sizeof(*nuevos));
        if (!nuevos) return 0;
        vars->items = nuevos;
        vars->capacity = capacidad;
    }
    vars->items[vars->count].nombre = nombre;
    vars->items[vars->count++].valor = 0.0;
    return 1;
}

static int recoger_variables(Variables *vars, const ASTExpr *expr) {
    switch (expr->kind) {
        case EXPR_NUMBER: return 1;
        case EXPR_ID: return agregar_variable(vars, expr->as.str_val);
        case EXPR_PAREN:
        case EXPR_UNARY_MINUS: return recoger_variables(vars, expr->as.unary_arg);
        case EXPR_BINARY:
            return recoger_variables(vars, expr->as.binary.left) &&
                   recoger_variables(vars, expr->as.binary.right);
        case EXPR_CALL:
            for (size_t i = 0; i < expr->as.call.args.count; i++)
                if (!recoger_variables(vars, expr->as.call.args.items[i])) return 0;
            return 1;
        default: return 0;
    }
}

static int resolver_variable(const char *nombre, void *contexto, double *valor) {
    const Variables *vars = contexto;
    for (size_t i = 0; i < vars->count; i++) {
        if (strcmp(vars->items[i].nombre, nombre) == 0) {
            *valor = vars->items[i].valor;
            return 1;
        }
    }
    return 0;
}

static int convertir_valor(const char *linea, double *valor) {
    char *fin;
    errno = 0;
    double numero = strtod(linea, &fin);
    if (fin == linea || errno == ERANGE || !isfinite(numero)) return 0;
    while (isspace((unsigned char)*fin)) fin++;
    if (*fin) return 0;
    *valor = numero;
    return 1;
}

int consola_expr_ejecutar(FILE *entrada, FILE *salida, FILE *errores) {
    char *texto = NULL;
    Variables vars = {NULL, 0, 0};
    int estado = 1;
    fputs("Expresión: ", salida);
    fflush(salida);
    int lectura = leer_linea(entrada, &texto);
    if (lectura != 1) {
        fprintf(errores, lectura == 0 ? "Fin de entrada al leer la expresión.\n" :
                                      "Error al leer la expresión.\n");
        return lectura == 0 ? 3 : 1;
    }

    ParserExpr parser;
    parser_expr_init(&parser, texto);
    const ASTExpr *ast = parser_expr_parse(&parser);
    if (!ast) {
        fprintf(errores, "Error de expresión: %s\n", parser.error_msg);
        estado = 2;
        goto fin;
    }
    if (!recoger_variables(&vars, ast)) {
        fputs("Error al recopilar variables.\n", errores);
        goto fin;
    }
    for (size_t i = 0; i < vars.count; i++) {
        for (;;) {
            char *linea = NULL;
            fprintf(salida, "Valor de %s: ", vars.items[i].nombre);
            fflush(salida);
            lectura = leer_linea(entrada, &linea);
            if (lectura != 1) {
                fprintf(errores, lectura == 0 ? "Fin de entrada al leer el valor de %s.\n" :
                                              "Error al leer el valor de %s.\n", vars.items[i].nombre);
                estado = lectura == 0 ? 3 : 1;
                goto fin;
            }
            int valido = convertir_valor(linea, &vars.items[i].valor);
            free(linea);
            if (valido) break;
            fprintf(errores, "Valor inválido para %s; introduzca un número finito.\n", vars.items[i].nombre);
        }
    }
    ExprResultado resultado = evaluador_expr(ast, resolver_variable, &vars);
    if (!resultado.exito) {
        fprintf(errores, "Error matemático: %s\n", resultado.error);
        estado = 4;
        goto fin;
    }
    fprintf(salida, "Resultado: %.17g\n", resultado.valor);
    estado = 0;
fin:
    free(vars.items);
    parser_expr_destroy(&parser);
    free(texto);
    return estado;
}
