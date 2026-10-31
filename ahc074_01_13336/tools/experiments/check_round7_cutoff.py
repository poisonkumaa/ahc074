"""Sequential cutoff/replay checks for a snapshot of current main.cpp.

Default: five CPU-budget pairs on official seeds 0, 50, 99 (15 cases).
--ubsan: budgets 20/30, seed 0, and a complete adaptive R=32 refinement.
Only round7 experiment artifacts are written; main.cpp/a.out are untouched.
"""

from pathlib import Path
import argparse
import hashlib
import json
import re
import subprocess

from check_multi_cutoff import validate_transcript


ROOT = Path(__file__).resolve().parents[2]
EXPERIMENTS = ROOT / "tools/experiments"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ubsan", action="store_true")
    args = parser.parse_args()
    budgets = [("20", "30")] if args.ubsan else [
        ("0", "0"), ("0.1", "0.15"), ("0.5", "0.6"),
        ("0.5", "0.9"), ("0.5", "1.2"),
    ]
    seeds = [0] if args.ubsan else [0, 50, 99]
    source = (ROOT / "main.cpp").read_bytes()
    digest = hashlib.sha256(source).hexdigest()
    binary_digest = hashlib.sha256((ROOT / "a.out").read_bytes()).hexdigest()
    snapshot = EXPERIMENTS / "round7_cutoff_source.cpp"
    snapshot.write_bytes(source)
    output = EXPERIMENTS / ("round7_ubsan_results.json" if args.ubsan
                            else "round7_cutoff_results.json")
    result = dict(source_sha256=digest, a_out_sha256=binary_digest,
                  ubsan=args.ubsan, status="running", cases=[])

    def unchanged():
        assert hashlib.sha256((ROOT / "main.cpp").read_bytes()).hexdigest() == digest, "main.cpp changed during validation"
        assert hashlib.sha256((ROOT / "a.out").read_bytes()).hexdigest() == binary_digest, "a.out changed during validation"

    def save():
        output.write_text(json.dumps(result, indent=2))

    print("Source SHA256:", digest, flush=True)
    save()
    try:
        for learning, refinement in budgets:
            unchanged()
            label = f"{learning}_{refinement}".replace(".", "p")
            if args.ubsan:
                label += "_ubsan"
            binary = EXPERIMENTS / f"round7_cutoff_{label}"
            flags = (["-O1", "-fsanitize=undefined", "-fno-sanitize-recover=all"]
                     if args.ubsan else ["-O2"])
            subprocess.run([
                "g++", "-std=c++17", *flags,
                f"-DAHC_LEARNING_BUDGET={learning}",
                f"-DAHC_REFINEMENT_BUDGET={refinement}",
                str(snapshot), "-o", str(binary),
            ], cwd=ROOT, check=True)
            for seed in seeds:
                unchanged()
                data = (ROOT / f"tools/in/{seed:04d}.txt").read_bytes()
                process = subprocess.run(
                    [str(ROOT / "tools/target/release/tester"), str(binary)],
                    input=data, capture_output=True, cwd=ROOT, timeout=300,
                )
                diagnostics = process.stderr.decode(errors="replace")
                assert process.returncode == 0, (learning, refinement, seed, diagnostics)
                assert "runtime error:" not in diagnostics, diagnostics
                score_match = re.search(r"Score = (\d+)", diagnostics)
                cpu_match = re.search(r"FinishCPU=([\d.eE+-]+)", diagnostics)
                prep_match = re.search(r"PrepCPU=([\d.eE+-]+)", diagnostics)
                fallback_match = re.search(r"PathFallbacks=(\d+)", diagnostics)
                adaptive_match = re.search(r"AdaptiveR=(\d+)", diagnostics)
                assert score_match and cpu_match and prep_match and fallback_match, diagnostics
                score = int(score_match[1])
                total, maximum = validate_transcript(data, process.stdout, score)
                repetitions = int(adaptive_match[1]) if adaptive_match else None
                assert repetitions is None or repetitions in (8, 12, 16, 24, 32), diagnostics
                prep_cpu = float(prep_match[1])
                if args.ubsan:
                    assert repetitions == 32, ("UBSan did not exercise full R32", diagnostics)
                    assert "exact_refit accepted=" in diagnostics, diagnostics
                    assert prep_cpu < float(refinement), ("UBSan refinement reached its CPU cutoff", diagnostics)
                row = dict(
                    learning=float(learning), refinement=float(refinement), seed=seed,
                    score=score, moves=total, errors=0, max_moves=maximum,
                    adaptive_repetitions=repetitions, prep_cpu=prep_cpu,
                    cpu=float(cpu_match[1]), fallbacks=int(fallback_match[1]),
                    output_sha256=hashlib.sha256(process.stdout).hexdigest(),
                    diagnostics=diagnostics,
                )
                result["cases"].append(row)
                unchanged()
                save()
                print({k: v for k, v in row.items()
                       if k not in ("diagnostics", "output_sha256")}, flush=True)
        result["status"] = "passed"
        unchanged()
        save()
        print(f"Validated {len(result['cases'])} cases: every move legal, E=0; {output.name}", flush=True)
    except BaseException as error:
        result["status"] = "failed"
        result["failure"] = repr(error)
        save()
        raise


if __name__ == "__main__":
    main()
