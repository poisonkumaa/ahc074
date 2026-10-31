from pathlib import Path
import hashlib
import json
import re
import shutil
from collections import Counter

root = Path(__file__).resolve().parents[2]
d = root / 'tools/experiments'
load = lambda name: json.loads((d / name).read_text())
main = (root / 'main.cpp').read_bytes()
digest = hashlib.sha256(main).hexdigest()
assert digest == '12f6ae6570b5d322e73a2e169e7f05d7ce5058a775fd65892f7b5da6f4209142'
validation = load('validation.json')
s = validation['summary']
assert s['source_sha256'] == digest and s['errors'] == 0
extra = load('round5_final_validation.json')
assert extra['source_sha256'] == digest and all(x['errors'] == 0 for x in extra['cases'])
for check in ['round5_cutoff_results.json', 'round5_ubsan_results.json']:
    result = load(check)
    assert result['source_sha256'] == digest and result['status'] == 'passed'
    assert all(x['errors'] == 0 for x in result['cases'])
old = load('baseline_13530_validation.json')['summary']
oldextra = load('round4_final_validation.json')
ratio = 100 * (1 - s['score_mean'] / old['score_mean'])
maxcpu = max(s['max_cpu_seconds'], *(x['max_cpu'] for x in extra['batches']))
maxmoves = max(s['max_operations_per_order'], *(x['max_moves'] for x in extra['batches']))
counts = Counter()
fallbacks = 0
for seed in range(100):
    err = (root/f'tools/err/{seed:04d}.txt').read_text()
    found = re.search(r'AdaptiveR=(\d+)', err)
    counts[int(found[1]) if found else 0] += 1
    fallbacks += int(re.search(r'PathFallbacks=(\d+)', err)[1])
fallbacks += sum(x['fallbacks'] for x in extra['batches'])
name = f'main_{int(s["score_mean"])}_final.cpp'
(d/name).write_bytes(main)
shutil.copyfile(d/'validation.json', d/f'validation_{int(s["score_mean"])}.json')
rows = [f'| pahcer、seed 0–99 | 100 | {old["score_mean"]:,.2f} | {s["score_mean"]:,.2f} | 0 | {s["max_cpu_seconds"]:.3f}秒 |']
for before, after in zip(oldextra['batches'], extra['batches']):
    first = after['first_seed']
    rows.append(f'| seed {first}–{first+49} | 50 | {before["mean"]:,.2f} | {after["mean"]:,.2f} | 0 | {after["max_cpu"]:.3f}秒 |')
report = f'''# AHC074 実験記録 — 紙の配分と学習の高速化

提出候補は `main.cpp`。seed 0–99の平均は **{old['score_mean']:,.2f} → {s['score_mean']:,.2f}点**（{ratio:.2f}%削減）。目標の平均約6,554点には未到達。

前の版について、ユーザーからAtCoderで50ケース合計678,704点・実行時間613msとの報告があった。今回の版はローカル検証までで、AtCoderへは未提出。

| 検証 | 件数 | 変更前 | 変更後 | 誤注文 | 最大CPU時間 |
|---|---:|---:|---:|---:|---:|
{chr(10).join(rows)}

最終ソースをビルドした同一実行ファイルを公式テスタで200件検証し、全件成功。最大操作数は1注文あたり{maxmoves}回（上限384回）、経路生成fallbackは合計{fallbacks}回。CPU時間は対話待ちを除くローカル計測値で、200件最大{maxcpu:.3f}秒。WSLの対話I/Oを含むpahcerの実時間とは異なる。CPU時間に応じて学習量が変わるため、環境によって得点も変わる。

## 採用した変更

- 初期の追加紙を、長さ53以上には8枚ずつ、以下には4枚ずつ割り当てる。長い接頭辞を扱う紙を増やし、実操作数を減らした。計192枚の上限は維持。
- 再学習の前方DPは有限状態だけを保存し、距離・位置・累積個数を8bitに圧縮。後方DPは同じ長さの紙をまとめて更新し、長さをまたぐ経路の費用を再利用する。trieの重みも値が変わる深さだけ更新する。これらは元の費用計算と等価。
- 再学習に使う各商品の状態数を、計測した速度から8・12・16・24・32のいずれかへ調整。seed 0–99の選択回数は `{dict(sorted(counts.items()))}`。初期学習はCPU 0.5秒、再学習は開始から1.65秒を目安に打ち切る。打切り時も全256商品に到達できる設計を保つ。

各注文で現在の紙順からの最小合法操作数を求める方法、同数手の中で次に有利な紙順を選ぶ方法は維持した。設計・学習に使う入力は公開された256商品だけで、内部注文は独立した乱数で生成する。実注文は1件ずつ読み、各出力後にflushする。単一のC++17ソースで、外部ファイル・引数・環境変数に依存しない。

## 比較と検証

時間打切りを十分緩めた固定条件の100seed比較では、紙配分の変更のみで13,530.22→13,337.73点、状態数を32へ増やすと13,323.33点だった。複数設計から選ぶ方式は13,320.16点で、追加計算に対して改善が小さく不採用。固定条件の値と、上表の時間制限付き最終版の値は区別する。

圧縮・後方DPの集約・経路キャッシュ・trie差分化は、元の実装との出力または全ケースの得点一致を確認した。参照式を残す検査ビルドでは、全後方DPセルと各回避経路の費用も照合した。最終版では学習予算を縮めた15件を公式テスタと独立した全操作再生で検証し、さらに全学習を実行するUBSanビルドを検証した。詳細は `round5_cutoff_results.json`、`round5_ubsan_results.json`。

紙の長さの変更、隣接窓と紙模様の同時変更、先を見た操作選択、同じ長さの紙の再学習反復なども比較したが、安定した追加改善がなく採用していない。詳しい探索記録は `round5_notes.md`、実装上の根拠は `round5_implementation_notes.md`。

## 保存先

- `validation.json` / `validation_{int(s['score_mean'])}.json`: 最終版pahcer 100件。
- `round5_final_validation.json`: 同じ実行ファイルによる追加100件。
- `round5_cutoff_results.json` / `round5_ubsan_results.json`: 打切りとUBSan。
- `{name}`: 最終版の保存コピー。
- `main_13530_final.cpp` / `baseline_13530_validation.json`: 変更前の版と結果。

ソースSHA-256: `{digest}`

再実行:

```sh
pahcer run
python3 tools/experiments/summarize.py
python3 tools/experiments/check_round5_final.py
python3 tools/experiments/check_round5_cutoff.py
python3 tools/experiments/check_round5_cutoff.py --ubsan
```
'''
(d/'README.md').write_text(report, encoding='utf-8')
(root/f'record/README_{int(s["score_mean"])}.md').write_text(report, encoding='utf-8')
print(json.dumps(dict(mean=s['score_mean'], improvement_percent=ratio, max_cpu=maxcpu,
                     max_moves=maxmoves, fallbacks=fallbacks, adaptive_counts=counts,
                     archive=name, source_sha256=digest), indent=2))
