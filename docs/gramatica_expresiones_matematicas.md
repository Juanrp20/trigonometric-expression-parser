# Gramática de expresiones matemáticas

La gramática EBNF de esta etapa está en
[`gramatica/expresiones_matematicas.txt`](../gramatica/expresiones_matematicas.txt).
Su símbolo inicial es `<Inicio>`; la entrada completa debe contener una sola
expresión y terminar después de ella. En la notación, `{ ... }` indica cero o
más repeticiones y `[ ... ]` una aparición opcional.

## Reglas y precedencia

De menor a mayor precedencia:

1. `<Inicio>` y `<Expr>`: una expresión formada por términos con sumas y
   restas. Las operaciones sucesivas se agrupan por la izquierda.
2. `<Termino>`: factores unidos por multiplicaciones o divisiones, también
   agrupadas por la izquierda.
3. `<Unario>`: negación mediante `-`, que puede repetirse.
4. `<Potencia>`: un primario con exponente opcional. El exponente es un
   `<Unario>`, lo que permite exponentes negativos y hace que las potencias
   se agrupen por la derecha. Por tanto, `2^3^2` significa `2^(3^2)`,
   `-2^2` significa `-(2^2)` y `2^-2` significa `2^(-2)`.
5. `<Primario>`: número, variable, expresión entre paréntesis o función
   aplicada a una expresión. Los paréntesis permiten alterar la precedencia
   y anidar expresiones.
6. `<Funcion>`: `sin`, `sen`, `cos` o `tan`, con exactamente un argumento
   entre paréntesis. `sin` y `sen` designan la misma función seno; los
   argumentos trigonométricos se interpretarán en radianes.

## Contrato léxico

- `NUMBER` representa un entero o decimal con dígitos antes y, si hay punto,
  después de este: `[0-9]+(\.[0-9]+)?`. Es la forma ya reconocida por Toy;
  `.5`, `5.` y la notación científica quedan fuera de esta gramática.
- `ID` representa un identificador que comienza con letra ASCII o `_` y
  continúa con letras ASCII, dígitos o `_`. Se reserva exactamente `sin`,
  `sen`, `cos` y `tan` para las funciones; nombres como `seno` siguen siendo
  identificadores. Las palabras reservadas de Toy no se reservan aquí.
- Los símbolos entre comillas son terminales distintos de `NUMBER` e `ID`.
  En particular, el léxico matemático clasifica los cuatro
  nombres de función antes que `ID`. La verificación LL(1) depende de que
  estas clases no se solapen.
- Se admiten espacios entre tokens. El modo matemático no trata
  `--` como comentario de Toy: `1--2` representa `1 - (-2)`.

La gramática describe sintaxis. El punto medio `·` usado en el enunciado representa multiplicación;
su representación textual en este lenguaje es `*`.

## Verificación LL(1)

El verificador existente expande las repeticiones y opciones a reglas BNF,
calcula PRIMERO y SIGUIENTE y comprueba los conjuntos de selección de cada
alternativa. Para reproducir la verificación:

```sh
make toy_ll1
./toy_ll1 gramatica/expresiones_matematicas.txt --sets
```

La salida debe indicar `ninguno` para conflictos. El modo matemático se activa
con `lexer_init_matematico`; `lexer_init` conserva el comportamiento de Toy.
Los nombres de función se emiten como `TOK_SIN`, `TOK_SEN`, `TOK_COS` y
`TOK_TAN`, y los identificadores ordinarios como `TOK_ID`.

El verificador devuelve 0 sin conflictos, 1 si encuentra conflictos o no puede
leer el archivo, y 2 si la gramática está vacía o mal formada. Rechaza grupos
sin cerrar, alternativas vacías y no terminales sin definición.

## Reconocedor descendente

`parser_expr_init` prepara el lexer matemático y la arena del parser;
`parser_expr_parse` reconoce una expresión completa y devuelve su `ASTExpr`, o
`NULL` y un diagnóstico en `error_msg` si encuentra un error léxico o
sintáctico. Se realiza una llamada a `parser_expr_parse` por inicialización.
`parser_expr_destroy` libera el token de preanálisis, el lexer y la arena;
el AST solo es válido hasta esa llamada.

Las funciones internas siguen las reglas de la gramática: `expr`, `termino`,
`unario`, `potencia`, `primario` y `funcion`. Las repeticiones de suma,
resta, multiplicación y división construyen nodos por la izquierda; la
recursión desde el exponente construye potencias por la derecha. El punto de
entrada exige `TOK_EOF`. El AST reutiliza las estructuras de Toy y conserva
operandos y operadores para el evaluador; el parser no evalúa ni
solicita valores de variables.

El descenso sintáctico admite hasta 128 niveles de llamada a la regla
`<Unario>`. Los recorridos posteriores de variables y evaluación limitan la
profundidad del AST a 128 nodos; las cadenas de operadores asociativos pueden
alcanzar este límite aunque no utilicen paréntesis.

## Evaluación semántica

`evaluador_expr` recorre recursivamente el AST sin modificarlo, con el parser
todavía vivo. Primero se valida la expresión completa; después se evalúa el
árbol resultante. Así se realiza la evaluación semántica del enunciado en una
pasada posterior, en vez de calcular durante el descenso sintáctico, y se
puede validar antes de solicitar valores sin duplicar el parser. La futura
interfaz podrá reutilizar el mismo AST validado.

Los literales se convierten con `strtod`; se comprueba la conversión completa,
`ERANGE` y la finitud. Se usa el `LC_NUMERIC` activo, que debe conservar el
punto decimal del lenguaje (por defecto es la configuración C). La resolución
de identificadores se delega a un callback opcional con contexto; el
evaluador no realiza entrada ni salida. El llamador debe proporcionar un
valor finito por cada variable y conservar el AST hasta acabar la evaluación.

Se admiten los operadores y funciones de la gramática, con trigonometría en
radianes. Se rechazan divisiones entre cero, errores de dominio o rango de
`pow`, resultados no finitos, variables sin valor y nodos no admitidos. La
conversión numérica y `pow` también rechazan `ERANGE` por subdesbordamiento;
no se utiliza una tolerancia artificial para declarar indefinida `tan`.
`ExprResultado` distingue éxito y error; el valor solo es válido en caso de
éxito.
