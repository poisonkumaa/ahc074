調査対象：AHC074、2026-11-09（Asia/Tokyo）。

ユーザー確認済み：AtCoder は C++23 (GCC 15.2.0)、ローカルの pahcer は WSL / Ubuntu で実行。
この記録は公開仕様、手元の実測、現行ソースの静的確認を区別する。AtCoder 実機の診断出力はまだ受領していない。

| 項目 | ローカルで確認した値 | AtCoder で確認した値・状態 |
|---|---|---|
| GCC | Ubuntu GCC 11.4.0、/usr/bin/g++ | 選択言語 GCC 15.2.0（ユーザー・公式） |
| C++ 標準 | -std=c++20、__cplusplus=202002 | -std=gnu++23（公式） |
| コマンドライン最適化 | -O2 | -O2（公式） |
| 現 main.cpp の最適化 | O3 | O2、build_design は O1 |
| CPU | AMD Ryzen 7 5825U、8 core / 16 logical CPU | 実機値未確認 |
| native の展開 | -march=znver3 -mtune=znver3 | コンパイルホスト依存、未確認 |
| ISA | AVX2/FMA/POPCNT/BMI/BMI2 有効、AVX-512 無効 | 実機値未確認 |
| Linux | Ubuntu 22.04.2 LTS | ディストリビューション未確認 |
| kernel | 6.18.54.1-microsoft-standard-WSL2 | 実機値未確認 |
| Windows host | Windows 11 Home、build 26200 | 該当なし |
| Windows power plan | バランス。最大クロック欄は仕様値で実動作周波数ではない | 未確認 |
| glibc / libm | glibc 2.35、libc6 package 2.35-0ubuntu3.15 | 実機値未確認 |
| libstdc++ headers | _GLIBCXX_RELEASE=11、__GLIBCXX__=20230528 | 実機値未確認 |
| libstdc++ runtime package | libstdc++6 12.3.0-1ubuntu1~22.04.3 | 実機値未確認 |
| ABI | x86_64、little endian、pointer/long 8 bytes、int 4、long double 16 | 診断待ち |
| 定義済みマクロ | ONLINE_JUDGE/ATCODER 未定義 | ONLINE_JUDGE/ATCODER/NOMINMAX など定義（公式） |
| 自動変数初期化 | -ftrivial-auto-var-init 指定なし | -ftrivial-auto-var-init=zero（公式） |
| modules | 指定なし | -fmodules（公式） |
| pthread / OpenMP | 明示指定なし | -pthread -fopenmp（公式） |
| fast-math / Ofast | 明示なし、__FAST_MATH__ 未定義 | 公開コマンドに指定なし |
| FMA contraction | GCC -Q で fp-contract=fast | 公開コマンドに上書き指定なし。実際の命令生成は診断待ち |
| NDEBUG | 未定義、assert 有効 | 公開コマンドに -DNDEBUG なし |
| LTO | 指定なし | 現 GCC 欄に指定なし（Clang 欄とは異なる） |
| FP rounding | nearest、MXCSR=8064、FTZ=0、DAZ=0 | 診断待ち |
| 時計 | CLOCKS_PER_SEC=1000000、clock_getres は各時計 1ns | 診断待ち。1ns は API 報告値で実精度保証ではない |
| 可視メモリ | WSL 約6.70GiB、swap 2GiB | 問題制限・実機値未確認 |
| stack | soft 8MiB、hard unlimited | 診断待ち |
| CPU / address-space rlimit | unlimited | 診断待ち |
| cgroup | cpu.max=max 100000、memory.max=max、cpuset 0-15 | 診断待ち。上位・外部制限は別途あり得る |
| 実行対象 | Linux ELF、dynamic、PIE、libstdc++/libm/libgcc_s/libc | 公開実行コマンド ./a.out |

AtCoder 公開コンパイルコマンドの原文は atcoder_gcc_compile_command.txt、取得元の全体は language_list.html に保存。
通常のオプションに加え、constexpr depth=1024、loop-limit=524288、ops-limit=2097152、多数のライブラリ指定がある。
ローカル main.cpp が使用しない外部ライブラリも列挙されているが、実際にロードされるものは別途実機で確認すべき。
公開 install script は参照用として保存しただけで、実行していない。インストール先の既定値は実ホスト観測値ではない。
Windows PowerShell側の別環境も確認：MinGW GCC8.1.0、pahcer0.2.0、cargo/rustc1.84.1。ユーザーが普段使用するWSLのGCC11.4/pahcer0.3.1とは別物であり、今回の比較の主対象ではない。

**現ソースと実行ファイル**

- main.cpp SHA256: e52093b649200e8c52da8f398071c586e429e48101f457848718696fbd1fd6e7
- main.cpp 更新日時: 2026-11-09T18:32:26.581512+09:00
- a.out 更新日時: 2026-11-09T16:46:13.413705+09:00
- a.out の .comment は GCC 11.4.0。現在の main.cpp より古いため、現在のソースから作ったものとは扱えない。
- 調査では main.cpp、pahcer_config.toml、既存 a.out、tools のソースを変更していない。
- 最新スナップショットでは O3 がローカル限定、O1 が AtCoder 用マクロ限定に戻っている。環境別最適化差が現在も残っている。
- 調査途中にもユーザー側でmain.cppが更新されたため、この記録はaudited_main.cppとそのハッシュに対応する。調査冒頭の状態とは区別する。
- 明示的 std::fma は入っていない。式の融合位置はコンパイラに依存する。
- SSE2、AVX2 のコード分岐と target("avx2")、実行時 __builtin_cpu_supports("avx2") が存在する。
- local-only target_clones("popcnt","default") は native により __POPCNT__ が定義される通常ビルドでは無効。

**時間と pahcer**

- pahcer 実バイナリは0.3.1。Rust/cargoは1.91.0。
- threads=0 は pahcer 0.3.1 の実装上 num_cpus::get_physical()。このホストでは通常8ケース同時実行。
- 1ケースの実行コマンドは cargo run --bin tester --release ../a.out、cwd は tools。
- measure_time は Instant::now() から cmd.output() 完了までの経過時間。Cargo起動、必要なビルド、ロック待ち、tester、solver、pipe通信を含む。
- 保存済みseed0ログにはCargoのロック待ちと Finished ... 3.14s がある。pahcerの約5秒をsolver単体のCPU時間やAtCoderのTLEと同一視しない。
- AtCoder公式ルールは実時間とCPU時間の大きい方。対話待ちの細かな除外方法までは今回確認できない。
- main.cppはclock()を多用し、Linuxでは探索時間倍率0.60。WSLもAtCoderもLinuxならこのOS分岐差はない。
- NOV8_TIME_SCALEはLinuxでは最大0.60。1を設定しても0.60に丸められる。
- main.cppの主な締切を実CPU秒へ換算：pool1.05、pack1.17、matching1.20、layout1.23、selection1.308、selection hard1.374、init1.482、MDP1.524、route1.878。
- optional_end=1.8というログは全プログラムの強制終了時刻ではない。Linuxの追加経路予算や必須処理が別にある。
- 候補数、焼きなまし回数、評価要求数、追加経路の有無が実測時間で変わる。固定ミリ秒の局所予算もある。
- CPU時間計測でも、周波数・キャッシュ競合・帯域・同時実行により単位CPU時間に進む演算量が変わる。
- 調査時のbash -lc環境では、ソースが参照する107個の環境変数は全て未設定。過去のユーザーの対話シェルも同じだった保証はない。
- NO_*等には変数の値でなく存在を判定するものがあり、=0とunsetは同じではない。

**入力・tester**

- 入力はtools/inの0000〜0499、500件。各入力のハッシュをinputs_manifest.jsonへ保存。
- ユーザー申告：先頭100件には低得点の偏りがある。先頭だけの標本を全体や隠しケースの代表として使わない。
- 公式 ZIP からsrc/lib.rs、src/bin/tester.rs、Cargo.tomlを比較し、全てバイト単位で一致。
- ローカルtesterはpipe対話で1要求ずつsolverへ渡す。採点と対話処理に独自変更は確認されなかった。
- tester自体に競技の実行時間上限を強制する処理はない。pahcer計測値とAtCoder判定は別。
- testerバイナリと現在ソースの完全同一性は、ソース照合だけでは証明しない。
- 暫定50件はユーザー情報。非公開入力でありローカル500件との同一入力比較ではない。
- 公式トップに、暫定とシステムテストは異なる入力と記載がある。
- 今回の公式問題本文URLはHTTP404だったため、問題固有の時間・メモリ制限は未確認として残した。

**FMA の実測**

atcoder_probe.cppをローカルの-GCC11.4/-O2/-march=native/-std=c++20でコンパイルした。
引数a=1+2^-27、b=1-2^-27、c=-1を渡すa*b+cの結果：

| 関数の指定 | 結果 |
|---|---|
| optimize("O1") | 0x0p+0 |
| optimize("O3") | -0x1p-54 |
| std::fma | -0x1p-54 |
| volatileで積を別丸め | 0x0p+0 |

これは、FMAにより比較結果が変わる実例。乱数の短絡消費を含む探索では経路全体が変わり得る。
前回のsolver比較結果は ../compiler_repro_20261109/ に保存。あれはWSL内での比較でありAtCoder実機での比較ではない。

**残っている未確認項目と再現方法**

- AtCoderコードテストにatcoder_probe.cppをC++23(GCC15.2.0)、空入力で実行すると、実機のCPU・ISA・glibc・kernel・FP・clock・制限が分かる。
- コードテストで分かった値も、その一回の環境の観測値。本提出50ケースの全ホスト・負荷が同一である保証ではない。
- 公開設定をローカルへ近づけるにはGCC15.2と対応libstdc++、gnu++23、公開の定義・初期化・最適化設定を揃える必要がある。
- CPU環境の同一化をnativeという文字列だけで実現することはできない。
- 再現性の確認はtesterを一度ビルドして直接起動し、threads=1で単独実行するのが適切。普段の8並列性能評価とは測定条件を分けて記録する。
- 同じソース・コンパイラでも時間依存のある限り完全一致は保証できない。固定反復・固定候補数へ変更し、時間は非常停止に限定する設計が必要になる。
- 非公開入力の平均10505の保証は、環境一致とは別の問題。

参照：
- https://atcoder.jp/contests/ahc074/rules
- https://img.atcoder.jp/file/language-update/2025-10/language-list.html
- https://img.atcoder.jp/file/language-update/2025-10/116-23_clang_19-1-3.toml
- https://img.atcoder.jp/ahc074/YAAu43G9.zip
- https://atcoder.jp/contests/ahc074
- https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/x86-Options.html
- https://gcc.gnu.org/onlinedocs/gcc-15.2.0/gcc/Optimize-Options.html
- https://gcc.gnu.org/gcc-12/changes.html
- https://raw.githubusercontent.com/terry-u16/pahcer/v0.3.1/src/runner/multi.rs
- https://raw.githubusercontent.com/terry-u16/pahcer/v0.3.1/src/runner/single.rs
