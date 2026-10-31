from pathlib import Path
d=Path(__file__).resolve().parent
s=(d/'policy2_agent_perf_training.cpp').read_text()
s=s.replace('#define STRUCTURE2_TRAINING_CPU_LIMIT 0.9','#define STRUCTURE2_TRAINING_CPU_LIMIT 0.75')
s=s.replace('// AHC074: public-data prefix dictionaries, exact legal reactive paths.', '''// AHC074: public-data prefix dictionaries, exact legal reactive paths.
// Train only on the initial 256 barcodes. The 64-bit access model is initialized
// with internally generated requests, then updated only from processed orders.
// Each query minimizes its move count exactly; both goal and move tie-breakers
// favor papers that are likely to be useful again. No future order is read.''')
s=s.replace('// Among shortest-path moves prefer low inversions by prefix length, which\n// makes papers easier to expose on future requests.', '''// Among shortest-path moves minimize the number of mandatory removals averaged
// over 64 previous goals. A suffix union counts correlated choices exactly.
// Length inversions break remaining ties and initialize the access model.''')
s=s.replace('int prefix_min[65];prefix_min[l]=INF;', '''// A route avoiding l crosses it once, along an edge j < l < upper.
   // The prefix and suffix distances on that edge cannot use level l.
   int prefix_min[65];prefix_min[l]=INF;''')
assert 'argc' not in s and 'argv' not in s
(d/'final_139xx.cpp').write_text(s)
(d.parents[1]/'main.cpp').write_text(s)
