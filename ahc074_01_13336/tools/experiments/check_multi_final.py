"""Validate the existing final solver; run from WSL with no arguments.

Uses existing inputs and a.out. It does not compile, generate inputs, or alter
main.cpp. Successful official reactive-tester completion checks the protocol,
including that output becomes available at each interaction boundary.
"""

from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
from pathlib import Path
import hashlib
import json
import math
import re
import subprocess
import sys
import time


ROOT = Path(__file__).resolve().parents[2]
EXPERIMENTS = ROOT / "tools" / "experiments"
SOURCE = ROOT / "main.cpp"
BINARY = ROOT / "a.out"
TESTER = ROOT / "tools" / "target" / "release" / "tester"
OUTPUT = EXPERIMENTS / "multi_final_validation.json"
WORKERS = 3
TIMEOUT_SECONDS = 90
BATCHES = (("final_holdout", 5000), ("multi_fresh6000", 6000))


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require_match(pattern, text, description):
    match = re.search(pattern, text)
    assert match is not None, f"Missing {description} in tester diagnostics"
    return match


def validate_case(batch_name, first_seed, index, source_hash, binary_hash):
    seed = first_seed + index
    input_path = EXPERIMENTS / batch_name / "in" / f"{index:04d}.txt"
    row = {
        "batch": batch_name,
        "seed": seed,
        "input_index": index,
        "input_path": str(input_path.relative_to(ROOT)),
        "main_sha256": source_hash,
        "binary_sha256": binary_hash,
        "passed": False,
    }
    started = time.perf_counter()
    diagnostics = ""
    try:
        input_bytes = input_path.read_bytes()
        n, k, p, rounds = map(int, input_bytes.splitlines()[0].split())
        assert (n, k, p, rounds) == (64, 192, 256, 1024)
        result = subprocess.run(
            [str(TESTER), str(BINARY)],
            cwd=ROOT,
            input=input_bytes,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=TIMEOUT_SECONDS,
            check=False,
        )
        diagnostics = result.stderr.decode("utf-8", errors="replace")
        row["tester_returncode"] = result.returncode
        assert result.returncode == 0, diagnostics

        score = int(require_match(r"Score\s*=\s*(\d+)", diagnostics, "score")[1])
        cpu = float(require_match(r"FinishCPU=([\d.eE+-]+)", diagnostics, "CPU time")[1])
        assert math.isfinite(cpu) and cpu >= 0
        selection = require_match(
            r"Round selected=(\d+) states=(\d+) models=(\d+) PrepCPU=([\d.eE+-]+)",
            diagnostics,
            "model-selection summary",
        )
        chosen_model, states, model_count = map(int, selection.group(1, 2, 3))
        prep_cpu = float(selection[4])
        trained_models = [int(value) for value in re.findall(r"Train model=(\d+)", diagnostics)]
        model_evaluations = [
            {"model": int(model), "cost": int(cost), "states": int(count)}
            for model, cost, count in re.findall(
                r"Round model=(\d+) cost=(\d+) states=(\d+)", diagnostics
            )
        ]
        assert 1 <= model_count <= 2
        assert len(trained_models) == model_count
        assert chosen_model in trained_models
        assert 0 <= states <= 128

        lines = result.stdout.splitlines()
        assert len(lines) >= k, "Missing paper designs"
        for pattern in lines[:k]:
            assert len(pattern) == n and set(pattern) <= {ord("0"), ord("1"), ord(".")}
        cursor = k
        total_moves = 0
        maximum_moves = 0
        for request in range(rounds):
            assert cursor < len(lines), f"Missing answer for request {request}"
            count_tokens = lines[cursor].split()
            assert len(count_tokens) == 1, f"Invalid move count at request {request}"
            count = int(count_tokens[0])
            assert 0 <= count <= 2 * k, (request, count)
            cursor += 1
            for _ in range(count):
                assert cursor < len(lines), f"Truncated operation at request {request}"
                operation = lines[cursor].split()
                assert len(operation) == 2, f"Invalid operation at request {request}"
                paper, depth = map(int, operation)
                assert 0 <= paper < k and 0 <= depth < k, (request, paper, depth)
                cursor += 1
            total_moves += count
            maximum_moves = max(maximum_moves, count)
        assert cursor == len(lines), "Extra output after the last request"
        assert score == total_moves, f"Nonzero error penalty: score={score}, moves={total_moves}"
        row.update(
            passed=True,
            score=score,
            total_moves=total_moves,
            errors=0,
            max_moves=maximum_moves,
            requests=rounds,
            reactive_completed=True,
            cpu=cpu,
            prep_cpu=prep_cpu,
            chosen_model=chosen_model,
            models=model_count,
            states=states,
            trained_models=trained_models,
            model_evaluations=model_evaluations,
        )
    except Exception as error:
        row["error"] = f"{type(error).__name__}: {error}"
        if diagnostics:
            row["diagnostics"] = diagnostics
    row["elapsed_seconds"] = time.perf_counter() - started
    return row


def summarize(cases):
    passed = [case for case in cases if case["passed"]]
    summary = {"count": len(cases), "passed": len(passed), "failed": len(cases) - len(passed)}
    if passed:
        summary.update(
            mean_score=sum(case["score"] for case in passed) / len(passed),
            minimum_score=min(case["score"] for case in passed),
            maximum_score=max(case["score"] for case in passed),
            errors=sum(case["errors"] for case in passed),
            max_moves=max(case["max_moves"] for case in passed),
            max_cpu=max(case["cpu"] for case in passed),
            mean_cpu=sum(case["cpu"] for case in passed) / len(passed),
            minimum_states=min(case["states"] for case in passed),
            maximum_states=max(case["states"] for case in passed),
            chosen_models={str(model): sum(case["chosen_model"] == model for case in passed)
                           for model in sorted({case["chosen_model"] for case in passed})},
        )
    return summary


def main():
    assert len(sys.argv) == 1, "This validation uses fixed inputs and accepts no arguments"
    for path in (SOURCE, BINARY, TESTER):
        assert path.is_file(), f"Missing {path}"
    source_hash, binary_hash = sha256(SOURCE), sha256(BINARY)
    report = {
        "started_utc": datetime.now(timezone.utc).isoformat(),
        "main_sha256": source_hash,
        "binary_sha256": binary_hash,
        "workers": WORKERS,
        "batches": [],
        "cases": [],
    }
    for batch_name, first_seed in BATCHES:
        seeds = [int(line) for line in (EXPERIMENTS / batch_name / "seeds.txt").read_text().split()]
        assert seeds == list(range(first_seed, first_seed + 50)), (batch_name, seeds)
        assert sha256(SOURCE) == source_hash and sha256(BINARY) == binary_hash, "Solver changed during validation"
        with ThreadPoolExecutor(max_workers=WORKERS) as pool:
            cases = list(pool.map(
                lambda index: validate_case(batch_name, first_seed, index, source_hash, binary_hash),
                range(50),
            ))
        assert sha256(SOURCE) == source_hash and sha256(BINARY) == binary_hash, "Solver changed during validation"
        summary = {"batch": batch_name, "seed_first": first_seed, "seed_last": first_seed + 49,
                   **summarize(cases)}
        report["batches"].append(summary)
        report["cases"].extend(cases)
        report["summary"] = summarize(report["cases"])
        OUTPUT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(summary, ensure_ascii=False), flush=True)
    report["finished_utc"] = datetime.now(timezone.utc).isoformat()
    OUTPUT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    assert report["summary"]["failed"] == 0, f"Validation failed; see {OUTPUT}"


if __name__ == "__main__":
    main()
