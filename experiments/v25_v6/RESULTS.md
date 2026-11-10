2026-11-06 時点の v25 派生実験の記録です。**main.cpp は変更しておらず、採用できる改善はまだ得られていません。**
確認した main.cpp と main_baseline.cpp の SHA256 はともに
`A9E5CB6B45EE1562C60721BE09BDE2D0246AD8EEB95585C7721048D74143D7DA` です。
以下は各 JSON の `summary` と必要な診断ログから集計しました。差分は候補−対照で、負が良い値です。

**測定条件と限界**

- 「設計込み」は入力から紙設計、候補選択、1024 注文の処理までを実行します。時計による打切りがあるため、同じソースでも実行速度や負荷が候補数・選択設計・後半の探索量を変えます。
- 「固定設計」は保存済み fixture を読み、オンライン部分を比較します。主に絶対期限を 30 local units、短縮累積枠を 10 local units に広げた研究用条件です。2 秒の設計込み性能を示すものではありません。quota 実験だけは累積短縮枠の既定値 .52 local units を残しました。
- 0000–0005 は繰り返し使用した開発用 6 ケースです。そこで設定を選んだ後の評価には、別のケース群を用います。小さな平均改善や単発の速度差だけでは採用しません。
- JSON の `max_cpu=0` は CPU 時間が測定されていない集計の値であり、実行時間ゼロを意味しません。`max_wall` はホスト負荷や tester 待ち時間も含むため、純粋な計算速度の比較には使えません。
- ここに記載する完了済み比較はすべて E=0 でした。誤答なしと、平均手数の改善・実環境での TLE 余裕は別に評価します。

**コンパイラ条件の訂正**

公式 C++23 は GCC 15.2.0 で、`-O2`、`-march=native`、`-ftrivial-auto-var-init=zero`、`-std=gnu++23` 等を使用します。[AHC074 公式 rules](https://atcoder.jp/contests/ahc074/rules)
初期の比較は generic-O2 のローカルビルドでした。後半は native ビルドで再確認していますが、ローカル GCC 11.4 は `-ftrivial-auto-var-init=zero` に対応していないため、**公式環境の完全再現ではありません**。CPU とコンパイラの版も異なります。
generic-O2 に POPCNT pragma を追加した差を、そのまま公式の `-march=native` 環境での改善とは扱いません。

**非線形 demand 目的関数：最終検証で不採用**

最終 anchor 焼きなましの線形露出費用に、`F(z,o)=2zo/(z+o)` の厳密な差分を混ぜる実験です。`DESIGN_FLIP=.5` が half、1 が exact。詳しいモデルと差分検証は [design_notes.md](design_notes.md) にあります。

| 条件・ケース | 対照平均 | 候補平均 | 差分 | 記録 |
| --- | ---: | ---: | ---: | --- |
| 設計込み・開発6、half | 10677.8333 | 10665.8333 | -12.0000 | [design_probe.json](design_probe.json) |
| 同上、exact | 10677.8333 | 10710.1667 | +32.3333 | 同上 |
| generic-O2・0240–0249、half | 10725.60 | 10691.90 | -33.70 | [design_validate10.json](design_validate10.json) |
| generic-O2・0250–0259、half | 10666.90 | 10634.90 | -32.00 | [design_validate_next10.json](design_validate_next10.json) |
| 上記検証20件合計 | 10696.25 | 10663.40 | -32.85 | 上記2件を件数加重 |
| release・未使用0260–0279 | 10664.65 | 10664.25 | -0.40 | [release_holdout20.json](release_holdout20.json) |
| release・未使用0280–0299 | 10666.05 | 10668.95 | +2.90 | [release_holdout_next20.json](release_holdout_next20.json) |
| **release の新規40件合計** | **10665.35** | **10666.60** | **+1.25** | 上記2件を件数加重 |

前半20件の -32.85 を最終的な改善と呼ぶことはできません。half を選んだ後の新規40件で平均が悪化したため、release.cpp を main.cpp に採用していません。選択に使った20件を新規40件と合算すると選択バイアスを隠すため、採用根拠となる60件平均は掲げません。
新規40件での最大 wall は対照 1.8654 秒、候補 1.9159 秒でしたが、これは当該ローカル測定の値です。

native の開発6件でも half は 10655.0000 → 10664.6667（+9.6667）。元の候補を残し replay/offspring のみ half にする diverse は 10669.0000（+14.0000）でした。[native_design_probe.json](native_design_probe.json)
`official3.json` は Windows バイナリをローカル公式 tester で確認した3件の E=0 の記録であり、公式コンパイラと本番環境の性能再現を意味しません。

**候補選択と設計変更**

| 実験 | ケース数 | 対照平均 | 候補平均 | 差分 | 判定・記録 |
| --- | ---: | ---: | ---: | ---: | --- |
| 合成評価512要求 | 開発6 | 10663.1667 | 10637.6667 | -25.5000 | [selection_budget_probe.json](selection_budget_probe.json) |
| 合成評価1024要求 | 開発6 | 10663.1667 | 10630.0000 | -33.1667 | 同上 |
| 選択期限を早める | 開発6 | 10663.1667 | 10661.6667 | -1.5000 | 同上、未採用 |
| 合成評価512要求 | 新規0300–0309 | 10678.90 | 10687.60 | +8.70 | [selection_validate10.json](selection_validate10.json)、不採用 |
| 合成評価1024要求 | 新規0300–0309 | 10678.90 | 10697.20 | +18.30 | 同上、不採用 |
| 最終 pair 固定で core map 再最適化 | 開発6 | 10660.0000 | 10656.8333 | -3.1667 | [core_probe.json](core_probe.json)、変更列0のため改善根拠なし |
| CoreGenesis の直接更新 | 開発6 | 10657.3333 | 10656.5000 | -0.8333 | [genesis_probe.json](genesis_probe.json)、微差のみ・未採用 |
| native で header 前 target 指定 | 開発6 | 10663.0000 | 10660.3333 | -2.6667 | [headers_probe.json](headers_probe.json)、未採用 |
| generic-O2 に POPCNT 追加 | 開発6 | 10679.6667 | 10690.6667 | +11.0000 | [normal_probe.json](normal_probe.json)、公式 native と混同しない |

core map 再最適化は各ケース約1.16万提案に対して feasible が0–7件、accepted/improvements/changed/committed はすべて0でした。表示設計が変わっていないため、-3.1667 を core 改善と解釈できません。時計に依存する後続探索の影響が含まれます。詳細は [design_core_notes.md](design_core_notes.md) にあります。

**固定設計でのオンライン比較**

以下はすべて開発6件で、各実験内の対照平均は 10621.6667 でした。

| 変更 | 候補平均 | 差分 | 記録・結論 |
| --- | ---: | ---: | --- |
| primary 短縮 quota を予約 | 10621.6667 | 0 | [route_fixed_probe.json](route_fixed_probe.json) |
| alternative も短縮固定点まで反復 | 10623.1667 | +1.5000 | 同上、不採用 |
| 上記両方 | 10623.3333 | +1.6667 | 同上、不採用 |
| primary を共通基準にした候補比較 | 10622.3333 | +0.6667 | [anchor_fixed_probe.json](anchor_fixed_probe.json)、不採用 |
| 選択的 learned-critic 1手先 rollout | 10626.5000 | +4.8333 | [critic_fixed_probe.json](critic_fixed_probe.json)、不採用 |
| 支払済み2 donor の共同配置 | 10621.6667 | 0 | [duo_fixed_probe.json](duo_fixed_probe.json)、通常/広い予算/VERIFY すべて同手数 |
| terminal 特徴計算統合 | 10621.6667 | 0 | [terminal_fixed_probe.json](terminal_fixed_probe.json) |
| terminal 特徴の4状態 memo | 10621.6667 | 0 | 同上、VERIFY も同手数 |
| future horizon .5 | 10641.6667 | +20.0000 | [discount_fixed_probe.json](discount_fixed_probe.json)、不採用 |
| future horizon 1.5 | 10629.5000 | +7.8333 | 同上、不採用 |
| learned/raw blend .5 | 10633.3333 | +11.6667 | 同上、不採用 |
| learned のみ（blend 1） | 10625.6667 | +4.0000 | 同上、不採用 |
| 同手数候補だけ learned 比較 | 10617.5000 | -4.1667 | 同上、追加検証は下記 |
| bucket MDP horizon 6 | 10632.6667 | +11.0000 | [mdp_fixed_probe.json](mdp_fixed_probe.json)、不採用 |
| bucket MDP horizon 12 | 10620.5000 | -1.1667 | 同上、小幅のみ・未採用 |
| bucket MDP repair weight .25 | 10630.0000 | +8.3333 | 同上、不採用 |
| bucket MDP repair weight .75 | 10642.8333 | +21.1667 | 同上、不採用 |

quota 予約だけの最終手数は全件同一でした。反復短縮の `repeat_saved` は評価した代替 trace 内の累積であり、最終出力の削減数ではありません。duo も一部の代替 trace は短くなりましたが、最終出力は改善していません。
critic は同手数・近い learned override だけを対象に、同じ12商品を scratch で1要求先まで処理します。全ケース32比較を完了し、追加 CPU は約 .017–.021 native 秒、誤答はありませんでしたが、平均手数は悪化しました。MDP/branching/短縮を省いた近似評価であり、実際の全方策の厳密な将来価値ではありません。

**equal-only の追加検証：不採用**

固定設計の未使用20件（0076–0095）では 10734.75 → 10733.85、差 -0.90 でした。[equal_fixed_hold20.json](equal_fixed_hold20.json)
その後、native の設計込み・新規20件（0310–0329）では **10666.80 → 10671.10、差 +4.30** となったため採用しません。[equal_normal_hold20.json](equal_normal_hold20.json)
固定設計での小さな改善が、通常の予算配分下での設計込み改善を保証しない例です。

**継続中・未確定**

- native 開発6件の PACK_W=39 は 10656.8333 → 10648.5000（-8.3333）、PACK_W=40 は 10667.3333（+10.5000）。[width_probe.json](width_probe.json) だけでは採用を決めず、より広い比較を継続中です。
- `design_wide.cpp` の CHILDMAP_WIDE は、元の候補群に inherited W40 を置換/追加する実験です。監査済みですが、完了した比較結果はまだ本記録にありません。
- 現方策に合わせた18特徴 critic の再学習を研究用 collector で検討中です。実際の未来注文は用いず、学習用ケースと係数選定後の検証ケースを分けます。未測定であり、改善は主張していません。

ソース・生成器・fixture・各 `*_spec.json`・出力ログはこのディレクトリに保存しています。中間的に良かった設定を採用済みと混同しないこと、同一 JSON 内で対照との差を比較することを今後の評価でも維持します。
