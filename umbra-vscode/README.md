# Umbra Language Support for VS Code

Esta extensión proporciona soporte de syntax highlighting para el lenguaje de programación **Umbra**.

## Características

- ✅ Syntax highlighting para archivos `.umbra`
- ✅ Resaltado de palabras clave del lenguaje
- ✅ Soporte para comentarios (`//`)
- ✅ Resaltado de strings y caracteres
- ✅ Resaltado de números (enteros y decimales)
- ✅ Resaltado de tipos de datos
- ✅ Resaltado de operadores
- ✅ Auto-cerrado de paréntesis, llaves y corchetes
- ✅ Plegado de código (code folding)

## Sintaxis Soportada

### Palabras Clave

| Categoría | Palabras Clave |
|-----------|----------------|
| Control de flujo | `if`, `else`, `elseif`, `repeat`, `times`, `return` |
| Funciones | `func` |
| Operadores lógicos | `and`, `or`, `not` |
| Comparaciones | `equal`, `different`, `less_than`, `greater_than`, `less_or_equal`, `greater_or_equal` |
| Memoria | `new`, `delete`, `ptr`, `ref`, `access` |

### Tipos de Datos

`int`, `float`, `bool`, `char`, `string`, `array`, `void`

### Constantes

`true`, `false`

### Operadores

- Aritméticos: `+`, `-`, `*`, `/`, `%`
- Asignación: `=`
- Incremento/Decremento: `++`, `--`
- Flecha: `->`
- Bit shift: `<<`, `>>`

## Ejemplo de Código

```umbra
func factorial(int n) -> int {
    if (n less_than 2) {
        return 1
    }
    return n * factorial(n - 1)
}

func start() -> void {
    int result = factorial(5)
    print("Factorial: {}", result)
}
```

## Instalación

### Opción 1: Instalación desde VSIX (Recomendado)

1. Empaqueta la extensión:
   ```bash
   cd umbra-vscode
   npm install
   npx vsce package
   ```

2. En VS Code:
   - Presiona `Ctrl+Shift+P` (o `Cmd+Shift+P` en Mac)
   - Escribe "Extensions: Install from VSIX..."
   - Selecciona el archivo `.vsix` generado

### Opción 2: Instalación para desarrollo

1. Copia la carpeta `umbra-vscode` a:
   - **Linux**: `~/.vscode/extensions/`
   - **macOS**: `~/.vscode/extensions/`
   - **Windows**: `%USERPROFILE%\.vscode\extensions\`

2. Reinicia VS Code

## Desarrollo

Para contribuir al desarrollo de esta extensión:

1. Clona el repositorio
2. Abre la carpeta en VS Code
3. Presiona `F5` para abrir una nueva ventana con la extensión cargada
4. Abre un archivo `.umbra` para probar los cambios

## Licencia

MIT License
