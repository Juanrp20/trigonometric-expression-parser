#include "consola_expr.h"
#include "evaluador_expr.h"
#include "expr_limites.h"
#include "lexer.h"
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

/* 1: linea leida, 0: EOF sin datos, -1: error de lectura o memoria. */
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

/* 1: archivo leido completo, 0: error de apertura, lectura o memoria. */
static int leer_archivo_completo(const char *ruta, char **texto, FILE *errores) {
    *texto = NULL;
    FILE *f = fopen(ruta, "rb");
    if (!f) {
        fprintf(errores, "Error al abrir archivo '%s'\n", ruta);
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        fprintf(errores, "Error al leer archivo '%s'\n", ruta);
        return 0;
    }
    long tamano = ftell(f);
    if (tamano < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        fprintf(errores, "Error al leer archivo '%s'\n", ruta);
        return 0;
    }
    if ((uintmax_t)tamano >= SIZE_MAX) {
        fclose(f);
        fprintf(errores, "Error: archivo demasiado grande\n");
        return 0;
    }

    char *buffer = (char *)malloc((size_t)tamano + 1);
    if (!buffer) {
        fclose(f);
        fprintf(errores, "Error: memoria insuficiente\n");
        return 0;
    }
    size_t leidos = fread(buffer, 1, (size_t)tamano, f);
    if (ferror(f)) {
        free(buffer);
        fclose(f);
        fprintf(errores, "Error al leer archivo '%s'\n", ruta);
        return 0;
    }
    fclose(f);
    buffer[leidos] = '\0';

    if (strlen(buffer) != leidos) {
        free(buffer);
        fprintf(errores, "Error: el archivo contiene un byte nulo\n");
        return 0;
    }

    *texto = buffer;
    return 1;
}

int consola_expr_mostrar_tokens(const char *texto, FILE *salida, FILE *errores) {
    Lexer lexer;
    lexer_init(&lexer, texto ? texto : "");
    int estado = 0;

    for (;;) {
        Token tok = lexer_siguiente(&lexer);
        if (tok.kind == TOK_ERROR) {
            fflush(salida);
            fprintf(errores, "Error lexico: Linea %d, columna %d: %s\n",
                    tok.line, tok.col,
                    tok.lexeme ? tok.lexeme : "memoria insuficiente");
            token_free(&tok);
            estado = 2;
            break;
        }
        if (tok.kind == TOK_EOF) {
            fprintf(salida, "%s\n", token_kind_to_str(tok.kind));
            token_free(&tok);
            break;
        }
        fprintf(salida, "%s %s\n", token_kind_to_str(tok.kind),
                tok.lexeme ? tok.lexeme : "");
        token_free(&tok);
    }

    lexer_destroy(&lexer);
    if (ferror(salida)) {
        fprintf(errores, "Error al escribir la salida de tokens\n");
        return 1;
    }
    return estado;
}

int consola_expr_tokens_archivo(const char *ruta, FILE *salida, FILE *errores) {
    if (!ruta) {
        fprintf(errores, "Error: ruta de archivo ausente\n");
        return 1;
    }

    char *texto = NULL;
    if (!leer_archivo_completo(ruta, &texto, errores)) {
        return 1;
    }

    int estado = consola_expr_mostrar_tokens(texto, salida, errores);
    free(texto);
    return estado;
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

static int recoger_variables(Variables *vars, const ASTExpr *expr, size_t profundidad) {
    if (profundidad > EXPR_PROFUNDIDAD_MAX) return -1;
    switch (expr->kind) {
        case EXPR_NUMBER: return 1;
        case EXPR_ID: return agregar_variable(vars, expr->as.str_val);
        case EXPR_PAREN:
        case EXPR_UNARY_MINUS: return recoger_variables(vars, expr->as.unary_arg, profundidad + 1);
        case EXPR_BINARY: {
            int izq = recoger_variables(vars, expr->as.binary.left, profundidad + 1);
            if (izq != 1) return izq;
            return recoger_variables(vars, expr->as.binary.right, profundidad + 1);
        }
        case EXPR_CALL:
            for (size_t i = 0; i < expr->as.call.args.count; i++)
                { int r = recoger_variables(vars, expr->as.call.args.items[i], profundidad + 1); if (r != 1) return r; }
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
    fputs("Expresion: ", salida);
    fflush(salida);
    int lectura = leer_linea(entrada, &texto);
    if (lectura != 1) {
        fprintf(errores, lectura == 0 ? "Fin de entrada al leer la expresion.\n" :
                                      "Error al leer la expresion.\n");
        return lectura == 0 ? 3 : 1;
    }

    ParserExpr parser;
    parser_expr_init(&parser, texto);
    const ASTExpr *ast = parser_expr_parse(&parser);
    if (!ast) {
        fprintf(errores, "Error de expresion: %s\n", parser.error_msg);
        estado = 2;
        goto fin;
    }
    int recogida = recoger_variables(&vars, ast, 1);
    if (recogida != 1) {
        fputs(recogida < 0 ? "Error: profundidad maxima del AST excedida.\n" : "Error al recopilar variables.\n", errores);
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
            fprintf(errores, "Valor invalido para %s; introduzca un numero finito.\n", vars.items[i].nombre);
        }
    }
    ExprResultado resultado = evaluador_expr(ast, resolver_variable, &vars);
    if (!resultado.exito) {
        fprintf(errores, "Error matematico: %s\n", resultado.error);
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
