# Analizador y Evaluador de Expresiones Trigonométricas — Trabajo Práctico 3 de Compiladores

Calculadora de consola en C99 que reconoce expresiones aritméticas y trigonométricas, solicita los valores de sus variables y calcula un resultado `double`. Se desarrolló reutilizando el lexer, el AST, el gestor de memoria y el verificador LL(1) de la Tarea 2 (Toy).

## Objetivos y arquitectura

El trabajo aplica análisis léxico, sintáctico y semántico a una gramática LL(1). El flujo es:

```text
expresión → lexer matemático → parser descendente → AST validado
          → variables únicas → evaluador double → resultado
```

El parser valida toda la entrada antes de solicitar valores. El evaluador recorre el AST mediante un callback para resolver variables; no modifica el árbol. Los nombres se solicitan una vez, en orden de primera aparición, distinguiendo mayúsculas de minúsculas.

El proyecto utiliza C99, `make`, la biblioteca estándar C y `math.h` (`-lm`). No usa bibliotecas externas para analizar expresiones.

## Estructura

```text
include/   interfaces: lexer, tokens, parser, AST, evaluador y consola
src/       implementaciones, verificador LL(1) y ejecutables
    main_expr.c, consola_expr.c     calculadora y entrada de variables
    parser_expr.c, evaluador_expr.c reconocedor y evaluación
    lexer.c, token.c, ast.c, arena.c componentes reutilizados
    parser.c, main.c, ll1.c, set.c, main_ll1.c componentes Toy y LL(1)
gramatica/ expresiones_matematicas.txt y gramáticas Toy
tests/     pruebas Toy, lexer, parser, evaluador, consola y validación final
docs/      enunciado, gramática, trazabilidad, casos e informe técnico
ejemplos/  programas Toy de regresión
```

## Compilación y ejecución

Requisitos: compilador C99, `make` y biblioteca matemática. En Linux:

```sh
make all                    # calculadora_expr, toy_c y toy_ll1
make calculadora_expr       # solo la calculadora
./calculadora_expr
```

En Windows con MinGW se puede usar `mingw32-make`; el ejecutable se denomina `calculadora_expr.exe`. `make debug` compila los tres ejecutables en modo de depuración; en sistemas distintos de Windows añade AddressSanitizer y UBSan.

Una expresión ocupa una línea y puede contener espacios:

```text
Expresión: x+x*2
Valor de x: 3
Resultado: 9
```

Admite `+`, `-`, `*`, `/`, `^`, negación, paréntesis y `sin`, `sen`, `cos`, `tan`. Las funciones usan radianes. La potencia es asociativa por la derecha y tiene mayor precedencia que la negación: `2^3^2 = 512` y `-2^2 = -4`.

Los números **en expresiones** son enteros o decimales con dígitos a ambos lados del punto: `3.14`, no `.5`, `5.` ni `1e3`. Los valores introducidos para variables se interpretan con `strtod`, admiten signo y notación científica y deben ser finitos. El programa fija `LC_NUMERIC` en `C`, por lo que el separador decimal es el punto.

Las expresiones inválidas se informan con línea y columna antes de solicitar valores. Las entradas de variables inválidas se pueden reintentar. Se informan errores de dominio, división por cero y resultados fuera de rango. La salida es 0 si tiene éxito; 1 por error de lectura o recursos; 2 por expresión inválida; 3 por fin de entrada; y 4 por error de evaluación.

## Pruebas y comprobación LL(1)

```sh
make clean
make all
make test
./toy_ll1 gramatica/expresiones_matematicas.txt --sets
python3 tests/validacion_final.py   # opcional: pruebas de extremo a extremo
```

En Linux/macOS `make test` usa AddressSanitizer y UBSan. Si el entorno impide ejecutar LeakSanitizer bajo `ptrace`, se puede ejecutar `ASAN_OPTIONS=detect_leaks=0 make test`; esa ejecución no verifica fugas. No se ha medido el porcentaje de cobertura. La validación adicional requiere Python 3 y usa solo su biblioteca estándar.

Los resultados y la comparación matemática se encuentran en [casos de prueba](docs/casos_prueba_validacion.md); la [matriz de trazabilidad](docs/matriz_trazabilidad.md) vincula requisitos, implementación y pruebas. La [gramática documentada](docs/gramatica_expresiones_matematicas.md) explica el contrato léxico y la verificación. El [informe PDF](docs/informe_tecnico.pdf) y su [fuente editable](docs/informe_tecnico.tex) son entregables académicos.

Para regenerar el PDF a partir de la fuente, con pdfLaTeX instalado, ejecuta
dos veces `pdflatex -output-directory=docs docs/informe_tecnico.tex`; la segunda
pasada actualiza el índice.

## Limitaciones conocidas

- La consola evalúa una sola expresión por ejecución y requiere multiplicación explícita (`2*x`). No se incluyen constantes predefinidas como `pi`.
- La profundidad de anidamiento está limitada por la pila del proceso. Los literales y potencias que producen `ERANGE`, incluido subdesbordamiento, se rechazan. Las funciones cercanas a singularidades no se rechazan mediante tolerancias arbitrarias.
- Algunas asignaciones de memoria heredadas del lexer y del AST no recuperan de forma controlada una falla de reserva; no se ha probado ese escenario. LeakSanitizer no pudo comprobarse en el entorno de validación por la restricción `ptrace`.

## Componentes heredados de Toy

La Tarea 2 conserva su parser, lexer predeterminado, gramática y verificaciones. Se puede ejecutar `./toy_c ejemplos/ok_completo.toy` o verificar la gramática con `./toy_ll1 gramatica/toy_gramatica.txt`. El siguiente ejemplo sigue siendo un programa Toy:

```toy
func main()
  local x := 3.14
  println x
  ret 0
end main()
```

El analizador y evaluador de expresiones es independiente de la sintaxis de programas Toy. La documentación heredada permanece en `docs/` y `ARQUITECTURA_C.md`.
