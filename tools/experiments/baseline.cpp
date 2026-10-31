
#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, K, P, R;
    if (!(cin >> N >> K >> P >> R)) return 0;

    vector<string> A(P);
    for (int v = 0; v < P; ++v) cin >> A[v];

    // Each window j gets two papers:
    // paper 2*j     = white at j, holes elsewhere
    // paper 2*j + 1 = black at j, holes elsewhere
    vector<string> S(K, string(N, '.'));
    for (int j = 0; j < N; ++j) {
        S[2 * j][j] = '0';
        S[2 * j + 1][j] = '1';
    }

    // Initial display is all white.
    for (const string& s : S) {
        cout << s << '\n';
    }
    cout.flush();

    string current(N, '0');

    for (int r = 0; r < R; ++r) {
        int a;
        if (!(cin >> a) || a == -1) return 0;
        if (a < 0 || a >= P) return 0;

        vector<pair<int, int>> operations;

        for (int j = 0; j < N; ++j) {
            if (current[j] == A[a][j]) continue;

            // Move the currently visible paper to bottom.
            int visible_paper =
                2 * j + (current[j] == '1' ? 1 : 0);

            operations.emplace_back(visible_paper, K - 1);
            current[j] = A[a][j];
        }

        cout << operations.size() << '\n';

        for (auto [k, d] : operations) {
            cout << k << ' ' << d << '\n';
        }

        cout.flush();
    }

    return 0;
}
