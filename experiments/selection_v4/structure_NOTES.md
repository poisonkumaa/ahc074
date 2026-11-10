**全アンカーへの主紙再配置の試作（2026-11-05）**

[structure_candidate.cpp](structure_candidate.cpp) は [main_before_v4.cpp](main_before_v4.cpp) から [structure_make.py](structure_make.py) で生成する。提出用 main は変更しない。実装本体は [structure_assign.inc](structure_assign.inc)。**開発 6 ケースすべてで悪化したため不採用。**

既存の sampled_bank_swaps は、近いアンカーの同じ頻度・同じ色のスロット間で交換する。今回の試作は単独移動と交換により全アンカーを探索し、銀行ごとの枚数、ペア／単独紙の比率、色構成を変えられる。core、商品のペア関係、各所有商品の suffix は保持する。各アンカーでは各色を元の枚数と 2 の小さい方以上残す。

取り出しの従来式は、ペア紙 p 枚が単独紙 s 枚より前に並ぶと仮定した `p(p+1) + ps + s(s+1)/2`。実際の合成注文から得た head sample で、色×頻度の 4 種類について「種類 u の紙が種類 t の紙より前にある確率」を観測する。種類内の対称性と種類間の順序確率から、重み付き取り出し費用を対ごとに加算する。色を無視した簡略形では、単独紙がペア紙より前に来る確率 q による `qps` が従来式へ加わる。

目的関数には所有商品の prefix の残り修復数と、ほかの商品へ貸し出す補助紙としての限界価値も含める。いずれも現在の head sample と donor 競合を固定した反実仮想評価。配置変更後も順序確率が保たれる仮定や、複数紙を同時に変えた場合の相互作用には近似がある。全体の最適な期待手数を求めるものではない。

採用する再配置では主紙 ID をアンカー順に振り直し、商品・ペアの対応を更新する。旧 ID の head sample を流用せず、候補配置から独立の合成注文を処理して全 sample を作り直す。時間切れや不正な最終表示があれば、配置と sample の変更を丸ごと取り消す。実際の未来の注文は参照しない。

| 環境変数 | 既定 | 意味 |
|---|---:|---|
| STRUCTURE_ASSIGN | 0 | 再配置を有効化 |
| STRUCTURE_EMPIRICAL | 1 | 1 は観測順序、0 は従来 F による取り出し評価 |
| STRUCTURE_SAMPLES | 12 | 目的関数に用いる既存 sample 数 |
| STRUCTURE_STEPS | 24 | 最良の単独移動／交換を選ぶ回数の上限 |
| STRUCTURE_CPU | .25 | 再配置と sample 再生成の累積予算、search_budget の scale 適用前 |
| STRUCTURE_END | 2.45 | 設計開始からの絶対期限、同上 |
| STRUCTURE_ANCHOR_ALPHA | 1 | 元の設計構築における F の係数 |
| STRUCTURE_ANCHOR_BETA | .5 | 元の設計構築における exposure の係数 |
| STRUCTURE_JOINT_GAMMA | 1 | 元のペア・アンカー同時探索の pair_route_cost 係数 route_scale |

`STRUCTURE_JOINT_GAMMA` は donor coverage の `joint_gamma` を変えない。最後の三変数は設計構築用なので、既存 fixture を読む比較には影響しない。通常の Linux scale は .6。初回の研究比較では `STRUCTURE_ASSIGN=1, STRUCTURE_END=30, STRUCTURE_CPU=2` 等で時間切れとモデルの性能を切り分けるが、これは通常予算での性能検証の代用ではない。

生成は ahc074 ディレクトリで `python experiments/selection_v4/structure_make.py`。変更呼び出しは CHOSEN ログの直後にあり、既存 [make_fixture.py](../beam_v1/make_fixture.py) で作る固定設計版にも残る。初回凍結 SHA256 は `9ADFB1437968E528B5ACCC590843190A89D5DA14EB679DB31535C4DB9F6ED6C7`。

[structure_test.cpp](structure_test.cpp) は独立した不変条件・再生テスト。C++17 / O2 でコンパイルし、引数に fixture ファイルを 1 個与える。再配置が実際に採用されたこと、core・suffix・商品群・アンカー構造・対応表・head sample の整合を検査し、全商品を含む 1024 件の合成注文で各操作の可視性・範囲・表示・物理順序を別の再生器で照合する。提出用コードにこのテストは含めない。

**固定設計での比較結果**

[structure_probe.json](structure_probe.json) は既存 fixture の seed `0000`–`0005`、WSL / Linux、独立再生検証 `official:false`。再配置以外の初期設計を統一し、再配置・online 処理とも時間枠を拡張した研究比較。

| variant | 平均手数 | baseline 差 | 改善 / 同点 / 悪化 |
|---|---:|---:|---:|
| baseline | 10687.500 | 0 | — |
| empirical | 10871.167 | +183.667 | 0 / 0 / 6 |
| original_f | 10818.167 | +130.667 | 0 / 0 / 6 |

全件 `E=0`。empirical は各 40–45 枚、original_f は 42–45 枚の主紙を再配置した。再配置と新しい head sample の生成に要した solver CPU は前者 0.041–0.074 秒、後者 0.040–0.057 秒。変更なしや期限切れではなく、構造を実際に変えたうえで悪化した。固定した donor 競合・順序確率による予測を全体の性能へ結び付けられず、観測順序による F の置換も今回の比較では改善しなかった。通常予算での採用検証へは進まない。
