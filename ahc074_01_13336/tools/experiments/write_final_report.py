from pathlib import Path
import hashlib,json

d=Path(__file__).resolve().parent;root=d.parents[1]
v=json.loads((d/'validation.json').read_text())['summary']
h=json.loads((d/'holdout/results.json').read_text())['summary']
f=json.loads((d/'final_holdout/results.json').read_text())['summary']
sha=hashlib.sha256((root/'main.cpp').read_bytes()).hexdigest()
assert all(x['source_sha256']==sha for x in (v,h,f))
assert v['errors']==h['errors']==f['errors']==0
label=str(int(v['score_mean']))
report=f'''# AHC074 実験記録

提出候補は `main.cpp`。目標の平均約6,554点には未到達。
前回の平均14,967.92点から **{v['score_mean']:,.2f}点**へ改善した（{100*(1-v['score_mean']/14967.92):.2f}%削減）。
最初の平均32,770点からの削減率は {100*(1-v['score_mean']/32770):.2f}%。

| 検証 | 件数 | 平均スコア | 誤注文 | 最大CPU時間 |
|---|---:|---:|---:|---:|
| `pahcer run`、seed 0–99 | 100 | {v['score_mean']:,.2f} | 0 | {v['max_cpu_seconds']:.5f}秒 |
| 追加seed 1000–1049 | 50 | {h['mean']:,.2f} | 0 | {h['max_cpu']:.5f}秒 |
| 最終版を確定してから生成したseed 5000–5049 | 50 | {f['mean']:,.2f} | 0 | {f['max_cpu']:.5f}秒 |

全200件で公式テスタが正常終了し、スコアと総操作数が一致した。最大操作数は1注文あたり {max(v['max_operations_per_order'],h['max_moves'],f['max_moves'])}回（制限384回）。
実行時間制限は2秒。表は対話待ちを除くプロセスCPU時間で、WSLの対話I/Oを含むpahcerの実時間とは区別する。
CPU時間による学習打切りがあるため、負荷によって学習回数とスコアが僅かに変わる。

## 現在の方式

- 全64窓をまとめて扱う。窓の並びを隣接窓の一致度で焼きなまし、各列の白黒反転も選ぶ。
- 各紙は並べ替えた窓の先頭からある長さまで不透明、残りは穴。可視紙の長さは上から増える。
- 最初に全バーコードを表示可能な65枚を用意し、残る127枚を長い紙へ配る。公開256商品の必要区間数を減らすようtrie上で色を学習し、その後は最初の65枚も含めて全紙を4巡再学習する。更新ごとに全商品の到達可能性を保つ。
- 注文ごとに「確定窓数・選択紙の元位置の最大値」のDPで最小操作数を求める。同じ操作数なら直近の注文でも使われやすい紙を優先する。
- 合法な最短操作の中から、直近64個の到達状態に対する必須取り出し回数が小さくなる操作を選ぶ。紙ごとの64bit集合の和集合で、同時に選ばれる紙の相関を評価する。初期集合だけは公開商品から内部生成した注文で作る。
- 学習をCPU 0.75秒で打ち切る。更新単位の途中では止めず、打切り後も全商品を表示できる。固定配列・DP領域再利用・前後距離による学習計算の省略で時間を短縮した。

紙の設計に使う入力は最初の256商品のみ。実注文は回答をflushしてから1つずつ読む。オンラインの64状態バッファは処理済み注文だけで更新する。
提出ファイルは単独でコンパイルでき、外部ファイル・引数・環境変数に依存しない。

## 正当性と高速化の確認

- 4窓8紙・全40,320順列で、future評価付き操作2,136,960件と重み付きDP645,120件をBFSと照合し、不一致0。
- 高速化前後について、時間打切りを無効にした100件で標準出力全文が一致。平均CPUは0.768秒から0.656秒へ14.6%短縮した。
- 学習上限0 / 0.1 / 0.3秒の9件を公式テスタで確認し、途中打切りでも誤注文・不正操作0。
- 未定義動作検出付きビルドも公式テスタで正常終了した。

紙の単体交換・長さ配分変更・隣接窓の再学習・複数到達状態の次注文評価・設計候補選別・任意の内部穴/2D構造も試したが、採用版を上回る安定した改善は得られなかった。試作と測定値は `tools/experiments` に保存してある。

## 保存結果

ソースSHA-256: `{sha}`

- `validation.json`: pahcer 100件のスコア、誤注文、CPU時間、ソースhash。
- `holdout/results.json`: seed 1000–1049の50件。
- `final_holdout/results.json`: seed 5000–5049の50件。
- `prefix_cutoff_results.json`: 学習途中打切り9件。
- `policy2_agent_perf_equivalence100.json`: 高速化の全文一致検証。
- `main_14967.cpp` / `main_14305.cpp` / `main_13938.cpp` / `main_13903.cpp`: 過去の提出候補。
- `main_{label}_final.cpp`: 今回の提出候補の保存コピー。

再評価:

```sh
pahcer run
python3 tools/experiments/summarize.py
python3 tools/experiments/holdout/check.py
python3 tools/experiments/final_holdout/check.py
```
'''
old=(d/'README.md').read_text(encoding='utf-8')
marker='## 旧4窓ブロック方式の記録'
tail='\n'+old[old.index(marker):] if marker in old else ''
(d/'README.md').write_text(report+tail,encoding='utf-8')
(root/'record'/f'README_{label}.md').write_text(report,encoding='utf-8')
(d/f'main_{label}_final.cpp').write_bytes((root/'main.cpp').read_bytes())
print(f'Wrote record/README_{label}.md and current experiment report')
