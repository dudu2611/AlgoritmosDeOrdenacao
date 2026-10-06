#!/usr/bin/env python3
"""Registra sistema, compilador e SHA-256 dos fontes/executavel, sem dependencias."""
import argparse
import hashlib
import json
import os
import platform
import subprocess
from datetime import datetime, timezone
from pathlib import Path


def command_output(command):
    try:
        result = subprocess.run(command, capture_output=True, text=True,
                                errors="replace", timeout=15, check=False)
        return {"command": command, "returncode": result.returncode,
                "stdout": result.stdout.strip(), "stderr": result.stderr.strip()}
    except (OSError, subprocess.TimeoutExpired) as error:
        return {"command": command, "error": str(error)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--cc", default="gcc")
    parser.add_argument("--flags", default="-std=c23 -O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow",
                        help="Flags declaradas por voce; nao sao inferidas do binario")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    data = {
        "utc": datetime.now(timezone.utc).isoformat(),
        "system": platform.system(), "release": platform.release(),
        "version": platform.version(), "architecture": platform.machine(),
        "processor": platform.processor() or os.environ.get("PROCESSOR_IDENTIFIER", ""),
        "logical_cpus": os.cpu_count(), "python": platform.python_version(),
        "compiler_version": command_output([args.cc, "--version"]),
        "compiler_target": command_output([args.cc, "-dumpmachine"]),
        "declared_flags": args.flags,
        "flags_note": "Declaradas pelo usuario; confira o comando de compilacao.",
        "fill_manually": {"ram_gb": "", "power_plan": "", "on_ac_power": "",
                          "background_apps": "", "room_conditions": "", "notes": ""},
    }
    if platform.system() == "Windows":
        data["cpu_details"] = command_output([
            "powershell.exe", "-NoProfile", "-Command",
            "Get-CimInstance Win32_Processor | Select-Object Name,NumberOfCores,NumberOfLogicalProcessors | ConvertTo-Json"
        ])
    elif Path("/proc/cpuinfo").is_file():
        text = Path("/proc/cpuinfo").read_text(errors="replace")
        data["cpu_model"] = next((line.split(":", 1)[1].strip() for line in text.splitlines()
                                  if line.startswith("model name")), "")
    files = [*sorted((root / "src").glob("*")), *sorted((root / "tools").glob("*.py")),
             root / "Makefile", root / "benchmark.exe"]
    data["sha256"] = {str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
                      for path in files if path.is_file()}
    try:
        with args.out.open("x", encoding="utf-8") as stream:
            json.dump(data, stream, indent=2, ensure_ascii=False)
            stream.write("\n")
    except OSError as error:
        parser.exit(1, f"Erro ao salvar (diretorio deve existir e arquivo ser novo): {error}\n")
    print(f"Ambiente registrado em {args.out.resolve()}")


if __name__ == "__main__":
    main()
