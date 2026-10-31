from pathlib import Path
import hashlib,json,re
d=Path(__file__).resolve().parent;root=d.parents[1]
source=(root/'main.cpp').read_bytes();digest=hashlib.sha256(source).hexdigest()
primary=json.loads((d/'validation.json').read_text());extra=json.loads((d/'round4_final_validation.json').read_text());cutoff=json.loads((d/'round4_cutoff_results.json').read_text());ubsan=json.loads((d/'round4_ubsan_results.json').read_text())
assert primary['summary']['source_sha256']==extra['source_sha256']==cutoff['source_sha256']==ubsan['source_sha256']==digest
p=primary['summary'];assert not p['errors'] and all(not r['errors'] for r in extra['cases'])
table=[f"| pahcer、seed 0–99 | 100 | {p['score_mean']:,.2f} | 0 | {p['max_cpu_seconds']:.3f}秒 |"]
for b in extra['batches']:table.append(f"| seed {b['first_seed']}–{b['first_seed']+49} | {b['count']} | {b['mean']:,.2f} | 0 | {b['max_cpu']:.3f}秒 |")
maximum_moves=max([p['max_operations_per_order']]+[b['max_moves'] for b in extra['batches']])
fallbacks=sum(int(re.search(r'PathFallbacks=(\d+)',(root/f'tools/err/{i:04d}.txt').read_text())[1]) for i in range(100))+sum(b['fallbacks'] for b in extra['batches'])
name=f"main_{int(p['score_mean'])}_final.cpp"
report=f'''# AHC074 実験記録 — 実操作数による再学習

提出候補は `main.cpp`。seed 0–99の平均は **13,853.88 → {p['score_mean']:,.2f}点**（{(1-p['score_mean']/13853.88)*100:.2f}%削減）。目標の平均約6,554点には未到達。

| 検証 | 件数 | 平均スコア | 誤注文 | 最大CPU時間 |
|---|---:|---:|---:|---:|
{chr(10).join(table)}

最終ソースをビルドした同一実行ファイルを、公式テスタで200件検証して全件成功。最大操作数は1注文あたり{maximum_moves}回（上限384回）。CPU時間は対話待ちを除く値で、WSLの対話I/Oを含むpahcer実時間とは異なる。探索はCPU時間で打ち切るため、環境により学習量と得点が変わる場合がある。

## 今回の実装

公開された256商品から内部注文を生成し、各商品につき8通り、計2048通りの紙順と目標の組を集める。その組に対する最小の合法操作数を基準に、長さ64から43までの紙模様を再学習する。色の変更で変わるのは紙を利用できる開始位置だけなので、前後のDP距離とtrieから最良の候補を求められる。全商品の到達性を保ち、サンプル上の費用が減る変更だけを受理する。

前後DPの距離を長さごとに再利用し、trieでは葉数を超える不可能な選択を省く。操作生成は到達する紙順を先に計算し、その順に合法な移動を構成する。直接構成が成立しない場合の従来経路も残している（今回200件でfallback {fallbacks}回）。各注文では引き続き最小操作回数を求める。

初期設計の学習上限はCPU 0.5秒、実操作数による再学習は開始から1.45秒。打切りは有効な設計を残す。設計と内部シミュレーションに使う入力は初期の256商品だけで、実注文は1件ずつ読み、設計・各回答後にflushする。単一のC++17ソースで、外部ファイル・引数・環境変数に依存しない。

ユーザー提供v7の256商品履歴、初期模様の変更、複数設計の選択も比較した。今回の再学習との組合せでは既定の64状態履歴が良かったため維持した。少数seedだけでは再学習の反復が良く見えたが、100seed比較では8状態を使う1回の再学習を採用した。詳細は `round4_notes.md`。

## 検証

- 直接の操作列生成は、旧設計100件と新設計100件で従来経路と各スコアが一致し、全E0。
- 最終順序計算の差分更新は、ランダムな到達状態10000例と新設計10件の全注文で従来版と一致。
- 初期trie高速化と距離キャッシュも比較検証し、設計・スコアの一致を確認。
- 最終ソースの学習打切り{len(cutoff['cases'])}件は、公式テスタと独立した全操作の再生で合法性・誤注文0を確認。
- 最終ソースのUBSanビルドも、再学習を最後まで実行して公式テスタで正常終了。

## 保存先

- `validation.json`: 最終版のpahcer 100件。
- `round4_final_validation.json`: 最終版の追加100件とソース・実行ファイルhash。
- `round4_cutoff_results.json` / `round4_ubsan_results.json`: 打切りとUBSan。
- `{name}`: 最終版の保存コピー。
- `main_13853_final.cpp` / `baseline_13853_validation.json`: 変更前の版と結果。
- `round4_notes.md`: 探索の経過と不採用案。

ソースSHA-256: `{digest}`

再実行:

```sh
pahcer run
python3 tools/experiments/summarize.py
python3 tools/experiments/check_round4_final.py
python3 tools/experiments/check_round4_cutoff.py
python3 tools/experiments/check_round4_cutoff.py --ubsan
```
'''
old=d/'README.md';archive=d/'README_before_13530.md'
if not archive.exists():archive.write_bytes(old.read_bytes())
old.write_text(report,encoding='utf-8');(root/'record'/f"README_{int(p['score_mean'])}.md").write_text(report,encoding='utf-8');(d/name).write_bytes(source)
print(report)
