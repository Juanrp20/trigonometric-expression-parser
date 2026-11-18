"""Pruebas de integración del ejecutable LL(1); requiere Python 3."""
import os
import subprocess
import tempfile

exe = './toy_ll1.exe' if os.name == 'nt' else './toy_ll1'
for archivo in ('gramatica/expresiones_matematicas.txt', 'gramatica/toy_gramatica.txt'):
    resultado = subprocess.run([exe, archivo, '--sets'], capture_output=True, text=True, encoding='utf-8')
    assert resultado.returncode == 0 and 'ninguno' in resultado.stdout
    assert 'PRIMERO(' in resultado.stdout and 'SIGUIENTE(' in resultado.stdout
resultado = subprocess.run([exe, 'gramatica/toy_gramatica_original.txt'], capture_output=True, text=True, encoding='utf-8')
assert resultado.returncode == 1 and resultado.stdout.count('Conflicto en') == 2
with tempfile.TemporaryDirectory() as carpeta:
    for contenido in ('', '<S> -> [ ID', '<S> -> <N>'):
        nombre = os.path.join(carpeta, 'gramatica.txt')
        with open(nombre, 'w', encoding='utf-8') as archivo:
            archivo.write(contenido)
        resultado = subprocess.run([exe, nombre], capture_output=True, text=True, encoding='utf-8')
        assert resultado.returncode == 2 and 'gramática vacía o mal formada' in resultado.stderr
print('OK CLI LL(1): códigos, conflictos, gramáticas inválidas y conjuntos')
