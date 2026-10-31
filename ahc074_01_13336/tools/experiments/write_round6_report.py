from pathlib import Path
import hashlib,json,re,shutil
from collections import Counter
root=Path(__file__).resolve().parents[2];d=root/'tools/experiments'
load=lambda name:json.loads((d/name).read_text())
digest=hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest()
assert digest=='9c78b086622882223782412512fa1ab4b3d2b0c538e818f75be0aa5f4cbba7f4'
v=load('validation.json');s=v['summary'];old=load('baseline_13350_validation.json')['summary']
extra=load('round6_final_validation.json');oldextra=load('round5_final_validation.json')
assert s['source_sha256']==digest and s['errors']==0
assert extra['source_sha256']==digest and len(extra['cases'])==100 and all(x['errors']==0 for x in extra['cases'])
for name in ['round6_cutoff_results.json','round6_ubsan_results.json']:
    check=load(name);assert check['source_sha256']==digest and check['status']=='passed'
    assert all(x['errors']==0 for x in check['cases'])
rows=[f'| pahcer、seed 0–99 | 100 | {old["score_mean"]:,.2f} | {s["score_mean"]:,.2f} | {s["max_cpu_seconds"]:.3f}秒 |']
for a,b in zip(oldextra['batches'],extra['batches']):
    rows.append(f'| seed {b["first_seed"]}–{b["first_seed"]+49} | 50 | {a["mean"]:,.2f} | {b["mean"]:,.2f} | {b["max_cpu"]:.3f}秒 |')
counts=Counter();fallbacks=0
for seed in range(100):
    err=(root/f'tools/err/{seed:04d}.txt').read_text()
    m=re.search(r'AdaptiveR=(\d+)',err);counts[int(m[1]) if m else 0]+=1
    fallbacks+=int(re.search(r'PathFallbacks=(\d+)',err)[1])
fallbacks+=sum(x['fallbacks'] for x in extra['batches'])
maxcpu=max(s['max_cpu_seconds'],*(x['max_cpu'] for x in extra['batches']))
maxmoves=max(s['max_operations_per_order'],*(x['max_moves'] for x in extra['batches']))
archive=f'main_{int(s["score_mean"])}_round6_final.cpp'
shutil.copyfile(root/'main.cpp',d/archive)
shutil.copyfile(d/'validation.json',d/'round6_validation.json')
report=f'''# AHC074 Round 6 — 構造探索と学習処理の高速化

今回、大幅な操作数削減を確認できた設計は得られなかった。採用した変更は、既存の学習結果を保つDP・trieの高速化と、学習量見積もり・時間ガードの調整。提出候補は `main.cpp`。

ユーザー報告による変更前のAtCoder結果は、50ケース合計667,994点、実行時間1739ms。今回の版はローカル検証までで、AtCoderには未提出。

| 検証 | 件数 | 変更前の平均 | 変更後の平均 | 変更後の最大CPU時間 |
|---|---:|---:|---:|---:|
{chr(10).join(rows)}

同じ最終ソース・実行ファイルを公式テスタで200件検証し、全件で誤注文・不正操作ゼロ。1注文の最大操作数は{maxmoves}回（上限384回）、経路fallbackは{fallbacks}回。対話待ちを除くローカルCPU時間は200件最大{maxcpu:.3f}秒。WSLの対話I/Oを含むpahcer実時間とは異なる。

## 採用内容

公開256商品のsuffix trieは紙模様の学習中も構造が変わらない。この構造を長さごとに保存し、子が1つだけの連続部分をまとめた。各長さのnode数は最大511になり、同じ構造を初期学習と実操作数による再学習で再利用する。まとめた部分の重みは同じ子孫へ加算し、葉・分岐の列挙順も維持するため、最適値と同点時の選択が変わらない。

初期学習の前方DPは降順passごとに一度計算し、後方DPと長さをまたぐ経路の費用は確定した長い紙から順に再利用する。固定R8・R32の各5seedでは、元のソースと設計・全操作のstdoutが完全に一致した。参照式を残したビルドでも、前後DP、trieの全選択枚数に対する値、葉列、回避経路を照合した。

固定条件の各5件の比較では、初期学習の平均CPUが約0.48～0.52秒から約0.058～0.062秒へ短縮した。これはローカルでの診断用測定で、AtCoder上の新しい実行時間ではない。

高速化後は従来の学習量見積もりが多めの状態数を選び、最初の100seed計測で最大CPU1.943秒となった。そのため見積もり係数を4.0から5.0へ変更し、再学習中の64状態ごと・紙の更新前にも期限を確認するようにした。途中で終了しても、それまでに受理した紙模様は全256商品の到達性を保つ。初期学習0.5秒、再学習1.65秒、最大32状態/商品という上限は維持。

時間ガード付き実行では、処理速度に応じて学習量や完了範囲が変わるため得点も変わる。今回のpahcerで選ばれた状態数の件数は `{dict(sorted(counts.items()))}`。固定した学習量での高速化自体は出力を変えない。

## 構造探索

接頭辞の前方を穴にする案、末尾2～4窓を複数紙で分担する案、全2窓ブロックの専用辞書学習、保存状態を保つ任意mask、可視制約を外した終状態最適化、紙の剪定、長さ再配分、列順・反転最適化、共同模様学習、連続注文でのrollout判定、複数設計の選択を試した。小さい改善が出た案もあったが、広い比較での改善や2秒以内の計算量を両立せず、採用していない。

現行の紙は特定商品の長いprefixを一度に表示する役割を持つ。穴を増やして取り出しやすくすると、この役割や別のgoalを選ぶ自由度を失い、補う操作が増えるケースが多かった。可視制約を外す下限との隔たりは残っているが、その下限は実行できる手順を意味しない。上位に追いつく大幅な改善、当初の平均約6,554点という目標には未到達。

詳細は `round6_notes.md`、`round6_design_notes.md`、`round6_policy_notes.md`、`round6_column_notes.txt`。高速化の根拠と照合結果は `round6_initial_fast_notes.md`。

## 最終検証と保存先

- `validation.json` / `round6_validation.json`: 最終版pahcer 100件。
- `round6_final_validation.json`: 同じ実行ファイルによる追加100件。
- `round6_cutoff_results.json`: 最終版の低予算15件。全操作を独立replayして合法性・表示一致を確認。
- `round6_ubsan_results.json`: 最終版の全学習・R32を実行するUBSan検証。
- `{archive}`: 提出候補の保存コピー。
- `main_13350_final.cpp` / `baseline_13350_validation.json`: 変更前の版と結果。
- `round6_triefast_*validation.json`: 学習量見積もりを調整する前の中間結果。

ソースSHA-256: `{digest}`

再実行:

```sh
pahcer run
python3 tools/experiments/summarize.py
python3 tools/experiments/check_round6_final.py
python3 tools/experiments/check_round6_cutoff.py
python3 tools/experiments/check_round6_cutoff.py --ubsan
```
'''
(d/'README.md').write_text(report,encoding='utf-8')
(root/f'record/README_round6_{int(s["score_mean"])}.md').write_text(report,encoding='utf-8')
print(json.dumps(dict(mean=s['score_mean'],max_cpu=maxcpu,max_moves=maxmoves,
                     fallbacks=fallbacks,adaptive_counts=counts,archive=archive,
                     source_sha256=digest),indent=2))
