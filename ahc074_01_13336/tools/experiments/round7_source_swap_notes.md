# Round 7: exchanging source products across different paper lengths

This experiment exchanges the public source-product identities of two papers
with different lengths. If the papers have lengths L1, L2 and sources v1, v2,
the replacement colors are `A[v2] & low(L1)` and `A[v1] & low(L2)`.
The length histogram and paper count remain unchanged. This is distinct from
the earlier same-level two-paper color optimization.

No changes were made to `main.cpp`, the submission executable, or pahcer
outputs. No result from this experiment is recommended for adoption.

## Implementation and validation

Files:

- `round7_source_swap.h`
- `round7_source_swap_build.py` and generated `round7_source_swap.cpp`
- `round7_source_swap_bench.py`
- The existing `round7_lengthlocal.h` supplies the exact snapshot evaluator.

Eligible papers have length at least 43 and an identifiable public source
product. Candidate pairs use distinct lengths and distinct source products.
Pairs are deterministically shuffled; the search examines the requested
number of candidates. Each candidate first passes a reachability test for all
256 products. This recomputes the minimum matching start at the two modified
lengths while retaining the existing values at all other lengths.

The candidate evaluator updates both papers' cached matching edges. Since
only colors change, the stack-position and length-count arrays stay valid.
Each candidate's exact minimum move count is summed over 256 internally
generated public-data snapshots. Both paper colors and matching-edge caches
are restored after evaluation. The best strictly improving candidate is
accepted. Multiple passes recapture all snapshots after each accepted change.

All diagnostic binaries used `SOURCESWAP_CHECK`. Every accepted candidate's
score was independently recomputed by the original `PrefixDP` on every
snapshot and asserted equal. All official-tester cases completed with E=0 and
legal operation counts. Future official orders are never used by learning.

## Results without rollout veto

The exact color learner used fixed R32 unless stated otherwise. Time budgets
were disabled for these diagnostic comparisons.

| Cases | Setting | Mean score | Reference | Delta |
|---|---|---:|---:|---:|
| Seeds 0..2 | 1000 pairs, one accepted pass | 13205.67 | 13252.33 | -46.67 |
| Seeds 0..2 | 1000 pairs, three passes | 13238.67 | 13252.33 | -13.67 |
| Seeds 0..9 | 1000 pairs, one pass | 13239.00 | 13248.60 | -9.60 |
| Seeds 0..9 | 256 pairs, one pass | 13249.10 | 13248.60 | +0.50 |
| Seeds 0..29 | 1000 pairs, one pass | 13303.60 | 13299.80 | +3.80 |
| Seeds 0..9 | R8 + 1000 pairs, one pass | 13311.80 | 13265.50 | +46.30 |

The 30-case comparison had 14 wins and 16 losses. The standard deviation of
the paired score difference was 61.13. The apparent gain on the first three
cases therefore did not persist.

The R32 reference scores were checked against
`round5_sampling_cut53_tools_32_1_results.json`, whose full 100-case mean is
13323.33. Its first ten cases match the freshly run reference exactly.

For the initial three cases, 926--955 of the 1000 candidate swaps preserved
reachability. One pass added roughly 0.95--1.04 seconds CPU; the complete R32
diagnostic run took about 2.5--2.6 seconds. Timing varied under later parallel
workloads. This implementation does not fit the 2-second limit as measured.

The fixed-snapshot gain can disappear after recapturing the new policy's
states. For example, seed 0's first pass improves 3471 to 3457 on the old
snapshots, but the recaptured next-pass starting score is 3474. This motivated
the rollout veto below.

## Veto using newly generated states under the candidate policy

An optional `validation_states` parameter evaluates both the old and changed
dictionary on the same internally generated request sequence. Each model
replays 128 burn-in requests followed by 1024 public-data requests drawn
uniformly with adjacent repetitions excluded. Thirty-two source states are
sampled evenly across that 1024-request history. At every sampled state, the
exact minimum move count is summed over all 256 possible next products.

The new dictionary is rejected unless this independently captured expected
cost is strictly smaller. The old dictionary is restored atomically when
rejected. These synthetic requests are generated entirely from the initial
public product list and are unrelated to the official future orders.

| Cases | Setting | Mean score | Reference | Delta |
|---|---|---:|---:|---:|
| Seeds 0..2 | R32, 1000 pairs, veto32 | 13230.67 | 13252.33 | -21.67 |
| Seeds 0..9 | R32, 1000 pairs, veto32 | 13237.40 | 13248.60 | -11.20 |

Only seeds 1 and 4 accepted the swap in the ten-case run. They improved by 65
and 47 points; the other eight cases returned exactly to the reference.
The additional computation is substantial relative to this small gain.
No 100-case adoption test or independent holdout test was run. The parent
requested that CPU experiments stop so final timing could run quietly.

## Result artifacts

- `round7_source_swap_c3_p1000_s256_r32_results.json`
- `round7_source_swap_c10_p1000_s256_r32_results.json`
- `round7_source_swap_c10_p256_s256_r32_results.json`
- `round7_source_swap_c30_p1000_s256_r32_results.json`
- `round7_source_swap_c10_p1000_s256_r8_results.json`
- `round7_source_swap_c3_p1000_s256_r32_v32_results.json`
- `round7_source_swap_c10_p1000_s256_r32_v32_results.json`

The generated source includes the optional veto. The original
`round7_source_swap` executable predates that option, while
`round7_source_swap_validate` includes it. Both use identical swap generation
and scoring when validation is disabled.

## Reproduction

The builder reads the current `main.cpp`; the generated source retains the
baseline used for these results. Run from the ahc074 directory in WSL.

```sh
g++ -O3 -std=c++17 -DSOURCESWAP_CHECK \
  -DAHC_LEARNING_BUDGET=100 -DAHC_REFINEMENT_BUDGET=100 \
  tools/experiments/round7_source_swap.cpp \
  -o tools/experiments/round7_source_swap_validate
python3 tools/experiments/round7_source_swap_bench.py \
  --binary round7_source_swap_validate --passes 1 --pairs 1000 \
  --snapshots 256 --reps 32 --validation 32 --count 10
```
