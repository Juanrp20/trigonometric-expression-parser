# Matriz de trazabilidad y cumplimiento

| Requisito | Implementación o evidencia | Prueba o verificación | Estado |
|---|---|---|---|
| Gramática LL(1) | `gramatica/expresiones_matematicas.txt`, `src/ll1.c` | `toy_ll1 ... --sets`, `test_ll1` | Cumple: cero conflictos |
| Tokenización, reales y posiciones | `src/lexer.c`, `src/token.c` | `test_lexer_matematico`, `test_lexer_errores` | Cumple para el formato decimal documentado |
| Parser descendente y entrada completa | `src/parser_expr.c`, `include/parser_expr.h` | `tests/test_parser_expr.c` | Cumple |
| Operadores y precedencia | `src/parser_expr.c`, `src/evaluador_expr.c` | estructura AST y A01–A07 | Cumple |
| `sin`, `sen`, `cos`, `tan` en radianes | `src/evaluador_expr.c` | `tests/test_evaluador_expr.c`, E01–E07 | Cumple |
| Paréntesis y funciones anidadas | `src/parser_expr.c`, `src/evaluador_expr.c` | `tests/test_parser_expr.c`, E03 | Cumple |
| Variables y solicitud única | `src/consola_expr.c` | `tests/test_consola_expr.c`, A08 | Cumple |
| Resultado `double` | `src/evaluador_expr.c` | `tests/test_evaluador_expr.c`, E01–E07 | Cumple |
| Error léxico, sintáctico y matemático | `src/lexer.c`, `src/parser_expr.c`, `src/evaluador_expr.c`, `src/consola_expr.c` | `tests/test_parser_expr.c`, `tests/test_evaluador_expr.c`, I01–I10, M01–M04 | Cumple para los casos verificados |
| Validación de entrada, EOF y nombres largos | `src/consola_expr.c` | `tests/test_consola_expr.c` | Cumple |
| Análisis antes de solicitar valores | `src/consola_expr.c` | `tests/test_consola_expr.c`, I02 e I03 | Cumple |
| Evaluación directa durante descenso | enunciado, `src/parser_expr.c`, `src/evaluador_expr.c` | diseño documentado en `docs/informe_tecnico.tex` | Adaptación acordada: evaluación posterior del AST, no directa durante descenso |
| Pruebas y validación matemática | `tests/`, `docs/casos_prueba_validacion.md` | `make test`, `python3 tests/validacion_final.py` | Cumple; cobertura no medida |
| Conservación de Toy | `src/parser.c`, `src/lexer.c`, `gramatica/toy_gramatica.txt` | `tests/test_c.c`, `make test` | Regresiones existentes pasan |
| Informe técnico y código | `docs/informe_tecnico.tex`, `docs/informe_tecnico.pdf`, `src/` | compilación y lectura de PDF | Entregados |

La evidencia de ausencia de conflictos presupone que los tokens `sin`, `sen`, `cos` y `tan` son distintos de `ID`, condición que cumple el modo matemático del lexer. Las expresiones se limitan a 128 niveles sintácticos y 128 nodos de profundidad del AST. El verificador interrumpe y libera los resultados parciales ante errores de reserva; `make test-ll1-memoria` inyecta fallos en sus etapas y en la CLI. LeakSanitizer no estuvo disponible bajo `ptrace`.
