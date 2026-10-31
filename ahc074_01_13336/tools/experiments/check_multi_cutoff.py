"""Run from WSL: python3 tools/experiments/check_multi_cutoff.py.

Builds a snapshot of current main.cpp and checks the five requested budget
pairs on official seeds 0, 50, and 99. Runs sequentially to avoid affecting
CPU-clock cutoffs through contention. Optional example for partial selection:
  python3 tools/experiments/check_multi_cutoff.py --budgets 1/1.05
UBSan mode builds with instrumentation and runs seed 0 with budgets 10/20:
  python3 tools/experiments/check_multi_cutoff.py --ubsan
The script does not modify main.cpp.
"""

from pathlib import Path
import argparse
import hashlib
import json
import math
import re
import subprocess


ROOT = Path(__file__).resolve().parents[2]
EXPERIMENTS = ROOT / "tools/experiments"


def parse_budget(text):
    parts = text.split("/")
    if len(parts) != 2:
        raise argparse.ArgumentTypeError("budget must be LEARNING/SELECTION")
    try:
        numbers = [float(x) for x in parts]
    except ValueError as error:
        raise argparse.ArgumentTypeError(str(error)) from error
    if any(not math.isfinite(x) or x < 0 or x > 60 for x in numbers):
        raise argparse.ArgumentTypeError("budgets must be between 0 and 60 seconds")
    return tuple(format(x, ".9g") for x in numbers)


def validate_transcript(input_bytes, transcript, official_score):
    """Independently replay every move, including the visible-paper rule."""
    given = input_bytes.split()
    n, k_count, products, rounds = map(int, given[:4])
    assert (n, k_count, products, rounds) == (64, 192, 256, 1024)
    targets = []
    for barcode in given[4:4 + products]:
        assert len(barcode) == n and set(barcode) <= {48, 49}
        targets.append(sum(1 << j for j, c in enumerate(barcode) if c == 49))
    queries = list(map(int, given[4 + products:]))
    assert len(queries) == rounds

    tokens = transcript.split()
    assert len(tokens) >= k_count, "missing design"
    masks, colors = [], []
    for pattern in tokens[:k_count]:
        assert len(pattern) == n and set(pattern) <= {46, 48, 49}, "invalid paper"
        masks.append(sum(1 << j for j, c in enumerate(pattern) if c != 46))
        colors.append(sum(1 << j for j, c in enumerate(pattern) if c == 49))

    order = list(range(k_count))
    at, total, maximum, errors = k_count, 0, 0, 0
    for r, product in enumerate(queries):
        assert 0 <= product < products
        assert at < len(tokens), (r, "missing move count")
        moves = int(tokens[at])
        at += 1
        assert 0 <= moves <= 2 * k_count, (r, "move limit", moves)
        maximum = max(maximum, moves)
        total += moves
        assert at + 2 * moves <= len(tokens), (r, "missing move")
        for i in range(moves):
            paper, destination = int(tokens[at]), int(tokens[at + 1])
            at += 2
            assert 0 <= paper < k_count and 0 <= destination < k_count, (r, i)
            position = order.index(paper)
            covered = 0
            for previous in order[:position]:
                covered |= masks[previous]
            assert masks[paper] & ~covered, (r, i, "paper not visible", paper)
            order.pop(position)
            order.insert(destination, paper)

        covered = display = 0
        for paper in order:
            display |= colors[paper] & masks[paper] & ~covered
            covered |= masks[paper]
        errors += display != targets[product]

    assert at == len(tokens), "extra output after final order"
    assert official_score == total + 2 * k_count * errors, (official_score, total, errors)
    assert errors == 0, ("incorrect orders", errors)
    return total, maximum


def run_case(binary, learning, selection, seed):
    input_bytes = (ROOT / f"tools/in/{seed:04d}.txt").read_bytes()
    process = subprocess.run(
        [str(ROOT / "tools/target/release/tester"), str(binary)],
        input=input_bytes, capture_output=True, cwd=ROOT, timeout=300,
    )
    diagnostic = process.stderr.decode(errors="replace")
    assert process.returncode == 0, (learning, selection, seed, diagnostic)
    score_match = re.search(r"Score = (\d+)", diagnostic)
    selected_match = re.search(
        r"Round selected=(\d+) states=(\d+) models=(\d+) PrepCPU=([\d.eE+-]+)",
        diagnostic,
    )
    cpu_match = re.search(r"FinishCPU=([\d.eE+-]+)", diagnostic)
    assert score_match and selected_match and cpu_match, diagnostic
    score = int(score_match[1])
    total, maximum = validate_transcript(input_bytes, process.stdout, score)
    model_id, states, models = map(int, selected_match.group(1, 2, 3))
    assert models in (1, 2) and 0 <= states <= 128
    if models == 1:
        assert states == 0, "single-model simulation should be skipped"
    comparisons = re.findall(r"Round model=\d+ cost=(\d+) states=(\d+)", diagnostic)
    if models == 2:
        assert len(comparisons) == 2
        assert all(int(count) == states for _, count in comparisons), "unequal sample counts"
        if states == 0:
            assert all(int(cost) == 0 for cost, _ in comparisons)
    return dict(
        learning=float(learning), selection=float(selection), seed=seed,
        score=score, moves=total, errors=0, max_moves=maximum,
        selected_model=model_id, models=models, states=states,
        prep_cpu=float(selected_match[4]), cpu=float(cpu_match[1]),
        diagnostics=diagnostic,
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--budgets", nargs="+", type=parse_budget,
                        default=[("0", "0"), ("0.1", "0"), ("1", "0"),
                                 ("0.1", "0.15"), ("1", "0.7")])
    parser.add_argument("--seeds", nargs="+", type=int, default=[0, 50, 99])
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--ubsan", action="store_true",
                        help="Use UBSan, budgets 10/20, and seed 0 only")
    args = parser.parse_args()
    if args.ubsan:
        args.budgets = [("10", "20")]
        args.seeds = [0]

    source = (ROOT / "main.cpp").read_bytes()
    digest = hashlib.sha256(source).hexdigest()
    snapshot = EXPERIMENTS / "multi_cutoff_source.cpp"
    snapshot.write_bytes(source)
    print("Source SHA256:", digest, flush=True)
    rows = []
    for learning, selection in args.budgets:
        label = f"{learning}_{selection}".replace(".", "p")
        if args.ubsan:
            label += "_ubsan"
        binary = EXPERIMENTS / f"multi_cutoff_{label}"
        flags = (["-O1", "-fsanitize=undefined", "-fno-sanitize-recover=all"]
                 if args.ubsan else ["-O2"])
        subprocess.run([
            args.compiler, "-std=c++17", *flags,
            f"-DAHC_LEARNING_BUDGET={learning}",
            f"-DAHC_SELECTION_BUDGET={selection}",
            str(snapshot), "-o", str(binary),
        ], cwd=ROOT, check=True)
        for seed in args.seeds:
            row = run_case(binary, learning, selection, seed)
            rows.append(row)
            print(
                f"budgets={learning}/{selection} seed={seed:04d} "
                f"score={row['score']} E=0 max_moves={row['max_moves']} "
                f"models={row['models']} states={row['states']} "
                f"cpu={row['cpu']:.6f}", flush=True,
            )
    result = dict(source_sha256=digest, ubsan=args.ubsan, cases=rows,
                  partial_selection_cases=sum(0 < r["states"] < 128 for r in rows))
    output = EXPERIMENTS / ("multi_cutoff_ubsan_results.json" if args.ubsan
                            else "multi_cutoff_results.json")
    output.write_text(json.dumps(result, indent=2))
    print(f"Validated {len(rows)} cases: every move legal, E=0; results: {output}", flush=True)


if __name__ == "__main__":
    main()
