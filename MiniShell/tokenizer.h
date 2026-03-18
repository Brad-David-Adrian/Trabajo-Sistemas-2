#pragma once  // Evita inclusión múltiple del header

/* Estructura que representa una lista de palabras (tokens). */
struct tokens;

/* Convierte una cadena en una lista de palabras. */
struct tokens *tokenize(const char *line);

/* Devuelve el número de palabras (tokens). */
size_t tokens_get_length(struct tokens *tokens);

/* Obtiene la N-ésima palabra (indexada desde 0). */
char *tokens_get_token(struct tokens *tokens, size_t n);

/* Libera la memoria asociada a la estructura. */
void tokens_destroy(struct tokens *tokens);