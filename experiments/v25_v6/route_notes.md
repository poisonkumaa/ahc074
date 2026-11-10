Routing experiments, 2026-11-06. Both variants use the frozen v25 source
`main_baseline.cpp` (SHA256
`A9E5CB6B45EE1562C60721BE09BDE2D0246AD8EEB95585C7721048D74143D7DA`)
and put the guarded x86 GCC POPCNT target before every standard header.
The production `main.cpp` was not changed by these experiments. All new flags
default to zero, and the source generators and fixture sources are preserved.

`route_quota.cpp` tests two independent changes. `ROUTE_QUOTA_RESERVE=1`
reserves the predicted remaining primary-shortening CPU from the shared
`NOV5_RELAX_BUDGET` quota before evaluating secondary routes. Its prediction
uses only observed primary optimization time, excluding alternatives.
`ROUTE_ALT_FIXPOINT=1` repeats shortening of an alternative after every strict
length reduction, matching the existing primary route's stopping rule.

The fixed-layout six-case screen in `route_fixed_probe.json` used cases
0000–0005, the same saved designs, and route/relax/short absolute deadlines
of 30 local budget units. The shared relaxation quota retained its default
0.52 local units (0.312 native CPU seconds on this test configuration).
Lower move counts are better; every run had zero errors.

| Flags | Mean moves | Difference from control |
| --- | ---: | ---: |
| Both off | 10621.6667 | 0 |
| Reserve only | 10621.6667 | 0 |
| Fixed-point repeat only | 10623.1667 | +1.5000 |
| Both on | 10623.3333 | +1.6667 |

Reservation alone left every final move count unchanged, although it did
refuse some secondary evaluations on case 0002. Repeated shortening saved
77–114 additional moves across the *evaluated alternative traces* per case;
these are not savings in the emitted solution. Its changed choices and shared
quota consumption produced a worse final mean. Neither flag was adopted.
Because this screen retained a CPU-based quota, repeat versus both is not a
pure deterministic comparison under unlimited computation.

`route_anchor.cpp` tests `ROUTE_ANCHORED=1`. The fully shortened primary route
is captured once. Every challenger receives a scalar difference relative to
that same primary: raw length-plus-state-value difference, replaced by the
existing learned forecast when the primary-relative applicability gate and
0.05 deadband allow it. The primary's rank is zero, and the minimum rank wins.
This makes comparisons transitive for a fixed evaluated candidate set.
The outer candidate-admission gates and time limits still depend on the
incumbent, so the entire search is not order-independent.

The separate fixed-layout screen in `anchor_fixed_probe.json` used the same
six cases and absolute deadlines of 30, with `NOV5_RELAX_BUDGET=10` to avoid
the normal shortening quota affecting this comparison. All runs had zero
errors. Control averaged 10621.6667 moves; anchored ranking averaged
10622.3333 (+0.6667). Per-case differences were `[0, -1, +12, -1, -10, +4]`:
three wins, two losses, and one tie. This comparator was not adopted.

Read-only audits found no state-restoration or legality defect in either
variant. These six development cases establish no general improvement and
are not a full 2-second end-to-end validation. Selective learned-critic rollout
was initially postponed after these screens, then reopened after the separate
production candidate failed its larger validation set.

`route_critic.cpp` is the subsequent, default-off `ROUTE_CRITIC_ROLLOUT=1`
experiment. It considers only equal-length shortened routes for which the
existing learned comparator reverses the raw comparison and has an absolute
forecast of at most 0.35. A separate scratch router evaluates twelve identical,
unique catalog requests from both terminal states, excluding the current
request. Its one-step cost is the simple policy-0 move count plus the existing
18-feature learned critic (weight 0.75) and route-state value (weight 0.25).
The paired average changes the existing decision only outside a 0.05 deadband.
No actual future request is read.

At most 32 comparisons and 0.04 native CPU seconds are allowed per case;
incomplete comparisons retain the old decision. Deadline checks surround each
bounded scratch query and leaf evaluation, so the last call can slightly
overrun that time threshold. Scratch caches and flags are reset between
samples, and current feedback counts, utility and core bias are copied from
the live router. MDP, route branching, trace shortening and hypothetical
feedback updates are deliberately absent from this approximate evaluator.
The source and fixture are frozen for coordinated benchmarking; no improvement
is claimed before those measurements.
