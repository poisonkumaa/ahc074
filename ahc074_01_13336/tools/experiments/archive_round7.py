from pathlib import Path
import hashlib,json,re
d=Path(__file__).resolve().parent;r=d.parents[1]
source=(r/'main.cpp').read_bytes();digest=hashlib.sha256(source).hexdigest()
assert digest=='e32b1e9b7db72a51ccbb540e069084cddac14d00dfb93399a144c9a2fb5164f0'
primary=json.loads((d/'validation.json').read_text());extra=json.loads((d/'round7_final_validation.json').read_text())
assert primary['summary']['source_sha256']==digest==extra['source_sha256']
assert primary['summary']['errors']==0 and all(x['errors']==0 for x in extra['batches'])
assert extra['binary_sha256']==hashlib.sha256((r/'a.out').read_bytes()).hexdigest()
for f in ['round7_cutoff_results.json','round7_ubsan_results.json']:
 report=json.loads((d/f).read_text());assert report['source_sha256']==digest and report['status']=='passed'
(d/'round7_validation.json').write_bytes((d/'validation.json').read_bytes())
(d/'main_13336_round7_final.cpp').write_bytes(source)
counts={};complete=0
for i in range(100):
 log=(r/f'tools/err/{i:04d}.txt').read_text();m=re.search(r'AdaptiveR=(\d+)',log)
 if m:counts[m[1]]=counts.get(m[1],0)+1
 complete+='exact_refit accepted=' in log
note=f'''# AHC074 Round 7 — 等価高速化と追加探索

大幅な得点改善は確認できなかった。提出用 `main.cpp` には、固定した学習量で設計・操作列を変えない高速化を反映した。時間制限に応じて学習量が変わる実行では得点も変動する。200件全体の平均は13,345.55→13,346.38で、今回の得点改善を主張する結果ではない。

ユーザー報告のAtCoder結果は50件合計667,994点、1739ms。今回の版はローカル検証までで、AtCoderには未提出。

| 検証 | 件数 | Round 6平均 | Round 7平均 | Round 7最大CPU |
|---|---:|---:|---:|---:|
| pahcer seed 0–99 | 100 | 13,340.40 | 13,336.45 | 1.785秒 |
| seed 5000–5049 | 50 | 13,322.26 | 13,353.06 | 1.758秒 |
| seed 6000–6049 | 50 | 13,379.14 | 13,359.56 | 1.773秒 |
| 全体 | 200 | 13,345.55 | 13,346.38 | 1.785秒 |

同じ最終ソースと実行ファイルで全200件を公式テスタに通し、誤注文・不正操作ゼロ、1注文最大34手、経路fallbackゼロ。時間は対話待ちを除くCPU時間であり、WSLのpahcer実時間とは異なる。

## 採用した高速化

PrefixDPの位置別累積個数表を、長さごとの192位置bitsetへ置き換えた。区間内の長い紙の数はpopcount、中間の長さの紙があるかはbitsetの差で求める。区間マスクは重ね順に依存しないため、DPオブジェクト作成時に一度だけ用意する。

終状態を構成する際、挿入する紙の長さLに対して既存の子列が「Lより短い・同じ・長い」の3区間に分かれる場合は、同じ長さの区間だけで挿入位置を選べる。同長の紙は1つのgoalに同時出現しないので、64bit集合の和の増分は使用回数の差になる。各呼出しで条件を検査し、満たさない一般状態は元の実装へ戻す。この高速化は到達状態の経験的な形だけに依存しない。

終状態の高速化単体は固定R8・100件でstdout全文が一致。2つの高速化を組み合わせた診断版は固定R8/R32の各5件で全文一致。固定R32・5件の逐次CPU平均は1.624秒→1.493秒、約8%短縮した。これは診断用のローカル計測で、新しいAtCoder実行時間ではない。

最終ソースを使ったランダム検証では、DP辺費用100万通りが旧式と一致。任意順序・goal・アクセス集合の4,200呼出しも元の終状態と一致し、うち3,040回で一般fallbackを実際に通った。

captureの高速化後は従来の学習量見積もりが過大になったため、係数を5.0→6.5へ調整。初期学習0.5秒、再学習1.65秒、最大32状態/商品は維持した。最終pahcerの学習量分布は `{counts}`、再学習完了ログは{complete}/100件。時間ガードで途中終了しても、受理済みの辞書は全256商品を表現できる。

## 得点を変える案の結果

紙の同時挿入DP、商品別費用重み、固定owner／固定分解による将来費用補正、自由境界のprefix＋suffix、紙の長さ配分、個別長さ変更、隣接窓交換、長期rollout、goalのrecord数・逆転数への価格、異なる長さ間の担当商品交換を比較した。少数ケースの改善が30〜100件で失われるものや、追加計算に見合わないものが多く、採用しなかった。

代表例はowner補正の10件平均13247.2に対し、100件では13337.26で基準13337.73とほぼ同じ。担当商品交換も3件では46.7改善したが、30件では3.8悪化した。左右のprefix/suffixは理想被覆枚数を9.637→9.457へ減らせたものの、合法アクセスの費用が増え実得点は59286まで悪化した。

詳細・再現ファイルは `round7_notes.txt` と各 `round7_*notes.*`。当初の平均約6,554点という目標、および上位に追いつく改善は未達。

## 保存・検証

- `main_13336_round7_final.cpp`: 今回の提出用ソース保存。
- `main_13340_round6_final.cpp`: 変更前の提出用ソース保存。
- `validation.json` / `round7_validation.json`: pahcer最終100件。
- `round7_final_validation.json`: 同一バイナリの追加100件。
- `round7_cutoff_results.json`: 低予算15件の独立replay。全件合法・E=0。
- `round7_ubsan_results.json`: 全R32学習を完了するUBSan検証。異常なし。
- `round7_fast_compare.json`: 固定学習量の出力同一性・CPU比較。
- `round7_fast_unit.cpp`: 一般状態でのDP費用・終状態の参照照合。

ソースSHA-256: `{digest}`

実行ファイルSHA-256: `{extra['binary_sha256']}`

```sh
pahcer run
python3 tools/experiments/summarize.py
python3 tools/experiments/check_round7_final.py
python3 tools/experiments/check_round7_cutoff.py
python3 tools/experiments/check_round7_cutoff.py --ubsan
```

実験の生成済みcppは当時の条件を保持している。過去のbuilderはその時点のmain.cppを入力にするものがあるため、新しいmain.cppへ無条件に再適用しない。
'''
(d/'README.md').write_text(note)
(r/'record/README_round7_13336.md').write_text(note)
with (d/'round7_notes.txt').open('a') as f:
 f.write('\nFinal: capture factor6.5; source '+digest+'\n')
 f.write('pahcer mean13336.45, extra50 means13353.06/13359.56; all200 E0, maxCPU1.78479.\n')
 f.write('200-case mean13346.38 vs old13345.55: no demonstrated score gain.\n')
 f.write('Final-source random checks: 1000000 edge comparisons, 4200 final calls, 3040 fallbacks.\n')
print(json.dumps(dict(source=digest,adaptive_counts=counts,complete=complete,mean200=13346.38),indent=2))
