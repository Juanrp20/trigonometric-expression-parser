CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -Iinclude -O2

ifeq ($(OS),Windows_NT)
    # En Windows (MinGW), libasan y libubsan normalmente no vienen por defecto
    DEBUG_FLAGS = -Wall -Wextra -std=c99 -pedantic -Iinclude -g
    EXE = .exe
    RM = cmd /C del /Q /F
    RUN_TEST = cmd /C $(TEST_BIN)
    RUN_EXPR_TEST = cmd /C $(TEST_EXPR_BIN)
    RUN_EVAL_TEST = cmd /C $(TEST_EVAL_BIN)
    RUN_CONSOLE_TEST = cmd /C $(TEST_CONSOLE_BIN)
else
    DEBUG_FLAGS = -Wall -Wextra -std=c99 -pedantic -Iinclude -g -fsanitize=address,undefined
    EXE =
    RM = rm -f
    RUN_TEST = ./$(TEST_BIN)
    RUN_EXPR_TEST = ./$(TEST_EXPR_BIN)
    RUN_EVAL_TEST = ./$(TEST_EVAL_BIN)
    RUN_CONSOLE_TEST = ./$(TEST_CONSOLE_BIN)
endif

SRC = src/token.c src/lexer.c src/arena.c src/ast.c src/parser.c
LL1_SRC = src/set.c src/ll1.c
MAIN_SRC = src/main.c
MAIN_LL1_SRC = src/main_ll1.c
TEST_SRC = tests/test_c.c
EXPR_SRC = src/token.c src/lexer.c src/arena.c src/ast.c src/parser_expr.c
TEST_EXPR_SRC = tests/test_parser_expr.c
EVAL_SRC = $(EXPR_SRC) src/evaluador_expr.c
TEST_EVAL_SRC = tests/test_evaluador_expr.c
CONSOLE_SRC = $(EVAL_SRC) src/consola_expr.c
TEST_CONSOLE_SRC = tests/test_consola_expr.c
HEADERS = $(wildcard include/*.h)
LDLIBS = -lm

TARGET = toy_c$(EXE)
TARGET_LL1 = toy_ll1$(EXE)
TEST_BIN = test_c$(EXE)
TEST_EXPR_BIN = test_parser_expr$(EXE)
TEST_EVAL_BIN = test_evaluador_expr$(EXE)
TEST_CONSOLE_BIN = test_consola_expr$(EXE)
TARGET_EXPR = calculadora_expr$(EXE)

all: $(TARGET) $(TARGET_LL1) $(TARGET_EXPR)

ifeq ($(OS),Windows_NT)
calculadora_expr: $(TARGET_EXPR)
toy_c: $(TARGET)
toy_ll1: $(TARGET_LL1)
endif

$(TARGET): $(SRC) $(MAIN_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(MAIN_SRC)

$(TARGET_LL1): $(LL1_SRC) $(MAIN_LL1_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(LL1_SRC) $(MAIN_LL1_SRC)

$(TARGET_EXPR): $(CONSOLE_SRC) src/main_expr.c $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $(CONSOLE_SRC) src/main_expr.c $(LDLIBS)

debug: $(SRC) $(LL1_SRC) $(MAIN_SRC) $(MAIN_LL1_SRC) $(CONSOLE_SRC) src/main_expr.c
	$(CC) $(DEBUG_FLAGS) -o $(TARGET) $(SRC) $(MAIN_SRC)
	$(CC) $(DEBUG_FLAGS) -o $(TARGET_LL1) $(LL1_SRC) $(MAIN_LL1_SRC)
	$(CC) $(DEBUG_FLAGS) -o $(TARGET_EXPR) $(CONSOLE_SRC) src/main_expr.c $(LDLIBS)

$(TEST_BIN): $(SRC) $(LL1_SRC) $(TEST_SRC) $(HEADERS)
	$(CC) $(DEBUG_FLAGS) -o $@ $(SRC) $(LL1_SRC) $(TEST_SRC)

$(TEST_EXPR_BIN): $(EXPR_SRC) $(TEST_EXPR_SRC) $(HEADERS)
	$(CC) $(DEBUG_FLAGS) -o $@ $(EXPR_SRC) $(TEST_EXPR_SRC)

$(TEST_EVAL_BIN): $(EVAL_SRC) $(TEST_EVAL_SRC) $(HEADERS)
	$(CC) $(DEBUG_FLAGS) -o $@ $(EVAL_SRC) $(TEST_EVAL_SRC) $(LDLIBS)

$(TEST_CONSOLE_BIN): $(CONSOLE_SRC) $(TEST_CONSOLE_SRC) $(HEADERS)
	$(CC) $(DEBUG_FLAGS) -o $@ $(CONSOLE_SRC) $(TEST_CONSOLE_SRC) $(LDLIBS)

test: $(TEST_BIN) $(TEST_EXPR_BIN) $(TEST_EVAL_BIN) $(TEST_CONSOLE_BIN)
	$(RUN_TEST)
	$(RUN_EXPR_TEST)
	$(RUN_EVAL_TEST)
	$(RUN_CONSOLE_TEST)

clean:
	-$(RM) $(TARGET) $(TARGET_LL1) $(TARGET_EXPR) $(TEST_BIN) $(TEST_EXPR_BIN) $(TEST_EVAL_BIN) $(TEST_CONSOLE_BIN)

.PHONY: all debug test clean


