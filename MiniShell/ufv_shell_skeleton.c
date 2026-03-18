//gcc -Wall -o ufv_shell ufv_shell.c tokenizer.c

// Inclusión de bibliotecas estándar y del sistema
#include <errno.h>      // Para manejo de errores
#include <stdio.h>      // Para entrada/salida estándar
#include <stdlib.h>     // Para funciones de utilidad como exit, malloc
#include <string.h>     // Para manipulación de cadenas
#include <sys/types.h>  // Para tipos de datos del sistema
#include <signal.h>     // Para manejo de señales (no usado directamente)
#include <sys/wait.h>   // Para waitpid y macros de estado de procesos
#include <unistd.h>     // Para fork, exec, getcwd, chdir
#include "tokenizer.h"  // Inclusión del módulo de tokenización

#define unused __attribute__((unused))  // Macro para marcar parámetros no usados

// Definición de tipo para funciones de comandos
typedef int cmd_fun_t(struct tokens *tokens);

// Estructura que describe un comando built-in
typedef struct fun_desc {
  cmd_fun_t *fun;  // Puntero a la función del comando
  char *cmd;       // Nombre del comando
  char *doc;       // Descripción del comando
} fun_desc_t;

// Declaraciones de funciones built-in
int cmd_exit(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

// Tabla de comandos built-in
fun_desc_t cmd_table[] = {
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_pwd,  "pwd",  "print current working directory"},
  {cmd_cd,   "cd",   "change current working directory"},
};

/* ── Implementaciones de comandos built-in ── */

// Comando exit: termina la shell
int cmd_exit(unused struct tokens *tokens) {
  (void) tokens;  // Evita warning de parámetro no usado
  exit(0);        // Termina el programa con código 0
  return 0;       // No se alcanza, pero para consistencia
}

// Comando pwd: imprime el directorio de trabajo actual
int cmd_pwd(unused struct tokens *tokens) {
  char cwd[4096];  // Buffer para almacenar el path del directorio actual
  if (getcwd(cwd, sizeof(cwd)) != NULL) {  // Obtiene el directorio actual
    printf("%s\n", cwd);  // Imprime el path
  } else {
    perror("pwd");  // Imprime error si falla
    return -1;      // Retorna error
  }
  return 0;  // Éxito
}

// Comando cd: cambia el directorio de trabajo actual
int cmd_cd(struct tokens *tokens) {
  if (tokens_get_length(tokens) < 2) {  // Si no hay argumento, ir a HOME
    char *home = getenv("HOME");  // Obtiene la variable de entorno HOME
    if (home == NULL) {
      fprintf(stderr, "cd: HOME not set\n");  // Error si HOME no está definida
      return -1;
    }
    if (chdir(home) != 0) {  // Cambia al directorio HOME
      perror("cd");  // Imprime error si falla
      return -1;
    }
  } else {  // Si hay argumento, usar el path proporcionado
    char *path = tokens_get_token(tokens, 1);  // Obtiene el path del segundo token
    if (chdir(path) != 0) {  // Cambia al directorio especificado
      perror("cd");  // Imprime error si falla
      return -1;
    }
  }
  return 0;  // Éxito
}

/* ── Ejecución de programas ── */

// Función auxiliar para ejecutar programa buscando manualmente en PATH
int run_program_thru_path(char *prog, char *args[]) {
  char *PATH_orig = getenv("PATH");  // Obtiene la variable PATH
  if (PATH_orig == NULL) return -1;  // Si no hay PATH, falla

  char PATH[4096];  // Copia local de PATH
  strncpy(PATH, PATH_orig, sizeof(PATH) - 1);
  PATH[sizeof(PATH) - 1] = '\0';  // Asegura terminación nula

  char prog_path[4096];  // Buffer para construir path completo
  char *saveptr = NULL;  // Para strtok_r
  char *path_dir = strtok_r(PATH, ":", &saveptr);  // Divide PATH por ':'

  while (path_dir != NULL) {  // Itera sobre cada directorio en PATH
    snprintf(prog_path, sizeof(prog_path), "%s/%s", path_dir, prog);  // Construye path
    execv(prog_path, args);  // Intenta ejecutar; solo retorna si falla
    path_dir = strtok_r(NULL, ":", &saveptr);  // Siguiente directorio
  }
  return -1;  // No encontrado
}

// Función principal para ejecutar un programa externo
int run_program(struct tokens *tokens) {
  int length = tokens_get_length(tokens);  // Obtiene número de tokens
  if (length == 0) return 0;  // Si no hay tokens, nada que hacer

  // Reserva memoria para el array de argumentos
  char **args = malloc((length + 1) * sizeof(char *));
  if (args == NULL) { perror("malloc"); return -1; }  // Error si no hay memoria

  // Copia los tokens al array de argumentos
  for (int i = 0; i < length; i++)
    args[i] = tokens_get_token(tokens, i);
  args[length] = NULL;  // Termina con NULL como requiere exec

  char *prog = args[0];  // El primer argumento es el programa

  pid_t pid = fork();  // Crea un proceso hijo
  if (pid < 0) {  // Error en fork
    perror("fork");
    free(args);  // Libera memoria
    return -1;
  }

  if (pid == 0) {  // Código del proceso hijo
    /* Intenta ejecutar con execvp (busca en PATH) */
    execvp(prog, args);
    /* Si falla, busca manualmente en PATH por motivos educativos */
    run_program_thru_path(prog, args);
    // Si llega aquí, el comando no se encontró
    fprintf(stderr, "ufv: %s: command not found\n", prog);
    free(args);  // Libera memoria en el hijo
    exit(1);     // Termina el hijo con error
  } else {  // Código del proceso padre
    /* Espera a que el hijo termine */
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {  // Espera al hijo
      perror("waitpid");
      status = -1;  // Error en espera
    }
    free(args);  // Libera memoria en el padre
    return status;  // Retorna el status del hijo
  }
}

// Función para buscar un comando en la tabla de built-ins
int lookup(char cmd[]) {
  // Itera sobre la tabla de comandos
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))  // Compara nombres
      return i;  // Retorna índice si encontrado
  return -1;  // No encontrado
}

// Función principal: bucle REPL de la shell
int main(unused int argc, unused char *argv[]) {
  char line[4096];        // Buffer para la línea de entrada
  char *prompt = "ufv";   // Prompt de la shell

  fprintf(stdout, "%s: ", prompt);  // Imprime el prompt inicial
  fflush(stdout);  // Fuerza la salida para que aparezca inmediatamente

  // Bucle infinito para leer comandos
  while (fgets(line, sizeof(line), stdin)) {  // Lee línea desde stdin
    struct tokens *tokens = tokenize(line);   // Tokeniza la línea
    if (tokens == NULL) {  // Si tokenización falla, continúa
      continue;
    }

    if (tokens_get_length(tokens) == 0) {  // Si no hay tokens (línea vacía)
      tokens_destroy(tokens);  // Libera memoria
      fprintf(stdout, "%s: ", prompt);  // Vuelve a mostrar prompt
      fflush(stdout);
      continue;
    }

    int fundex = lookup(tokens_get_token(tokens, 0));  // Busca comando built-in

    if (fundex >= 0) {  // Si es built-in
      cmd_table[fundex].fun(tokens);  // Ejecuta la función correspondiente
    } else {  // Si no es built-in
      run_program(tokens);  // Ejecuta como programa externo
    }

    fprintf(stdout, "%s: ", prompt);  // Muestra prompt para siguiente comando
    fflush(stdout);

    tokens_destroy(tokens);  // Libera la estructura de tokens
  }

  return 0;  // Fin del programa (normalmente no se alcanza por exit)
}