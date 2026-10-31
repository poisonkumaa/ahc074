# Round 7: overlapping prefix/suffix dictionaries

This experiment uses the same 64 windows for both directions. It is not the
earlier independent 32-window / 96-paper split. The current `main.cpp`, the
submission binary, and pahcer outputs were not modified.

## Representation and exact intrinsic distance

A paper is either an opaque prefix `[0,L)` or an opaque suffix `[64-L,64)`.
The uncovered windows form one interval `[l,r)`. A prefix paper advances l,
a suffix paper decreases r, and the last paper may overlap the other side's
already covered windows. It only needs to match the remaining visible gap.

`BidirectionalDistance` computes the exact minimum number of visible owners
with an interval DP. The final source stack and legal retrieval operations
are not included in this intrinsic objective. Before the final overlapping
paper, the left-prefix and right-suffix visible regions are disjoint, so their
interleaving need not be enumerated.

For fitting one paper, remove it and compute left-only prefix distances,
right-only suffix distances, and all interval backward distances. A candidate
prefix paper of length p is either:

- The final paper covering a matching interval `[l,r)` with r <= p; or
- A nonfinal prefix ending at p, followed by a solution of `[p,r)`.

Scanning matching runs of the candidate color evaluates both cases in O(64)
per target. The suffix case uses the reversed coordinates. All 256 public
product colors are candidate paper colors; a change is accepted only if every
public product remains representable and the exact intrinsic sum decreases.

`BIDIRECTIONAL_CHECK` rebuilds the full interval DP after each accepted color
change. Seed 0, 32 suffix papers, one pass, 64 candidate colors accepted 38
changes; all predicted costs equalled the full recomputation.

## Dictionary capacity results

Files:

- `round7_bidirectional.h`
- `round7_bidirectional_build.py` and generated `round7_bidirectional.cpp`
- `round7_bidirectional_bench.py`
- `round7_bidirectional_p2_results.json`

The initial allocation uses the current 192-paper dictionary lengths. A
universal 65-paper prefix backbone is reset, and a deterministic subset of
the remaining papers becomes suffix papers. This keeps the starting
dictionary feasible. All papers, including the backbone, can then be refit.

Seed 0, two coordinate passes with all 256 candidate colors:

| Number of suffix papers | Intrinsic minimum-owner mean |
|---:|---:|
| 0 | 9.63671875 |
| 32 | 9.45703125 |
| 64 | 9.58203125 |
| 96 | 9.94921875 |

The best mixture improves this training baseline by about 0.18 owners. The
training is an offline prototype, approximately 5--6 seconds for two passes,
and is not a valid 2-second submission candidate as implemented.

## Legal planner prototype

Files: `round7_bidirectional_plan.h`,
`round7_bidirectional_solver_build.py`, generated
`round7_bidirectional_solver.cpp`.

The goal DP can weight a paper by its intrinsic cost plus a multiple of the
minimum number of current blockers over its opaque windows. The resulting
left and right owner chains can be interleaved according to their current
positions without changing their disjoint prefinal regions.

The first planner maps each paper to a virtual rank according to the last
goal region it touches and whether it covers that region's witness. Every
virtual-visible paper is actually visible. It then uses the existing generic
future-union planner, permitting actual-visible moves that reduce virtual
distance by more than one. This supplies a valid but restrictive ordering.

Seed 0, 64 suffix papers, two training passes, access weight 0.2:

- Official score **59286**, E=0.
- Preparation CPU about 6.29 seconds; total CPU about 9.70 seconds.
- Every move is checked for actual visibility, and every final display is
  checked against the target inside the experiment.

This is much worse than the current prefix solver. The virtual ordering
forces unrelated left/right regions to be ordered and is too restrictive.

## Stronger planner attempts that did not succeed

`round7_bidirectional_reorder.h` contains two unfinished diagnostic planners.
They must not be adopted into a submission.

1. Enumerate the moved subset of the roughly ten selected owners. Remaining
   inverted precedence edges force nonselected papers to move. A subset-zeta
   transform gives the minimum unrestricted move count for that conflict
   model; same-direction containment adds some mandatory legal moves. A
   topological extension guides actual-visible moves. The greedy policy
   entered plateaus and exceeded the 384-operation assertion.
2. Retrieve available owners in an order respecting their ancestor owners,
   clearing the least-blocked witness window and placing the owner after its
   ancestors. This is constructive, but some queries exceeded 384 moves due
   to repeated retrieval of long overlapping blockers. The official tester
   aborted those diagnostic runs with score zero.

The principal unresolved issue is joint visibility: a prefix of length L and
a suffix of length S can hide every later paper when L+S >= 64, even though
neither paper individually contains the selected paper's entire mask. Thus
independent same-direction mandatory sets or unrestricted move counts do not
capture the additional retrieval cost.

The structure was not adopted. Its small intrinsic-capacity improvement does
not, by itself, justify replacing the proven prefix planner. A successful
future attempt would need a substantially stronger legal two-direction move
DP, rather than further tuning of the generic virtual ordering.

## Reproduction

The builders read the current main file, so regenerate only when a new
baseline is intended. The generated sources retain the code used here.

```sh
g++ -O3 -std=c++17 -DAHC_LEARNING_BUDGET=100 \
  -DAHC_REFINEMENT_BUDGET=100 tools/experiments/round7_bidirectional.cpp \
  -o tools/experiments/round7_bidirectional
python3 tools/experiments/round7_bidirectional_bench.py \
  --rights 0 32 64 96 --passes 2 --workers 1

g++ -O3 -std=c++17 -DBIDIRECTIONAL_CHECK -DAHC_LEARNING_BUDGET=100 \
  -DAHC_REFINEMENT_BUDGET=100 tools/experiments/round7_bidirectional.cpp \
  -o tools/experiments/round7_bidirectional_check
tools/experiments/round7_bidirectional_check 32 1 64 < tools/in/0000.txt
```
