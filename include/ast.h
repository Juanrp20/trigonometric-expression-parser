#ifndef EXPR_AST_H
#define EXPR_AST_H

#include "arena.h"
#include <stddef.h>

typedef struct ASTExpr ASTExpr;

typedef struct {
    ASTExpr **items;
    size_t count;
    size_t capacity;
} ASTExprList;

typedef enum {
    EXPR_NUMBER,
    EXPR_ID,
    EXPR_PAREN,
    EXPR_UNARY_MINUS,
    EXPR_BINARY,
    EXPR_CALL
} ASTExprKind;

struct ASTExpr {
    ASTExprKind kind;
    union {
        char *str_val;       /* EXPR_NUMBER, EXPR_ID */
        ASTExpr *unary_arg;  /* EXPR_PAREN, EXPR_UNARY_MINUS */
        struct {
            const char *op_str;
            ASTExpr *left;
            ASTExpr *right;
        } binary;
        struct {
            char *callee;
            ASTExprList args;
        } call;
    } as;
};

ASTExpr *ast_expr_number(Arena *arena, const char *lex);
ASTExpr *ast_expr_id(Arena *arena, const char *name);
ASTExpr *ast_expr_paren(Arena *arena, ASTExpr *inner);
ASTExpr *ast_expr_unary(Arena *arena, ASTExprKind kind, ASTExpr *inner);
ASTExpr *ast_expr_binary(Arena *arena, const char *op_str, ASTExpr *left, ASTExpr *right);
ASTExpr *ast_expr_call(Arena *arena, const char *name, ASTExprList args);

void ast_expr_list_init(Arena *arena, ASTExprList *list);
void ast_expr_list_push(Arena *arena, ASTExprList *list, ASTExpr *item);

#endif /* EXPR_AST_H */
