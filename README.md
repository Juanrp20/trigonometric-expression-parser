# Trabajo Práctico 3 — Analizador y Evaluador de Expresiones Trigonométricas

Calculadora de consola que reconoce expresiones matemáticas y trigonométricas, solicita valores para sus variables y calcula un resultado `double`. Reutiliza el lexer, el AST, la gestión de memoria y el verificador LL(1) desarrollados en la Tarea 2 de Compiladores para el lenguaje Toy.

## Características

- Gramática LL(1), lexer y parser descendente recursivo. Se valida toda la expresión antes de solicitar valores.
- Operadores `+`, `-`, `*`, `/`, `^`, negación y paréntesis. La potencia se agrupa por la derecha y tiene mayor precedencia que la negación.
- Funciones `sin`, `sen`, `cos` y `tan`, con argumentos en **radianes**.
- Variables simbólicas sensibles a mayúsculas; se pide una vez cada nombre, en orden de primera aparición.
- Detección de errores léxicos, sintácticos y matemáticos; resultados con precisión `double`.

La aplicación está escrita en C99 y utiliza la biblioteca estándar de C, `math.h` y GNU Make. No necesita bibliotecas externas de análisis.

## Estructura

```text
include/     interfaces del lexer, parser, AST, evaluador, consola y LL(1)
src/         implementaciones y puntos de entrada
  main_expr.c, consola_expr.c       calculadora y variables
  parser_expr.c, evaluador_expr.c   parser matemático y evaluación
  lexer.c, token.c, ast.c, arena.c  componentes compartidos
  parser.c, main.c                  parser y ejecutable Toy
  ll1.c, set.c, main_ll1.c         verificador de gramáticas
gramatica/   gramáticas matemática y Toy
tests/       pruebas C y validacion_final.py opcional
ejemplos/    programas Toy usados en pruebas de regresión
docs/        enunciado, gramática, casos, trazabilidad e informe
Makefile     compilación, pruebas y limpieza
```

## Requisitos e instalación

Se necesita GCC compatible con C99, GNU Make y la biblioteca matemática incluida en el entorno de desarrollo. Python 3 solo es necesario para la validación adicional `tests/validacion_final.py`. Para regenerar el informe se necesita pdfLaTeX.

### Linux Mint, Ubuntu y derivados

```bash
sudo apt update
sudo apt install build-essential
```

El paquete [build-essential de Ubuntu](https://packages.ubuntu.com/noble/build-essential) incluye GCC y Make. Comprueba la instalación con `gcc --version` y `make --version`.

### Windows con MSYS2 y MinGW-w64

1. Instala [MSYS2](https://www.msys2.org/) siguiendo sus instrucciones de actualización.
2. Abre la terminal **MSYS2 UCRT64** desde el menú Inicio. En ella, actualiza los paquetes con `pacman -Syu`; si MSYS2 indica que debes cerrar la terminal, vuelve a abrir UCRT64 y repite la actualización.
3. Instala GCC y GNU Make para UCRT64:

   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make
   gcc --version
   mingw32-make --version
   ```

   Los paquetes oficiales son [GCC UCRT64](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-gcc) y [Make UCRT64](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-make). La terminal UCRT64 configura el `PATH` para incluir `/ucrt64/bin`. Si se usa otra terminal, hay que configurar el `PATH` de esa instalación; por ejemplo, si MSYS2 está en `C:\msys64`, añadir `C:\msys64\ucrt64\bin`. Comprueba `gcc --version` y `mingw32-make --version` antes de compilar.

El paquete indicado proporciona `mingw32-make`. Algunas instalaciones también proporcionan un comando `make`, pero su presencia y entorno dependen de los paquetes instalados; las órdenes siguientes usan `mingw32-make` de UCRT64.

## Compilación con Makefile

Ejecuta los comandos desde la raíz del repositorio.

### Linux

```bash
make all                  # compila calculadora_expr, toy_c y toy_ll1
make calculadora_expr     # compila solo la calculadora, si es necesario
./calculadora_expr        # inicia la calculadora
make test                 # compila y ejecuta las cuatro suites de pruebas
make debug                # compila los tres programas con depuración y sanitizadores
make clean                # elimina únicamente ejecutables generados
```

### Windows en MSYS2 UCRT64

```bash
mingw32-make all
mingw32-make calculadora_expr
./calculadora_expr.exe
mingw32-make test
mingw32-make debug
mingw32-make clean
```

En Windows los ejecutables tienen extensión `.exe`. `debug` y `test` utilizan símbolos de depuración, sin activar sanitizadores por defecto. No se ha comprobado la ejecución de Windows en el entorno de esta revisión.

## Compilación manual con GCC, sin Makefile

Desde la raíz del repositorio, en Linux:

```bash
gcc -Wall -Wextra -std=c99 -pedantic -Iinclude -O2 -o calculadora_expr src/token.c src/lexer.c src/arena.c src/ast.c src/parser_expr.c src/evaluador_expr.c src/consola_expr.c src/main_expr.c -lm
gcc -Wall -Wextra -std=c99 -pedantic -Iinclude -O2 -o toy_c src/token.c src/lexer.c src/arena.c src/ast.c src/parser.c src/main.c
gcc -Wall -Wextra -std=c99 -pedantic -Iinclude -O2 -o toy_ll1 src/set.c src/ll1.c src/main_ll1.c
```

En la terminal MSYS2 UCRT64 de Windows, los mismos fuentes se compilan con nombres `.exe`:

```bash
gcc -Wall -Wextra -std=c99 -pedantic -Iinclude -O2 -o calculadora_expr.exe src/token.c src/lexer.c src/arena.c src/ast.c src/parser_expr.c src/evaluador_expr.c src/consola_expr.c src/main_expr.c -lm
gcc -Wall -Wextra -std=c99 -pedantic -Iinclude -O2 -o toy_c.exe src/token.c src/lexer.c src/arena.c src/ast.c src/parser.c src/main.c
gcc -Wall -Wextra -std=c99 -pedantic -Iinclude -O2 -o toy_ll1.exe src/set.c src/ll1.c src/main_ll1.c
```

`-Iinclude` busca los encabezados; `-lm`, al final del primer comando, enlaza las funciones matemáticas. En Windows estos comandos no se han ejecutado en esta revisión.

## Uso de la calculadora

Introduzca una expresión completa en una línea; se permiten espacios:

```text
Expresión: x+x*2
Valor de x: 3
Resultado: 9
```

Otros ejemplos: `sin(3.14/2)+cos(0)*2^2`, `sin(x)+cos(y)` y `cos(x)^2+sen(x)^2`. La multiplicación debe ser explícita: `2*x`. Los literales de la expresión son enteros o decimales con dígitos a ambos lados del punto (`3.14`); `.5`, `5.` y `1e3` no son literales admitidos en la expresión. Los valores de las variables usan `strtod`, admiten signo y notación científica y deben ser finitos. La consola configura `LC_NUMERIC=C` para usar punto decimal.

Una entrada numérica inválida permite reintentar; EOF termina el programa. El código de salida es 0 en éxito, 1 por error de lectura o recursos, 2 por expresión inválida, 3 por EOF y 4 por error de evaluación.

## Pruebas y verificación LL(1)

`make test` en Linux o `mingw32-make test` en UCRT64 ejecuta las pruebas de Toy y del lexer, parser, evaluador y consola matemáticos. Cada grupo informa `OK`; un fallo interrumpe la ejecución y produce un código distinto de cero. No se ha medido el porcentaje de cobertura.

En Linux, `all` compila con optimización `-O2`; `debug` usa `-g` y habilita AddressSanitizer (ASan) y UndefinedBehaviorSanitizer (UBSan). Las pruebas también se compilan con esas opciones. ASan detecta diversos errores de acceso a memoria; UBSan detecta ciertas operaciones de comportamiento indefinido. LeakSanitizer (LSan), cuando está disponible, busca fugas al finalizar. No todos los entornos lo permiten: bajo la restricción `ptrace` observada en esta revisión, se usó `ASAN_OPTIONS=detect_leaks=0 make test`; esa ejecución no comprueba fugas. En Windows el Makefile no activa ASan ni UBSan por defecto, pues su disponibilidad depende de la instalación.

Verificación de la gramática matemática en Linux:

```bash
./toy_ll1 gramatica/expresiones_matematicas.txt --sets
python3 tests/validacion_final.py  # opcional: resultados de extremo a extremo
```

En UCRT64 use `./toy_ll1.exe gramatica/expresiones_matematicas.txt --sets`. La salida `ninguno` indica que el verificador no detectó conflictos. La validación adicional con Python usa su biblioteca estándar; requiere que `calculadora_expr` esté compilada en Linux.

## Informe y limitaciones

El [informe técnico PDF](docs/informe_tecnico.pdf), su [fuente LaTeX](docs/informe_tecnico.tex), los [casos y resultados](docs/casos_prueba_validacion.md), la [matriz de trazabilidad](docs/matriz_trazabilidad.md) y la [gramática documentada](docs/gramatica_expresiones_matematicas.md) forman parte de la entrega. Para regenerar el PDF, ejecuta dos veces `pdflatex -output-directory=docs docs/informe_tecnico.tex`.

La consola evalúa una expresión por ejecución y no incorpora constantes predefinidas como `pi`. La profundidad de anidamiento depende de la pila. Las conversiones y potencias que producen `ERANGE` se rechazan, incluido el subdesbordamiento. Algunas reservas de memoria heredadas no se recuperan de forma controlada ante agotamiento de memoria. La disponibilidad de sanitizadores depende del entorno.

## Componentes heredados de Toy

La Tarea 2 conserva su analizador y verificador. En Linux, `./toy_c ejemplos/ok_completo.toy` comprueba un programa y `./toy_ll1 gramatica/toy_gramatica.txt` comprueba su gramática. El ejemplo siguiente también es válido:

```toy
func main()
  local x := 3.14
  println x
  ret 0
end main()
```

Las gramáticas originales, ejemplos y documentos de Toy se conservan como material académico y para las pruebas de regresión.
