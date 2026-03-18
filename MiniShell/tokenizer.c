#include <ctype.h>   // Para funciones de caracteres como isspace
#include <stdlib.h>  // Para malloc, realloc, free
#include <string.h>  // Para strlen, strncpy
#include "tokenizer.h"  // Inclusión del header

// Estructura interna para representar tokens
struct tokens {
  size_t tokens_length;    // Número de tokens
  char **tokens;           // Array de punteros a tokens
  size_t buffers_length;   // Número de buffers (palabras copiadas)
  char **buffers;          // Array de punteros a buffers
};

// Función auxiliar para agregar un elemento a un vector dinámico
static void *vector_push(char ***pointer, size_t *size, void *elem) {
  *pointer = (char**) realloc(*pointer, sizeof(char *) * (*size + 1));  // Redimensiona el array
  (*pointer)[*size] = elem;  // Agrega el elemento
  *size += 1;  // Incrementa el tamaño
  return elem;
}

// Función auxiliar para copiar una palabra desde source
static void *copy_word(char *source, size_t n) {
  source[n] = '\0';  // Termina la cadena en source
  char *word = (char *) malloc(n + 1);  // Reserva memoria para la copia
  strncpy(word, source, n + 1);  // Copia la palabra
  return word;
}

// Función principal de tokenización: convierte una línea en tokens
struct tokens *tokenize(const char *line) {
  if (line == NULL) {  // Si la línea es nula, retorna NULL
    return NULL;
  }

  static char token[4096];  // Buffer estático para construir tokens
  size_t n = 0, n_max = 4096;  // Contadores para el buffer
  struct tokens *tokens;  // Estructura a retornar
  size_t line_length = strlen(line);  // Longitud de la línea

  // Reserva memoria para la estructura tokens
  tokens = (struct tokens *) malloc(sizeof(struct tokens));
  tokens->tokens_length = 0;
  tokens->tokens = NULL;
  tokens->buffers_length = 0;
  tokens->buffers = NULL;

  // Modos de parsing: normal, comillas simples, comillas dobles
  const int MODE_NORMAL = 0,
        MODE_SQUOTE = 1,
        MODE_DQUOTE = 2;
  int mode = MODE_NORMAL;  // Modo inicial

  // Itera sobre cada carácter de la línea
  for (unsigned int i = 0; i < line_length; i++) {
    char c = line[i];
    if (mode == MODE_NORMAL) {  // Modo normal: procesa espacios y comillas
      if (c == '\'') {  // Inicio de comillas simples
        mode = MODE_SQUOTE;
      } else if (c == '"') {  // Inicio de comillas dobles
        mode = MODE_DQUOTE;
      } else if (c == '\\') {  // Carácter de escape
        if (i + 1 < line_length) {  // Si hay siguiente carácter
          token[n++] = line[++i];  // Agrega el carácter escapado
        }
      } else if (isspace((unsigned char)c)) {  // Si es espacio
        if (n > 0) {  // Si hay token acumulado
          void *word = copy_word(token, n);  // Copia la palabra
          vector_push(&tokens->tokens, &tokens->tokens_length, word);  // Agrega a tokens
          n = 0;  // Reinicia contador
        }
      } else {  // Carácter normal
        token[n++] = c;  // Agrega al token actual
      }
    } else if (mode == MODE_SQUOTE) {  // Modo comillas simples
      if (c == '\'') {  // Fin de comillas simples
        mode = MODE_NORMAL;
      } else if (c == '\\') {  // Escape en comillas simples
        if (i + 1 < line_length) {
          token[n++] = line[++i];
        }
      } else {  // Carácter dentro de comillas
        token[n++] = c;
      }
    } else if (mode == MODE_DQUOTE) {  // Modo comillas dobles
      if (c == '"') {  // Fin de comillas dobles
        mode = MODE_NORMAL;
      } else if (c == '\\') {  // Escape en comillas dobles
        if (i + 1 < line_length) {
          token[n++] = line[++i];
        }
      } else {  // Carácter dentro de comillas
        token[n++] = c;
      }
    }
    if (n + 1 >= n_max) abort();  // Evita overflow (no debería ocurrir)
  }

  if (n > 0) {  // Si queda token al final
    void *word = copy_word(token, n);  // Copia la última palabra
    vector_push(&tokens->tokens, &tokens->tokens_length, word);  // Agrega a tokens
    n = 0;
  }
  return tokens;  // Retorna la estructura de tokens
}

// Devuelve el número de tokens
size_t tokens_get_length(struct tokens *tokens) {
  if (tokens == NULL) {  // Si la estructura es nula
    return 0;
  } else {
    return tokens->tokens_length;  // Retorna la longitud
  }
}

// Obtiene el token en la posición n (0-indexed)
char *tokens_get_token(struct tokens *tokens, size_t n) {
  if (tokens == NULL || n >= tokens->tokens_length) {  // Validación
    return NULL;  // Retorna NULL si inválido
  } else {
    return tokens->tokens[n];  // Retorna el token
  }
}

// Libera toda la memoria asociada a la estructura tokens
void tokens_destroy(struct tokens *tokens) {
  if (tokens == NULL) {  // Si es nula, nada que hacer
    return;
  }
  // Libera cada token individual
  for (size_t i = 0; i < tokens->tokens_length; i++) {
    free(tokens->tokens[i]);
  }
  free(tokens->tokens);  // Libera el array de tokens

  // Libera cada buffer (aunque en este código no se usa buffers)
  for (size_t i = 0; i < tokens->buffers_length; i++) {
    free(tokens->buffers[i]);
  }
  free(tokens->buffers);  // Libera el array de buffers

  free(tokens);  // Libera la estructura principal
}