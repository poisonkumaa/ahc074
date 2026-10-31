// AHC074 v5: catalogue-dependent overlays + exact partial-display correction.
// Standalone C++17. No command-line options, external files, or extra libraries.
// Build: g++ -O2 -std=c++17 main.cpp -o main
// Only the initial catalogue is used in the design phase. Real orders are read
// one at a time, after flushing the previous answer.
#include <bits/stdc++.h>
using namespace std;
using U64 = uint64_t;

namespace {
constexpr int N = 64, K = 192, P = 256;
constexpr int CUT = 30;       // Windows [0,30): five independent 6-window groups.
constexpr int REFINEMENTS = 5;
constexpr double ALLOCATION_SLOPE = 0.7;

int popcount(U64 x) { return __builtin_popcountll(x); }

// Exact graph for a seven-paper, six-window controller. A partial target may
// leave any subset of windows unspecified (because an overlay will cover them).
struct Controller {
    struct Edge {
        uint16_t next;
        uint8_t paper, position;
    };
    static constexpr int B = 6, PAPERS = 7, VALUES = 64, PARTIALS = 729;
    const vector<string> pattern = {
        "0....0", "10...0", ".10.00", "111000",
        ".11100", "11..10", ".....1"
    };
    int states = 0;
    vector<array<uint8_t, PAPERS>> permutations;
    vector<vector<Edge>> edges;
    vector<uint8_t> visible, displayed, distance_table;
    vector<uint16_t> terminal_cost, average_cost;
    array<array<int, VALUES>, VALUES> partial_code{};

    static int rank_permutation(const array<uint8_t, PAPERS>& p) {
        int rank = 0;
        for (int i = 0; i < PAPERS; ++i) {
            int smaller = 0;
            for (int j = i + 1; j < PAPERS; ++j) smaller += p[j] < p[i];
            rank = rank * (PAPERS - i) + smaller;
        }
        return rank;
    }

    Controller() {
        array<uint8_t, PAPERS> p{};
        iota(p.begin(), p.end(), 0);
        do { permutations.push_back(p); }
        while (next_permutation(p.begin(), p.end()));
        states = int(permutations.size());
        edges.resize(states);
        visible.assign(states, 0);
        displayed.assign(states, 0);

        array<int, PAPERS> masks{}, colors{};
        for (int k = 0; k < PAPERS; ++k) {
            for (int b = 0; b < B; ++b) {
                char c = pattern[k][b];
                if (c == '.') continue;
                masks[k] |= 1 << b;
                if (c == '1') colors[k] |= 1 << b;
            }
        }
        vector<vector<int>> reverse_edges(states);
        for (int s = 0; s < states; ++s) {
            int covered = 0;
            for (int k : permutations[s]) {
                int exposed = masks[k] & ~covered;
                if (exposed) visible[s] |= 1 << k;
                displayed[s] |= colors[k] & exposed;
                covered |= masks[k];
            }
            for (int from = 0; from < PAPERS; ++from) {
                for (int to = 0; to < PAPERS; ++to) {
                    if (from == to) continue;
                    auto next = permutations[s];
                    int k = next[from];
                    if (from < to) {
                        for (int j = from; j < to; ++j) next[j] = next[j + 1];
                    } else {
                        for (int j = from; j > to; --j) next[j] = next[j - 1];
                    }
                    next[to] = uint8_t(k);
                    int ns = rank_permutation(next);
                    edges[s].push_back({uint16_t(ns), uint8_t(k), uint8_t(to)});
                    if ((visible[s] >> k) & 1) reverse_edges[ns].push_back(s);
                }
            }
        }

        array<int, B + 1> power3{};
        power3[0] = 1;
        for (int b = 1; b <= B; ++b) power3[b] = 3 * power3[b - 1];
        // Ternary digits: 0 = unspecified, 1 = require white, 2 = require black.
        for (int mask = 0; mask < VALUES; ++mask) {
            for (int target = 0; target < VALUES; ++target) {
                for (int b = 0; b < B; ++b) {
                    if ((mask >> b) & 1)
                        partial_code[mask][target] += (1 + ((target >> b) & 1)) * power3[b];
                }
            }
        }
        distance_table.assign(PARTIALS * states, 255);
        terminal_cost.assign(PARTIALS * states, 65535);
        average_cost.assign(states, 0);
        vector<int> queue(states);

        // Multi-source reverse BFS for all fully specified targets.
        for (int target = 0; target < VALUES; ++target) {
            uint8_t* d = &distance_table[partial_code[VALUES - 1][target] * states];
            int head = 0, tail = 0;
            for (int s = 0; s < states; ++s) {
                if (displayed[s] == target) {
                    d[s] = 0;
                    queue[tail++] = s;
                }
            }
            while (head < tail) {
                int v = queue[head++];
                for (int u : reverse_edges[v]) {
                    if (d[u] != 255) continue;
                    d[u] = uint8_t(d[v] + 1);
                    queue[tail++] = u;
                }
            }
            if (tail != states) abort();  // The fixed template is universal.
            for (int s = 0; s < states; ++s) average_cost[s] += d[s];
        }
        // A partial target is the union of its two more-specific children.
        for (int code = PARTIALS - 1; code >= 0; --code) {
            int wildcard = -1;
            for (int b = 0; b < B; ++b) {
                if ((code / power3[b]) % 3 == 0) { wildcard = b; break; }
            }
            if (wildcard < 0) continue;
            uint8_t* d = &distance_table[code * states];
            const uint8_t* zero = &distance_table[(code + power3[wildcard]) * states];
            const uint8_t* one = &distance_table[(code + 2 * power3[wildcard]) * states];
            for (int s = 0; s < states; ++s) d[s] = min(zero[s], one[s]);
        }
    }

    int distance(int state, int required, int target) const {
        return distance_table[partial_code[required][target] * states + state];
    }

    // Among shortest paths, prefer a final stack with a lower average distance
    // to generic future targets. The recursion strictly decreases distance.
    uint16_t future_cost(int code, int state) {
        uint16_t& cached = terminal_cost[code * states + state];
        if (cached != 65535) return cached;
        int d = distance_table[code * states + state];
        if (d == 0) return cached = average_cost[state];
        int best = 65535;
        for (const Edge& e : edges[state]) {
            if (!((visible[state] >> e.paper) & 1)) continue;
            if (distance_table[code * states + e.next] + 1 != d) continue;
            best = min(best, int(future_cost(code, e.next)));
        }
        return cached = uint16_t(best);
    }

    Edge step(int state, int required, int target) {
        int code = partial_code[required][target];
        int d = distance_table[code * states + state];
        int best = future_cost(code, state);
        for (const Edge& e : edges[state]) {
            if (!((visible[state] >> e.paper) & 1)) continue;
            if (distance_table[code * states + e.next] + 1 != d) continue;
            if (future_cost(code, e.next) == best) return e;
        }
        abort();
    }
};

struct Group {
    int column, paper_base, state = 0;
};

struct Design {
    array<int, N> count{}, base{}, phase{};
    array<int, K> level{};
    array<U64, K> mask{}, color{};
    int first_dictionary = 35, used_papers = 35;

    explicit Design(const vector<U64>& catalogue) {
        for (int j = CUT; j < N; ++j) count[j] = 2;
        int extra = K - first_dictionary - 2 * (N - CUT);
        // Even queue lengths preserve alternating handle colors under rotation.
        for (int t = 0; t < extra / 2; ++t) {
            int chosen = -1;
            double best = 1e100;
            for (int j = CUT; j < N; ++j) {
                double marginal = 2 * count[j] - ALLOCATION_SLOPE * j;
                if (marginal < best) { best = marginal; chosen = j; }
            }
            count[chosen] += 2;
        }
        for (int j = CUT; j < N; ++j) {
            base[j] = used_papers;
            used_papers += count[j];
            for (int z = 0; z < count[j]; ++z) level[base[j] + z] = j;
        }

        array<U64, N> prefix{};
        for (int j = 0; j < N; ++j)
            prefix[j] = j == 63 ? ~U64(0) : (U64(1) << (j + 1)) - 1;
        vector<int> benefit(P, 0);
        struct PairCandidate { int gain, a, b; U64 mask; };

        // Greedy construction, followed by coordinate improvement. The
        // objective estimates savings after accounting for retrieval cost.
        for (int pass = 0; pass <= REFINEMENTS; ++pass) {
            for (int j = N - 1; j >= CUT; --j) {
                for (int z = 0; z < count[j]; ++z) {
                    int k = base[j] + z, sign = z & 1;
                    if (pass != 0) {
                        fill(benefit.begin(), benefit.end(), 0);
                        for (int q = first_dictionary; q < used_papers; ++q) {
                            if (q == k) continue;
                            int value = popcount(mask[q]) - count[level[q]] - 1;
                            if (value <= 0) continue;
                            for (int a = 0; a < P; ++a)
                                if (((catalogue[a] ^ color[q]) & mask[q]) == 0)
                                    benefit[a] = max(benefit[a], value);
                        }
                        if (*min_element(benefit.begin(), benefit.end()) >= j - count[j])
                            continue;
                    }
                    vector<int> members;
                    for (int a = 0; a < P; ++a)
                        if (int((catalogue[a] >> j) & 1) == sign) members.push_back(a);
                    // This also handles unusual catalogues where an entire
                    // column is constant. A handle must never be a hole.
                    if (members.empty()) {
                        mask[k] = U64(1) << j;
                        color[k] = sign ? mask[k] : 0;
                        continue;
                    }

                    int best_gain = -1;
                    U64 best_mask = 0, best_color = 0;
                    vector<PairCandidate> pairs;
                    for (int a : members) {
                        int value = j - count[j];
                        int gain = max(0, value - benefit[a]);
                        if (gain > best_gain) {
                            best_gain = gain;
                            best_mask = prefix[j];
                            best_color = catalogue[a] & best_mask;
                        }
                        for (int b : members) {
                            if (b <= a) continue;
                            U64 common = prefix[j] & ~(catalogue[a] ^ catalogue[b]);
                            value = popcount(common) - count[j] - 1;
                            gain = max(0, value - benefit[a]) + max(0, value - benefit[b]);
                            pairs.push_back({gain, a, b, common});
                            if (gain > best_gain) {
                                best_gain = gain;
                                best_mask = common;
                                best_color = catalogue[a] & common;
                            }
                        }
                    }
                    constexpr int TOP_PAIRS = 12;
                    if (int(pairs.size()) > TOP_PAIRS) {
                        nth_element(pairs.begin(), pairs.begin() + TOP_PAIRS, pairs.end(),
                            [](const PairCandidate& a, const PairCandidate& b) {
                                return a.gain > b.gain;
                            });
                        pairs.resize(TOP_PAIRS);
                    }
                    // Extend promising pairs to groups of up to six products.
                    for (const auto& pair : pairs) {
                        vector<int> chosen{pair.a, pair.b};
                        U64 common = pair.mask;
                        for (int iteration = 0; iteration < 4; ++iteration) {
                            int next = -1, extension_gain = -1;
                            U64 extended_mask = 0;
                            for (int a : members) {
                                if (find(chosen.begin(), chosen.end(), a) != chosen.end()) continue;
                                U64 trial = common & ~(catalogue[a] ^ catalogue[chosen[0]]);
                                int value = popcount(trial) - count[j] - 1;
                                int gain = max(0, value - benefit[a]);
                                for (int q : chosen) gain += max(0, value - benefit[q]);
                                if (gain > extension_gain) {
                                    extension_gain = gain;
                                    next = a;
                                    extended_mask = trial;
                                }
                            }
                            if (next < 0) break;
                            chosen.push_back(next);
                            common = extended_mask;
                            if (extension_gain > best_gain) {
                                best_gain = extension_gain;
                                best_mask = common;
                                best_color = catalogue[chosen[0]] & common;
                            }
                        }
                    }
                    if (pass != 0) {
                        int old_gain = 0, new_gain = 0;
                        int old_value = popcount(mask[k]) - count[j] - 1;
                        int new_value = popcount(best_mask) - count[j] - 1;
                        for (int a = 0; a < P; ++a) {
                            if (((catalogue[a] ^ color[k]) & mask[k]) == 0)
                                old_gain += max(0, old_value - benefit[a]);
                            if (((catalogue[a] ^ best_color) & best_mask) == 0)
                                new_gain += max(0, new_value - benefit[a]);
                        }
                        if (new_gain <= old_gain) {
                            best_mask = mask[k];
                            best_color = color[k];
                        }
                    }
                    mask[k] = best_mask;
                    color[k] = best_color;
                    int value = popcount(best_mask) - count[j] - 1;
                    for (int a = 0; a < P; ++a)
                        if (((catalogue[a] ^ best_color) & best_mask) == 0)
                            benefit[a] = max(benefit[a], value);
                }
            }
        }
    }
};
}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int input_n, input_k, input_p, rounds;
    if (!(cin >> input_n >> input_k >> input_p >> rounds)) return 0;
    if (input_n != N || input_k != K || input_p != P || rounds < 0) return 0;
    vector<U64> catalogue(P, 0);
    for (U64& a : catalogue) {
        string text;
        if (!(cin >> text) || int(text.size()) != N) return 0;
        for (int j = 0; j < N; ++j) {
            if (text[j] != '0' && text[j] != '1') return 0;
            if (text[j] == '1') a |= U64(1) << j;
        }
    }
    Controller controller;
    vector<Group> groups;
    for (int g = 0; g < CUT / Controller::B; ++g)
        groups.push_back({g * Controller::B, g * Controller::PAPERS, 0});
    Design design(catalogue);

    vector<string> papers(K, string(N, '.'));
    for (const Group& g : groups)
        for (int k = 0; k < Controller::PAPERS; ++k)
            for (int b = 0; b < Controller::B; ++b)
                papers[g.paper_base + k][g.column + b] = controller.pattern[k][b];
    for (int k = design.first_dictionary; k < design.used_papers; ++k)
        for (int b = 0; b < N; ++b)
            if ((design.mask[k] >> b) & 1)
                papers[k][b] = char('0' + ((design.color[k] >> b) & 1));
    for (const string& s : papers) cout << s << '\n';
    cout.flush();

    U64 canonical_handles = 0;
    vector<int> overlays;  // Listed in increasing handle-window order.
    for (int turn = 0; turn < rounds; ++turn) {
        int product;
        if (!(cin >> product) || product < 0 || product >= P) return 0;
        U64 target = catalogue[product];
        vector<pair<int, int>> operations;
        operations.reserve(80);

        // Restore previous overlays from top to bottom. Their handle windows
        // make every selected paper visible when it is removed.
        for (int i = int(overlays.size()) - 1; i >= 0; --i) {
            int j = overlays[i];
            operations.emplace_back(design.base[j] + design.phase[j], design.base[j]);
        }
        overlays.clear();
        U64 wrong_handles = (target ^ canonical_handles) & (~U64(0) << CUT);
        auto residual_cost = [&](U64 covered) {
            int cost = popcount(wrong_handles & ~covered);
            for (const Group& g : groups) {
                int required = int((~covered >> g.column) & 63);
                int local_target = int((target >> g.column) & 63);
                cost += controller.distance(g.state, required, local_target);
            }
            return cost;
        };
        int initial_cost = residual_cost(0);

        struct Candidate { int handle, slot, cost; U64 mask; };
        vector<Candidate> candidates;
        for (int j = CUT; j < N; ++j) {
            for (int pos = 0; pos < design.count[j]; ++pos) {
                int slot = (design.phase[j] + pos) % design.count[j];
                int k = design.base[j] + slot;
                if (((design.color[k] ^ target) & design.mask[k]) != 0) continue;
                // Retrieval + activation + restoration on the next order.
                int cost = pos + 2;
                if (initial_cost - residual_cost(design.mask[k]) >= cost - 1)
                    candidates.push_back({j, slot, cost, design.mask[k]});
            }
        }
        int best_cost = initial_cost;
        vector<int> chosen;
        for (int i = 0; i < int(candidates.size()); ++i) {
            const auto& a = candidates[i];
            int cost = a.cost + residual_cost(a.mask);
            if (cost < best_cost) { best_cost = cost; chosen = {i}; }
            for (int j = i + 1; j < int(candidates.size()); ++j) {
                const auto& b = candidates[j];
                if (a.handle == b.handle) continue;
                U64 covered = a.mask | b.mask;
                int subtotal = a.cost + b.cost;
                cost = subtotal + residual_cost(covered);
                if (cost < best_cost) { best_cost = cost; chosen = {i, j}; }
                for (int k = j + 1; k < int(candidates.size()); ++k) {
                    const auto& c = candidates[k];
                    if (c.handle == a.handle || c.handle == b.handle) continue;
                    cost = subtotal + c.cost + residual_cost(covered | c.mask);
                    if (cost < best_cost) { best_cost = cost; chosen = {i, j, k}; }
                }
            }
        }
        sort(chosen.begin(), chosen.end(), [&](int a, int b) {
            return candidates[a].handle < candidates[b].handle;
        });

        auto rotate_queue = [&](int j) {
            int k = design.base[j] + design.phase[j];
            operations.emplace_back(k, design.base[j] + design.count[j] - 1);
            design.phase[j] = (design.phase[j] + 1) % design.count[j];
            canonical_handles ^= U64(1) << j;
        };
        U64 covered = 0;
        for (int index : chosen) {
            const auto& c = candidates[index];
            while (design.phase[c.handle] != c.slot) rotate_queue(c.handle);
            covered |= c.mask;
        }
        // Correct only the windows that will remain visible through holes.
        for (Group& g : groups) {
            int required = int((~covered >> g.column) & 63);
            int local_target = int((target >> g.column) & 63);
            while (controller.distance(g.state, required, local_target) != 0) {
                auto e = controller.step(g.state, required, local_target);
                operations.emplace_back(g.paper_base + e.paper, g.paper_base + e.position);
                g.state = e.next;
            }
        }
        for (int j = CUT; j < N; ++j)
            if ((((target ^ canonical_handles) & ~covered) >> j) & 1)
                rotate_queue(j);
        // Activate in increasing handle order: earlier overlays cannot cover
        // the handle of a later overlay, so every activation is legal.
        for (int index : chosen) {
            int j = candidates[index].handle;
            operations.emplace_back(design.base[j] + design.phase[j], 0);
            overlays.push_back(j);
        }
        if (operations.size() > 2 * K) return 0;  // Defensive; proven bound is 67.
        cout << operations.size() << '\n';
        for (auto [k, d] : operations) cout << k << ' ' << d << '\n';
        cout.flush();
    }
    return 0;
}
