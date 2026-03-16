# Trabajo-Sistemas-2
Segundo trabajo de SI

## MiniShell UFV
Proyecto: un mini-shell en C con built-ins y ejecución de comandos externos.

### Archivos principales
- `MiniShell/ufv_shell_skeleton.c`
- `MiniShell/tokenizer.c`
- `MiniShell/tokenizer.h`

### Cómo compilar
Recomendado: usar MSYS2 + MinGW-w64 en Windows, o GCC en Linux.

En terminal (MSYS2/Mingw64):
```bash
gcc -Wall -o ufv_shell ufv_shell_skeleton.c tokenizer.c
```

### Cómo ejecutar
```bash
./ufv_shell
```
Comandos de ejemplo:
- `pwd`
- `cd <ruta>`
- `exit`
- `ls -la`
- `echo hola`

### Cambios y correcciones clave aplicadas
1. **Built-in `exit`**: se mantiene la firma y se retorna `0`.
2. **Validación de tokens**: se comprueba `tokens == NULL` y `tokens_get_length(tokens) == 0` para evitar crash en líneas vacías.
3. **Ejecución externa mejorada**:
   - En el hijo se usa `execvp(prog, args)` para usar PATH automáticamente.
   - Si falla, se intenta `run_program_thru_path(...)` manual.
   - Se imprime mensaje "command not found" y se libera memory antes de `exit(1)`.
4. **Manejo de errores**:
   - Verificación de `malloc` y `fork`.
   - Mensajes de `perror(...)` cuando fallan.
5. **Tokenizador robusto**:
   - Usa `isspace((unsigned char)c)`.
   - Soporte básico de comillas simples, dobles y escapes.
6. **Limpieza de memoria**:
   - `tokens_destroy(...)` libera `tokens->tokens`, `tokens->buffers` y `tokens`.
7. **Prompt interactivo**:
   - Se imprime `ufv: ` antes y después de cada comando; se hace `fflush(stdout)` para visualización inmediata.

### Estructura de funcionamiento (concepto)
1. Leer línea con `fgets`.
2. Tokenizar con `tokenize(line)`.
3. Si no hay tokens, mostrar prompt y continuar.
4. Buscar built-in en `cmd_table`; si existe, ejecutar.
5. Si no es built-in, crear proceso hijo con `fork`:
   - en hijo: `execvp(...)` o PATH manual.
   - en padre: `waitpid(...)`.
6. Devolver control al loop y mostrar prompt.

### Diagrama en texto
```text
Usuario -> fgets
         -> tokenize
         -> tokens len == 0 ? prompt
         -> lookup built-in
           -> sí -> ejecutar built-in
           -> no -> fork -> hijo execvp -> padre waitpid
         -> prompt
```

### Secciones para la defensa (recomendado)
- **Tokenización**: cómo se manejan espacios, comillas y escapes.
- **Built-ins**: `exit`, `pwd`, `cd`.
- **Ejecución externa**: `fork`, `execvp`, PATH.
- **Robustez**: validaciones y liberación de memoria.

### Comandos de prueba
```bash
gcc -Wall -o ufv_shell ufv_shell_skeleton.c tokenizer.c
./ufv_shell
```
Pruebas sugeridas:
- `pwd`
- `cd ..`
- `pwd`
- `echo hola`
- `ls -la`
- `exit`

### Notas de instalación en Windows
- Instalar MSYS2 + mingw-w64.
- En terminal MSYS2:
  `pacman -Syu`
  `pacman -S --needed base-devel mingw-w64-x86_64-toolchain`
- Ejecutar `gcc` desde mingw64/bin.

---

### Resultado final
Este mini-shell es un prototipo funcional de un intérprete de comandos: lee input, tokeniza, ejecuta built-ins y programas externos con búsqueda en PATH, manejo de errores y un prompt interactivo.

