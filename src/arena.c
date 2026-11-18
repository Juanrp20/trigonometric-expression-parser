#include "arena.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void arena_init(Arena *a) {
    a->count = 0;
    a->capacity = 0;
    a->ptrs = NULL;
    a->error_destino = NULL;
    a->sin_memoria = 0;
}

void arena_set_error_destino(Arena *a, jmp_buf *destino) { a->error_destino = destino; }

static void *arena_error(Arena *a) {
    a->sin_memoria = 1;
    if (a->error_destino) longjmp(*a->error_destino, 1);
    return NULL;
}

void *arena_alloc(Arena *a, size_t size) {
    if (size > PTRDIFF_MAX) return arena_error(a);
    if (a->count >= a->capacity) {
        if (a->capacity > SIZE_MAX / 2 / sizeof(void *)) return arena_error(a);
        size_t capacidad = a->capacity ? a->capacity * 2 : 64;
        void **ptrs = (void **)realloc(a->ptrs, capacidad * sizeof(void *));
        if (!ptrs) return arena_error(a);
        a->ptrs = ptrs;
        a->capacity = capacidad;
    }
    void *ptr = malloc(size);
    if (!ptr) return arena_error(a);
    a->ptrs[a->count++] = ptr;
    return ptr;
}

char *arena_strdup(Arena *a, const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *copy = (char *)arena_alloc(a, len + 1);
    if (copy) {
        memcpy(copy, str, len + 1);
    }
    return copy;
}

void arena_free(Arena *a) {
    if (!a) return;
    for (size_t i = 0; i < a->count; i++) {
        if (a->ptrs[i]) {
            free(a->ptrs[i]);
        }
    }
    if (a->ptrs) {
        free(a->ptrs);
        a->ptrs = NULL;
    }
    a->count = 0;
    a->capacity = 0;
    a->error_destino = NULL;
}
