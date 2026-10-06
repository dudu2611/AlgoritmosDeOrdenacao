"""Confere o agrupamento por entrada, sem usar resultados de desempenho."""
import importlib.util
import math
from pathlib import Path

path = Path(__file__).resolve().parents[1] / "tools" / "analisar.py"
spec = importlib.util.spec_from_file_location("analisar", path)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

# Tres entradas; tempos tecnicos ficticios [1,3], [2,4], [3,5] ns.
# Medias por entrada: 2,3,4. Media final 3 e DP AMOSTRAL 1 ns.
# Esses valores sao somente um teste matematico, nao dados experimentais.
samples = []
for index, mean in enumerate((2.0, 3.0, 4.0), start=1):
    samples.append({
        "scenario": "aleatorio", "algorithm": "insertion", "n": 16,
        "sample": index, "repeats": 2, "input_hash": str(index),
        "time_mean_ns": mean, "technical_std_ns": math.sqrt(2),
        "short_measurements": 2, "comparisons": 10 * index,
        "swaps": 0, "writes_main": 20, "writes_aux": 0, "writes_total": 20,
    })
row = module.summarize(samples)[0]
assert math.isclose(row["time_mean_ms"], 3e-6)
assert math.isclose(row["time_std_ms"], 1e-6)
assert row["samples"] == 3 and row["unique_inputs"] == 3
assert row["comparisons_mean"] == 20 and row["comparisons_std"] == 10
assert row["writes_total_std"] == 0
assert math.isclose(row["cv_percent"], 100 / 3)
assert math.isclose(row["time_per_n_ns"], 3 / 16)
print("OK: media por entrada, desvio amostral, contagens e normalizacao.")
