#!/usr/bin/env python3
import re
import subprocess
import sys
from pathlib import Path

raiz = Path(__file__).resolve().parents[1]
setup = (raiz / "setup.h").read_text(encoding="utf-8")

match = re.search(r"CHAVES_RODADAS\[25\]\[26\]\s*=\s*\{(.*?)\n\};", setup, re.S)
if not match:
    raise SystemExit("Nao foi possivel localizar CHAVES_RODADAS em setup.h")

esperado = [list(map(int, re.findall(r"\d+", linha)))
            for linha in re.findall(r"\{([^{}]+)\}", match.group(1))]

proc = subprocess.run(
    [str(raiz / "gerador_tabela")],
    cwd=raiz,
    check=True,
    text=True,
    capture_output=True,
)
obtido = [list(map(int, re.findall(r"\d+", linha)))
          for linha in re.findall(r"\{([^{}]+)\}", proc.stdout)]

if obtido != esperado:
    print("A matriz gerada difere de CHAVES_RODADAS.", file=sys.stderr)
    sys.exit(1)

print("Gerador reproduz exatamente CHAVES_RODADAS (25 x 26).")
