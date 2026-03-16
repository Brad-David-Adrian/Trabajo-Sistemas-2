//gcc -Wall -o ufv_shell ufv_shell.c tokenizer.c

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include "tokenizer.h"

#define unused __attribute__((unused))

typedef int cmd_fun_t(struct tokens *tokens);

typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

int cmd_exit(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

fun_desc_t cmd_table[] = {
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_pwd,  "pwd",  "print current working directory"},
  {cmd_cd,   "cd",   "change current working directory"},
};

/* ── Built-in implementations ── */

int cmd_exit(unused struct tokens *tokens) {
  (void) tokens;
  exit(0);
  return 0;
}

int cmd_pwd(unused struct tokens *tokens) {
  char cwd[4096];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s\n", cwd);
  } else {
    perror("pwd");
    return -1;
  }
  return 0;
}

int cmd_cd(struct tokens *tokens) {
  if (tokens_get_length(tokens) < 2) {
    char *home = getenv("HOME");
    if (home == NULL) {
      fprintf(stderr, "cd: HOME not set\n");
      return -1;
    }
    if (chdir(home) != 0) {
      perror("cd");
      return -1;
    }
  } else {
    char *path = tokens_get_token(tokens, 1);
    if (chdir(path) != 0) {
      perror("cd");
      return -1;
    }
  }
  return 0;
}

/* ── Program execution ── */

int run_program_thru_path(char *prog, char *args[]) {
  char *PATH_orig = getenv("PATH");
  if (PATH_orig == NULL) return -1;

  char PATH[4096];
  strncpy(PATH, PATH_orig, sizeof(PATH) - 1);
  PATH[sizeof(PATH) - 1] = '\0';

  char prog_path[4096];
  char *saveptr = NULL;
  char *path_dir = strtok_r(PATH, ":", &saveptr);

  while (path_dir != NULL) {
    snprintf(prog_path, sizeof(prog_path), "%s/%s", path_dir, prog);
    execv(prog_path, args);  /* only returns on failure */
    path_dir = strtok_r(NULL, ":", &saveptr);
  }
  return -1;
}

int run_program(struct tokens *tokens) {
  int length = tokens_get_length(tokens);
  if (length == 0) return 0;

  char **args = malloc((length + 1) * sizeof(char *));
  if (args == NULL) { perror("malloc"); return -1; }

  for (int i = 0; i < length; i++)
    args[i] = tokens_get_token(tokens, i);
  args[length] = NULL;

  char *prog = args[0];

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    free(args);
    return -1;
  }

  if (pid == 0) {
    /* Child: try direct path using execvp (PATH-aware) */
    execvp(prog, args);
    /* Fallback to manual PATH search for educational purposes */
    run_program_thru_path(prog, args);
    fprintf(stderr, "ufv: %s: command not found\n", prog);
    free(args);
    exit(1);
  } else {
    /* Parent: wait for child */
    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
      perror("waitpid");
      status = -1;
    }
    free(args);
    return status;
  }
}

int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

int main(unused int argc, unused char *argv[]) {
  char line[4096];
  char *prompt = "ufv";

  fprintf(stdout, "%s: ", prompt);
  fflush(stdout);

  while (fgets(line, sizeof(line), stdin)) {
    struct tokens *tokens = tokenize(line);
    if (tokens == NULL) {
      continue;
    }

    if (tokens_get_length(tokens) == 0) {
      tokens_destroy(tokens);
      fprintf(stdout, "%s: ", prompt);
      fflush(stdout);
      continue;
    }

    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
      run_program(tokens);
    }

    fprintf(stdout, "%s: ", prompt);
    fflush(stdout);

    tokens_destroy(tokens);
  }

  return 0;
}