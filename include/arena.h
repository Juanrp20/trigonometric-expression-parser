#ifndef TOY_ARENA_H
#define TOY_ARENA_H

#include <stddef.h>
#include <setjmp.h>

typedef struct {
    void **ptrs;
    size_t count;
    size_t capacity;
    jmp_buf *error_destino;
    int sin_memoria;
} Arena;

void arena_init(Arena *a);
void arena_set_error_destino(Arena *a, jmp_buf *destino);
void *arena_alloc(Arena *a, size_t size);
char *arena_strdup(Arena *a, const char *str);
void arena_free(Arena *a);

#endif /* TOY_ARENA_H */
