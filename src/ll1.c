#include "ll1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <limits.h>

static char *my_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    if (len == SIZE_MAX) return NULL;
    char *p = (char *)malloc(len + 1);
    if (p) memcpy(p, s, len + 1);
    return p;
}

typedef enum {
    TOK_EBNF_EOF,
    TOK_EBNF_ARROW,
    TOK_EBNF_LBRACE,
    TOK_EBNF_RBRACE,
    TOK_EBNF_LBRACKET,
    TOK_EBNF_RBRACKET,
    TOK_EBNF_LPAR,
    TOK_EBNF_RPAR,
    TOK_EBNF_PIPE,
    TOK_EBNF_NT,
    TOK_EBNF_T,
    TOK_EBNF_ID
} EbnfTokKind;

typedef struct {
    EbnfTokKind kind;
    char *val;
} EbnfToken;

typedef struct {
    EbnfToken *items;
    size_t count;
    size_t capacity;
} EbnfTokenList;

static void tok_list_init(EbnfTokenList *l) {
    l->items = NULL;
    l->count = 0;
    l->capacity = 0;
}

/* Solo actualiza la capacidad después de reservar correctamente. */
static void *grow(void *ptr, size_t *capacity, size_t width, size_t initial) {
    if (*capacity > SIZE_MAX / 2) return NULL;
    size_t next = *capacity ? *capacity * 2 : initial;
    if (next > SIZE_MAX / width) return NULL;
    void *new_ptr = realloc(ptr, next * width);
    if (!new_ptr) return NULL;
    *capacity = next;
    return new_ptr;
}

static int tok_list_push(EbnfTokenList *l, EbnfTokKind kind, const char *val) {
    if (l->count >= l->capacity) {
        EbnfToken *items = grow(l->items, &l->capacity, sizeof(*items), 32);
        if (!items) return 0;
        l->items = items;
    }
    char *copy = my_strdup(val);
    if (!copy) return 0;
    l->items[l->count].kind = kind;
    l->items[l->count++].val = copy;
    return 1;
}

static void tok_list_free(EbnfTokenList *l) {
    if (!l) return;
    for (size_t i = 0; i < l->count; i++) {
        if (l->items[i].val) free(l->items[i].val);
    }
    if (l->items) free(l->items);
    l->items = NULL;
    l->count = 0;
    l->capacity = 0;
}

static void seq_init(SymbolSeq *s) {
    s->symbols = NULL;
    s->count = 0;
    s->capacity = 0;
}

static int seq_push(SymbolSeq *s, const char *sym) {
    if (s->count >= s->capacity) {
        char **symbols = grow(s->symbols, &s->capacity, sizeof(*symbols), 4);
        if (!symbols) return 0;
        s->symbols = symbols;
    }
    char *copy = my_strdup(sym);
    if (!copy) return 0;
    s->symbols[s->count++] = copy;
    return 1;
}

static void seq_free(SymbolSeq *s) {
    if (!s) return;
    for (size_t i = 0; i < s->count; i++) {
        if (s->symbols[i]) free(s->symbols[i]);
    }
    if (s->symbols) free(s->symbols);
    s->symbols = NULL;
    s->count = 0;
    s->capacity = 0;
}

static void alts_init(AltList *a) {
    a->items = NULL;
    a->count = 0;
    a->capacity = 0;
}

/* La secuencia pasa a ser propiedad de la lista solo en caso de éxito. */
static int alts_push(AltList *a, SymbolSeq seq) {
    if (a->count >= a->capacity) {
        SymbolSeq *items = grow(a->items, &a->capacity, sizeof(*items), 4);
        if (!items) return 0;
        a->items = items;
    }
    a->items[a->count++] = seq;
    return 1;
}

static void alts_free(AltList *a) {
    if (!a) return;
    for (size_t i = 0; i < a->count; i++) {
        seq_free(&a->items[i]);
    }
    if (a->items) free(a->items);
    a->items = NULL;
    a->count = 0;
    a->capacity = 0;
}

/* Helper para contadores de auxiliares */
typedef struct {
    char *base;
    int counter;
} AuxCounter;

typedef struct {
    AuxCounter *items;
    size_t count;
    size_t capacity;
} AuxMap;

static char *make_aux_name(AuxMap *map, const char *orig_lhs) {
    /* base = orig_lhs.strip("<>").split("_aux")[0] */
    char stripped[256];
    size_t len = strlen(orig_lhs);
    size_t s = 0, e = len;
    if (len > 0 && orig_lhs[0] == '<') s = 1;
    if (len > 1 && orig_lhs[len - 1] == '>') e = len - 1;

    size_t k = 0;
    for (size_t i = s; i < e; i++) {
        if (orig_lhs[i] == '_' && strncmp(&orig_lhs[i], "_aux", 4) == 0) {
            break;
        }
        stripped[k++] = orig_lhs[i];
    }
    stripped[k] = '\0';

    int count = 1;
    int found = 0;
    for (size_t i = 0; i < map->count; i++) {
        if (strcmp(map->items[i].base, stripped) == 0) {
            if (map->items[i].counter == INT_MAX) return NULL;
            map->items[i].counter++;
            count = map->items[i].counter;
            found = 1;
            break;
        }
    }
    if (!found) {
        if (map->count >= map->capacity) {
            AuxCounter *items = grow(map->items, &map->capacity, sizeof(*items), 8);
            if (!items) return NULL;
            map->items = items;
        }
        map->items[map->count].base = my_strdup(stripped);
        if (!map->items[map->count].base) return NULL;
        map->items[map->count].counter = 1;
        map->count++;
    }

    char res[512];
    snprintf(res, sizeof(res), "<%s_aux%d>", stripped, count);
    return my_strdup(res);
}

static void aux_map_free(AuxMap *map) {
    if (!map) return;
    for (size_t i = 0; i < map->count; i++) {
        free(map->items[i].base);
    }
    if (map->items) free(map->items);
    map->items = NULL;
    map->count = 0;
    map->capacity = 0;
}

static int tokenize_ebnf(const char *text, EbnfTokenList *tokens) {
    tok_list_init(tokens);
    const char *p = text;

    while (*p) {
        /* Line-by-line processing for comments */
        const char *line_start = p;
        while (*p && *p != '\n') p++;
        size_t line_len = p - line_start;
        if (*p == '\n') p++;

        char line[2048];
        if (line_len >= sizeof(line)) return 0;
        memcpy(line, line_start, line_len);
        line[line_len] = '\0';

        char *comment = strstr(line, "--");
        if (comment) {
            *comment = '\0';
            line_len = comment - line;
        }

        size_t i = 0;
        while (i < line_len) {
            char c = line[i];
            if (isspace((unsigned char)c)) {
                i++;
                continue;
            }
            if (line[i] == '-' && line[i + 1] == '>') {
                if (!tok_list_push(tokens, TOK_EBNF_ARROW, "->")) return -1;
                i += 2;
                continue;
            }
            if (c == '{') { if (!tok_list_push(tokens, TOK_EBNF_LBRACE, "{")) return -1; i++; continue; }
            if (c == '}') { if (!tok_list_push(tokens, TOK_EBNF_RBRACE, "}")) return -1; i++; continue; }
            if (c == '[') { if (!tok_list_push(tokens, TOK_EBNF_LBRACKET, "[")) return -1; i++; continue; }
            if (c == ']') { if (!tok_list_push(tokens, TOK_EBNF_RBRACKET, "]")) return -1; i++; continue; }
            if (c == '(') { if (!tok_list_push(tokens, TOK_EBNF_LPAR, "(")) return -1; i++; continue; }
            if (c == ')') { if (!tok_list_push(tokens, TOK_EBNF_RPAR, ")")) return -1; i++; continue; }
            if (c == '|') { if (!tok_list_push(tokens, TOK_EBNF_PIPE, "|")) return -1; i++; continue; }

            if (c == '<') {
                size_t j = i + 1;
                while (j < line_len && line[j] != '>') j++;
                if (j == line_len || j == i + 1 || j - i >= 255) return 0;
                size_t end_j = (j < line_len) ? j + 1 : line_len;
                char buf[256];
                size_t tok_l = end_j - i;
                if (tok_l >= sizeof(buf)) tok_l = sizeof(buf) - 1;
                memcpy(buf, &line[i], tok_l);
                buf[tok_l] = '\0';
                if (!tok_list_push(tokens, TOK_EBNF_NT, buf)) return -1;
                i = end_j;
                continue;
            }
            if (c == '"') {
                size_t j = i + 1;
                while (j < line_len && line[j] != '"') j++;
                if (j == line_len || j - i >= 255) return 0;
                size_t end_j = (j < line_len) ? j + 1 : line_len;
                char buf[256];
                size_t tok_l = end_j - i;
                if (tok_l >= sizeof(buf)) tok_l = sizeof(buf) - 1;
                memcpy(buf, &line[i], tok_l);
                buf[tok_l] = '\0';
                if (!tok_list_push(tokens, TOK_EBNF_T, buf)) return -1;
                i = end_j;
                continue;
            }
            if (c == '\'') {
                size_t j = i + 1;
                while (j < line_len && line[j] != '\'') j++;
                if (j == line_len || j - i >= 254) return 0;
                size_t end_j = (j < line_len) ? j + 1 : line_len;
                char buf[256];
                buf[0] = '"';
                size_t inner_l = (j < line_len) ? j - (i + 1) : line_len - (i + 1);
                if (inner_l >= sizeof(buf) - 2) inner_l = sizeof(buf) - 3;
                memcpy(&buf[1], &line[i + 1], inner_l);
                buf[1 + inner_l] = '"';
                buf[2 + inner_l] = '\0';
                if (!tok_list_push(tokens, TOK_EBNF_T, buf)) return -1;
                i = end_j;
                continue;
            }
            if (isalnum((unsigned char)c) || c == '_') {
                size_t j = i;
                while (j < line_len && (isalnum((unsigned char)line[j]) || line[j] == '_')) j++;
                char buf[256];
                size_t tok_l = j - i;
                if (tok_l >= sizeof(buf)) tok_l = sizeof(buf) - 1;
                memcpy(buf, &line[i], tok_l);
                buf[tok_l] = '\0';
                if (!tok_list_push(tokens, TOK_EBNF_ID, buf)) return -1;
                i = j;
                continue;
            }

            return 0; /* Los terminales de puntuación deben ir entre comillas. */
        }
    }
    return 1;
}

/* Comprueba la estructura antes de expandir EBNF a reglas auxiliares. */
static int validar_rhs(const EbnfTokenList *t, size_t *i, EbnfTokKind cierre, int depth) {
    int elementos = 0;
    if (depth > 128) return 0;
    while (*i < t->count) {
        EbnfTokKind k = t->items[*i].kind;
        if (k == cierre || (cierre == TOK_EBNF_EOF && k == TOK_EBNF_NT &&
            *i + 1 < t->count && t->items[*i + 1].kind == TOK_EBNF_ARROW)) break;
        if (k == TOK_EBNF_PIPE) {
            if (!elementos) return 0;
            elementos = 0; (*i)++; continue;
        }
        if (k == TOK_EBNF_LPAR || k == TOK_EBNF_LBRACKET || k == TOK_EBNF_LBRACE) {
            EbnfTokKind fin = k == TOK_EBNF_LPAR ? TOK_EBNF_RPAR :
                              k == TOK_EBNF_LBRACKET ? TOK_EBNF_RBRACKET : TOK_EBNF_RBRACE;
            (*i)++;
            if (!validar_rhs(t, i, fin, depth + 1) || *i >= t->count || t->items[*i].kind != fin) return 0;
            (*i)++;
        } else if (k == TOK_EBNF_NT || k == TOK_EBNF_T || k == TOK_EBNF_ID) {
            (*i)++;
        } else return 0;
        elementos = 1;
    }
    return elementos;
}

static int validar_tokens(const EbnfTokenList *t) {
    size_t i = 0;
    if (!t->count) return 0;
    while (i < t->count) {
        if (t->items[i].kind != TOK_EBNF_NT || i + 1 >= t->count ||
            t->items[i + 1].kind != TOK_EBNF_ARROW) return 0;
        i += 2;
        if (!validar_rhs(t, &i, TOK_EBNF_EOF, 0)) return 0;
    }
    for (size_t j = 0; j < t->count; j++) {
        if (t->items[j].kind == TOK_EBNF_NT &&
            !(j + 1 < t->count && t->items[j + 1].kind == TOK_EBNF_ARROW)) {
            int definido = 0;
            for (size_t k = 0; k + 1 < t->count; k++)
                if (t->items[k].kind == TOK_EBNF_NT && t->items[k + 1].kind == TOK_EBNF_ARROW &&
                    strcmp(t->items[j].val, t->items[k].val) == 0) { definido = 1; break; }
            if (!definido) return 0;
        }
    }
    return 1;
}

/* Parser recursivo de RHS */
typedef struct {
    EbnfToken *tokens;
    size_t count;
    size_t pos;
    const char *orig_lhs;
    AuxMap *aux_map;
    Rule *aux_rules;
    size_t aux_count;
    size_t aux_capacity;
} RhsParser;

static int rhs_add_aux_rule(RhsParser *rp, const char *lhs, AltList alts) {
    if (rp->aux_count >= rp->aux_capacity) {
        Rule *rules = grow(rp->aux_rules, &rp->aux_capacity, sizeof(*rules), 8);
        if (!rules) return 0;
        rp->aux_rules = rules;
    }
    char *nombre = my_strdup(lhs);
    if (!nombre) return 0;
    Rule *r = &rp->aux_rules[rp->aux_count++];
    r->lhs = nombre;
    r->alts = alts;
    set_init(&r->first);
    set_init(&r->follow);
    return 1;
}

static int parse_rhs_expr(RhsParser *rp, AltList *out);

static char *parse_rhs_factor(RhsParser *rp) {
    if (rp->pos >= rp->count) return NULL;
    EbnfToken t = rp->tokens[rp->pos++];
    if (t.kind == TOK_EBNF_NT || t.kind == TOK_EBNF_T || t.kind == TOK_EBNF_ID)
        return my_strdup(t.val);
    if (t.kind != TOK_EBNF_LBRACE && t.kind != TOK_EBNF_LBRACKET && t.kind != TOK_EBNF_LPAR)
        return NULL;
    AltList inner, aux;
    alts_init(&inner);
    alts_init(&aux);
    char *nombre = NULL;
    if (!parse_rhs_expr(rp, &inner)) goto error;
    rp->pos++; /* El cierre ya fue validado antes del análisis. */
    nombre = make_aux_name(rp->aux_map, rp->orig_lhs);
    if (!nombre) goto error;
    for (size_t i = 0; i < inner.count; i++) {
        SymbolSeq copia;
        seq_init(&copia);
        for (size_t j = 0; j < inner.items[i].count; j++) {
            if (!seq_push(&copia, inner.items[i].symbols[j])) { seq_free(&copia); goto error; }
        }
        if (t.kind == TOK_EBNF_LBRACE && !seq_push(&copia, nombre)) { seq_free(&copia); goto error; }
        if (!alts_push(&aux, copia)) { seq_free(&copia); goto error; }
    }
    if (t.kind != TOK_EBNF_LPAR) {
        SymbolSeq eps;
        seq_init(&eps);
        if (!alts_push(&aux, eps)) goto error;
    }
    if (!rhs_add_aux_rule(rp, nombre, aux)) goto error;
    alts_free(&inner);
    return nombre;
error:
    alts_free(&inner);
    alts_free(&aux);
    free(nombre);
    return NULL;
}

static int parse_rhs_seq(RhsParser *rp, SymbolSeq *seq) {
    seq_init(seq);
    while (rp->pos < rp->count) {
        EbnfTokKind k = rp->tokens[rp->pos].kind;
        if (k == TOK_EBNF_PIPE || k == TOK_EBNF_RBRACE ||
            k == TOK_EBNF_RBRACKET || k == TOK_EBNF_RPAR) break;
        char *f = parse_rhs_factor(rp);
        if (!f) { seq_free(seq); return 0; }
        int ok = seq_push(seq, f);
        free(f);
        if (!ok) { seq_free(seq); return 0; }
    }
    return 1;
}

static int parse_rhs_expr(RhsParser *rp, AltList *out) {
    alts_init(out);
    for (;;) {
        SymbolSeq seq;
        if (!parse_rhs_seq(rp, &seq)) { alts_free(out); return 0; }
        if (!alts_push(out, seq)) { seq_free(&seq); alts_free(out); return 0; }
        if (rp->pos >= rp->count || rp->tokens[rp->pos].kind != TOK_EBNF_PIPE) return 1;
        rp->pos++;
    }
}

void ll1_grammar_init(LL1Grammar *g) {
    g->rules = NULL;
    g->rule_count = 0;
    g->rule_capacity = 0;
    g->calculo_valido = 0;
    set_init(&g->non_terminals);
    set_init(&g->nullable);
}

static void rule_free(Rule *r) {
    free(r->lhs);
    r->lhs = NULL;
    alts_free(&r->alts);
    set_free(&r->first);
    set_free(&r->follow);
}

int ll1_grammar_parse(LL1Grammar *g, const char *ebnf_text) {
    EbnfTokenList tokens;
    AuxMap aux_map = {NULL, 0, 0};
    RhsParser rp = {NULL, 0, 0, NULL, &aux_map, NULL, 0, 0};
    size_t *starts = NULL, start_count = 0;
    int result = -1;
    ll1_grammar_init(g);
    tok_list_init(&tokens);
    if (!ebnf_text) return 0;
    int lex = tokenize_ebnf(ebnf_text, &tokens);
    if (lex < 0) goto cleanup;
    if (!lex || !validar_tokens(&tokens)) { result = 0; goto cleanup; }
    if (tokens.count > SIZE_MAX / sizeof(*starts)) goto cleanup;
    starts = malloc(tokens.count * sizeof(*starts));
    if (!starts) goto cleanup;
    for (size_t i = 0; i + 1 < tokens.count; i++)
        if (tokens.items[i].kind == TOK_EBNF_NT && tokens.items[i + 1].kind == TOK_EBNF_ARROW)
            starts[start_count++] = i;
    rp.tokens = tokens.items;
    for (size_t idx = 0; idx < start_count; idx++) {
        size_t start = starts[idx];
        const char *lhs = tokens.items[start].val;
        rp.orig_lhs = lhs;
        rp.pos = start + 2;
        rp.count = idx + 1 < start_count ? starts[idx + 1] : tokens.count;
        AltList alts;
        if (!parse_rhs_expr(&rp, &alts)) goto cleanup;
        Rule *existing = NULL;
        for (size_t r = 0; r < g->rule_count; r++)
            if (strcmp(g->rules[r].lhs, lhs) == 0) { existing = &g->rules[r]; break; }
        if (existing) {
            for (size_t a = 0; a < alts.count; a++) {
                if (!alts_push(&existing->alts, alts.items[a])) { alts_free(&alts); goto cleanup; }
                seq_init(&alts.items[a]);
            }
            alts_free(&alts);
        } else {
            if (g->rule_count >= g->rule_capacity) {
                Rule *rules = grow(g->rules, &g->rule_capacity, sizeof(*rules), 16);
                if (!rules) { alts_free(&alts); goto cleanup; }
                g->rules = rules;
            }
            char *name = my_strdup(lhs);
            if (!name) { alts_free(&alts); goto cleanup; }
            Rule *r = &g->rules[g->rule_count++];
            r->lhs = name; r->alts = alts;
            set_init(&r->first); set_init(&r->follow);
        }
    }
    for (size_t i = 0; i < rp.aux_count; i++) {
        if (g->rule_count >= g->rule_capacity) {
            Rule *rules = grow(g->rules, &g->rule_capacity, sizeof(*rules), 16);
            if (!rules) goto cleanup;
            g->rules = rules;
        }
        g->rules[g->rule_count++] = rp.aux_rules[i];
        rp.aux_rules[i].lhs = NULL;
        alts_init(&rp.aux_rules[i].alts);
        set_init(&rp.aux_rules[i].first);
        set_init(&rp.aux_rules[i].follow);
    }
    for (size_t i = 0; i < g->rule_count; i++)
        if (set_add(&g->non_terminals, g->rules[i].lhs) == SET_ERROR) goto cleanup;
    result = 1;
cleanup:
    for (size_t i = 0; i < rp.aux_count; i++) rule_free(&rp.aux_rules[i]);
    free(rp.aux_rules);
    aux_map_free(&aux_map);
    free(starts);
    tok_list_free(&tokens);
    if (result != 1) ll1_grammar_free(g);
    return result;
}

static Rule *find_rule(LL1Grammar *g, const char *lhs) {
    for (size_t i = 0; i < g->rule_count; i++) {
        if (strcmp(g->rules[i].lhs, lhs) == 0) {
            return &g->rules[i];
        }
    }
    return NULL;
}

static int is_sequence_nullable(LL1Grammar *g, const char *const *symbols, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (!set_contains(&g->nullable, symbols[i])) {
            return 0;
        }
    }
    return 1;
}

static int first_of_sequence(LL1Grammar *g, const char *const *symbols, size_t count, StringSet *res) {
    set_init(res);
    for (size_t i = 0; i < count; i++) {
        const char *sym = symbols[i];
        if (set_contains(&g->non_terminals, sym)) {
            Rule *r = find_rule(g, sym);
            if (r && set_union(res, &r->first) == SET_ERROR) goto error;
            if (!set_contains(&g->nullable, sym)) break;
        } else {
            if (set_add(res, sym) == SET_ERROR) goto error;
            break;
        }
    }
    return 1;
error:
    set_free(res);
    return 0;
}

int ll1_grammar_calculate(LL1Grammar *g) {
    g->calculo_valido = 0;
    if (!g->rule_count) return 0;
    int changed = 1;
    while (changed) {
        changed = 0;
        for (size_t i = 0; i < g->rule_count; i++) {
            Rule *r = &g->rules[i];
            if (set_contains(&g->nullable, r->lhs)) continue;
            for (size_t a = 0; a < r->alts.count; a++) {
                SymbolSeq *seq = &r->alts.items[a];
                if (is_sequence_nullable(g, (const char *const *)seq->symbols, seq->count)) {
                    int n = set_add(&g->nullable, r->lhs);
                    if (n == SET_ERROR) return 0;
                    changed |= n;
                    break;
                }
            }
        }
    }
    changed = 1;
    while (changed) {
        changed = 0;
        for (size_t i = 0; i < g->rule_count; i++) {
            Rule *r = &g->rules[i];
            for (size_t a = 0; a < r->alts.count; a++) {
                SymbolSeq *seq = &r->alts.items[a];
                StringSet first;
                if (!first_of_sequence(g, (const char *const *)seq->symbols, seq->count, &first)) return 0;
                int n = set_union(&r->first, &first);
                set_free(&first);
                if (n == SET_ERROR) return 0;
                changed |= n;
            }
        }
    }
    if (g->rule_count && set_add(&g->rules[0].follow, "$") == SET_ERROR) return 0;
    changed = 1;
    while (changed) {
        changed = 0;
        for (size_t i = 0; i < g->rule_count; i++) {
            Rule *r = &g->rules[i];
            for (size_t a = 0; a < r->alts.count; a++) {
                SymbolSeq *seq = &r->alts.items[a];
                for (size_t k = 0; k < seq->count; k++) {
                    const char *sym = seq->symbols[k];
                    if (!set_contains(&g->non_terminals, sym)) continue;
                    Rule *target = find_rule(g, sym);
                    if (!target) continue;
                    const char *const *beta = (const char *const *)&seq->symbols[k + 1];
                    size_t beta_count = seq->count - (k + 1);
                    StringSet first;
                    if (!first_of_sequence(g, beta, beta_count, &first)) return 0;
                    int n = set_union(&target->follow, &first);
                    set_free(&first);
                    if (n == SET_ERROR) return 0;
                    changed |= n;
                    if (is_sequence_nullable(g, beta, beta_count)) {
                        n = set_union(&target->follow, &r->follow);
                        if (n == SET_ERROR) return 0;
                        changed |= n;
                    }
                }
            }
        }
    }
    g->calculo_valido = 1;
    return 1;
}

void ll1_conflicts_init(LL1ConflictList *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

void ll1_conflicts_free(LL1ConflictList *list) {
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) {
        if (list->items[i].non_terminal) free(list->items[i].non_terminal);
        set_free(&list->items[i].intersection);
    }
    if (list->items) free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

int ll1_grammar_find_conflicts(LL1Grammar *g, LL1ConflictList *confs) {
    ll1_conflicts_init(confs);
    if (!g->calculo_valido) return 0;
    for (size_t r = 0; r < g->rule_count; r++) {
        Rule *rule = &g->rules[r];
        size_t count = rule->alts.count;
        if (count < 2) continue;
        if (count > SIZE_MAX / sizeof(StringSet)) goto error;
        StringSet *sets = malloc(count * sizeof(*sets));
        if (!sets) goto error;
        size_t initialized = 0;
        for (; initialized < count; initialized++) {
            SymbolSeq *seq = &rule->alts.items[initialized];
            if (!first_of_sequence(g, (const char *const *)seq->symbols, seq->count, &sets[initialized])) goto sets_error;
            if (is_sequence_nullable(g, (const char *const *)seq->symbols, seq->count) &&
                set_union(&sets[initialized], &rule->follow) == SET_ERROR) {
                set_free(&sets[initialized]);
                goto sets_error;
            }
        }
        for (size_t i = 0; i < count; i++) {
            for (size_t j = i + 1; j < count; j++) {
                StringSet inter;
                if (set_intersect(&inter, &sets[i], &sets[j]) == SET_ERROR) { set_free(&inter); goto sets_error; }
                if (!inter.count) { set_free(&inter); continue; }
                if (confs->count >= confs->capacity) {
                    LL1Conflict *items = grow(confs->items, &confs->capacity, sizeof(*items), 4);
                    if (!items) { set_free(&inter); goto sets_error; }
                    confs->items = items;
                }
                char *name = my_strdup(rule->lhs);
                if (!name) { set_free(&inter); goto sets_error; }
                LL1Conflict *c = &confs->items[confs->count++];
                c->non_terminal = name; c->alt1 = i; c->alt2 = j; c->intersection = inter;
            }
        }
        for (size_t i = 0; i < initialized; i++) set_free(&sets[i]);
        free(sets);
        continue;
sets_error:
        for (size_t i = 0; i < initialized; i++) set_free(&sets[i]);
        free(sets);
        goto error;
    }
    return 1;
error:
    ll1_conflicts_free(confs);
    return 0;
}

void ll1_grammar_free(LL1Grammar *g) {
    if (!g) return;
    for (size_t i = 0; i < g->rule_count; i++) {
        rule_free(&g->rules[i]);
    }
    if (g->rules) free(g->rules);
    g->rules = NULL;
    g->rule_count = 0;
    g->rule_capacity = 0;
    g->calculo_valido = 0;
    set_free(&g->non_terminals);
    set_free(&g->nullable);
}
