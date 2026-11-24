# 『C++で自作言語を作ろう！』配布ソースコード

技術書『C++で自作言語を作ろう！』の配布リポジトリである。  
`start/` に、本書に沿ってE++の処理系（`epp`）を作るための開始用プロジェクトが入っている。

`start/` には各章で使う補助コードと、これから実装する未完成のファイルを含めている。  
作業するときは `start/` を直接編集せず、作業用ディレクトリへコピーして使う。

## 動作環境と必要ツール

Linux環境を対象としている。  
Windowsの場合はWSL上のLinux環境で進められる。  
Mac環境では動作を確認していない。

ビルドと実行には以下のツールを使う。

- C++20対応のGCCまたはClang
- CMake 3.20以上
- Make
- Bash
- LLVM 17（第6章以降で使用）
- Git（ZIPで取得する場合は不要）

Ubuntu 24.04 LTS（WSLを含む）でGCCを使う場合は、以下で導入できる。

```bash
sudo apt update
sudo apt install build-essential git cmake bash
sudo apt install llvm-17-dev
```

導入後、各ツールのバージョンを確認する。  
Clangを使う場合は `g++ --version` の代わりに `clang++ --version` を実行する。

```bash
git --version
g++ --version
cmake --version
make --version
bash --version
llvm-config-17 --version
```

CMakeは3.20以上、LLVMは17系であることを確認する。  
`llvm-config-17` が見つからない場合は、Bashで次を実行して使える `llvm-config` コマンドを探す。

```bash
compgen -c llvm-config | sort -u
```

## 作業用プロジェクトの用意

第1章の手順に沿って、このリポジトリをクローンするかZIPファイルをダウンロードして展開する。

```bash
git clone https://github.com/Tamagosushio/Esperopp-book-code.git
cd Esperopp-book-code
```

ZIPで取得した場合は、展開したディレクトリに移動してから以降を実行する。  
続いて `start/` を `work/epp/` にコピーし、初期状態のビルドと起動を確認する。

```bash
mkdir -p work
cp -R start work/epp
cd work/epp
chmod +x build_and_run.sh
./build_and_run.sh
```

`./build_and_run.sh` は内部で `cmake -S . -B build`、`cmake --build build`、`./build/epp` を順に実行する。  
`chmod +x build_and_run.sh` は初回だけ実行する。ビルドに成功すると次のように表示される。

```text
epp: ready
```

本書では、`CMakeLists.txt` がある `work/epp/` をプロジェクトのルートディレクトリと呼ぶ。  
エディタで `work/epp/` を開き、ソースファイルはUTF-8で保存する。ファイル名やディレクトリ名の大文字・小文字もそのまま保つ。

作業用プロジェクトの主な構成は次のとおりである。

| 場所 | 役割 |
| --- | --- |
| [`CMakeLists.txt`](start/CMakeLists.txt) | ビルド用設定ファイル |
| [`build_and_run.sh`](start/build_and_run.sh) | CMakeの設定、ビルド、起動をまとめたスクリプト |
| [`include/epp/`](start/include/epp/) | 処理系の型や関数の宣言 |
| [`include/book/`](start/include/book/) | 各章の動作確認に使うテスト用補助ヘッダ |
| [`src/`](start/src/) | 処理の実装 |
| [`src/main.cc`](start/src/main.cc) | 第6章までの動作確認に使う `main` 関数 |

## 本との進め方

### 基本の流れ

章が進んでもプロジェクトをコピーし直す必要はない。  
同じ `work/epp/` をそのまま編集し続ける。

1. 本文の指示に沿って `CMakeLists.txt` にソースや設定を追加する。
2. 指定されたファイルに関数を追加するか、未完成の関数を書き換える。
3. 第2章から第6章では各節のテストに合わせて `src/main.cc` を書き換える。
4. ルートディレクトリ（`work/epp/`）で `./build_and_run.sh` を実行し、出力結果を本文と比べる。

### 第6章でLLVM 17が見つからないとき

第6章でLLVMの設定を追加したあと、CMakeがLLVM 17を見つけられない場合は、設定ファイルの場所を調べて初回だけ `-DLLVM_DIR=` を指定する。

```bash
llvm-config-17 --cmakedir
```

たとえば `/usr/lib/llvm-17/lib/cmake/llvm` と表示された場合は、`work/epp/` で次を実行する。

```bash
cmake -S . -B build -DLLVM_DIR=/usr/lib/llvm-17/lib/cmake/llvm
```

指定した設定は `build/` に保存されるため、その後は `./build_and_run.sh` をそのまま使える。

### 第7章での切り替え

第7章では動作確認用の `src/main.cc` からコマンドライン用の `src/driver/cli_main.cc` へ切り替える。  
本文に沿って `CMakeLists.txt` を更新し、`build_and_run.sh` の最終行も `./build/epp --version` に変更する。

実行時処理とリンクを実装したあとは、`./build_and_run.sh` で処理系をビルドし、生成した `./build/epp` でE++のソースを検査して実行ファイルを作る。

```bash
./build/epp check <ソースファイル.epp>
./build/epp build -o <出力先> <ソースファイル.epp>
```
