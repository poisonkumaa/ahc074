Round 6 オンライン方策・自由終状態・剪定の実験記録

この系統から採用した変更はありません。実験中、main.cpp は変更していません。記録時の main.cpp SHA256 は `12f6ae6570b5d322e73a2e169e7f05d7ce5058a775fd65892f7b5da6f4209142` です。実験は公開された256商品と内部生成注文だけを設計に使用し、実注文は回答をflushした後に1件ずつ読みます。

R32・cut53・seed0 の固定設計を用いた手数内訳は次のとおりです。設計比較を安定させるため、実験では学習時間ガードを100秒へ広げています。

| 計測対象 | 1024注文の合計 |
| --- | ---: |
| 現行の合法操作 | 13236 |
| mandatory移動 | 9457 |
| mandatoryのうち今回selectedの紙 | 1940 |
| mandatoryのうち注文開始時に隠れていた紙 | 5502 |
| optional移動 | 3779 |
| 選択されたvisible-record紙の総数 | 16994 |
| 同じdirect_final終状態への可視制約なしの最少移動 K−LIS | 11072 |
| 固定goal・終状態自由・可視制約なしの最少移動 | 9501 |

LISによる差は859注文で発生し、1注文の最大差は7手でした。固定goalの自由終状態DPでは、最適解の中に注文開始時の可視紙の移動を含められる注文が939件ありました。そのような可視紙を最大限動かす同点解で、対象枚数は合計3664枚です。

**以前報告した7487手（平均7.31手）は無効です。** 最初のDPは残存紙の物理的なlengthだけを状態にし、非selected紙を、その紙のretained ownerより前に残すことを許していました。具体的な終状態を構築するassertで検出しました。この値、およびその版で測った可視940件・3551枚は撤回しています。正しい値は9501手、939件、3664枚です。

訂正版の定義と確認方法を残します。selected紙をlength昇順に並べ、紙kのownerを「lengthがlength[k]以上で最小のselected紙」とします。表示を保つprefix終状態にはselected紙の鎖と、ownerが各非selected紙より前という制約があります。元の順序から残す部分列を走査し、状態hを既に残した紙の最大owner-lengthとします。紙の削除は費用1、非selected紙の保持は常に可能、selected紙の保持は自身のlengthがhより大きい場合だけ可能です。保持時はhをowner-lengthで更新します。このDPは固定goalを満たす終状態への無制約最少移動を求めます。可視紙のみ動かせる条件を外した値なので、9501手が実際に達成可能という主張ではありません。

[round6_policy_free_bound_check.py](round6_policy_free_bound_check.py) で、6枚の全順列から正しい表示の全終状態を列挙した距離と、訂正後のDPを300ケース比較し一致しました。終状態構築は [round6_policy_free_goal.h](round6_policy_free_goal.h)、実ケースの計測は [round6_policy_stats_seed0.err](round6_policy_stats_seed0.err) にあります。

紙の剪定では、公開512注文を内部生成し、削除対象の紙が選択される場面は削除後のDPを解き直しました。非selected紙のmandatory移動と、sole optional blockerの消滅も評価し、上位40候補は全保存状態の最短距離を再計算しました。可到達性は全256商品について確認しています。

seed0では単独削除可能な120紙のうち、正の推定値はlength61の1紙だけでした。その削除は実注文で13236から13373へ悪化しました。seed1・2およびseed0のexact-refit前では、正の候補がありませんでした。利用が2回程度の長い紙でも、その紙が必要な場面で削除すると追加6〜10手以上を要することがあり、移動の邪魔になる分を上回りました。削除だけでは改善しませんでした。

剪定関連は [round6_policy_prune.h](round6_policy_prune.h)、[round6_policy_prune_results.json](round6_policy_prune_results.json)、[round6_policy_prune_exact_seed0.err](round6_policy_prune_exact_seed0.err) に保存しています。

穴の分位点実験では、各紙が教師goalで必要とする開始位置を収集し、その最小値の代わりに低い分位点を使いました。全256商品の到達を修復してから、可視な紙だけを動かすplannerで公式testerを通しました。この系列はR8・cut53の設計で、基準値は13343です。

| 分位点 | 穴を開ける紙数／マス数 | witness DAG版の得点 | DAGへの自由移動下限 |
| --- | --- | ---: | ---: |
| 0 | 0／0 | 13343 | 9713 |
| 0.005 | 1／3 | 13353 | 9725 |
| 0.01 | 16／184 | 13701 | 9996 |
| 0.02 | 51／1426 | 15291 | 11503 |
| 0.05 | 80／2705 | 16797 | 12885 |
| 0.1 | 87／3131 | 17303 | 13437 |

このDAG版で、prefix経路より短い合法操作列を得た注文はありませんでした。prefix終状態と自由goal終状態を使う前段階の版も、節約は0〜1手に留まりました。穴により由来商品の長いprefixを一度に描く経路を失う損が大きい結果です。

[round6_policy_dag_target.h](round6_policy_dag_target.h) は、正しい表示をする既知の順序から窓ごとのwitness紙を選び、その窓で逆色を描く全紙への先行辺を作ります。推移閉包と現順序の逆順の交差がposetとなり、二部最大matchingから最大反鎖を求めます。その部分列を保持する鎖辺を足し、topological sortで終状態を構築します。実験では構築した全終状態の表示をassertで確認しています。既知のfeasible_orderを渡せば、prefix以外のマスクにも使用できます。結果は [round6_policy_dag_results.json](round6_policy_dag_results.json) に保存しています。

maskfitでは、商品ごとに1つの実現済み順序を保存し、その全表示を保つ範囲でopaqueセルを削除した親エージェントの設計を使用しました。個別の穴は4261マスでした。canonicalのwitnessに対するDAG終状態と可視LIS plannerは、穴なし158415、穴あり179516でした。対応する自由移動下限も13079と19250で、保存された1つのwitness構成が現在の状態では高価になる問題がありました。

さらに、未確定窓で逆色を描かない紙を動的に選ぶcover greedyを実装しました。「新しく覆う窓数の0.75／1／1.25乗」を可視化コストで割った3案を比較します。隠れた候補紙は、露出させる窓を1つ選び、その窓を覆う手前の紙を候補紙の直後へ移してから、候補紙を確定済みprefixの直後へ挿入します。穴なし30393、穴あり64163で、これも現行を下回りました。

maskfit/DAGとcover greedyの最終確認はすべて公式testerでE=0、操作数上限384以内でした。結果は [round6_policy_maskfit_results.json](round6_policy_maskfit_results.json) と [round6_policy_cover_results.json](round6_policy_cover_results.json) にあります。採用候補がないため100ケースへの拡大検証は行っていません。

この記録後、CPU実験を停止します。次の担当作業は、別エージェントが作成するtrie単項鎖圧縮の等価高速化に対する独立レビューと、専用出力先を用いた低予算・UBSan検証です。
