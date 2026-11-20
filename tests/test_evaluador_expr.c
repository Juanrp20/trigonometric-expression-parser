#include "evaluador_expr.h"
#include "parser_expr.h"
#include "expr_limites.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct { const char *nombre; double valor; } Variable;
typedef struct { const Variable *variables; size_t count; } Variables;

static int resolver(const char *nombre, void *contexto, double *valor) {
    const Variables *v = contexto;
    for (size_t i = 0; i < v->count; i++) {
        if (strcmp(nombre, v->variables[i].nombre) == 0) {
            *valor = v->variables[i].valor;
            return 1;
        }
    }
    return 0;
}

static ExprResultado calcular(const char *texto, const Variables *vars) {
    ParserExpr p;
    parser_expr_init(&p, texto);
    const ASTExpr *ast = parser_expr_parse(&p);
    if (!ast) fprintf(stderr, "Error al analizar %s: %s\n", texto, p.error_msg);
    assert(ast);
    ExprResultado r = evaluador_expr(ast, vars ? resolver : NULL, (void *)vars);
    parser_expr_destroy(&p);
    return r;
}

static void cercano(const char *texto, const Variables *vars, double esperado) {
    ExprResultado r = calcular(texto, vars);
    if (!r.exito) fprintf(stderr, "Error al evaluar %s: %s\n", texto, r.error);
    assert(r.exito && r.error[0] == '\0');
    assert(fabs(r.valor - esperado) <= 1e-12 * fmax(1.0, fabs(esperado)));
}

static void error_esperado(const char *texto, const Variables *vars, const char *mensaje) {
    ExprResultado r = calcular(texto, vars);
    if (r.exito || !strstr(r.error, mensaje))
        fprintf(stderr, "Error inesperado para %s: %s\n", texto, r.error);
    assert(!r.exito && strstr(r.error, mensaje));
}

static void probar_aritmetica_y_trigonometria(void) {
    struct Caso { const char *texto; double valor; } casos[] = {
        {"2+3*4", 14}, {"8-3-1", 4}, {"8/4/2", 1}, {"2^3^2", 512},
        {"-2^2", -4}, {"2^-2", 0.25}, {"1--2", 3}, {"sin(0)", 0},
        {"sen(0)", 0}, {"cos(0)", 1}, {"tan(0)", 0},
        {"sin(cos(0))", 0.8414709848078965},
        {"tan(1.5707963267948966)", 1.6331239353195370e+16},
        {"sin(3.14/2)+cos(0)*2^2", 4.9999996829318345}
    };
    for (size_t i = 0; i < sizeof(casos)/sizeof(casos[0]); i++)
        cercano(casos[i].texto, NULL, casos[i].valor);
    ExprResultado ejemplo = calcular("sin(3.14/2)+cos(0)*2^2", NULL);
    assert(ejemplo.exito && fabs(ejemplo.valor - 5.0) < 0.000001);
    printf("OK aritmetica y trigonometria: %zu casos (ejemplo = %.15g)\n",
           sizeof(casos)/sizeof(casos[0]), ejemplo.valor);
}

static void probar_variables(void) {
    Variable entradas[] = {{"x", 3.0}, {"y", 0.0}};
    Variables vars = {entradas, 2};
    cercano("x+x", &vars, 6.0);
    entradas[0].valor = 0.0;
    cercano("sin(x)+cos(y)", &vars, 1.0);
    cercano("sin(cos(x))+tan(y)/cos(x)", &vars, 0.8414709848078965);
    entradas[0].valor = 1.0;
    cercano("tan(x^2+sin(x))", &vars, -3.6038035320088055);
    const double valores[] = {-7.0, -1.0, 0.0, 0.5, 3.14, 100.0};
    for (size_t i = 0; i < sizeof(valores)/sizeof(valores[0]); i++) {
        entradas[0].valor = valores[i];
        cercano("cos(x)^2+sin(x)^2", &vars, 1.0);
    }
    {
        ParserExpr p;
        parser_expr_init(&p, "x+x");
        const ASTExpr *ast = parser_expr_parse(&p);
        assert(ast);
        entradas[0].valor = 3.0;
        ExprResultado primero = evaluador_expr(ast, resolver, &vars);
        entradas[0].valor = 4.0;
        ExprResultado segundo = evaluador_expr(ast, resolver, &vars);
        assert(primero.exito && segundo.exito);
        assert(fabs(primero.valor - 6.0) < 1e-12 && fabs(segundo.valor - 8.0) < 1e-12);
        parser_expr_destroy(&p);
    }
    error_esperado("z+1", &vars, "Variable sin valor: z");
    error_esperado("x", NULL, "Variable sin valor: x");
    entradas[0].valor = INFINITY;
    error_esperado("x", &vars, "Valor no finito de variable: x");
    entradas[0].valor = NAN;
    error_esperado("x", &vars, "Valor no finito de variable: x");
    puts("OK variables e identidad trigonometrica");
}

static void probar_errores(void) {
    error_esperado("1/0", NULL, "Division entre cero");
    error_esperado("1/-0", NULL, "Division entre cero");
    error_esperado("0^-1", NULL, "Potencia indefinida");
    error_esperado("(-1)^0.5", NULL, "Potencia fuera del dominio real");
    error_esperado("10^1000", NULL, "Potencia fuera de rango");
    error_esperado("10^308*10", NULL, "Resultado no finito");
    char numero[401];
    memset(numero, '9', sizeof(numero) - 1);
    numero[sizeof(numero) - 1] = '\0';
    error_esperado(numero, NULL, "Literal numerico fuera de rango");
    char pequeno[405] = "0.";
    memset(pequeno + 2, '0', 400);
    pequeno[402] = '1';
    pequeno[403] = '\0';
    error_esperado(pequeno, NULL, "Literal numerico fuera de rango");

    ASTExpr no_admitido = {0};
    no_admitido.kind = (ASTExprKind)999;
    ExprResultado r = evaluador_expr(&no_admitido, NULL, NULL);
    assert(!r.exito && strstr(r.error, "Tipo de nodo no admitido"));
    r = evaluador_expr(NULL, NULL, NULL);
    assert(!r.exito && strstr(r.error, "Nodo de expresion ausente"));
    ASTExpr operacion = {0};
    operacion.kind = EXPR_BINARY;
    operacion.as.binary.op_str = "TOK-MOD";
    r = evaluador_expr(&operacion, NULL, NULL);
    assert(!r.exito && strstr(r.error, "Operador no admitido"));
    ASTExpr llamada = {0};
    llamada.kind = EXPR_CALL;
    llamada.as.call.callee = "otra";
    r = evaluador_expr(&llamada, NULL, NULL);
    assert(!r.exito && strstr(r.error, "Funcion no admitida"));
    llamada.as.call.callee = "sin";
    r = evaluador_expr(&llamada, NULL, NULL);
    assert(!r.exito && strstr(r.error, "requiere un argumento"));
    ASTExpr numero_invalido = {0};
    numero_invalido.kind = EXPR_NUMBER;
    numero_invalido.as.str_val = "12x";
    r = evaluador_expr(&numero_invalido, NULL, NULL);
    assert(!r.exito && strstr(r.error, "Literal numerico invalido"));
    puts("OK errores matematicos y nodos no admitidos");
}

static void probar_arbol_profundo(void) {
    char expresion[EXPR_PROFUNDIDAD_MAX * 2 + 4];
    for (int i = 0; i < EXPR_PROFUNDIDAD_MAX; i++) {
        expresion[2*i] = '1'; expresion[2*i+1] = '+';
    }
    expresion[2*EXPR_PROFUNDIDAD_MAX] = '1';
    expresion[2*EXPR_PROFUNDIDAD_MAX+1] = '\0';
    error_esperado(expresion, NULL, "Profundidad maxima del AST");
    expresion[2*(EXPR_PROFUNDIDAD_MAX-1)-1] = '\0';
    cercano(expresion, NULL, EXPR_PROFUNDIDAD_MAX - 1);
    puts("OK evaluador: limite de profundidad del AST");
}

int main(void) {
    probar_aritmetica_y_trigonometria();
    probar_variables();
    probar_errores();
    probar_arbol_profundo();
    return 0;
}
