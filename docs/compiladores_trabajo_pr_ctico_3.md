# Diseño de Gramática, Reconocedor Descendente y Evaluador para Expresiones Trigonométricas

## Objetivos
- Diseñar una gramática $\text{LL}(1)$ que genere expresiones aritméticas y trigonométricas bien formadas.
- Implementar un reconocedor descendente (parser recursivo o por máquina de pila).
- Evaluar las expresiones ingresadas y entregar el resultado numérico.
- Aplicar conocimientos de análisis sintáctico, gramáticas, y evaluación semántica.

## Descripción del problema
Se desea construir un módulo capaz de reconocer y evaluar expresiones que combinan:
- Constantes numéricas reales
- Funciones trigonométricas: $\sin$, $\cos$, $\tan$
- Operaciones aritméticas: $+$, $-$, $*$, $/$, $\wedge$ (potencia)
- Agrupación mediante paréntesis
- Pedir al usuario el valor numérico de cada variable simbólica detectada
- Sustituir dichos valores durante la evaluación

Ejemplos de expresiones que acepta:
- $\sin(3.14 / 2) + \cos(0) \cdot 2^2$
- $\sin(x) + \cos(y)$
- $\sin(\cos(x)) + \tan(y) / \cos(x)$ *(luego se debería ingresar o pedir los valores de $x$, $y$)*
- $\cos(x)^2 + \sin(x)^2$
- $\sin(x)$
- $\cos(x + y)$
- $\tan(x^2 + \sin(x))$

y devolver los valores numéricos con precisión `double`.

## Implementación sugerida

### a. Tokenización
Se recomienda separar la entrada en tokens:
- $\sin$, $\cos$, $\tan$ $\to$ funciones
- $+$, $-$, $*$, $/$, $\wedge$ $\to$ operadores
- $(,)$ $\to$ paréntesis
- Números reales $\to$ operandos
- $x$, $y$, $z$ $\to$ variables simbólicas

## Evaluación semántica
Durante el análisis descendente, se debe construir el resultado de la expresión en forma directa, evaluando:
- Aritmética común con $+$, $-$, $*$, $/$, $\wedge$
- Funciones trigonométricas usando funciones como `sin`, `cos`, etc.

Ejemplo:

\[
\sin(\pi/2) + \cos(0)\cdot 2^2 = 1 + 1 \cdot 4 = 5.0
\]

## Entregables
1. Informe técnico (PDF o DOCX) con:
   - Introducción y objetivos
   - Gramática diseñada
   - Tabla $\text{LL}(1)$ (opcional, si utiliza autómata de pila)
   - Descripción del algoritmo de reconocimiento (si es recursivo)
   - Casos de prueba y resultados
2. Código fuente del parser evaluador (recomendado en C++, C o Java)
3. Entradas de prueba y resultados obtenidos con su respectiva validación matemática

## Criterios de evaluación

| Criterio | Ponderación |
| :--- | :--- |
| Corrección de la gramática | 20% |
| Implementación del parser | 30% |
| Evaluación correcta de expresiones | 25% |
| Calidad del informe | 15% |
| Pruebas y validación | 10% |