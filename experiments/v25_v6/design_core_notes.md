# Core refit with the final product pairs fixed

`design_core.cpp` / `design_core.inc` are an independent experiment based on frozen
v25 SHA256 `A9E5CB6B45EE1562C60721BE09BDE2D0246AD8EEB95585C7721048D74143D7DA`.
Both control and treatment enable POPCNT. `design_core_make.py` reproduces the
source. The nonlinear-demand experiment is not included in this variant.

The baseline chooses a comparison map before its anchor/pair annealing and
assignment-flow stages can change the product pairing. This experiment fixes
the final selected pairing and all main papers, then refits the backing map to
those actual pairs. It runs after history edits and before construction of the
fresh actual router. The original candidate race is unchanged.

Three proposals are allowed: reverse a column's zero/one backing assignment;
reassign one column to another backing comparison; or exchange two columns'
comparisons, optionally reversing either. Every unordered backing pair retains
at least one two-paper comparison window. Thus every nonbottom backing remains
visible while the backing stack is above the main papers, preserving the existing
exact backing move graph and distance-cache semantics.

For each final product pair, the implementation maintains the multiplicity of
every precedence edge contributed by its differing suffix windows. Proposals
are rejected if any resulting requirement has a cycle or loses a clique witness.
The cost is the sum of existing `OrderMetric` values, each of which averages the
directed legal distance for the two opposite product orientations over uniform
starting core orders. Simulated annealing may traverse worse valid maps, but only
the best strictly improved map can be committed.

Only the selected core rows' suffix characters are rewritten. All main rows,
product ownership, pair indices, anchors, prefix columns, and column permutation
are preserved. Before commit, an independent validator constructs a topological
core order for each of the 256 products and checks its literal suffix display
under that product's main. Existing main ink must also match each owner.

Settings:

- `DESIGN_CORE=0` (default): disabled.
- `DESIGN_CORE=1`: enabled.
- `DESIGN_CORE_CPU=.06`: local units, about .036 native seconds on Linux.
- `DESIGN_CORE_END=2.43`: absolute local deadline after input begins.
- `DESIGN_CORE_ITERS=12000`, `DESIGN_CORE_TEMP=8`: proposal quota and initial
  temperature in the integer metric's units.
- `DESIGN_CORE_VERIFY=1`: after every attempted proposal, independently recompute
  all edge multiplicities and costs and compare with maintained state. Suitable
  for a research run with a generous deadline.

The log distinguishes attempts, feasible proposals, witness rejections, accepted
steps, strict best improvements, changed columns, and commit status. Its
`predicted_moves=(initial_metric-best_metric)/8` follows the model's scale64,
two products per pair, and 1024/256 request count. This is a model prediction,
not a measured operation reduction: actual starting core orders are nonuniform,
and the router can borrow a main or support in place of a core paper.

Frozen source SHA256:
`37A037DF95099569B98FE061CCBBA1EB6F599ACF1167579474DC047BAAAD27E9`.
Include SHA256:
`D70369BD4896A8C7739A3B594BF00A6E1E188CC694FC0B210F1E04122C73B64D`.
No compilation or benchmark was run by the implementing agent.

## Measured screen (root-coordinated)

`core_probe.json` contains six development cases. Control averaged 10660.00
moves and refit averaged 10656.83 (difference -3.17), all `E=0`. However, every
refit run committed zero changed columns: 11597–11734 proposals per case yielded
only 0–7 feasible proposals, no accepted step, and no objective improvement.
The observed operation-count difference therefore cannot be credited to a core
map change; it includes time-dependent search/routing effects. Do not adopt.

With the final 69/70 disjoint product pairs fixed, changing one comparison affects
the precedence constraints of many pairs at once. A direction that helps one pair
often makes another pair cyclic. The retained clique witnesses impose an
additional restriction. This measured neighborhood is almost locked; its failure
does not prove that a larger joint pairing/map neighborhood cannot improve.
