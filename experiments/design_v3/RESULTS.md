**非ビーム候補の実験記録（2026-11-05）**

元の設計候補を保持し、時間に余裕があれば 1 件だけ追加する **append1 を main.cpp に採用した**。通常予算の新規 20 件ずつで平均 −6.35 手、−8.60 手だった。ただし前者の改善は 1 件に集中し、後者では追加候補自体は一度も選ばれていない。既存候補の選択・合成評価回数・実行時間の影響を含む小さな観測差であり、統計的に安定した改善や全ケースの改善を保証しない。後者では最大 +103 手の悪化もあった。

[main.cpp](../../main.cpp) は検証済みの [main_candidate.cpp](main_candidate.cpp) とバイト単位に一致し、SHA256 は `40F66C0166537AD258BD20BB4D95B4B7DFB451FDDB36C3EB7FFF0DB67E4BAC55`。変更前は [main_before_append.cpp](main_before_append.cpp) に保存した（SHA256 `B16DA6013ED4A60F4ECA58246DE37833CF16A9AFBF0AE1E29DF0AA7A941376F5`）。**1 ケース 10000 手の目標は未達。** Portfolio / adaptive は通常実行で改善が再現せず不採用とし、以下の他の新方式も採用していない。

**入力と比較条件**

- 開発ケースは既存の `0000`–`0005`。新規 20 件は [generate_validation.py](generate_validation.py) が作成したローカル ID `0100`–`0119`。**公式ジェネレータの seed 100–119 ではない。** Python の乱数 seed は `740000 + file_id`。64 箇所から 32 箇所を一様に選んだ相異なる 256 商品を作り、初回注文は 256 商品から、以後は直前以外の 255 商品から選ぶ、という問題文と同じ分布に従う。
- 新規入力は `validation_in/` に保存。portfolio の開発比較後に生成した検証用集合だが、結果を見た後の追加調整では使用済みの集合として扱う。
- 研究比較と最終 20 件比較は WSL / Linux、`official:false` の独立再生検証で、誤り数と操作合法性を確認した。最後の [confirm_official.json](confirm_official.json) だけは Windows の公式 reactive tester による 3 件の機能確認。実行環境と時間の意味を区別する。
- [beam_v1 の fixture](../beam_v1/fixture_io.inc) に設計・対応表・列順・catalogue 由来の head sample を保存し、同じ設計からルーティングを比較する。bank 並べ替えの候補だけは、読み込んだ設計を意図的に変更する。fixture に実際の将来注文は含まれず、各探索も次の実注文を読まない。
- 固定設計の研究比較では `NOV8_ROUTE_END / NOV8_RELAX_END / NOV8_SHORT_END=30`、`NOV5_RELAX_BUDGET=10`、portfolio の `PORTFOLIO_CPU=10` 等で探索を完走させた。Linux scale は .6。**固定設計版の時間は設計探索を含まず、通常版の 2 秒制限を満たす証明ではない。** 設計から実行した比較と通常予算の比較は、それぞれの節に明記する。
- wall はハーネスの経過時間。通常版の solver CPU は各 stderr の `BUDGET cpu=` に保存される。ただし後述の charged fixture では仮想課金額もこの値に含まれる。JSON の `solver_cpu_seconds:null` / `max_cpu:0` は CPU がゼロという意味ではない。

**保存時と固定設計版の baseline の差**

[validation_dump.json](validation_dump.json) の設計保存実行は平均 10806.75 手、固定設計版の baseline は 10802.50 手だった。差 −4.25 手を新方式の改善に数えてはいけない。

全 20 件で出力先頭 192 行の紙設計が一致し、19 件は全注文の出力まで一致した。local ID `0104` だけ保存時 10824 手、固定設計版 10739 手で −85 手。保存時は設計から実行しており `trace_skips=269`、固定設計版は `trace_skips=0` だった。設計後に使える時間と操作列短縮の打ち切り条件が異なることによる差である。以下は各比較内の固定設計 baseline を基準にする。

**通常候補を短縮後に選ぶ portfolio**

[portfolio.cpp](portfolio.cpp) は通常のポリシー比較で得た複数の合法候補を保存する。従来は短縮前の評価で一つに絞っていたが、追加候補にも既存の `nov5_relaxed_trace` を適用し、短縮後の操作数＋終端評価で選び直す。候補数を深さ方向へ増やすビームや、次注文の rollout ではない。`PORTFOLIO_TRIES=1/3` は追加で短縮する候補数の上限。

| 記録 / variant | 件数 | 平均手数 | baseline 差 | 改善 / 同点 / 悪化 | wall 平均 / 最大（秒） |
|---|---:|---:|---:|---:|---:|
| [portfolio_probe](portfolio_probe.json) / baseline | 6 | 10687.500 | 0 | — | 0.449 / 0.551 |
| portfolio1 | 6 | 10683.667 | −3.833 | 4 / 0 / 2 | 0.620 / 0.661 |
| portfolio3 | 6 | 10666.500 | −21.000 | 6 / 0 / 0 | 0.979 / 0.995 |
| [portfolio_validation](portfolio_validation.json) / baseline | 20 | 10802.500 | 0 | — | 0.503 / 0.657 |
| portfolio1 | 20 | 10792.500 | −10.000 | 18 / 0 / 2 | 0.742 / 1.021 |
| portfolio3 | 20 | 10785.900 | −16.600 | 16 / 0 / 4 | 1.149 / 1.488 |

全件 `E=0`。新規 20 件の最大は baseline 22 手/注文、portfolio 25 手/注文で、384 手の制限内。平均は改善したが、portfolio3 でも 4 件は悪化した。新規 20 件の全体 CPU 平均は baseline 0.487 秒、portfolio3 1.132 秒と約 0.645 秒増えており、通常の 2 秒予算にそのまま追加できるとは判断しない。

**軽量化・bank 並べ替え・bucket 内 DP**

[quick_portfolio.cpp](quick_portfolio.cpp) は追加候補に対する短縮だけを軽くし、二操作削除の候補数を 12、三操作削除を 0 にした。baseline として残す元の解の短縮処理は変更しない。開発比較は [development_probe.json](development_probe.json)。

| variant（開発 6 件） | 平均手数 | baseline 差 | 改善 / 同点 / 悪化 | wall 平均 / 最大 |
|---|---:|---:|---:|---:|
| baseline | 10687.500 | 0 | — | 0.435 / 0.542 |
| quick3 | 10668.167 | −19.333 | 5 / 1 / 0 | 0.860 / 0.903 |
| bank_donor | 10682.833 | −4.667 | 2 / 4 / 0 | 0.434 / 0.492 |
| bank_coverage | 10698.833 | +11.333 | 0 / 5 / 1 | 0.425 / 0.463 |
| bucket_dp | 10687.500 | 0.000 | 0 / 6 / 0 | 0.437 / 0.508 |

全件 `E=0`、最大 20 手/注文。

- **Bank 並べ替え**: [global_candidate.cpp](global_candidate.cpp) / [bank_order_optimizer.inc](bank_order_optimizer.inc) は商品対と core、bank の構成を固定したまま、prefix bank と対応する列の順番を割当後に変更する。bank_donor は prefix の被覆だけでなく紙の借用効果も評価する。開発 6 件では `0004` / `0005` の 2 件だけ実際の設計変更が採用され、−4 / −24 手。他の 4 件は採用なし。被覆のみを評価する bank_coverage は 1 件で設計変更が採用され、+68 手だった。
- **Bucket の共同色修復**: [bucket_candidate.cpp](bucket_candidate.cpp) / [bucket_joint_repair.inc](bucket_joint_repair.inc) は最初の正しい色の紙を固定し、その上の同色 blocker を同じ手数で共同再挿入する subset DP。加算できる queue 評価を最小化し、既存 MDP 評価も悪化しない案だけを使う。開発 6 件合計で対象はわずか 4 回（すべて blocker 2 枚）、`improved=applied=0`。採用案がないだけでなく、この修復に複数 blocker が残る機会自体が少なく、大きな改善余地は確認できなかった。

**軽量 portfolio と bank の新規 20 件比較**

[quick_bank_validation.json](quick_bank_validation.json)、同じ local ID `0100`–`0119`。

| variant | 平均手数 | baseline 差 | 改善 / 同点 / 悪化 | wall 平均 / 最大 |
|---|---:|---:|---:|---:|
| baseline | 10802.50 | 0 | — | 0.480 / 0.737 |
| quick3 | 10788.80 | −13.70 | 17 / 1 / 2 | 0.897 / 1.153 |
| bank_donor | 10802.50 | 0.00 | 0 / 20 / 0 | 0.457 / 0.553 |

全件 `E=0`。quick3 の最大は 25 手/注文、baseline / bank_donor は 22 手/注文。quick3 は full portfolio3 より平均 2.9 手多いが、別実行の測定では wall 平均が約 0.25 秒短い。時間とスコアの候補として通常予算での確認を進める段階。

bank_donor は新規 20 件で全件 `committed=0`、baseline と出力全文も全件一致した。開発 2 件での改善を独立した設計変更の成功として一般化しない。

**修復手数で重み付けした bank 並べ替え**

[weighted_probe.json](weighted_probe.json) は開発 6 件。 [global_weighted.cpp](global_weighted.cpp) / [bank_order_weighted.inc](bank_order_weighted.inc) で、単なる不一致数に代えて、head sample 内の正しい色までの blocker 数を評価へ反映した。

| variant | 平均手数 | baseline 差 | wall 平均 / 最大 |
|---|---:|---:|---:|
| baseline | 10687.500 | 0 | 0.441 / 0.582 |
| weighted12 | 10686.333 | −1.167 | 0.408 / 0.421 |
| weighted24 | 10684.167 | −3.333 | 0.454 / 0.554 |

全件 `E=0`、最大 20 手/注文。数字は head sample 数 12/24。どちらも同じ開発 6 件での単純な bank_donor（−4.667 手）を上回らず、この候補は採用しない。実行ごとの wall の小さな差を高速化の効果と解釈しない。

**最終設計候補の再評価**

[selection_probe.json](selection_probe.json) は seed `0000`–`0005` の 6 件。設計候補を合成注文で再評価する [selection_candidate.cpp](selection_candidate.cpp) / [finalist_selection.inc](finalist_selection.inc) を試した。`FINAL_RACE_QUERIES=128/256`、route 系・初期化等の時間枠 `30`、`FINAL_RACE_CPU=10` の研究設定であり、通常の 2 秒制限の確認ではない。

| variant | 平均手数 | baseline 差 | wall 平均 / 最大 |
|---|---:|---:|---:|
| baseline | 10687.50 | 0 | 1.523 / 1.685 |
| final128 | 10687.50 | 0.00 | 1.760 / 1.922 |
| final256 | 10721.00 | +33.50 | 1.832 / 2.160 |

全件 `E=0`、最大 20 手/注文。final128 は全 6 件で先頭 192 行の設計と手数が baseline と一致し、追加時間に見合う改善なし。final256 の +201 手（平均 +33.5）の内訳は次の 2 件だけ。

- `0002`: baseline は `W=36,index=0`、final256 は `W=38,index=2`。`FINAL_RACE original=0 chosen=2` で再評価が別設計を選び、10653→10825 手（+172）。これは実際の選択変更による結果。
- `0004`: 両方とも `W=37,index=5` だが出力した紙模様が異なり、10581→10610 手（+29）。final256 は `original=chosen=5` のままで、候補数 7→6、合成評価回数 1920→880 と、再評価前の設計生成・評価が既に違っている。この +29 を最終候補選択単独の効果には数えない。

**設計から実行する通常予算の比較**

[normal_probe.json](normal_probe.json)、seed `0000`–`0005`。fixture を使用せず、設計探索と実注文処理を実行した。

| variant | 平均手数 | baseline 差 | wall 平均 / 最大 |
|---|---:|---:|---:|
| baseline | 10715.500 | 0 | 1.657 / 1.821 |
| portfolio1 | 10738.333 | +22.833 | 1.744 / 1.820 |
| quick3 | 10695.500 | −20.000 | 1.809 / 1.811 |

全件 `E=0`、最大 20 手/注文。portfolio1 は追加候補 1、pair/triple 候補数 100/24、`PORTFOLIO_CPU=.4, NOV5_RELAX_BUDGET=.7`。quick3 は追加候補 3、`PORTFOLIO_CPU=.55, NOV5_RELAX_BUDGET=.9`。route の全体期限は通常設定のまま。quick3 の solver CPU は全件約 1.787–1.789 秒で打ち切り付近に達した。

出力先頭 192 行を比較すると、設計が一致しないケースは **portfolio1 が 0000/0002/0005、quick3 が 0000/0002**。W / index が同じでも紙模様まで同じとは限らない。

| seed | baseline 手数 | portfolio1 の設計 / 手数差 | quick3 の設計 / 手数差 |
|---|---:|---|---|
| 0000 | 10717 | 不一致 / −4 | 不一致 / −16 |
| 0001 | 10701 | 一致 / −6 | 一致 / −1 |
| 0002 | 10758 | 不一致 / −80 | 不一致 / −80 |
| 0003 | 10905 | 一致 / −16 | 一致 / −32 |
| 0004 | 10581 | 一致 / +12 | 一致 / −3 |
| 0005 | 10631 | 不一致 / +231 | 一致 / +12 |

portfolio1 の `0005` は baseline `W=36,index=0` に対し `W=37,index=1` で、平均悪化への寄与が大きい。設計一致分だけでは portfolio1 は 3 件平均 −3.333 手、quick3 は 4 件平均 −6.0 手だが、この部分集合でも合成評価回数や残り時間を固定していないので、純粋な方式の効果とは断定しない。

追加候補の短縮は、通常 query と元の解の短縮が使う全体期限を消費する。例えば設計が同じ `0004` でも `trace_skips` は baseline 0、portfolio1 34、quick3 71。quick3 は全件で 31–87 回の短縮スキップがあり、候補を増やした利益と本来の短縮機会を減らした損失が混在する。平均 −20 手を無条件に採用根拠としない。

**時間を予約する adaptive 候補**

[adaptive_portfolio.cpp](adaptive_portfolio.cpp) は既定で `ROUTE_PORTFOLIO=1`、追加候補 1 件、`PORTFOLIO_CPU=.4`、共有の `NOV5_RELAX_BUDGET=.9`。Linux では scale .6 により、それぞれ .24 / .54 CPU 秒に相当する。ソース SHA256 は `45C526D3017A9F37958933ADDF99DE7B6785A4C2B9C161E32AA1CE5BF4075D35`。

- 最初の 16 注文では候補収集も追加短縮も行わない。
- 通常 query＋元の解の短縮に要した CPU を計測し、残り注文の平均予測量 ×1.10 ＋10 ms を予約する。10 ms は内部 scale を適用しない実 CPU 秒。追加候補の CPU はこの平均から除外する。
- query 前に候補収集の可否を決め、元解の短縮後に再び残り予算を計算する。追加処理の期限は全体期限、予約後の期限、追加候補の累積上限の最小。期限切れでも選択済みの deck / supports / cuts を復元する。
- 追加候補 1 件なら保持する候補は既定 2 件で足りるため、同点順序を保つ厳密な score cutoff で収集を軽くする。追加探索を無効にした注文では raw 操作列のコピー等を省く。

予約するのは全体 CPU 時間であり、`NOV5_RELAX_BUDGET` は元解と追加候補で共有される。したがって追加候補が共有の短縮枠を消費する可能性は残る。また既定値を baseline の .52 から .9 へ変えた影響も含む実装で、追加候補選択だけの比較ではない。

**仮想設計 CPU を課金する固定設計試験**

[charged_probe.json](charged_probe.json)、開発 6 件。[make_charged_fixture.py](make_charged_fixture.py) が `design_started` を `FIXTURE_CHARGE` 秒だけ過去へずらし、設計に時間を使った状態の予算判定を再現する。実際にその設計を計算したり、指定秒数の CPU を消費するわけではない。

| 仮想課金 / variant | 平均手数 | 同課金 baseline 差 | 実 wall 平均 / 最大 | `BUDGET cpu` 平均（課金込み） |
|---|---:|---:|---:|---:|
| 1.20 秒 / baseline | 10698.667 | 0 | 0.520 / 0.632 | 1.697 |
| 1.20 秒 / adaptive | 10688.167 | −10.500 | 0.602 / 0.609 | 1.783 |
| 1.35 秒 / baseline | 10744.167 | 0 | 0.460 / 0.504 | 1.790 |
| 1.35 秒 / adaptive | 10718.833 | −25.333 | 0.462 / 0.469 | 1.792 |

全件 `E=0`。`BUDGET cpu` は仮想課金を含む時計差で、実 CPU や設計を含む通常版の実行時間として引用しない。固定した紙設計と課金額でも、実際の CPU 速度によって打ち切り位置や評価可能な処理量が変わるため、時間変動が消えた試験ではない。

**Adaptive の新規 20 件・通常実行**

[final_validation.json](final_validation.json) は別の新規 local ID `0120`–`0139`。同じ [generate_validation.py](generate_validation.py) の `--start 120 --end 140` で生成したもので、公式 seed ではない。CPU 0 に affinity を固定し、fixture を使わず設計から実行、両者とも既定設定で比較した。

| variant | 平均手数 | baseline 差 | wall 平均 / 最大 | solver CPU 平均 |
|---|---:|---:|---:|---:|
| baseline | 10787.00 | 0 | 1.652 / 1.878 | 1.602 |
| adaptive | 10792.05 | +5.05 | 1.792 / 1.888 | 1.737 |

全件 `E=0`、最大 21 手/注文。初期設計 192 行は 13 件で一致し、不一致は `0121/0122/0123/0124/0125/0138/0139`。一致 13 件でも差の合計 +57 手、平均 +4.385 手で、CPU 平均は 1.538→1.709 秒だった。単に設計差だけで平均悪化を説明することはできない。

- 追加候補は合計 8775 試行、proxy 改善による `wins=173`、その場の操作数差の合計 `saved=67`。しかし全ケースの最終差は +101 手。`wins` は即時操作数が必ず減った回数ではなく、終端評価も含む改善回数で、以後の状態変化まで含む最終利益とは異なる。
- 同じ設計で両者とも `trace_skips=0` の `0126/0130/0131` でも +6/+18/+10 手。時間切れだけでは説明できず、候補が残す状態や評価の近似も確認が必要。この条件の 8 件全体では合計 −5 手、平均 −0.625 手で、大きな改善は確認できない。
- 一方、`0120/0137` は同じ設計で追加候補試行が 0 回なのに +54/+8 手。`trace_skips` は 4→86 / 0→51 と増えており、初期化・通常短縮の時間差や共有予算設定の影響もある。すべての悪化を候補の終端評価だけに帰属させない。

この新規集合も結果を参照済みとなった。固定設計での改善が通常実行では再現せず、この adaptive portfolio は採用しなかった。

**Portfolio の採用条件を厳しくする試行**

[guarded_probe.json](guarded_probe.json)、固定設計の開発 6 件。 [guarded_portfolio.cpp](guarded_portfolio.cpp) の `PORTFOLIO_ACCEPT_MODE` を比較した。

| mode / 条件 | 平均手数 | baseline 差 | wall 平均 / 最大 |
|---|---:|---:|---:|
| baseline | 10687.500 | 0 | 0.450 / 0.614 |
| 0：操作数＋終端 proxy | 10683.667 | −3.833 | 0.621 / 0.701 |
| 1：現在の操作数も厳密に減少 | 10687.000 | −0.500 | 0.667 / 0.728 |
| 2：さらに保持補助紙数を増やさない | 10687.333 | −0.167 | 0.719 / 1.033 |

全件 `E=0`、最大 20 手/注文。厳しい条件で開発結果が改善したわけではなく、この変更は採用しない。

**商品対評価の core 距離と重み**

既存の `graph_dag_metric` kind 3 は、サンプルした順序の反転グラフに対する **B − 最大独立集合サイズ**を使う下界評価。見えていない紙を動かせない制約まで含めた正確な合法操作距離ではない。この区別を曖昧にしない。

[metric_candidate.cpp](metric_candidate.cpp) で core 評価の係数 `PAIR_CORE_WEIGHT` を変更した。 [metric_probe.json](metric_probe.json) は開発 6 件を設計から実行し、online 系の時間枠を `30` へ延長した研究比較。

| core 係数 | 平均手数 | 係数 4 との差 | wall 平均 / 最大 |
|---|---:|---:|---:|
| 4（baseline） | 10687.500 | 0 | 1.343 / 1.426 |
| 2 | 10677.000 | −10.500 | 1.366 / 1.415 |
| 6 | 10748.333 | +60.833 | 1.408 / 1.439 |
| 8 | 10746.333 | +58.833 | 1.370 / 1.392 |

全件 `E=0`。係数 2 の改善は均一でなく、`0003` の −191 手と `0005` の −108 手に偏り、他の 4 件は悪化した。6 件の平均だけで係数を採用しない。

[legal_metric_candidate.cpp](legal_metric_candidate.cpp) / [legal_pair_metric.inc](legal_pair_metric.inc) は、完全比較 core の順列状態グラフで最下段の紙を動かさない合法辺を使い、goal 集合から逆向き BFS で距離を計算する。各開始状態からの距離はこの core モデル内で正確。`legal_sample` は開始順序をサンプルして平均し、`legal_all` は全 B! 順序を平均する。紙全体のルーティングの最適手数を直接求めるものではない。対になる商品で制約を反転しても距離が同じとは限らないため、両方向を別々に求めて平均する。

[legal_metric_test.cpp](legal_metric_test.cpp) は **560 ケースと 612 最短経路再生に合格**。独立した forward BFS との照合、可視性を守った経路再生、方向別 cache、期限切れ時の部分結果非公開を確認した。3 枚 core で最下段を先頭へ持ってくる例では、制約なしの距離 1 に対し合法距離 2 となる差も検査した。これは距離計算の正しさの検査であり、スコア改善の証拠ではない。

[legal_mix_probe.json](legal_mix_probe.json) の開発 6 件結果。mix は [metric_mix.cpp](metric_mix.cpp) で設計 root ごとに係数 4/2 を使い分けたもの。

| variant | 平均手数 | baseline 差 | wall 平均 / 最大 |
|---|---:|---:|---:|
| weight4 | 10687.500 | 0 | 1.268 / 1.346 |
| legal_sample | 10698.000 | +10.500 | 1.335 / 1.389 |
| legal_all | 10737.333 | +49.833 | 1.354 / 1.421 |
| mix10（`PAIR_ALT_MASK=10`） | 10810.833 | +123.333 | 1.324 / 1.389 |
| mix5（`PAIR_ALT_MASK=5`） | 10706.833 | +19.333 | 1.347 / 1.489 |

全件 `E=0`。距離の精密化や重みの併用が、今回の設計・選択パイプラインで実手数の改善につながった結果は得られていない。

**設計候補の新規 20 件検証**

[design_validation.json](design_validation.json) は同じ生成方法による新規 local ID `0140`–`0159`。公式 seed ではない。全件、紙の設計から実行し、online 系・初期化等の時間枠を `30` に延長した研究設定。snapshot2 は [pair_snapshot_candidate.cpp](pair_snapshot_candidate.cpp) / [pair_snapshot_rematch.inc](pair_snapshot_rematch.inc) により、snapshot 候補の一部を係数 2 で再対応付けした候補へ置き換える。

| variant | 平均手数 | baseline 差 | wall 平均 / 最大 |
|---|---:|---:|---:|
| weight4（baseline） | 10751.75 | 0 | 1.637 / 2.009 |
| weight2 | 10784.25 | +32.50 | 1.615 / 1.799 |
| legal_sample | 10784.35 | +32.60 | 1.654 / 1.962 |
| snapshot2 | 10764.80 | +13.05 | 1.650 / 1.895 |

全件 `E=0`、最大 20〜22 手/注文。三つの変更はいずれも平均で悪化した。baseline の wall 最大 2.009 秒は研究ハーネスの実測であり、公式 TLE の判定結果ではない。同時に、通常の 2 秒制限を保証する結果でもない。

**元の候補を保持して追加する方式**

[pair_snapshot_append.cpp](pair_snapshot_append.cpp) は既存候補を置き換えず、元の 4 root＋最大 3 snapshot の生成を先に行い、時間に余裕がある場合だけ係数 2 の候補を 1〜2 件追加する。 [make_pair_snapshot_append.py](make_pair_snapshot_append.py) で生成した。snapshot2 のように元の良い候補を失う問題を避けるための変更。

[append_validation.json](append_validation.json) は新規 local ID `0160`–`0179` を**設計から通常予算で実行**した。拡張 online 時間枠や fixture は使用していない。baseline は同じ追加版ソースの `PAIR_SNAPSHOT=0`、append1 / append2 は `PAIR_SNAPSHOT=1, PAIR_APPEND_COUNT=1/2`。

| variant | 平均手数 | baseline 差 | wall 平均 / 最大 |
|---|---:|---:|---:|
| baseline | 10763.65 | 0 | 1.489 / 1.613 |
| append1 | 10757.30 | −6.35 | 1.605 / 1.772 |
| append2 | 10761.20 | −2.45 | 1.671 / 1.839 |

全件 `E=0`。最大は baseline 21、追加版 22 手/注文。

- **append1**: 19 件は設計 192 行だけでなく全注文の操作列も baseline とバイト単位に一致した。`0167` だけ追加候補 index 7（W=36）を採用し、baseline の元候補 index 3（W=39）から 10759→10632 手（−127）。双方 `trace_skips=0`、`PAIR_APPEND original_candidates=7, added=1`。ただしこのケースの再ペアリングは `changed=0, before=after=979` だったため、係数変更による再ペアリング単独の改善とは言わない。追加生成した設計候補を採用した効果として記録する。
- **append2**: 16 件は出力全文一致。`0167/0178` は追加 index 7 を採用し −127/−31 手、`0160/0171` は元の index 2 を採用して +56/+53 手。後者の合成評価回数は 494/601（baseline はともに 2048）、`0171` では `trace_skips=51`（baseline 0）。追加候補自体の質に加え、既存候補を評価する時間が減る影響も混在している。

**採用版の最終確認**

[main_candidate.cpp](main_candidate.cpp) は helper を埋め込んだ単独コンパイル可能な採用版。既定を `PAIR_SNAPSHOT=1, PAIR_APPEND_COUNT=1` とし、この検証済みファイルを main.cpp へ反映した。

[confirm_validation.json](confirm_validation.json) は未使用だった local ID `0180`–`0199` の 20 件を、元の B16… の main と通常予算で比較した。Linux の CPU 0 に固定し、設計から全注文まで実行した。

| 通常予算の確認 | 件数 | baseline 平均 | append1 平均 | 差 | append1 最大 wall |
|---|---:|---:|---:|---:|---:|
| [append_validation](append_validation.json)、0160–0179 | 20 | 10763.65 | 10757.30 | −6.35 | 1.772 秒 |
| [confirm_validation](confirm_validation.json)、0180–0199 | 20 | 10753.00 | 10744.40 | −8.60 | 1.882 秒 |

全件 `E=0`。後者の solver CPU 最大は 1.80566 秒、wall 最大は 1.8818348 秒だった。**後者では追加候補を選んだケースは 0 件**であり、平均 −8.60 手を新しいペアリングの効果と説明してはいけない。既存候補の選択、合成評価回数、期限までの時間配分の変化が混在する。前者の唯一の改善 `0167` も再ペアリング自体は `changed=0` で、追加生成した設計候補を選べた効果だった。両集合で観測した小幅な改善を根拠に、元の版を保存したうえで append1 のみ採用した。

[confirm_official.json](confirm_official.json) は Windows の公式 reactive tester で local ID `0167,0180,0193` を確認した。baseline / 採用版とも全件 `E=0` で、公式スコアは独立再生の手数に一致した。採用版の手数は順に `10632,10726,10916`（baseline は `10759,10726,10916`）。採用版の最大 wall は **2.0616 秒で tester 自体を含む**。これは対話・操作・表示の機能確認であり、公式環境での 2 秒以内を保証する測定ではない。Linux の 20 件測定も、未測定の全ケースや実行時間の揺らぎまで保証しない。

**再現情報**

- 各 JSON に実行ファイルの SHA256、環境変数、ケース別スコアとログを保存。出力は対応する `*_out/<variant>/<id>.txt`。variant ごとの設定は [portfolio_spec.json](portfolio_spec.json)、[development_spec.json](development_spec.json)、[validation_spec.json](validation_spec.json)。
- 生成・構築用スクリプトは [generate_validation.py](generate_validation.py)、[make_portfolio.py](make_portfolio.py)、[make_quick_portfolio.py](make_quick_portfolio.py)、[make_bucket_candidate.py](make_bucket_candidate.py)、[make_specs.py](make_specs.py)。固定設計版の生成には [make_fixture.py](../beam_v1/make_fixture.py)、実行には [bench.py](../improve_eval/bench.py) を使用する。
- 新規集合の再実行時は `--input-dir experiments/design_v3/validation_in` と対象の local ID を明示する。最終比較は `180,...,199`。通常の `tools/in/0180.txt` を同一ケースとみなさない。
- [make_portfolio.py](make_portfolio.py)、[make_bucket_candidate.py](make_bucket_candidate.py) の既定入力、および [make_pair_snapshot_append.py](make_pair_snapshot_append.py) の baseline は、採用前の固定ファイル [main_before_append.cpp](main_before_append.cpp) を参照する。これらと [make_submission_candidate.py](make_submission_candidate.py) を再生成し、生成される 4 個の C++ の SHA256 が更新前と一致することを確認した。
- 提出版は main.cpp 一つでコンパイルできる。比較時の設定・実行ファイルの SHA256・ログは各 JSON に保存した。研究用の時間拡張、fixture、追加候補 2 件、portfolio 等は採用していない。
