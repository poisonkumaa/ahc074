# Nonlinear replay-family diversification

`design_diverse.cpp` is derived from the frozen `design_candidate.cpp` by
`design_diverse_make.py`. It retains the same POPCNT setting and helper
`design_flip.inc`.

`DESIGN_FLIP=0` disables nonlinear exposure cost everywhere. With
`DESIGN_FLIP=.5`, only layouts whose `graph_seed_offset >= max(1,NOV8_POOL)`
use the half-linear/half-nonlinear exposure objective in the final support-aware
anchor stage. With the normal pool size4, original roots0–3 keep the original
objective, replay layouts4–6 and the optional offspring14 use the new objective.
Candidate selection and the number of requested designs remain unchanged.

The purpose is to retain the original design family while offering different
replay and offspring layouts. It does not assume that replacing every design
with the nonlinear model improves the score: the all-family half variant failed
to retain a mean improvement on the subsequent fresh40-case production test.
Ordinary deadline effects can still change how many later layouts finish.

The gate duplicates the existing configured pool value because `pool` is
declared outside and after the build-design lambda. Nondefault pool sizes at or
above14 can put the fixed offspring seed14 below the gate; this experiment's
intended setting is the normal pool4.

No compilation or benchmark was run by the implementing agent.
