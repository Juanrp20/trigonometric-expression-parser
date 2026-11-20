#include "set.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void set_init(StringSet *s) {
    s->items = NULL;
    s->count = 0;
    s->capacity = 0;
}

int set_contains(const StringSet *s, const char *str) {
    if (!s || s->count == 0 || !str) return 0;
    for (size_t i = 0; i < s->count; i++) {
        if (strcmp(s->items[i], str) == 0) {
            return 1;
        }
    }
    return 0;
}

int set_add(StringSet *s, const char *str) {
    if (!s || !str) return SET_ERROR;
    if (set_contains(s, str)) return SET_SIN_CAMBIOS;
    if (s->count >= s->capacity) {
        if (s->capacity > SIZE_MAX / 2 / sizeof(char *)) return SET_ERROR;
        size_t new_cap = s->capacity == 0 ? 8 : s->capacity * 2;
        char **new_items = (char **)realloc(s->items, new_cap * sizeof(char *));
        if (!new_items) return SET_ERROR;
        s->items = new_items;
        s->capacity = new_cap;
    }
    size_t len = strlen(str);
    if (len == SIZE_MAX) return SET_ERROR;
    char *copia = (char *)malloc(len + 1);
    if (!copia) return SET_ERROR;
    memcpy(copia, str, len + 1);
    /* Insercion ordenada */
    size_t pos = s->count;
    while (pos > 0 && strcmp(s->items[pos - 1], str) > 0) {
        s->items[pos] = s->items[pos - 1];
        pos--;
    }
    s->items[pos] = copia;
    s->count++;
    return 1;
}

int set_union(StringSet *dest, const StringSet *src) {
    if (!dest || !src) return SET_ERROR;
    int changed = 0;
    for (size_t i = 0; i < src->count; i++) {
        int r = set_add(dest, src->items[i]);
        if (r == SET_ERROR) return SET_ERROR;
        if (r == SET_CAMBIO) {
            changed = 1;
        }
    }
    return changed;
}

int set_intersect(StringSet *dest, const StringSet *s1, const StringSet *s2) {
    set_init(dest);
    if (!s1 || !s2) return SET_ERROR;
    for (size_t i = 0; i < s1->count; i++) {
        if (set_contains(s2, s1->items[i])) {
            if (set_add(dest, s1->items[i]) == SET_ERROR) return SET_ERROR;
        }
    }
    return dest->count ? SET_CAMBIO : SET_SIN_CAMBIOS;
}

int set_is_subset(const StringSet *sub, const StringSet *parent) {
    if (!sub) return 1;
    for (size_t i = 0; i < sub->count; i++) {
        if (!set_contains(parent, sub->items[i])) {
            return 0;
        }
    }
    return 1;
}

void set_clear(StringSet *s) {
    if (!s) return;
    for (size_t i = 0; i < s->count; i++) {
        if (s->items[i]) free(s->items[i]);
    }
    s->count = 0;
}

void set_free(StringSet *s) {
    if (!s) return;
    set_clear(s);
    if (s->items) {
        free(s->items);
        s->items = NULL;
    }
    s->capacity = 0;
}
