from pathlib import Path
import hashlib
import json

d = Path(__file__).resolve().parent
root = d.parents[1]
source = (root / 'main.cpp').read_bytes()
digest = hashlib.sha256(source).hexdigest()
primary = json.loads((d / 'validation.json').read_text())
extra = json.loads((d / 'multi_final_validation.json').read_text())
cutoff = json.loads((d / 'multi_cutoff_results.json').read_text())
ubsan = json.loads((d / 'multi_cutoff_ubsan_results.json').read_text())
assert primary['summary']['source_sha256'] == extra['main_sha256'] == cutoff['source_sha256'] == ubsan['source_sha256'] == digest
assert not primary['summary']['errors'] and not extra['summary']['failed']
p = primary['summary']
table = [f"| pahcer、seed 0–99 | 100 | {p['score_mean']:,.2f} | 0 | {p['max_cpu_seconds']:.3f}秒 |"]
for b in extra['batches']:
    table.append(f"| seed {b['seed_first']}–{b['seed_last']} | {b['count']} | {b['mean_score']:,.2f} | {b['errors']} | {b['max_cpu']:.3f}秒 |")
maximum_moves = max(p['max_operations_per_order'], extra['summary']['max_moves'])
name = f"main_{int(p['score_mean'])}_final.cpp"
report = f'''# AHC074 実験記録 — 2設計の選択

提出候補は `main.cpp`。seed 0–99の平均は **13,906.13 → {p['score_mean']:,.2f}点**（{(1-p['score_mean']/13906.13)*100:.3f}%削減）。目標の平均約6,554点には未到達。

| 検証 | 件数 | 平均スコア | 誤注文 | 最大CPU時間 |
|---|---:|---:|---:|---:|
{chr(10).join(table)}

最終ソースと同一の実行ファイルを公式テスタで200件検証し、全件成功。最大操作数は1注文あたり{maximum_moves}回（上限384回）。表は対話待ちを除くプロセスCPU時間で、WSLの対話I/Oを含むpahcer実時間とは異なる。CPU時間で探索を打ち切るため、実行環境の負荷で評価状態数やスコアが変わる場合がある。

## 今回の変更

- 従来の窓順と、それを32位置回転した窓順から2種類の紙設計を作る。公開256商品のみを用いる。
- 内部生成した共通注文列で両設計を進め、最大128個の途中状態について、次の全255商品への最短操作数を平均する。両設計の評価が完了した状態だけを比較へ加える。
- 学習上限は開始からCPU 1.0秒、設計比較上限は1.4秒。最初の設計だけでも成立し、1設計しか作れなかった場合は比較を省略する。
- 到達可能なDP状態だけを計算し、公開商品の適合紙を事前列挙する。設計学習の前後距離は模様が変わったときだけ再計算する。
- 内部シミュレーションでは到達する紙順を直接構成する。実注文では、最短距離を1減らす合法操作を1つずつ生成する。最初にそのような移動が可能な可視紙を選び、その挿入位置を将来コストで選ぶ。

紙の不透明部分を並べ替えた窓のprefixに限定し、全64窓を一体で扱う方式は継続。各注文の最小操作数をDPで求め、同点では直近64状態で使われた紙を優先する。実注文は1件ずつ読み、設計・各回答の末尾でflushする。未来の実注文は参照しない。提出ファイルは外部ファイル・引数・環境変数に依存しない。

## 正当性と打切りの検証

- 新しいDPと操作生成を、4窓6紙の全720順列×16目標＝11,520組について逆BFSと照合。最短距離・可視紙制約・最終表示に不一致なし。
- 内部直接構成は100seed・121,600遷移で逐次版と最終紙順が一致。任意の順列に対する逐次版との一般的同値性を主張するものではない。
- 学習／比較の予算を変えた{len(cutoff['cases'])}件で、途中打切り後も全操作の合法性と誤注文0を確認。部分比較まで到達したケースは{cutoff['partial_selection_cases']}件。
- 最終ソースのUBSanビルドを公式テスタで検証し、正常終了。

今回試して採用しなかった紙の割当・学習目的・オンライン探索などは `round3_notes.md` に記録。旧版と結果は削除せず保存している。

## 保存先

- `validation.json`: 最終版のpahcer 100件。
- `multi_final_validation.json`: 最終版の追加100件、ソース・実行ファイルhash付き。
- `multi_cutoff_results.json` / `multi_cutoff_ubsan_results.json`: 打切りとUBSan。
- `{name}`: 最終版の保存コピー。
- `main_13906_final.cpp`、`baseline_13906_validation.json`: 変更前の版と100件結果。
- `README_before_13853.md`: 変更前の説明と過去の実験履歴。

ソースSHA-256: `{digest}`

再実行:

```sh
pahcer run
python3 tools/experiments/summarize.py
python3 tools/experiments/check_multi_final.py
python3 tools/experiments/check_multi_cutoff.py
python3 tools/experiments/check_multi_cutoff.py --ubsan
```
'''
old = d / 'README.md'
archive = d / 'README_before_13853.md'
if not archive.exists():
    archive.write_bytes(old.read_bytes())
old.write_text(report, encoding='utf-8')
(root / 'record' / f"README_{int(p['score_mean'])}.md").write_text(report, encoding='utf-8')
(d / name).write_bytes(source)
print(report)
