#include <bits/stdc++.h>
using namespace std;

// Four single-window papers ensure reachability; two further papers can
// change both windows together. Independent blocks stay contiguous.
struct PairMachine {
    static constexpr int STATES = 720;
    static constexpr int MASK[6] = {1, 1, 2, 3, 2, 3};
    static constexpr int BLACK[6] = {0, 1, 0, 0, 2, 2};
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
    PairMachine() {
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
    using Policy = array<array<signed char, STATES>, 4>;
    Policy optimize(const array<int, 4>& count, int products) const {
        array<double, STATES> value{}, updated{};
        Policy policy;
        // Relative value iteration, accounting for exclusion of a repeated
        // product. Terminal values choose favorable orders for later requests.
        for (int iteration = 0; iteration < 32; ++iteration) {
            updated.fill(0);
            for (int target = 0; target < 4; ++target) {
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
    array<int, 64> ones{};
    double weight[64][64]{};
    for (auto& s : A) for (int i = 0; i < N; ++i) ones[i] += s[i] == '1';
    for (int i = 0; i < N; ++i) for (int j = 0; j < i; ++j) {
        int count[4]{};
        for (auto& s : A) ++count[(s[i] - '0') + 2 * (s[j] - '0')];
        double information = 0;
        for (int b = 0; b < 4; ++b) if (count[b]) {
            double q = double(count[b]) / P;
            double x = double((b & 1) ? ones[i] : P - ones[i]) / P;
            double y = double((b & 2) ? ones[j] : P - ones[j]) / P;
            information += q * log(q / (x * y));
        }
        weight[i][j] = weight[j][i] = information;
    }
    mt19937 rng(712367);
    array<int, 64> matching, best_matching;
    iota(matching.begin(), matching.end(), 0);
    auto score = [&]() { double s = 0; for (int i = 0; i < N; i += 2) s += weight[matching[i]][matching[i + 1]]; return s; };
    double current = score(), best = current;
    best_matching = matching;
    constexpr int ITERATIONS = 200000;
    for (int it = 0; it < ITERATIONS; ++it) {
        int x = rng() % N, y = rng() % N;
        if ((x / 2) == (y / 2)) continue;
        int xp = x ^ 1, yp = y ^ 1;
        double delta = weight[matching[y]][matching[xp]] + weight[matching[x]][matching[yp]]
                     - weight[matching[x]][matching[xp]] - weight[matching[y]][matching[yp]];
        double temperature = 0.008 * pow(0.0005, double(it) / ITERATIONS);
        if (delta >= 0 || double(rng()) / (double(rng.max()) + 1) < exp(delta / temperature)) {
            swap(matching[x], matching[y]); current += delta;
            if (current > best) { best = current; best_matching = matching; }
        }
    }
    matching = best_matching;
    PairMachine machine;
    vector<PairMachine::Policy> policy;
    array<array<int, 2>, 32> columns{}, invert{};
    vector<string> paper(K, string(N, '.'));
    for (int group = 0; group < 32; ++group) {
        int x = matching[2 * group], y = matching[2 * group + 1];
        if (abs(2 * ones[x] - P) < abs(2 * ones[y] - P)) swap(x, y);
        columns[group] = {x, y};
        invert[group] = {ones[x] > P / 2, ones[y] > P / 2};
        array<int, 4> count{};
        for (auto& s : A) {
            int code = ((s[x] - '0') ^ invert[group][0])
                     | (((s[y] - '0') ^ invert[group][1]) << 1);
            ++count[code];
        }
        policy.push_back(machine.optimize(count, P));
        for (int k = 0; k < 6; ++k) for (int b = 0; b < 2; ++b) {
            if ((PairMachine::MASK[k] >> b) & 1) {
                int color = ((PairMachine::BLACK[k] >> b) & 1) ^ invert[group][b];
                paper[6 * group + k][columns[group][b]] = char('0' + color);
            }
        }
    }
    for (auto& s : paper) cout << s << '\n';
    cout.flush();
    array<int, 32> state{};
    for (int r = 0; r < R; ++r) {
        int a;
        if (!(cin >> a) || a < 0 || a >= P) return 0;
        vector<pair<int, int>> moves;
        for (int g = 0; g < 32; ++g) {
            int target = ((A[a][columns[g][0]] - '0') ^ invert[g][0])
                       | (((A[a][columns[g][1]] - '0') ^ invert[g][1]) << 1);
            while (policy[g][target][state[g]] >= 0) {
                int action = policy[g][target][state[g]];
                int i = action / 6, j = action % 6;
                moves.emplace_back(6 * g + machine.order[state[g]][i], 6 * g + j);
                state[g] = machine.next[state[g]][action];
            }
        }
        cout << moves.size() << '\n';
        for (auto [k, d] : moves) cout << k << ' ' << d << '\n';
        cout.flush();
    }
}
