#include <bits/stdc++.h>
using namespace std;

// A six-paper design represents all sixteen patterns on four windows.
// Its 720 permutations permit exact planning inside each independent block.
struct BlockMachine {
    static constexpr int STATES = 720;
    static constexpr int MASK[6] = {11, 15, 2, 15, 15, 8};
    static constexpr int BLACK[6] = {8, 13, 2, 9, 8, 0};
    struct Edge { short from; unsigned char action; };
    array<array<unsigned char, 6>, STATES> order{};
    array<array<short, 36>, STATES> next{};
    array<vector<Edge>, STATES> reverse;
    array<int, STATES> display{};
    static int rank(const array<unsigned char, 6>& p) {
        int r = 0;
        for (int i = 0; i < 6; ++i) {
            int less = 0;
            for (int j = i + 1; j < 6; ++j) less += p[j] < p[i];
            r = r * (6 - i) + less;
        }
        return r;
    }
    BlockMachine() {
        array<unsigned char, 6> p = {0, 1, 2, 3, 4, 5};
        int s = 0;
        do { order[s++] = p; } while (next_permutation(p.begin(), p.end()));
        for (s = 0; s < STATES; ++s) {
            int covered = 0, bits = 0;
            next[s].fill(-1);
            for (int i = 0; i < 6; ++i) {
                int k = order[s][i];
                bits |= BLACK[k] & ~covered;
                if (MASK[k] & ~covered) {
                    for (int j = 0; j < 6; ++j) if (i != j) {
                        auto q = order[s];
                        if (i < j) rotate(q.begin() + i, q.begin() + i + 1, q.begin() + j + 1);
                        else rotate(q.begin() + j, q.begin() + i, q.begin() + i + 1);
                        int t = rank(q), action = i * 6 + j;
                        next[s][action] = t;
                        reverse[t].push_back({short(s), (unsigned char)action});
                    }
                }
                covered |= MASK[k];
            }
            display[s] = bits;
        }
    }
    using Policy = array<array<signed char, STATES>, 16>;
    Policy optimize(const array<int, 16>& count, int products) const {
        array<double, STATES> value{}, updated{};
        Policy policy;
        // Relative value iteration, accounting for exclusion of a repeated
        // product. Terminal values choose favorable orders for later requests.
        for (int iteration = 0; iteration < 32; ++iteration) {
            updated.fill(0);
            for (int target = 0; target < 16; ++target) {
                array<double, STATES> distance;
                distance.fill(1e100);
                policy[target].fill(-1);
                priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> queue;
                for (int s = 0; s < STATES; ++s) if (display[s] == target) {
                    distance[s] = value[s];
                    queue.push({value[s], s});
                }
                while (!queue.empty()) {
                    auto [cost, s] = queue.top(); queue.pop();
                    if (cost > distance[s] + 1e-10) continue;
                    for (auto e : reverse[s]) if (distance[e.from] > cost + 1 + 1e-10) {
                        distance[e.from] = cost + 1;
                        policy[target][e.from] = e.action;
                        queue.push({cost + 1, e.from});
                    }
                }
                for (int s = 0; s < STATES; ++s) {
                    double probability = double(count[target] - (display[s] == target)) / (products - 1);
                    updated[s] += probability * distance[s];
                }
            }
            double offset = updated[0];
            for (int s = 0; s < STATES; ++s) value[s] = updated[s] - offset;
        }
        return policy;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int N, K, P, R;
    if (!(cin >> N >> K >> P >> R)) return 0;
    vector<string> A(P);
    for (auto& s : A) cin >> s;
    if (N != 64 || K != 192 || P < 2) return 0;

    mt19937 rng(712367);
    array<int, 64> columns, best_columns;
    iota(columns.begin(), columns.end(), 0);
    auto entropy = [&](int group) {
        int count[16]{};
        for (auto& s : A) {
            int code = 0;
            for (int b = 0; b < 4; ++b) code |= (s[columns[group * 4 + b]] - '0') << b;
            ++count[code];
        }
        double h = 0;
        for (int c : count) if (c) h -= double(c) / P * log(double(c) / P);
        return h;
    };
    double current = 0;
    for (int g = 0; g < 16; ++g) current += entropy(g);
    double best = current;
    best_columns = columns;
    constexpr int ITERATIONS = 30000;
    for (int it = 0; it < ITERATIONS; ++it) {
        int x = rng() % N, y = rng() % N;
        if (x / 4 == y / 4) continue;
        double old = entropy(x / 4) + entropy(y / 4);
        swap(columns[x], columns[y]);
        double delta = entropy(x / 4) + entropy(y / 4) - old;
        double temperature = 0.006 * pow(0.001, double(it) / ITERATIONS);
        if (delta <= 0 || double(rng()) / (double(rng.max()) + 1) < exp(-delta / temperature)) {
            current += delta;
            if (current < best) best = current, best_columns = columns;
        } else swap(columns[x], columns[y]);
    }
    columns = best_columns;

    BlockMachine machine;
    array<int, 16> uniform_count;
    uniform_count.fill(16);
    auto uniform_policy = machine.optimize(uniform_count, 256);
    array<array<short, 720>, 16> end_state;
    array<array<unsigned char, 720>, 16> move_count;
    for (int t = 0; t < 16; ++t) for (int s = 0; s < 720; ++s) {
        int cur = s, length = 0;
        while (uniform_policy[t][cur] >= 0) {
            cur = machine.next[cur][uniform_policy[t][cur]];
            ++length;
        }
        end_state[t][s] = cur;
        move_count[t][s] = length;
    }
    // Use public barcodes, with an independently sampled sequence, to select
    // a coordinate/color mapping. Actual orders are not read ahead.
    vector<int> sample(2048);
    for (int& a : sample) a = rng() % P;
    vector<BlockMachine::Policy> policy;
    array<int, 16> flips{};
    vector<string> paper(K, string(N, '.'));
    for (int g = 0; g < 16; ++g) {
        array<int, 4> orig, perm = {0, 1, 2, 3}, chosen;
        for (int b = 0; b < 4; ++b) orig[b] = columns[g * 4 + b];
        int best_cost = INT_MAX, chosen_flip = 0;
        do {
            vector<int> codes(P);
            for (int a = 0; a < P; ++a) for (int b = 0; b < 4; ++b)
                codes[a] |= (A[a][orig[perm[b]]] - '0') << b;
            for (int flip = 0; flip < 16; ++flip) {
                int state = 0, cost = 0;
                for (int a : sample) {
                    int t = codes[a] ^ flip;
                    cost += move_count[t][state];
                    state = end_state[t][state];
                }
                if (cost < best_cost) best_cost = cost, chosen = perm, chosen_flip = flip;
            }
        } while (next_permutation(perm.begin(), perm.end()));
        for (int b = 0; b < 4; ++b) columns[g * 4 + b] = orig[chosen[b]];
        flips[g] = chosen_flip;
        array<int, 16> count{};
        for (auto& s : A) {
            int code = 0;
            for (int b = 0; b < 4; ++b) code |= (s[columns[g * 4 + b]] - '0') << b;
            ++count[code ^ flips[g]];
        }
        policy.push_back(machine.optimize(count, P));
        for (int k = 0; k < 6; ++k) for (int b = 0; b < 4; ++b) if (BlockMachine::MASK[k] >> b & 1) {
            int color = ((BlockMachine::BLACK[k] ^ flips[g]) >> b) & 1;
            paper[6 * g + k][columns[4 * g + b]] = char('0' + color);
        }
    }
    for (auto& s : paper) cout << s << '\n';
    cout.flush();
    array<int, 16> state{};
    for (int r = 0; r < R; ++r) {
        int a;
        if (!(cin >> a) || a < 0 || a >= P) return 0;
        vector<pair<int, int>> moves;
        for (int g = 0; g < 16; ++g) {
            int target = 0;
            for (int b = 0; b < 4; ++b) target |= (A[a][columns[4 * g + b]] - '0') << b;
            target ^= flips[g];
            while (policy[g][target][state[g]] >= 0) {
                int action = policy[g][target][state[g]];
                moves.emplace_back(6 * g + machine.order[state[g]][action / 6], 6 * g + action % 6);
                state[g] = machine.next[state[g]][action];
            }
        }
        cout << moves.size() << '\n';
        for (auto [k, d] : moves) cout << k << ' ' << d << '\n';
        cout.flush();
    }
}
