CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -Iinclude -O2
DEBUG_FLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -Iinclude -g
LDLIBS = -lm

ifeq ($(OS),Windows_NT)
    EXE = .exe
    RM = cmd /C del /Q /F
    RUN = cmd /C
else
    EXE =
    RM = rm -f
    RUN = ./
endif

COMMON_SRC = src/token.c src/lexer.c src/arena.c src/ast.c
EXPR_SRC = $(COMMON_SRC) src/parser_expr.c src/evaluador_expr.c src/consola_expr.c
LL1_SRC = src/set.c src/ll1.c

TARGET_EXPR = calculadora_expr$(EXE)
TARGET_LL1 = verificar_ll1$(EXE)

TEST_PARSER = test_parser_expr$(EXE)
TEST_EVAL = test_evaluador_expr$(EXE)
TEST_CONSOLE = test_consola_expr$(EXE)
TEST_TOKENS = test_tokens$(EXE)
TEST_TOKENS_CLI = test_tokens_cli$(EXE)
TEST_LL1 = test_ll1$(EXE)
TEST_CLI = test_ll1_cli$(EXE)
TESTS = $(TEST_PARSER) $(TEST_EVAL) $(TEST_CONSOLE) $(TEST_TOKENS) $(TEST_TOKENS_CLI) $(TEST_LL1) $(TEST_CLI)

HEADERS = $(wildcard include/*.h)

all: $(TARGET_EXPR) $(TARGET_LL1)

ifeq ($(OS),Windows_NT)
calculadora_expr: $(TARGET_EXPR)
verificar_ll1: $(TARGET_LL1)
endif

$(TARGET_EXPR): $(EXPR_SRC) src/main_expr.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(EXPR_SRC) src/main_expr.c $(LDLIBS)

$(TARGET_LL1): $(LL1_SRC) src/main_verificar_ll1.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(LL1_SRC) src/main_verificar_ll1.c

debug: $(EXPR_SRC) $(LL1_SRC) src/main_expr.c src/main_verificar_ll1.c $(HEADERS)
	$(CC) $(DEBUG_FLAGS) -o $(TARGET_EXPR) $(EXPR_SRC) src/main_expr.c $(LDLIBS)
	$(CC) $(DEBUG_FLAGS) -o $(TARGET_LL1) $(LL1_SRC) src/main_verificar_ll1.c

$(TEST_PARSER): $(COMMON_SRC) src/parser_expr.c tests/test_parser_expr.c $(HEADERS)
	$(CC) $(DEBUG_FLAGS) -o $@ $(COMMON_SRC) src/parser_expr.c tests/test_parser_expr.c

$(TEST_EVAL): $(COMMON_SRC) src/parser_expr.c src/evaluador_expr.c tests/test_evaluador_expr.c $(HEADERS)
	$(CC) $(DEBUG_FLAGS) -o $@ $(COMMON_SRC) src/parser_expr.c src/evaluador_expr.c tests/test_evaluador_expr.c $(LDLIBS)

$(TEST_CONSOLE): $(EXPR_SRC) tests/test_consola_expr.c $(HEADERS)
	$(CC) $(DEBUG_FLAGS) -o $@ $(EXPR_SRC) tests/test_consola_expr.c $(LDLIBS)

$(TEST_TOKENS): $(EXPR_SRC) tests/test_tokens.c $(HEADERS)
	$(CC) $(DEBUG_FLAGS) -o $@ $(EXPR_SRC) tests/test_tokens.c $(LDLIBS)

$(TEST_TOKENS_CLI): $(TARGET_EXPR) tests/test_tokens_cli.c
	$(CC) $(DEBUG_FLAGS) -o $@ tests/test_tokens_cli.c

$(TEST_LL1): $(LL1_SRC) tests/test_ll1.c $(HEADERS)
	$(CC) $(DEBUG_FLAGS) -o $@ $(LL1_SRC) tests/test_ll1.c

$(TEST_CLI): $(TARGET_LL1) tests/test_ll1_cli.c
	$(CC) $(DEBUG_FLAGS) -o $@ tests/test_ll1_cli.c

test: $(TESTS)
	$(RUN)test_parser_expr$(EXE)
	$(RUN)test_evaluador_expr$(EXE)
	$(RUN)test_consola_expr$(EXE)
	$(RUN)test_tokens$(EXE)
	$(RUN)test_tokens_cli$(EXE)
	$(RUN)test_ll1$(EXE)
	$(RUN)test_ll1_cli$(EXE)

clean:
	-$(RM) $(TARGET_EXPR) $(TARGET_LL1) $(TESTS)
	-$(RM) ll1_cli_*.tmp tokens_cli_*.tmp tokens_test_*.tmp

.PHONY: all debug test clean
