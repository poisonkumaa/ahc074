# Round 6: non-prefix structures and joint dictionary changes

This file records the experiments performed by `larger_templates` during the
round that began with the user's AtCoder result 667994 / 50 cases and 1739 ms.
The submission `main.cpp`, `a.out`, and pahcer result files were not modified.
Every experiment uses only the initial 256 public products for learning.
Official orders are read one at a time after printing/flushing the design.

## Baseline used for comparisons

Current submission dictionary allocation, fixed exact-refit repetitions 8,
learning/refinement budgets overridden to 100 seconds for deterministic
diagnostics. Public seeds 0, 1, 2: 13343, 13444, 13043; mean 13276.67.
Public seeds 0..9 mean 13265.5. These are fixed-R8 diagnostic results, not
the submission's adaptive repetition setting.

## Small whole-product permutation encoding

`round6_dual_shatter.cpp` enumerates distinct ternary-column truth functions
over all permutations of K papers. It searches for a small set of product
states that can realize many binary column labels.

- K=3 has 6 permutations and 14 distinct column functions. Four selected
  product states attain at most 10 of the 16 labels in the small search.
- K=4 has 24 permutations and 52 distinct functions. Exhausting all choices
  of five states attains at most 24 of 32 labels.
- K=5 / seven states reached 80 of 128 labels in stochastic search.

These are not general impossibility results. They do rule out the simplest
data-independent 3-paper/4-product and 4-paper/5-product templates. Dense
whole-product groups also have the unresolved problem of retrieving a hidden
group through the currently visible papers.

## Tail categories with a shared opaque core

Files: `round6_tailblock_build.py`, generated `round6_tailblock.cpp`,
`round6_tailblock_bench.py`.

For block size b, papers whose original lengths exceed 64-b are changed to
an opaque core `[0,64-b)` plus their own original endpoint. Thus there are
b tail categories, and the first paper in each category is visible on its
endpoint. The core PrefixDP only selects a category paper whose endpoint
color matches the target. `GeneralPrefix::best_move` considers every
actually visible paper in both directions, allowing a virtual-distance
decrease larger than one. After the core is correct, wrong category heads
can be moved down behind a correct one while their core remains hidden.

This gives a constructive legal solution, but choosing the core goal without
its tail repair costs is a poor approximation. Seed 0 results with the older
inversion-only move tie breaker:

| b | Score |
|---|---:|
| 1 | 14235 |
| 2 | 15219 |
| 4 | 16925 |
| 8 | 20504 |

For b=2, core moves=14296, tail repairs=923, and initial virtual potentials
sum to 14421. Extra visibility therefore saved only 125 core moves.
Replacing the final core-edge blocker count by its own-category count alone
made the chosen goals worse under this planner (b=2: 15815).

The parent subsequently added exact future-union tie breaking in
`round6_general_future.h`; this is the preferable generic planner. The
`global_design` agent separately tested complete tail-owner goals using
virtual category orientations, which outperformed this core-plus-repair
prototype but still did not beat the baseline.

## All two-window blocks, with all orientations optimized by one DP

Files: `round6_pairblocks.h`, `round6_pairblocks_train.h`,
`round6_pairblocks_build.py`, generated `round6_pairblocks.cpp`.

For a two-window block beginning at coordinate c, each paper ending in that
block has mask `[0,c)` plus its own endpoint. States are the completely
covered preceding blocks and the subset of the current block already
covered. There are at most 97 states. The first selected paper of a block
sets its virtual category order. The second selected category, if needed,
completes the block. Therefore all 2^32 orientations can be optimized without
enumeration.

The exact virtual-prefix move DP uses `(coverage_state, original_max_position)`.
For entering a new block, the moved-paper threshold counts all papers in that
block or above. For completing a partially covered block, it counts higher
blocks and the remaining category. The previous-state upper threshold is
the corresponding higher/remaining-category count. Reconstructed virtual
lengths reproduce `GlobalPrefix::fixed_distance` exactly; this equality is
asserted for every query. Physical visibility and the final displayed
barcode are also asserted on every query. The generic future-union planner
permits additional actually visible moves with decreases larger than one.

Post-processing the existing prefix dictionary gives these seed 0 results:

| First coordinate using pairs | Score |
|---|---:|
| 64 (no changed masks) | 13343 |
| 60 | 14105 |
| 48 | 15769 |
| 32 | Product 207 is not representable; no design is printed |

Thus changing masks alone loses dictionary feasibility or quality. A new
universal initialization needs 66 papers: the existing zero-endpoint
backbone plus both all-one endpoint categories in the final block.

`train_pairblocks` jointly fits the structure's colors by coordinate descent
on exact minimum-cover distances over all public products. For one paper,
forward/backward paths excluding it yield all 256 candidate-color costs.
Matching can be evaluated from the highest mismatch and one exceptional
partial-block state. Every accepted change keeps all public products
representable. Two passes on seed 0:

| First paired coordinate | Intrinsic before → after | Official score |
|---|---|---:|
| 0 | 14.8398 → 10.3711 | 17322 |
| 48 | 14.8438 → 10.3398 | 16097 |
| 60 | 14.8008 → 10.3672 | 14521 |

The all-block run had 17887 total virtual potential versus 17322 actual
moves, maximum 37 moves per request, E=0, and about 2.24 seconds diagnostic
CPU (including original dictionary learning). This structure's additional
visibility does not compensate for its lost shortcuts and extra owner
constraints. It was not adopted.

## Joint colors of adjacent single-paper levels

Files: `round6_joint_intrinsic.h`, `round6_joint_intrinsic_build.py`,
`round6_joint_exact.h`, `round6_joint_exact_build.py`,
`round6_joint_exact_bench.py` and their generated `.cpp` files.

For adjacent lengths L and L+1, the two-paper paths decompose into four
cases: neither paper, lower only, upper only, or both. When both are used,
only the upper paper's final bit is visible, so its other color bits do not
affect that path. This permits exact joint evaluation of all 256×256 public
candidate-prefix pairs while preserving all 256 products' reachability.

The intrinsic prototype updates only levels containing one paper each.
Seed 0: intrinsic 10.8711 → 10.8203 through three joint changes, additional
CPU 0.068 seconds, official score 13442 (worse than 13343).

The exact version captures one or more synthetic steady-state source stacks
per public product. It caches sparse forward distances below the current
pair and recomputes backward rows in descending order. Since updates descend
by length, forward rows below the next pair remain valid. The four route
costs are exact legal move counts to any displaying prefix state. Every
accepted update in the diagnostic binary is checked by rebuilding PrefixDP
and evaluating every captured source stack from scratch (`JOINT_EXACT_VERIFY`).

| Public seeds | Samples per product | Mean score |
|---|---:|---:|
| 0..2 | disabled | 13276.67 |
| 0..2 | 1 | 13270.67 |
| 0..2 | 4 | 13252.00 |
| 0..9 | disabled | 13265.50 |
| 0..9 | 4 | 13252.20 |

All these cases have E=0, and all accepted-update exact-distance checks pass.
The gain is only 13.3 points over ten seeds. The R4 diagnostic binary reached
1.94 seconds total CPU on its slowest case, including repeated verification.
This small gain has not been confirmed over 100 seeds and was not adopted.
Applying it after full-length single-paper exact refit would distinguish
joint-only improvements from improvements due simply to touching lower levels.

The parent independently implemented same-level joint exact updates in
`round6_joint_same.cpp`. One additional lightweight check used R8,
`joint_pairs=8` (capped internally at Q/2), and `joint_stride=16`, so the first
paper had 16 candidate colors while the second used an exact trie search.
Seeds 0..2 scored 13387, 13668, 13056; mean 13370.33. This also lost to the
baseline and was not adopted.

## Reproduction examples

Run in WSL from the `ahc074` directory. The builders read the current
`main.cpp`, so regenerating after a submission change changes their baseline.
All existing generated sources retain the exact code used for these tests.

```sh
g++ -O3 -std=c++17 -DAHC_LEARNING_BUDGET=100 \
  -DAHC_REFINEMENT_BUDGET=100 -DJOINT_EXACT_VERIFY \
  tools/experiments/round6_joint_exact.cpp -o tools/experiments/round6_joint_exact
python3 tools/experiments/round6_joint_exact_bench.py --count 10 --reps 0 4

g++ -O3 -std=c++17 -DAHC_LEARNING_BUDGET=100 \
  -DAHC_REFINEMENT_BUDGET=100 tools/experiments/round6_pairblocks.cpp \
  -o tools/experiments/round6_pairblocks
tools/target/release/tester tools/experiments/round6_pairblocks 0 8 0 2 \
  < tools/in/0000.txt > /tmp/pairblocks.out
```

## Equivalent exact-trie compression added after the search experiments

`round6_exact_triefast.cpp` applies the shared `round6_trie_topology.h`
Patricia topology to the trie inside `exact_state_refit`. The shared header
was supplied by `global_design`. A unary chain's weight increments map to its
first retained descendant. Retained nodes preserve their original creation
order, so parent-before-child propagation and first-leaf tie breaking are
unchanged. The topology is cached per public barcode array and prefix length;
it is also reusable by the initial dictionary learners.

`round6_exact_triefast_build.py` produces this candidate and an unmodified
reference from a source selected by `--source` (default `main.cpp`). It changes
only the diagnostic main signature and repetition argument in the reference.
`round6_exact_trie_compare.py` runs each pair sequentially through the official
tester and compares the complete stdout bytes, including design and all 1024
operation sequences.

- Fixed R8, seeds 0..4: all five complete outputs identical.
- Fixed R32, seeds 0..4: all five complete outputs identical.
- `EXACT_TRIE_CHECK` plus `SAMPLING_CHECK_B`, R8 seed 0: all retained-node
  cumulative weights, best leaf, selected sample, backward DP values and
  bypass values agree with their uncompressed/reference computations; the
  complete output also matches.

Mean CPU seconds from the sequential five-seed comparison:

| Repetitions | Exact-refit reference | Exact-refit compressed | Whole reference | Whole compressed |
|---|---:|---:|---:|---:|
| 8 | 0.305206 | 0.297081 | 0.897540 | 0.885862 |
| 32 | 1.274476 | 1.229502 | 2.151752 | 2.112008 |

These are diagnostic local measurements with the time budgets disabled;
individual case timings varied. The exact section improved by about 3% in
this run. The compressed exact implementation does not yet include the other
agent's initial-learning optimizations. With the real adaptive timer enabled,
faster computation may select a different repetition count, so output
identity is asserted only for fixed computation and sufficiently large budgets.

Detailed hashes and diagnostics are stored in
`round6_exact_trie_compare.json` and `round6_exact_trie_compare_check.json`.
