#include "ast.h"
#include <stdint.h>
#include <string.h>

void ast_expr_list_init(Arena *arena, ASTExprList *list) {
    (void)arena;
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void ast_expr_list_push(Arena *arena, ASTExprList *list, ASTExpr *item) {
    if (list->count >= list->capacity) {
        if (list->capacity > (size_t)PTRDIFF_MAX / 2 / sizeof(ASTExpr *)) {
            (void)arena_alloc(arena, SIZE_MAX);
            return;
        }
        size_t new_cap = list->capacity == 0 ? 4 : list->capacity * 2;
        ASTExpr **new_items = (ASTExpr **)arena_alloc(arena, new_cap * sizeof(ASTExpr *));
        if (!new_items) return;
        if (list->items && list->count > 0) {
            memcpy(new_items, list->items, list->count * sizeof(ASTExpr *));
        }
        list->items = new_items;
        list->capacity = new_cap;
    }
    list->items[list->count++] = item;
}

ASTExpr *ast_expr_number(Arena *arena, const char *lex) {
    ASTExpr *e = (ASTExpr *)arena_alloc(arena, sizeof(ASTExpr));
    if (!e) return NULL;
    e->kind = EXPR_NUMBER;
    e->as.str_val = arena_strdup(arena, lex);
    return e;
}

ASTExpr *ast_expr_id(Arena *arena, const char *name) {
    ASTExpr *e = (ASTExpr *)arena_alloc(arena, sizeof(ASTExpr));
    if (!e) return NULL;
    e->kind = EXPR_ID;
    e->as.str_val = arena_strdup(arena, name);
    return e;
}

ASTExpr *ast_expr_paren(Arena *arena, ASTExpr *inner) {
    ASTExpr *e = (ASTExpr *)arena_alloc(arena, sizeof(ASTExpr));
    if (!e) return NULL;
    e->kind = EXPR_PAREN;
    e->as.unary_arg = inner;
    return e;
}

ASTExpr *ast_expr_unary(Arena *arena, ASTExprKind kind, ASTExpr *inner) {
    ASTExpr *e = (ASTExpr *)arena_alloc(arena, sizeof(ASTExpr));
    if (!e) return NULL;
    e->kind = kind;
    e->as.unary_arg = inner;
    return e;
}

ASTExpr *ast_expr_binary(Arena *arena, const char *op_str, ASTExpr *left, ASTExpr *right) {
    ASTExpr *e = (ASTExpr *)arena_alloc(arena, sizeof(ASTExpr));
    if (!e) return NULL;
    e->kind = EXPR_BINARY;
    e->as.binary.op_str = arena_strdup(arena, op_str);
    e->as.binary.left = left;
    e->as.binary.right = right;
    return e;
}

ASTExpr *ast_expr_call(Arena *arena, const char *name, ASTExprList args) {
    ASTExpr *e = (ASTExpr *)arena_alloc(arena, sizeof(ASTExpr));
    if (!e) return NULL;
    e->kind = EXPR_CALL;
    e->as.call.callee = arena_strdup(arena, name);
    e->as.call.args = args;
    return e;
}
