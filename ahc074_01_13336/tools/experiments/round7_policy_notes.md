# Round 7: long-horizon policy and exact insertion shortcut

All experiments use the public 256 products. Real requests are consumed and
answered individually. `main.cpp`, `a.out`, `out`, and `err` are not modified.

## Exact final-order shortcut

Files: `round7_final_fast.h`, `round7_final_build.py`,
`round7_final_bench.py`, and `round7_final_results.json`.

The specialization keeps the existing general implementation as
`GlobalPrefix::final_order_incremental_reference`. For every mandatory
nonselected paper of length L, it checks that the current child list has the
three consecutive classes `< L`, `= L`, and `> L`. It also checks that future
goal masks of distinct papers with the same length are disjoint. Any failed
check invokes the original implementation on the entire original order.

Under these checked conditions, moving the insertion cut right across a
shorter child cannot increase the primary union cost: only the inserted
paper's suffix union can lose bits. Moving the cut right across a longer
child cannot decrease the cost: only that child's suffix union can gain
bits. A minimum is therefore present between those two classes. Every cut
in this middle class adds zero strict length inversions, so the existing
secondary objective also prefers this class over any exterior tied cut.

Inside the equal-length class, there are no shorter children afterward.
Distinct equal-length masks are disjoint. Crossing child x therefore changes
cost by exactly `popcount(future[k]) - popcount(future[x])`. A prefix-sum
minimum gives the same earliest optimal cut as the original enumeration.
Without a future mask, all middle cuts have zero cost and zero inversions;
the first cut is identical to the reference choice.

This argument uses only conditions checked at runtime; it does not require
the empirically observed pattern-avoidance invariant to hold for every input.

Validation at fixed exact R8 and generous learning budgets:

- Seed 0: every one of 3,584 internal/teacher/real final-order calls compared
  against the original implementation; all equal, no fallback.
- Seeds 0 through 99: complete official-tester stdout byte-for-byte equal
  between reference and shortcut, including design and every move.
- Both means: **13,337.73**, every order correct, all operation limits met.
- No shortcut fallback in these 100 cases.
- Two-worker mean CPU: reference 0.69063 s, shortcut 0.65654 s.
- Mean real-query CPU: 0.08332 s to 0.07398 s, about 11.2% lower.
- CPU figures are local comparisons under concurrent work; parent performs
  isolated final-budget measurements before adopting a change.

## Structural observations

`round7_invariant.cpp` observes 4,096 public synthetic requests on seed 0.
Children between visible records are **not sorted**: 4,030 states contain a
descent, 38,000 descents in total, and at most 22 per state. However, no
strict 231, 221, or 232 length pattern appears. All 30,166 mandatory child
insertions have an insertion position adding zero strict length inversions;
the union-optimal choice never sacrifices that minimum.

## Long-horizon post-response actions

`round7_policy_rollout.cpp` tests additional display-preserving visible-paper
insertions, plus a visible paper moved to the top/bottom followed by a legal
repair. It retains eight diverse states, screens with two common 32-request
rollouts, and validates with eight independent 32-request rollouts. Actions
are considered every 32 real queries. Additional costs are charged explicitly.
Noncanonical states use a checked legal fallback when the direct constructor
does not realize the exact distance.

No action was accepted on seeds 0, 1, or 2. Scores remain
13,343 / 13,444 / 13,043 (mean 13,276.67), while CPU increases. Rejected.

## Persistent insertion geometries

`round7_geometry.cpp` preserves minimum current moves but scores insertions
against either length order, mean positions under the current policy, or a
stationary state minimizing the true mean next-product distance over all 256
public products.

Three-seed means:

| Geometry | Mean score |
| --- | ---: |
| Current baseline | 13,276.67 |
| Pure length-inversion priority | 13,276.67 (identical outputs) |
| Length penalty plus union cost | 13,276.67 (identical outputs) |
| Mean-position reference | 13,709.33 |
| Best sampled stationary reference | 13,506.33 |
| Weak best-reference regularization | 13,305.00 |

On seed 0, unconditional next-product mean distances are initial ascending
12.8242, observed stationary 13.0747, best sampled stationary 12.0312, and
mean-position order 11.8594. Attractive reference orders exist, but imposing
their pair order on every shortest response harms the actual long-run cost.

## Goal-level inversion regularization

`round7_inversion_goal.h` and its builder implement a separate DP objective:
current legal moves plus a weight times the exact final strict length
inversions. This changes goal selection, unlike the rejected insertion-only
geometry. Its predicted final inversion count matches the actual constructed
permutation on every query in the tests below.

For DP edge `(j,h) -> (l,nh)` choosing a paper of length l, all papers with
length in `(j,l]` form one final owner group. The root contributes the number
of papers with length in `(j,l)`. Let `D(j,l,h)` count inversions among current
positions at least h with lengths in `(j,l)`. The remaining child contribution
is `D(j,l,h) + D(j,l+1,nh) - D(j,l,nh)`. This expression follows because shorter
nonmandatory children are precisely those at positions at least h, while
nonmandatory length-l children are precisely those at positions at least nh.
Inserted mandatory children add zero inversions under the checked structure.
The prototype compares the summed edge prediction to the constructed final
permutation on every real request before any result is considered valid.

Three-seed results, weights expressed as multiples of 1/1024:

| Weight | Scores (seeds 0,1,2) | Mean | Extra moves above immediate minimum |
| --- | --- | ---: | --- |
| 0 baseline | 13343,13444,13043 | 13276.67 | 0,0,0 |
| 8 | 13354,13443,13042 | 13279.67 | 4,4,0 |
| 16 | 13284,13436,13074 | 13264.67 | 46,73,70 |
| 32 | 13443,13711,13298 | 13484.00 | 336,391,380 |

Weight 16 reduces mean final inversions to about 155--158 (baseline seed 0:
163.4), but only improves mean score by 12 points, with one seed worse. Weight
32 reduces inversions further to about 137--139 and substantially worsens
score. The O(N²K) per-query inversion table adds computation, so this result
does not justify adoption or a large validation run. All cases have zero
errors and both the exact inversion formula and legal move checks pass.
