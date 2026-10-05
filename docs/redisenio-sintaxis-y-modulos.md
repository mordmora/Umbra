# Sintaxis y módulos: contrato provisional

Estado: contrato provisional para el rediseño del front end. La gramática de
[`sintasix.txt`](sintasix.txt) recoge las decisiones de sintaxis acordadas;
la implementación actual todavía no las reconoce todas.

## 1. Qué se conserva de la gramática actual

- Un archivo contiene definiciones de funciones como elementos principales.
- Las sentencias incluyen declaraciones, asignaciones, condicionales, bucles,
  gestión de memoria, retornos y expresiones.
- Las expresiones respetan los niveles de precedencia descritos en la gramática.
- Los saltos de línea separan sentencias. El parser nuevo debe precisar cuándo
  pueden omitirse dentro de paréntesis, corchetes y llaves.

## 2. Extensión propuesta para `use`

El código existente acepta una directiva de línea `use "ruta"` y pega el
contenido del archivo en el texto fuente. La nueva gramática conserva la forma
escrita y define `use` como declaración de módulo:

```ebnf
<program> ::= { <use_declaration> } { <function_definition> }
<use_declaration> ::= "use" <string_literal> [ "as" <identifier> ] <newline>
```

Ejemplo:

```umbra
use "math.umbra"

func start() -> int {
    return math::answer()
}
```

En el nuevo modelo, `use` declara una dependencia entre módulos. Cada archivo
se carga, tokeniza y analiza por separado. La ruta se interpreta respecto al
directorio del archivo que contiene el `use`; el resolvedor conserva la ruta
escrita para los diagnósticos y una identidad de archivo para evitar cargas
duplicadas. El orden de imports no determina si una función puede llamarse
desde otro archivo.

Un ciclo se informa con la cadena completa de imports. Un import repetido del
mismo archivo se procesa una vez. Una ruta inexistente produce un error en el
span de la declaración `use`. No se concatena texto ni se borran líneas de
origen.

Los símbolos de otro archivo se nombran con `::`, por ejemplo
`math::answer()`. El operador `.` se reserva para acceder a miembros de
estructuras, por ejemplo `point.x`. Por ahora, el nombre del módulo se deriva
del nombre del archivo sin extensión. Dos imports como `inc/math.umbra` y
`aux/math.umbra` producirían el mismo nombre `math`; el alias explícito
resuelve la colisión:

```umbra
use "inc/math.umbra" as inc_math
use "aux/math.umbra" as aux_math

func start() -> void {
    inc_math::sum()
    aux_math::sum()
}
```

El nombre efectivo del módulo es el alias si aparece `as`; en caso contrario,
es el nombre del archivo sin extensión. Si ese nombre no es un identificador
válido de Umbra, `as` es obligatorio. Dos imports de archivos distintos con
el mismo nombre efectivo producen un error en el segundo `use`, con referencia
al primero. Un mismo archivo puede importarse con varios aliases sin volver a
cargarlo o analizarlo. El alias pertenece al espacio de nombres de módulos;
`modulo::simbolo` siempre busca en ese espacio.

La identidad del archivo importado se determina por su ruta resuelta, mientras
que el alias controla únicamente cómo se nombra desde el archivo importador.
Los aliases son locales a cada archivo. La política de visibilidad pública de
los símbolos importados queda pendiente.

`::` resuelve nombres en un módulo y `.` resuelve miembros de un valor. Esta
distinción se mantiene incluso si una función retorna una estructura:
`math::origin().x`.

## 3. Fronteras del front end

```text
SourceManager -> SourceFile -> Lexer -> tokens -> Parser -> AST
                       |                           |
                       +-> spans/diagnósticos <----+

AST de cada archivo -> grafo de módulos -> símbolos -> tipos -> HIR tipado
```

`SourceManager` es dueño de los buffers durante toda la compilación. Cada
archivo recibe un `FileId`; cada token y nodo conserva un `Span` formado por
`FileId` y offsets de bytes. Las etapas consumen vistas o IDs, no copias de
lexemas. Esto permite que errores semánticos y expansiones futuras señalen el
archivo original.

La carga valida UTF-8 y retira un BOM UTF-8 solo al comienzo. La equivalencia
NFC de identificadores se aplicará al internarlos; el contenido de literales
de cadena conservará sus caracteres salvo que la especificación del lenguaje
disponga lo contrario. Cualquier transformación de bytes anterior al lexer
debe conservar un mapeo hacia las posiciones originales.

## 4. Decisiones de sintaxis y trabajo pendiente

| Tema | Contrato provisional | Estado de implementación |
| --- | --- | --- |
| `use`, `as` y `::` | Import por ruta, alias opcional y acceso externo con `::` | `use` todavía concatena texto; lexer sin `as` ni `::` |
| Miembros de structs | `.` accede a un miembro de un valor | Lexer ya tiene `.`; resolverlo requiere tipos de struct |
| `void` y `return` | `void` solo como retorno; `return` vacío solo en función `void` | Falta validación completa de retornos |
| `for` | `for {}`, `for condición {}` y `for inicio; condición; paso {}`; declaración inicial con tipo explícito | Lexer y parser todavía no reconocen `for` ni `;` |
| Identificadores | Unicode XID con `_`; comparación en NFC | Lexer y tabla de símbolos aún no aplican esta regla |
| Arreglos | `int[] myArr` y análogos; sin `array<int>` | Adaptar parser y semántica a la forma canónica |
| Comparaciones | La gramática conserva `<`, `>`, `<=`, `>=` | La implementación también usa palabras; decidir compatibilidad |
| `struct` | Forma provisional de la gramática existente | Lexer y parser aún no la implementan |

El `for` se inspira en las tres formas básicas de Go y omite paréntesis. La
condición debe ser `bool`; el inicializador puede declarar `int i = 0` y su
scope termina al salir del bucle. Ejemplo:

```umbra
for int i = 0; i < 10; i = i + 1 {
    print("{}", i)
}
```

`range` se reserva como función builtin futura, no como forma especial del
`for`. Su firma y el protocolo de iteración se definirán con el sistema de
tipos. Las formas `repeat` existentes siguen en la gramática hasta decidir si
se conservan o se sustituyen por `for`.

Los decoradores de funciones al estilo `__inline__()` quedan aplazados. Antes
de añadir su sintaxis habrá que definir si son atributos comprobados por el
compilador, macros o llamadas evaluadas durante la compilación, y qué efectos
pueden tener sobre la firma y el cuerpo de una función.

## 5. Primer incremento implementable

1. **Hecho:** definir `FileId`, `Span`, `SourceFile` y `SourceManager` sin cambiar
   todavía el parser ni la generación de código.
2. **Hecho:** cargar archivos por ruta, conservar bytes originales y offsets de
   líneas, validar UTF-8 y retirar el BOM inicial. El preprocesador actual ya
   usa esta carga; todavía concatena los archivos.
3. **Hecho:** el lexer puede recibir un `SourceFile` y cada token conserva su
   `Span` con offsets de bytes. El constructor con texto sigue disponible como
   adaptador para el compilador actual; esos tokens no tienen `FileId` válido.
4. **Siguiente:** reconocer `use` como token y declaración del parser; construir el grafo de
   dependencias sin concatenar archivos.
5. Registrar las firmas de todos los módulos antes de resolver llamadas;
   validar cuerpos en una segunda pasada.

El incremento termina cuando un error en una función importada muestra su
archivo y línea, los imports repetidos no duplican definiciones y un ciclo
produce un diagnóstico comprensible.
