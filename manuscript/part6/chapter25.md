# 第 25 章：CLI ツールの開発

## この章のゴール
- `std::env::args` を使って、コマンドライン引数を読み取る基本的な方法を理解する。
- `clap` や `structopt` のような、より高機能な引数解析クレートを使って、堅牢な CLI を構築できるようになる。
- `main` 関数から `Result<(), Box<dyn Error>>` を返すことで、エラー処理を簡潔に記述できることを理解する。
- ファイルの読み書きや標準入出力の基本的な使い方を理解する。
- `grep` のような標準的な CLI ツールを模倣した、簡単なアプリケーションを構築できる。

## 前章の復習
前の章では、実践的な非同期プログラミングについて学び、`tokio::spawn` や同期プリミティブを使って、より複雑な非同期アプリケーションを構築する方法を習得しました。

## なぜこれが必要なのか？
Rust は、そのパフォーマンス、信頼性、そして単一のバイナリにコンパイルできるという特性から、コマンドラインインターフェース（CLI）ツールの開発に非常に適しています。`grep` のようなテキスト検索ツール、`ls` のようなファイル一覧ツール、あるいは自作のスクリプトなど、CLI は開発者の生産性を高めるための強力な道具です。この章では、これまでに学んだ知識を応用して、最初の実践的なプロジェクトとして CLI ツールを開発します。

## プロジェクトの構成
`cargo new` で新しいバイナリプロジェクトを作成することから始めます。
```sh
cargo new minigrep
cd minigrep
```
`src/main.rs` がこのプロジェクトのエントリポイントになります。

## コマンドライン引数の受け取り
### シンプルな方法: `std::env::args`
標準ライブラリの `std::env::args` 関数は、コマンドライン引数を `Iterator` として返します。
```rust
use std::env;

fn main() {
    let args: Vec<String> = env::args().collect();
    println!("{:?}", args);
}
```
`cargo run -- arg1 arg2` のように実行すると、`["target/debug/minigrep", "arg1", "arg2"]` のような出力が得られます。

### より良い方法: `clap` クレート
`std::env::args` はシンプルですが、引数のパース、バリデーション、ヘルプメッセージの生成などをすべて手動で行う必要があります。`clap` は、これらの面倒な作業を宣言的に解決してくれる非常に人気のあるクレートです。
```toml
# Cargo.toml
[dependencies]
clap = { version = "4.0", features = ["derive"] }
```

```rust
use clap::Parser;

/// Simple program to greet a person
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    /// Name of the person to greet
    #[arg(short, long)]
    name: String,

    /// Number of times to greet
    #[arg(short, long, default_value_t = 1)]
    count: u8,
}

fn main() {
    let args = Args::parse();

    for _ in 0..args.count {
        println!("Hello {}!", args.name);
    }
}
```
構造体を定義し、フィールドに属性を追加するだけで、`clap` が自動的に引数の解析、型変換、ヘルプメッセージ (`--help`) の生成などを行ってくれます。

## ファイル I/O
CLI ツールでは、ファイルの読み書きが頻繁に発生します。
```rust
use std::fs;
use std::error::Error;

// main 関数から Result を返すことで、? 演算子が使えるようになる
fn main() -> Result<(), Box<dyn Error>> {
    let content = fs::read_to_string("poem.txt")?;

    println!("File content:\n{}", content);

    Ok(())
}
```

## `minigrep` の実装
The Rust Programming Language (TRPL) の中でチュートリアルとして作られる `minigrep` を参考に、ファイルから特定の文字列を検索する簡単なツールを構築してみましょう。

### 関心事の分離
1.  `main.rs`: 引数の解析とエラーハンドリングを担当。
2.  `lib.rs`: 検索ロジックやファイル読み込みなど、主要なビジネスロジックを担当。

こうすることで、ロジック部分をライブラリとしてテストしやすくなります。

## ベストプラクティス
- **エラー処理:** `main` から `Result` を返すことで、`?` 演算子を活用し、エラー処理を簡潔にします。`anyhow` クレートを使うとさらに便利になります。
- **標準入出力:** 標準入力 (`stdin`) からの読み込みや、標準出力 (`stdout`) / 標準エラー出力 (`stderr`) への書き込みを適切に使い分けることが重要です。
- **テスト:** `lib.rs` にロジックを分離し、単体テストを書くことで、CLI ツールの信頼性を高めます。

## 練習問題
### 問題 1: `minigrep` の拡張
現在の `minigrep` に、`-i` または `--ignore-case` というフラグを追加し、大文字・小文字を区別しない検索ができるように機能を拡張してください。

## この章のまとめ
- Rust は CLI ツールの開発に非常に適している。
- コマンドライン引数の解析には、`std::env::args` よりも `clap` クレートを使うのがはるかに効率的で堅牢。
- ロジックを `lib.rs` に分離することで、テストが容易になり、コードの関心事を分離できる。
- `main` から `Result` を返すパターンは、CLI アプリケーションのエラーハンドリングで広く使われるイディオム。

## 次の章へ
CLI ツールの次は、もう一つの主要なアプリケーション分野である Web アプリケーションに挑戦します。次の章では、`actix-web` や `axum` といった非同期 Web フレームワークを使って、簡単な Web サーバーと API エンドポイントを構築する方法を学びます。
