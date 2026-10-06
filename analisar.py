#!/usr/bin/env python3
"""Resume um ensaio completo. CSV: biblioteca padrao; graficos: matplotlib."""
import argparse
import csv
import math
import statistics as stats
from collections import defaultdict
from pathlib import Path

COUNTS = ("comparisons", "swaps", "writes_main", "writes_aux")
ALGORITHMS = ("insertion", "selection", "merge", "heap", "quick")
COLORS = ("#b45309", "#be123c", "#047857", "#7c3aed", "#0369a1")


def write_csv(path, rows):
    if not rows:
        raise ValueError(f"Sem dados para {path}")
    with path.open("x", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def metadata_for(raw):
    if not raw.name.endswith("_raw.csv"):
        raise ValueError("Use o CSV original terminado em _raw.csv, sem editar seu nome.")
    path = raw.with_name(raw.name[:-8] + "_meta.txt")
    values = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if "=" in line and not line.startswith("  "):
            key, value = line.split("=", 1)
            values[key] = value
    if values.get("completed") != "yes":
        raise ValueError("Ensaio interrompido/incompleto: falta completed=yes no _meta.txt.")
    if values.get("protocol") != "ordenacao-c23-v1":
        raise ValueError("Protocolo desconhecido.")
    return values


def read_samples(raw, metadata):
    groups = defaultdict(list)
    seen = set()
    pair_inputs = {}
    positions = defaultdict(set)
    algorithms_by_cell = defaultdict(set)
    count_rows = 0
    with raw.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        required = {"algorithm", "scenario", "n", "sample", "repeat", "seed_hex",
                    "input_hash", "order", "time_ns", "validated", "short_measurement", *COUNTS}
        if not required.issubset(reader.fieldnames or []):
            raise ValueError("Cabecalho CSV invalido.")
        for row in reader:
            if row["algorithm"] not in ALGORITHMS:
                raise ValueError("Algoritmo desconhecido.")
            for name in ("n", "sample", "repeat", "order", "short_measurement", *COUNTS):
                row[name] = int(row[name])
                if row[name] < 0:
                    raise ValueError(f"Valor negativo em {name}.")
            row["time_ns"] = float(row["time_ns"])
            if not math.isfinite(row["time_ns"]) or row["time_ns"] < 0:
                raise ValueError("Tempo invalido.")
            if row["validated"] != "1" or row["n"] < 2:
                raise ValueError("Ordenacao nao validada ou tamanho invalido.")
            if row["short_measurement"] not in (0, 1):
                raise ValueError("Marcador short_measurement invalido.")
            key = (row["scenario"], row["algorithm"], row["n"], row["sample"])
            key_rep = (*key, row["repeat"])
            if key_rep in seen:
                raise ValueError("Linhas duplicadas. Nao concatene sessoes neste arquivo.")
            seen.add(key_rep)
            pair = (row["scenario"], row["n"], row["sample"])
            identity = (row["seed_hex"], row["input_hash"])
            if pair in pair_inputs and pair_inputs[pair] != identity:
                raise ValueError("Algoritmos/repeticoes receberam entradas diferentes.")
            pair_inputs[pair] = identity
            round_key = (*pair, row["repeat"])
            if row["order"] in positions[round_key]:
                raise ValueError("Ordem duplicada na mesma rodada.")
            positions[round_key].add(row["order"])
            algorithms_by_cell[(row["scenario"], row["n"])].add(row["algorithm"])
            groups[key].append(row)
            count_rows += 1
    if count_rows != int(metadata["rows"]):
        raise ValueError("Total de linhas difere dos metadados.")
    expected_repeats = int(metadata["repeats"])
    expected_samples = int(metadata["samples"])
    all_algorithms = {key[1] for key in groups}
    for members in algorithms_by_cell.values():
        if members != all_algorithms:
            raise ValueError("Faltam algoritmos em alguma celula.")
    for used in positions.values():
        if used != set(range(1, len(all_algorithms) + 1)):
            raise ValueError("Faltam cronometragens em alguma rodada.")
    samples_by_cell = defaultdict(set)
    samples = []
    for (scenario, algorithm, n, sample), rows in sorted(groups.items()):
        if {r["repeat"] for r in rows} != set(range(1, expected_repeats + 1)):
            raise ValueError("Faltam repeticoes tecnicas.")
        for name in COUNTS:
            if len({r[name] for r in rows}) != 1:
                raise ValueError("Contagem mudou entre repeticoes da mesma entrada.")
        times = [r["time_ns"] for r in rows]
        record = {
            "scenario": scenario, "algorithm": algorithm, "n": n, "sample": sample,
            "seed_hex": rows[0]["seed_hex"], "input_hash": rows[0]["input_hash"],
            "repeats": len(rows), "time_mean_ns": stats.mean(times),
            "technical_std_ns": stats.stdev(times) if len(times) > 1 else "",
            "time_min_ns": min(times), "time_max_ns": max(times),
            "short_measurements": sum(r["short_measurement"] for r in rows),
        }
        record.update({name: rows[0][name] for name in COUNTS})
        record["writes_total"] = record["writes_main"] + record["writes_aux"]
        samples.append(record)
        samples_by_cell[(scenario, algorithm, n)].add(sample)
    for ids in samples_by_cell.values():
        if ids != set(range(1, expected_samples + 1)):
            raise ValueError("Faltam amostras/rodadas.")
    return samples


def summarize(samples):
    cells = defaultdict(list)
    for sample in samples:
        cells[(sample["scenario"], sample["algorithm"], sample["n"])].append(sample)
    result = []
    for (scenario, algorithm, n), rows in sorted(cells.items()):
        times = [r["time_mean_ns"] for r in rows]
        mean = stats.mean(times)
        std = stats.stdev(times)
        row = {
            "scenario": scenario, "algorithm": algorithm, "n": n,
            "samples": len(rows), "repeats_per_sample": rows[0]["repeats"],
            "unique_inputs": len({r["input_hash"] for r in rows}),
            "time_mean_ms": mean / 1e6, "time_std_ms": std / 1e6,
            "time_median_ms": stats.median(times) / 1e6,
            "time_min_ms": min(times) / 1e6, "time_max_ms": max(times) / 1e6,
            "cv_percent": 100 * std / mean if mean > 0 else "",
            "technical_std_mean_ms": (
                stats.mean(r["technical_std_ns"] for r in rows) / 1e6
                if rows[0]["repeats"] > 1 else ""),
            "short_measurements": sum(r["short_measurements"] for r in rows),
        }
        for name in (*COUNTS, "writes_total"):
            values = [r[name] for r in rows]
            row[name + "_mean"] = stats.mean(values)
            row[name + "_std"] = stats.stdev(values)
        row.update({
            "time_per_n_ns": mean / n,
            "time_per_nlog2n_ns": mean / (n * math.log2(n)),
            "time_per_n2_ns": mean / (n * n),
            "comparisons_per_nlog2n": row["comparisons_mean"] / (n * math.log2(n)),
            "comparisons_per_n2": row["comparisons_mean"] / (n * n),
        })
        result.append(row)
    return result


def growth_ratios(summary):
    groups = defaultdict(list)
    for row in summary:
        groups[(row["scenario"], row["algorithm"])].append(row)
    result = []
    for (scenario, algorithm), rows in sorted(groups.items()):
        rows.sort(key=lambda r: r["n"])
        for previous, current in zip(rows, rows[1:]):
            n0, n1 = previous["n"], current["n"]
            t0, t1 = previous["time_mean_ms"], current["time_mean_ms"]
            c0, c1 = previous["comparisons_mean"], current["comparisons_mean"]
            result.append({
                "scenario": scenario, "algorithm": algorithm,
                "n_previous": n0, "n": n1, "size_ratio": n1 / n0,
                "time_ratio": t1 / t0 if t0 > 0 else "",
                "comparisons_ratio": c1 / c0 if c0 > 0 else "",
                "expected_linear_ratio": n1 / n0,
                "expected_nlogn_ratio": n1 * math.log2(n1) / (n0 * math.log2(n0)),
                "expected_quadratic_ratio": (n1 / n0) ** 2,
            })
    return result


def plots(summary, output):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    plt.rcParams.update({"font.size": 10, "axes.spines.top": False,
                         "axes.spines.right": False, "savefig.dpi": 180})
    scenarios = sorted({row["scenario"] for row in summary})
    specifications = [
        ("tempo", "time_mean_ms", "Tempo por ordenação (ms)", False, "Média ± desvio padrão amostral"),
        ("tempo_log", "time_mean_ms", "Tempo por ordenação (ms), escala log", True, "Médias; desvios disponíveis em resumo.csv"),
        ("comparacoes", "comparisons_mean", "Comparações entre chaves", True, "Médias por entrada"),
        ("escritas", "writes_total_mean", "Escritas no vetor + auxiliar", True, "Médias por entrada"),
    ]
    for scenario in scenarios:
        for stem, metric, ylabel, log_y, subtitle in specifications:
            fig, ax = plt.subplots(figsize=(9.2, 5.4), layout="constrained")
            for algorithm, color in zip(ALGORITHMS, COLORS):
                rows = sorted((r for r in summary if r["scenario"] == scenario
                               and r["algorithm"] == algorithm), key=lambda r: r["n"])
                if log_y:
                    rows = [r for r in rows if r[metric] > 0]
                if not rows:
                    continue
                x = [r["n"] for r in rows]
                y = [r[metric] for r in rows]
                if stem == "tempo":
                    ax.errorbar(x, y, yerr=[r["time_std_ms"] for r in rows],
                                marker="o", markersize=4, capsize=3,
                                label=algorithm, color=color, linewidth=1.7)
                else:
                    ax.plot(x, y, marker="o", markersize=4,
                            label=algorithm, color=color, linewidth=1.7)
            ax.set_xscale("log", base=2)
            if log_y:
                ax.set_yscale("log")
            ax.set_xlabel("Número de elementos (n), escala log₂")
            ax.set_ylabel(ylabel)
            ax.set_title(scenario.replace("_", " ").capitalize() + "\n" + subtitle,
                         loc="left", pad=14, fontweight="bold")
            ax.grid(True, which="major", alpha=0.23)
            ax.legend(frameon=False, ncol=3)
            for extension in ("png", "svg"):
                fig.savefig(output / f"{stem}_{scenario}.{extension}")
            plt.close(fig)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("raw", type=Path)
    parser.add_argument("--out", type=Path, required=True, help="Diretorio novo ou vazio")
    parser.add_argument("--plots", action="store_true", help="Tambem exporta PNG e SVG")
    args = parser.parse_args()
    try:
        metadata = metadata_for(args.raw)
        samples = read_samples(args.raw, metadata)
        summary = summarize(samples)
        ratios = growth_ratios(summary)
        if args.plots:
            try:
                import matplotlib  # noqa: F401
            except ImportError as error:
                raise ValueError("Para graficos, instale matplotlib ou remova --plots.") from error
        if args.out.exists() and any(args.out.iterdir()):
            raise ValueError("Diretorio de saida deve estar vazio; preserve a analise anterior.")
        args.out.mkdir(parents=True, exist_ok=True)
        write_csv(args.out / "amostras.csv", samples)
        write_csv(args.out / "resumo.csv", summary)
        if ratios:
            write_csv(args.out / "crescimento.csv", ratios)
        note = (
            f"Fonte: {args.raw.resolve()}\n"
            f"Celulas: {len(summary)}; amostras por celula: {metadata['samples']}.\n"
            "Estatistica: primeiro media das repeticoes da mesma entrada; depois\n"
            "media e desvio padrao amostral (divisor S-1) dessas medias.\n"
            "Barras de erro = 1 desvio padrao; nao sao intervalo de confianca.\n"
            "Minimo, maximo e mediana do resumo referem-se as medias por amostra.\n"
            "Crescente, decrescente e iguais repetem a mesma entrada para cada n.\n"
            "unique_inputs conta hashes distintos, nao prova independencia estatistica.\n"
            "Nao houve remocao de outliers, subtracao de overhead ou arredondamento manual.\n"
            "Valores de tempo nulos sao preservados nos CSVs e omitidos nos graficos log.\n"
        )
        with (args.out / "LEIA-ME.txt").open("x", encoding="utf-8") as stream:
            stream.write(note)
        if args.plots:
            plots(summary, args.out)
        print(f"Analise salva em {args.out.resolve()}")
        short = sum(row["short_measurements"] for row in summary)
        if short:
            print(f"ATENCAO: {short} tempos muito curtos para o criterio do relogio; veja o guia.")
    except (OSError, ValueError, KeyError, stats.StatisticsError) as error:
        parser.exit(1, f"Erro: {error}\n")


if __name__ == "__main__":
    main()
