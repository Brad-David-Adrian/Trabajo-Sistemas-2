# Trabajo-Sistemas-2
Segundo trabajo de SI

## MiniShell UFV
Proyecto: un mini-shell en C con built-ins y ejecución de comandos externos.

### Archivos principales
- `MiniShell/ufv_shell_skeleton.c`
- `MiniShell/tokenizer.c`
- `MiniShell/tokenizer.h`

### Cómo compilar
Recomendado: usar MinGW-w64 en Windows.

En terminal (MSYS2/Mingw64):
```bash
gcc -Wall -o ufv_shell_skeleton ufv_shell_skeleton.c tokenizer.c
```

### Cómo ejecutar
```bash
./ufv_shell_skeleton
```
Usar comandos:
- `pwd`
- `cd <ruta>`
- `exit`
- `ls`, `echo`, etc.

### Correcciones aplicadas
1. Manejo de `exit` y retorno en `cmd_exit`.
2. Validación de tokens (`NULL` y longitud 0) antes de usar.
3. Manejo robusto de `fork` / `execv` y búsqueda en PATH.
4. Prompt y flush apropiado en cada iteración.

### Diagrama de funcionamiento

```text
[Usuario] --> (Entrar comando)
               | 
               v
         [tokenize(line)]
               | 
               v
         [tokens.len == 0?] -- sí --> [nuevo prompt]
               |
              no
               v
        [lookup built-in]
           /        \
        sí           no
        /             \
 [ejecutar built-in] [fork child]
        |               |
        v               v
 [retorna al main]  [execv(prog,args)]
                    |  
                    v
             [si falla: buscar en PATH]
                    |
                    v
          [si falla: command not found]
                    |
                    v
            [parent waitpid(child)]
                    |
                    v
              [mostrar prompt]
```

### Arquitectura y responsabilidades
- `tokenizer.c`: parsing de línea en tokens con soporte para comillas y escapes.
- `ufv_shell_skeleton.c`: flujo principal, built-ins y ejecución de programas externos.
- Tabla `cmd_table[]`: distingue y ejecuta comandos internos.

### Secciones clave para explicar en la defensa
1. **Entrada + tokenización**: `fgets` -> `tokenize` -> validación de tokens.
2. **Built-ins**: `exit`, `pwd`, `cd` ejecutados localmente.
3. **Ejecución externa**: `fork`, `execv`, búsqueda en PATH, y `waitpid`.
4. **Robustez**: validación de `NULL`, manejo de errores de `malloc` y `fork`, prompt consistente.

### Comandos recomendados para pruebas
```bash
gcc -Wall -o ufv_shell_skeleton ufv_shell_skeleton.c tokenizer.c
./ufv_shell_skeleton
``` 
Pruebas:
- `pwd`
- `cd ..`
- `pwd`
- `echo hola`
- `ls -la`
- `exit`

### Final
Este mini-shell está diseñado para ser simple, robusto y educativo, demostrando el núcleo de un intérprete de comandos UNIX básico.

