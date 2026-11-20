# Trabajo Práctico 3 — Analizador y Evaluador de Expresiones Trigonométricas

Calculadora de consola escrita en C99. Reconoce expresiones aritméticas y trigonométricas, pide el valor de cada variable y devuelve un resultado `double`.

## Características

- Gramática LL(1), lexer propio y parser descendente recursivo.
- Operadores `+`, `-`, `*`, `/`, `^`, negación y paréntesis.
- Potencia asociativa por derecha y con mayor precedencia que la negación.
- Funciones `sin`, `sen`, `cos` y `tan`, con argumentos en radianes.
- Variables sensibles a mayúsculas. Cada nombre se pide una sola vez, en orden de primera aparición.
- Errores léxicos, sintácticos y matemáticos con códigos de salida distintos.
- Mensajes de ejecución en ASCII, sin dependencia de la página de códigos de Windows.

## Estructura

```text
include/  interfaces del lexer, parser, AST, evaluador, consola y LL(1)
src/      implementaciones y puntos de entrada
  main_expr.c, consola_expr.c       calculadora y variables
  parser_expr.c, evaluador_expr.c   parser matemático y evaluación
  lexer.c, token.c, ast.c, arena.c  componentes del analizador
  main_verificar_ll1.c, ll1.c, set.c verificador de la gramática
gramatica/ gramática EBNF de las expresiones
tests/    cuatro suites C: parser, evaluador, consola y LL(1)
docs/     enunciado, gramática, casos, trazabilidad e informe
Makefile  compilación en Linux
build.bat compilación en Windows con MinGW-w64
```

## Requisitos

- Linux: GCC con soporte C99 y GNU Make.
- Windows: GCC de MinGW-w64, por ejemplo MSYS2 UCRT64. Añadir `C:\msys64\ucrt64\bin` al `PATH`.
- La biblioteca matemática del sistema (`-lm` con GCC).
- Opcional para regenerar el PDF: `pdflatex`.

No se necesitan bibliotecas externas de análisis.

## Compilación

### Linux

```bash
make clean
make all
make test
./calculadora_expr
./verificar_ll1 gramatica/expresiones_matematicas.txt --sets
```

`make debug` compila con símbolos de depuración y `-g`, sin sanitizadores obligatorios.

### Windows con MinGW-w64

Desde `cmd`, en la raíz del repositorio:

```bat
build.bat all
build.bat test
calculadora_expr.exe
verificar_ll1.exe gramatica\expresiones_matematicas.txt --sets
build.bat clean
```

`build.bat test` compila y ejecuta las cuatro suites C. No necesita MSYS shell, `make` ni herramientas GNU adicionales.

## Compilación manual con GCC

Desde la raíz del repositorio, en Linux:

```bash
gcc -Wall -Wextra -Werror -std=c99 -pedantic -Iinclude -O2 -o calculadora_expr src/token.c src/lexer.c src/arena.c src/ast.c src/parser_expr.c src/evaluador_expr.c src/consola_expr.c src/main_expr.c -lm
gcc -Wall -Wextra -Werror -std=c99 -pedantic -Iinclude -O2 -o verificar_ll1 src/set.c src/ll1.c src/main_verificar_ll1.c
```

En Windows se usan los mismos fuentes con salida `.exe`:

```bat
gcc -Wall -Wextra -Werror -std=c99 -pedantic -Iinclude -O2 -o calculadora_expr.exe src\token.c src\lexer.c src\arena.c src\ast.c src\parser_expr.c src\evaluador_expr.c src\consola_expr.c src\main_expr.c -lm
gcc -Wall -Wextra -Werror -std=c99 -pedantic -Iinclude -O2 -o verificar_ll1.exe src\set.c src\ll1.c src\main_verificar_ll1.c
```

## Uso

Se introduce una expresión completa por línea:

```text
Expresion: x+x*2
Valor de x: 3
Resultado: 9
```

Otros ejemplos: `sin(3.14/2)+cos(0)*2^2`, `sin(x)+cos(y)` y `cos(x)^2+sen(x)^2`. La multiplicación debe ser explícita: `2*x`.

Los literales de la expresión son enteros o decimales con dígitos a ambos lados del punto (`3.14`). No se admiten `.5`, `5.` ni `1e3`. Los valores de variables usan `strtod`, admiten signo y notación científica y deben ser finitos. Una entrada numérica inválida permite reintentar; EOF termina el programa.

Códigos de salida: `0` éxito, `1` error de lectura o recursos, `2` expresión inválida, `3` EOF, `4` error matemático.

## Pruebas y verificación LL(1)

`make test` o `build.bat test` ejecuta:

- `test_parser_expr`: expresiones válidas e inválidas, precedencia, diagnósticos y límites.
- `test_evaluador_expr`: aritmética, trigonometría, variables, errores y profundidad.
- `test_consola_expr`: casos del enunciado, variables, reintentos, EOF y errores.
- `test_ll1`: gramática sin conflictos, gramáticas inválidas y detección de conflictos.

Cada suite imprime `OK`; un fallo detiene la ejecución y devuelve código distinto de cero.

El verificador también se ejecuta de forma independiente:

```bash
./verificar_ll1 gramatica/expresiones_matematicas.txt --sets
```

La salida `ninguno` indica cero conflictos. Códigos: `0` sin conflictos, `1` conflictos o error de lectura, `2` gramática vacía o mal formada, `3` fallo de reserva.

## Informe y limitaciones

El [informe técnico PDF](docs/informe_tecnico.pdf), su [fuente LaTeX](docs/informe_tecnico.tex), los [casos y resultados](docs/casos_prueba_validacion.md), la [matriz de trazabilidad](docs/matriz_trazabilidad.md) y la [gramática documentada](docs/gramatica_expresiones_matematicas.md) forman parte de la entrega. Para regenerar el PDF: `pdflatex -output-directory=docs docs/informe_tecnico.tex` dos veces.

La consola evalúa una expresión por ejecución y no define constantes como `pi`. La profundidad del descenso y de los recorridos del AST está limitada a 128. Los errores `ERANGE` se rechazan, incluido el subdesbordamiento. No se aplican tolerancias artificiales a singularidades de la tangente.
