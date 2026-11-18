# Casos de prueba y validación matemática

Resultados obtenidos con `calculadora_expr` en Linux, GCC 13.3.0 y `double`, ejecutando `python3 tests/validacion_final.py`. Las referencias numéricas se calcularon independientemente con `math` de Python. Se comparó con tolerancia relativa y absoluta de `1e-12`; Python y C pueden usar la misma biblioteca matemática del sistema, por lo que esta comprobación no es una implementación independiente de las funciones trascendentes. Las entradas para variables aparecen en el orden en que la consola las solicita.

| ID | Expresión | Variables | Esperado | Obtenido | Validación matemática | Estado |
|---|---|---|---:|---:|---|---|
| E01 | `sin(3.14/2)+cos(0)*2^2` | — | 4.9999996829318345 | 4.9999996829318345 | sin(1.57)+4 | Pasa |
| E02 | `sin(x)+cos(y)` | x=0, y=0 | 1 | 1 | sin(0)+cos(0) | Pasa |
| E03 | `sin(cos(x))+tan(y)/cos(x)` | x=0, y=0 | 0.8414709848078965 | 0.8414709848078965 | sin(1)+0 | Pasa |
| E04 | `cos(x)^2+sin(x)^2` | x=1 | 1 | 1 | identidad trigonométrica | Pasa |
| E05 | `sin(x)` | x=1 | 0.8414709848078965 | 0.8414709848078965 | seno de 1 radián | Pasa |
| E06 | `cos(x+y)` | x=1, y=2 | -0.98999249660044542 | -0.98999249660044542 | cos(3) | Pasa |
| E07 | `tan(x^2+sin(x))` | x=1 | -3.6038035320088055 | -3.6038035320088055 | tan(1+sin(1)) | Pasa |
| A01 | `2+3*4` | — | 14 | 14 | multiplicación antes de suma | Pasa |
| A02 | `8-3-1` | — | 4 | 4 | (8−3)−1 | Pasa |
| A03 | `8/4/2` | — | 1 | 1 | (8/4)/2 | Pasa |
| A04 | `2^3^2` | — | 512 | 512 | 2^(3^2) | Pasa |
| A05 | `-2^2` | — | -4 | -4 | −(2^2) | Pasa |
| A06 | `2^-2` | — | 0.25 | 0.25 | 1/4 | Pasa |
| A07 | `1--2` | — | 3 | 3 | 1−(−2) | Pasa |
| A08 | `x+x` | x=3 | 6 | 6 | 3+3; x se solicita una vez | Pasa |

E01 es aproximadamente `4.99999968293183`, no exactamente 5, porque `3.14` aproxima π; la referencia usa `sin(1.57)`.

La identidad `cos(x)^2+sin(x)^2 ≈ 1` se ejecutó con x = −7, −1, 0, 0.5, 3.14 y 100. Los resultados impresos fueron respectivamente `0.99999999999999989`, `1`, `1`, `1`, `0.99999999999999989` y `0.99999999999999989`; todos cumplen la tolerancia.

| ID | Entrada | Esperado | Obtenido | Validación | Estado |
|---|---|---|---|---|---|
| I01 | `sin()` | rechazo | código 2 | argumento ausente | Pasa |
| I02 | `sin(x+)` | rechazo | código 2 | operando ausente; no se pidió x | Pasa |
| I03 | `cos(x` | rechazo | código 2 | falta `)`; no se pidió x | Pasa |
| I04 | `2+` | rechazo | código 2 | operando ausente | Pasa |
| I05 | `2**3` | rechazo | código 2 | operador inesperado | Pasa |
| I06 | `sin(1,2)` | rechazo | código 2 | coma inválida | Pasa |
| I07 | `foo(1)` | rechazo | código 2 | llamada no admitida | Pasa |
| I08 | `x(2)` | rechazo | código 2 | texto sobrante; no se pidió x | Pasa |
| I09 | `2 3` | rechazo | código 2 | texto sobrante | Pasa |
| I10 | `sin(x) texto` | rechazo | código 2 | texto sobrante; no se pidió x | Pasa |
| M01 | `1/0` | error matemático | código 4, división entre cero | denominador nulo | Pasa |
| M02 | `0^-1` | error matemático | código 4, potencia indefinida | cero con exponente negativo | Pasa |
| M03 | `(-1)^0.5` | error matemático | código 4, dominio real | raíz real de negativo | Pasa |
| M04 | `10^1000` | error matemático | código 4, fuera de rango | desbordamiento | Pasa |

La suite C además prueba `sen`, datos de variables vacíos, no numéricos, `NaN`, infinito y fuera de rango, reintentos, EOF, nombres largos y diferencias entre mayúsculas y minúsculas. La compilación limpia y todas las pruebas con AddressSanitizer y UBSan pasaron con `ASAN_OPTIONS=detect_leaks=0`. LeakSanitizer falló bajo `ptrace`; no se comprobó ausencia de fugas. No se midió cobertura.
