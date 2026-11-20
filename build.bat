@echo off
setlocal

set "CFLAGS=-Wall -Wextra -Werror -std=c99 -pedantic -Iinclude -O2"
set "TFLAGS=-Wall -Wextra -Werror -std=c99 -pedantic -Iinclude -g"
set "COMMON=src\token.c src\lexer.c src\arena.c src\ast.c"
set "EXPR=%COMMON% src\parser_expr.c src\evaluador_expr.c src\consola_expr.c"
set "LL1=src\set.c src\ll1.c"

if "%~1"=="" goto all
if /I "%~1"=="all" goto all
if /I "%~1"=="test" goto test
if /I "%~1"=="clean" goto clean
echo Uso: build.bat [all ^| test ^| clean]
exit /b 1

:check
where gcc >nul 2>nul
if errorlevel 1 (
    echo GCC no encontrado. Anada C:\msys64\ucrt64\bin al PATH.
    exit /b 1
)
exit /b 0

:build_all
gcc %CFLAGS% -o calculadora_expr.exe %EXPR% src\main_expr.c -lm
if errorlevel 1 exit /b 1
gcc %CFLAGS% -o verificar_ll1.exe %LL1% src\main_verificar_ll1.c
if errorlevel 1 exit /b 1
exit /b 0

:build_tests
gcc %TFLAGS% -o test_parser_expr.exe %COMMON% src\parser_expr.c tests\test_parser_expr.c
if errorlevel 1 exit /b 1
gcc %TFLAGS% -o test_evaluador_expr.exe %COMMON% src\parser_expr.c src\evaluador_expr.c tests\test_evaluador_expr.c -lm
if errorlevel 1 exit /b 1
gcc %TFLAGS% -o test_consola_expr.exe %EXPR% tests\test_consola_expr.c -lm
if errorlevel 1 exit /b 1
gcc %TFLAGS% -o test_ll1.exe %LL1% tests\test_ll1.c
if errorlevel 1 exit /b 1
exit /b 0

:all
call :check
if errorlevel 1 exit /b 1
call :build_all
if errorlevel 1 exit /b 1
goto done

:test
call :check
if errorlevel 1 exit /b 1
call :build_all
if errorlevel 1 exit /b 1
call :build_tests
if errorlevel 1 exit /b 1
test_parser_expr.exe
if errorlevel 1 exit /b 1
test_evaluador_expr.exe
if errorlevel 1 exit /b 1
test_consola_expr.exe
if errorlevel 1 exit /b 1
test_ll1.exe
if errorlevel 1 exit /b 1
goto done

:clean
if exist calculadora_expr.exe del /Q /F calculadora_expr.exe
if exist verificar_ll1.exe del /Q /F verificar_ll1.exe
if exist test_parser_expr.exe del /Q /F test_parser_expr.exe
if exist test_evaluador_expr.exe del /Q /F test_evaluador_expr.exe
if exist test_consola_expr.exe del /Q /F test_consola_expr.exe
if exist test_ll1.exe del /Q /F test_ll1.exe
goto done

:done
endlocal
exit /b 0
