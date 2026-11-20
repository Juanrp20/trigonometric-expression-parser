# Matriz de trazabilidad y cumplimiento

| Requisito | Implementación o evidencia | Prueba o verificación | Estado |
|---|---|---|---|
| Gramática LL(1) | `gramatica/expresiones_matematicas.txt`, `src/ll1.c` | `./verificar_ll1 ... --sets`, `test_ll1` sobre el archivo real | Cumple: cero conflictos |
| Ejecutable del verificador | `src/main_verificar_ll1.c`, `tests/test_ll1_cli.c` | `test_ll1_cli`: conjuntos, conflicto, gramáticas inválidas y archivo ausente | Cumple |
| Tokenización, reales y posiciones | `src/lexer.c`, `src/token.c` | `test_parser_expr` | Cumple para el formato decimal documentado |
| Parser descendente y entrada completa | `src/parser_expr.c`, `include/parser_expr.h` | `tests/test_parser_expr.c` | Cumple |
| Operadores y precedencia | `src/parser_expr.c`, `src/evaluador_expr.c` | estructura AST y A01–A07 | Cumple |
| `sin`, `sen`, `cos`, `tan` en radianes | `src/evaluador_expr.c` | `tests/test_evaluador_expr.c`, E01–E07 | Cumple |
| Paréntesis y funciones anidadas | `src/parser_expr.c`, `src/evaluador_expr.c` | `tests/test_parser_expr.c`, E03 | Cumple |
| Variables y solicitud única | `src/consola_expr.c` | `tests/test_consola_expr.c`, A08 | Cumple |
| Resultado `double` | `src/evaluador_expr.c` | `tests/test_evaluador_expr.c`, E01–E07 | Cumple |
| Error léxico, sintáctico y matemático | `src/lexer.c`, `src/parser_expr.c`, `src/evaluador_expr.c`, `src/consola_expr.c` | suites C, I01–I10, M01–M04 | Cumple para los casos verificados |
| Validación de entrada, EOF y nombres largos | `src/consola_expr.c` | `tests/test_consola_expr.c` | Cumple |
| Análisis antes de solicitar valores | `src/consola_expr.c` | `tests/test_consola_expr.c`, I02 e I03 | Cumple |
| Evaluación directa durante descenso | enunciado, `src/parser_expr.c`, `src/evaluador_expr.c` | diseño en `docs/informe_tecnico.tex` | Adaptación acordada: evaluación posterior del AST |
| Pruebas y validación matemática | `tests/`, `docs/casos_prueba_validacion.md` | `make test`, `build.bat test`: cinco suites C, esperados fijos | Cumple; cobertura no medida |
| Portabilidad Windows | `build.bat`, `Makefile`, fuentes C99 ASCII | `build.bat all`, `build.bat test` | Diseñada para MinGW-w64; `cmd`, sin shell GNU |
| Informe técnico y código | `docs/informe_tecnico.tex`, `docs/informe_tecnico.pdf`, `src/` | compilación y lectura del PDF | Entregados |

La evidencia de ausencia de conflictos presupone que los tokens `sin`, `sen`, `cos` y `tan` son distintos de `ID`, condición que cumple el lexer. Las expresiones se limitan a 128 niveles sintácticos y 128 nodos de profundidad del AST. `test_ll1` lee la gramática real y comprueba la API con gramáticas inválidas y un caso con conflictos. `test_ll1_cli` comprueba el ejecutable con esa misma gramática real, un conflicto, tres gramáticas inválidas y un archivo inexistente. Los esperados numéricos son constantes fijas en los tests, no valores calculados con la biblioteca matemática del evaluador.
