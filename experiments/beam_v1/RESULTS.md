**クエリ内ビーム探索の実験記録（2026-11-05）**

**開発ケースでの小さな改善は、検証用 20 ケースでは再現しなかった。** post4 は平均 +0.85 手、trace4 は +0.70 手、組合せは +3.00 手、全 255 商品による再評価を加えた版も +3.75 手と悪化した。現時点で本番への改善は実証できておらず、採用しない。1 ケース 10000 手の目標も未達。

開発用 10 ケースでは post4 が平均 5.6 手（約 0.052%）、post4＋trace4 が 7.7 手改善したが、その値だけで有効性を判断しない。[main.cpp](../../main.cpp) は変更していない。確認時の SHA256 は `B16DA6013ED4A60F4ECA58246DE37833CF16A9AFBF0AE1E29DF0AA7A941376F5`。設計時間を含む通常予算での少数ケースの動作確認も完了したが、平均手数は改善しなかった。

**固定設計による比較条件**

- 固定設計の探索比較は WSL / Linux で実行。これらの JSON は `official:false` で、ハーネスが保存出力の操作と表示を独立検証した結果。別途、末尾に示す通常版の Windows 2 ケースでは公式 reactive tester を使用し、`official_probe.json` は `official:true`。
- 時間打ち切りによる初期設計差を除くため、[fixture_io.inc](fixture_io.inc) により設計、商品・紙の対応、列順、catalogue から生成した head sample を保存し、各 variant に同じ内容を読み込ませた。fixture には実際の将来注文を保存しない。保存は最初の実注文を読む前に行う。
- [dump_report.json](dump_report.json) と [fixed_baseline_report.json](fixed_baseline_report.json) の seed `0000`–`0009` では、設計 192 行だけでなく、全 1024 注文の操作列を含む出力ファイルが全 10 件でバイト単位に一致した。固定設計版が元の baseline を再現することを確認した。
- 以下の fixture 版は設計探索を省略するため、表の wall は提出用プログラム全体の実行時間ではない。探索を完走させる研究設定として route 系の予算を `30`（Linux scale `0.6` で 18 秒）、beam の累積予算を `10`（同 6 秒）などへ延長した。実測が 2 秒未満でも、通常版の時間制限合格を意味しない。
- 探索が使うのは現在の注文、現在までの重ね順、最初に与えられた商品一覧。実際の次の注文を先読みする探索ではない。

**主紙回収後の補助紙選択：post beam**

一つの注文内で最良の通常ポリシーを 1 回再実行し、補助紙の選択を最大 3 層先まで分岐する。各状態で停止する候補と従来の貪欲解を保持し、残りの prefix 修復を実際に完了させた操作数＋終端状態評価で比較する。即時 gain が負の候補も含め、操作前の可視性、最終表示、操作上限を確認する。各層の幅は post1 / post4 / post8 で 1 / 4 / 8。shortlist は前二者が 6、post8 が 8 なので、post8 は幅だけの比較ではない。

[post_probe.json](post_probe.json)（seed `0000`–`0003`）と [post_probe_rest.json](post_probe_rest.json)（`0004`–`0009`）を合わせた全 10 件の結果。差は variant − baseline、負が改善。

| variant | 平均手数 | 平均差 | 改善 / 同点 / 悪化 | wall 平均 / 最大（秒） |
|---|---:|---:|---:|---:|
| baseline | 10740.0 | 0 | — | 0.331 / 0.368 |
| post1 | 10739.3 | −0.7 | 3 / 3 / 4 | 0.455 / 0.649 |
| post4 | 10734.4 | −5.6 | 7 / 1 / 2 | 0.554 / 0.647 |
| post8 | 10733.7 | −6.3 | 8 / 0 / 2 | 0.795 / 0.930 |

全件 `E=0`、最大 20 手/注文。post4 の seed 順の差は `+1, −1, −6, −30, +9, −5, 0, −6, −10, −8`。全件改善ではない。最初の 4 件では post4 と post8 の平均がともに 10731 手だった一方、beam 自体の CPU は post4 が 0.173–0.195 秒、post8 が 0.365–0.494 秒。計算量との釣合いから post4 を次の比較候補とした。

post4 は 1 ケースあたり約 2.5–2.9 万回の完成状態を評価し、最良候補の暫定更新は 1–18 回。ログの `selected` / `saved` はその時点の局所比較であり、後続の操作列最適化や次注文への状態変化を含むケース全体の得失とは一致しない。

**同じ判断を保つ高速化**

[fast_check.json](fast_check.json) は旧 post4 と [fast_post_beam.cpp](fast_post_beam.cpp) の 10 ケース比較。bucket の再ソートと一時 vector、子状態の重複復元、展開しない終端層の状態保存を削減し、反復順に依存しない重複検出を hash table に変更した。候補列挙、全停止候補の評価、比較順序、評価式は維持した。

全 10 件で**出力全文がバイト単位に一致**し、展開数・完成状態評価数・重複数・採用数も一致した。平均手数は双方 10734.4、`E=0`。測定値は次のとおり。

| variant | wall 平均 / 最大 | solver CPU 平均 / 最大 | post beam CPU 平均 / 最大 |
|---|---:|---:|---:|
| post4 | 0.529 / 0.553 | 0.516 / 0.540 | 0.177 / 0.192 |
| fast4 | 0.510 / 0.519 | 0.497 / 0.507 | 0.161 / 0.174 |

単位は秒。CPU は stderr の `BUDGET cpu=` / `POST_BEAM cpu=` から集計し、JSON の `solver_cpu_seconds:null` をゼロ秒として扱っていない。全体 CPU の平均短縮は約 0.019 秒、beam 部分は約 0.016 秒。beam を 0.10 秒まで短縮する目標には届いていない。時間打ち切りが起きる設定では、速さによって探索完了ノード数が変わり得るため、常に出力が一致するという主張ではない。

**候補の多様性を残す試行**

[diverse_probe.json](diverse_probe.json) は seed `0000`–`0005` の 6 件。修復する窓の集合や未一致窓の集合が同じ候補に偏らないよう shortlist / beam を構成する変更で、上の高速化と異なり探索の判断を変更する。

| variant | 平均手数 | baseline 差 | wall 平均 / 最大 |
|---|---:|---:|---:|
| baseline | 10687.500 | 0 | 0.328 / 0.345 |
| diverse1 | 10681.167 | −6.333 | 0.534 / 0.542 |
| diverse2 | 10682.167 | −5.333 | 0.547 / 0.556 |

全件 `E=0`、最大 20 手/注文。同じ 6 件の通常 post4 は平均 10682.167 手なので、diverse1 の差は追加で平均 1 手だけ。少数の既使用ケースであり、独立検証による裏付けはまだない。

**早期分岐・合同分岐・操作列のビーム**

各表は seed `0000`–`0003` の 4 件のみ。同じケースを用いた開発比較であり、独立検証ではない。

| 記録 / variant | baseline 平均 | variant 平均 | 平均差 | variant wall 平均 / 最大 |
|---|---:|---:|---:|---:|
| [early_probe](early_probe.json) / early3 | 10740.00 | 10743.75 | +3.75 | 0.550 / 0.572 |
| early6 | 10740.00 | 10743.75 | +3.75 | 0.621 / 0.633 |
| [joint_probe](joint_probe.json) / joint0 | 10740.00 | 10747.75 | +7.75 | 0.670 / 0.694 |
| joint_minus | 10740.00 | 10742.25 | +2.25 | 0.888 / 0.918 |
| [trace_probe](trace_probe.json) / trace4 | 10740.00 | 10734.75 | −5.25 | 0.434 / 0.474 |
| trace8 | 10740.00 | 10737.00 | −3.00 | 0.523 / 0.535 |

- early は返却先と主紙回収前の補助紙選択を強制分岐し、毎回クエリ全体を完了させて評価する。joint は主紙回収後の選択も連結し、joint_minus は gain −1 の候補まで許容する。いずれも開発平均が悪化し、現時点では不採用。
- trace は既存操作列の途中状態から、同じ注文を完成させる別の合法操作列を探索する。trace4 は 4 件合計で直接の局所短縮 `saved=4` に対し、ケース全体の差は −21 手。以後の重ね順や他の最適化の変化もあるため、局所短縮をそのまま最終改善と解釈しない。
- 全件 `E=0`。trace8 の最大は 22 手/注文、それ以外は 19 または 20 手。操作数上限 384 手は満たす。

**追加の開発比較**

| 記録 / variant | 件数 | baseline 平均 | variant 平均 | 平均差 | variant wall 平均 / 最大 |
|---|---:|---:|---:|---:|---:|
| [combo_probe](combo_probe.json) / combo | 10 | 10740.000 | 10732.300 | −7.700 | 0.627 / 0.720 |
| [v2_probe](v2_probe.json) / trace_v2_4 | 6 | 10687.500 | 10683.833 | −3.667 | 0.526 / 0.555 |
| trace_v2_8 | 6 | 10687.500 | 10685.333 | −2.167 | 0.707 / 0.758 |
| [validated_probe](validated_probe.json) / validated | 10 | 10740.000 | 10735.600 | −4.400 | 0.572 / 0.645 |
| [strict_probe](strict_probe.json) / strict | 10 | 10740.000 | 10740.000 | 0.000 | 0.535 / 0.601 |
| strict_plus1 | 10 | 10740.000 | 10737.700 | −2.300 | 0.535 / 0.651 |

10 件は seed `0000`–`0009`、6 件は `0000`–`0005`。全件 `E=0`。combo は fast post4 と trace4 の組合せで、個別の改善量を足し合わせた推定値ではなく実測値。trace_v2 は [trace_beam_v2.cpp](trace_beam_v2.cpp) の別実装で、未使用ケースによる裏付けはない。

[strict_post.cpp](strict_post.cpp) は、通常解より**現在の操作数が厳密に減ること**と、保持する補助紙数を増やさないことを追加条件とした。strict は開発 10 件で設計・全操作列が baseline とバイト単位に一致し、時間だけ増えた。`POST_BEAM_EXTRA_LOANS=1` の strict_plus1 は補助紙を 1 枚まで増やせる条件で、開発平均 −2.3 手。これも独立した検証による採用根拠はない。

[validated_post.cpp](validated_post.cpp) は、post beam が通常解を改善した候補だけを、現在の商品以外の **255 商品すべて**で再評価する。両終端状態から独立した scratch router の簡易方策を実行し、「現在の手数＋次注文の手数と終端 proxy の平均」を比較する。実際の次注文は読まない。次商品の分布の平均は全列挙しているが、ルータは branching・MDP 等を省いた簡易方策で、終端値も近似なので、真の最適な将来コストを計算しているわけではない。期限内に両状態の全比較が完了しなければ候補を採用しない。

**検証用 20 ケース：開発での改善が再現しない**

seed `0076`–`0095`。設計保存用の [dump_holdout.json](dump_holdout.json) と同じ fixture を使用した。主比較は [holdout.json](holdout.json)、validated は後続の [validated_holdout.json](validated_holdout.json) で、どちらも baseline 平均 10813.8 手。

| variant | 平均手数 | baseline 差 | wall 平均 / 最大（秒） |
|---|---:|---:|---:|
| baseline（holdout） | 10813.80 | 0 | 0.376 / 0.637 |
| post4 | 10814.65 | +0.85 | 0.561 / 0.757 |
| trace4 | 10814.50 | +0.70 | 0.463 / 0.584 |
| combo | 10816.80 | +3.00 | 0.655 / 0.806 |
| baseline（validated_holdout） | 10813.80 | 0 | 0.349 / 0.406 |
| validated | 10817.55 | +3.75 | 0.606 / 0.657 |

全件 `E=0`。baseline の最大は 23 手/注文、各 variant は 24 手/注文。違反がないことと高得点化を区別する。比較した 4 方式はいずれも平均が悪化し、開発結果だけを根拠とした採用は見送る。この検証集合も結果を参照した後は開発判断に使用済みなので、今後の調整を同じ集合だけで評価しない。

**設計を含む通常予算での最終動作確認**

fixture を使わず、最初から紙の設計を行う [fast_post_beam.cpp](fast_post_beam.cpp) を baseline と比較した。variant の追加設定は `POST_BEAM=1` のみで、研究用の拡張時間設定は使用していない。

| 記録 / 環境 | seed | baseline 平均 | post4 平均 | 平均差 | post4 wall 平均 / 最大 |
|---|---|---:|---:|---:|---:|
| [normal_probe](normal_probe.json) / Linux | 0000, 0001, 0076, 0088 | 10744.75 | 10752.50 | +7.75 | 1.385 / 1.431 |
| [official_probe](official_probe.json) / Windows 公式 reactive tester | 0000, 0076 | 10707.00 | 10711.00 | +4.00 | 1.784 / 1.790 |

baseline / post4 とも全件 `E=0`、最大 20 手/注文。Windows の公式 tester のスコアは保存出力を独立再生したスコアとも一致した。Windows の wall は tester を含む経過時間で、Linux の solver CPU や本番環境の時間と同一視しない。ここでは少数ケースの合法動作と通常予算での実行を確認しただけで、全ケースの 2 秒制限を保証しない。通常版でも平均手数が悪化したため、採用しない。

**コードと再現設定**

| 用途 | ソース / 設定 |
|---|---|
| 単独でコンパイルできる post beam | [fast_post_beam.cpp](fast_post_beam.cpp)。既定は無効、実行環境の `POST_BEAM=1` で有効化。幅 4、深さ 3、候補数 6 が既定 |
| 操作列探索と組合せ | [trace_beam.cpp](trace_beam.cpp)、[combo.cpp](combo.cpp)、[combo_spec.json](combo_spec.json) |
| 追加の候補採用条件 | [validated_post.cpp](validated_post.cpp)、[strict_post.cpp](strict_post.cpp)、[validated_spec.json](validated_spec.json)、[strict_spec.json](strict_spec.json) |
| 固定設計による研究用ソース生成 | [make_fixture.py](make_fixture.py)、[fixture_io.inc](fixture_io.inc)。`fixture_*.cpp` は保存設計を読む研究専用 |
| 比較条件の生成・実行 | [make_specs.py](make_specs.py)、[holdout_spec.json](holdout_spec.json)、[bench.py](../improve_eval/bench.py) |

`AHC/ahc074` を作業ディレクトリとした単独版のビルド例（Linux）：

```sh
g++ -std=c++17 -O2 experiments/beam_v1/fast_post_beam.cpp -o experiments/beam_v1/fast_post_beam_linux
```

通常予算で試す際は `POST_BEAM=1` のみを追加し、研究用の `NOV8_*_END=30` や `POST_BEAM_CPU=10` を引き継がない。既定の beam 累積予算は .6、1 呼出しの予算は .003（内部単位、Linux scale .6 を適用）。通常版には fixture の読込コードを含まない。

既存 fixture と対応する Linux 実行ファイルがある場合の研究用比較例：

```sh
python experiments/beam_v1/make_specs.py
python experiments/improve_eval/bench.py --spec experiments/beam_v1/post_spec.json --seeds 0,1,2,3,4,5,6,7,8,9 --rotate --report experiments/beam_v1/post_rerun.json
```

`make_specs.py` の fixture パスはこの作業環境の WSL パスなので、別環境では変更する。各結果 JSON は使用した実行ファイルの SHA256 と環境変数を保存している。固定設計による比較と、設計から実行する通常版の比較は分けて再現する。
