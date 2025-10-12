# 第 25 章：実践プロジェクト：CLIツール `mini-grep` の開発

## この章のゴール
- これまでに学んだ知識を総動員して、ゼロから実践的なCLIツールを構築できる。
- `clap` クレートを使い、構造体の定義からコマンドライン引数をパースする方法を体験する。
- `main` 関数から `Result<(), Box<dyn Error>>` を返すパターンを使い、ファイルI/Oなどのエラーを実践的に処理できる。
- CLIアプリケーションのコアロジックをライブラリとして分離し、テストを作成するメリットを体験する。

---

## 25.1 なぜこれが必要か？ 学んだ知識の総動員

これまでの章で、Rustの基本的な文法から所有権、非同期プログラミングまで、多くのことを学んできました。この第6部では、それらの知識を組み合わせて、実世界で役立つアプリケーションを構築します。

最初のプロジェクトは、Rustが非常に得意とする分野の一つ、コマンドラインインターフェース（CLI）ツールです。パフォーマンスが良く、単一の実行バイナリを配布できるRustは、CLIツールの開発に最適です。ここでは、多くのRustプログラマが最初に作る定番のプロジェクト、`grep`の簡易版である`mini-grep`を構築します。

## 25.2 プロジェクトの仕様と準備

`cargo new mini-grep` でプロジェクトを作りましょう。今回作成する`mini-grep`の仕様は以下の通りです。

1.  2つのコマンドライン引数を取ります：検索したい文字列と、検索対象のファイルパス。
2.  コマンドは `cargo run -- <検索語> <ファイルパス>` のように実行します。
3.  指定されたファイルを読み込み、検索語を含む行を標準出力に表示します。

## 25.3 `clap`による引数パース

CLIツールにとって、コマンドライン引数の解釈は最初の関門です。ここではデファクトスタンダードである `clap` クレートを使います。`derive` 機能を使うことで、構造体を定義するだけでリッチな引数パーサーを自動生成できます。

まず、`Cargo.toml`に`clap`を追加します。
```toml
# Cargo.toml
[dependencies]
clap = { version = "4.0", features = ["derive"] }
```

### 試してみよう：引数をパースする構造体を定義する

`src/main.rs` を編集し、`clap` を使って引数をパースするコードを書きましょう。

```rust
// src/main.rs
use clap::Parser;

/// ファイルから特定の文字列を含む行を検索します
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    /// 検索する文字列
    query: String,

    /// 検索対象のファイルパス
    file_path: std::path::PathBuf,
}

fn main() {
    let args = Args::parse();

    println!("Searching for '{}'", args.query);
    println!("In file '{}'", args.file_path.display());
}
```
この状態で `cargo run` を実行してみましょう。

- `cargo run`: 引数が足りないというエラーと、自動生成されたヘルプメッセージが表示されます。
- `cargo run -- --help`: 詳細なヘルプメッセージが表示されます。
- `cargo run -- test poem.txt`: `args` 構造体に値がパースされ、`println!` の内容が表示されます。（`poem.txt`というファイルはまだないので、後のステップで作成します）

## 25.4 ファイルの読み込みとエラーハンドリング

次に関数のコアロジックであるファイル読み込みを実装します。ここでは、実践的なアプリケーションで一般的なエラーハンドリングのパターンを体験します。

### 試してみよう：`main`から`Result`を返す

`main` 関数が `Result` を返すように変更することで、`?` 演算子を使ってエラー処理を非常に簡潔に書くことができます。

```rust
// src/main.rs
use clap::Parser;
use std::error::Error;

// ... Args構造体の定義は同じ ...
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    /// 検索する文字列
    query: String,
    /// 検索対象のファイルパス
    file_path: std::path::PathBuf,
}


fn main() -> Result<(), Box<dyn Error>> {
    let args = Args::parse();

    println!("Searching for '{}'", args.query);
    println!("In file '{}'", args.file_path.display());

    // ファイルの内容を読み込む
    let content = std::fs::read_to_string(&args.file_path)?;
    
    println!("With text:\n{}", content);

    Ok(())
}
```
ここで、プロジェクトのルートに `poem.txt` というファイルを作成し、何行かテキストを書いておきましょう。

```txt
# poem.txt
I'm nobody! Who are you?
Are you nobody, too?
Then there's a pair of us - don't tell!
They'd banish us, you know.
```

`cargo run -- nobody poem.txt` を実行するとファイル内容が表示されます。一方、`cargo run -- nobody no_such_file.txt` のように存在しないファイルを指定すると、`?` 演算子によってエラーが処理され、OS標準のエラーメッセージ（`No such file or directory`）が表示されることを確認できます。

## 25.5 ロジックの分離とテスト

現在の`main`関数は、引数パース、ファイル読み込み、ロジックの実行、結果の表示と、多くの責務を持ちすぎています。検索ロジックをライブラリに切り出し、テストを作成しましょう。

`cargo`では、`src/main.rs` と `src/lib.rs` を共存させることができます。

```rust
// src/lib.rs
pub fn search<'a>(query: &str, contents: &'a str) -> Vec<&'a str> {
    contents
        .lines()
        .filter(|line| line.contains(query))
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn one_result() {
        let query = "duct";
        let contents = "\
Rust:
safe, fast, productive.
Pick three.";
        assert_eq!(vec!["safe, fast, productive."], search(query, contents));
    }
}
```

`main`関数からこの`search`関数を呼び出すように変更します。

```rust
// src/main.rs
// ... use文とArgs構造体定義 ...
use clap::Parser;
use std::error::Error;
// mini_grepクレート（src/lib.rs）のsearch関数をインポート
use mini_grep::search;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    /// 検索する文字列
    query: String,
    /// 検索対象のファイルパス
    file_path: std::path::PathBuf,
}

fn main() -> Result<(), Box<dyn Error>> {
    let args = Args::parse();
    let content = std::fs::read_to_string(&args.file_path)?;

    for line in search(&args.query, &content) {
        println!("{}", line);
    }

    Ok(())
}
```

`cargo test` を実行してライブラリのテストがパスすることを確認し、`cargo run -- are poem.txt` を実行して、"Are you nobody, too?" が表示されることを確認しましょう。

## 25.6 まとめ

- `clap` の `derive` マクロは、構造体を定義するだけで堅牢なCLI引数パーサーを生成できる。
- `main` 関数から `Result<(), Box<dyn Error>>` を返すのは、アプリケーションレベルのエラーを扱うための実践的なイディオム。
- コアロジックを `src/lib.rs` に、CLIのI/Oやセットアップを `src/main.rs` に分離することで、テストが容易で再利用性の高いコードになる。

---

次の章では、もう一つの主要なアプリケーション分野であるWebアプリケーションに挑戦します。
