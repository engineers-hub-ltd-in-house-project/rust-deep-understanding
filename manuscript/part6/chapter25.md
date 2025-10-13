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

```sh
cargo new mini-grep
```
でプロジェクトを作りましょう。今回作成する`mini-grep`の仕様は以下の通りです。

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Ause%20clap%3A%3AParser%3B%0A%0A///%20%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%81%8B%E3%82%89%E7%89%B9%E5%AE%9A%E3%81%AE%E6%96%87%E5%AD%97%E5%88%97%E3%82%92%E5%90%AB%E3%82%80%E8%A1%8C%E3%82%92%E6%A4%9C%E7%B4%A2%E3%81%97%E3%81%BE%E3%81%99%0A%23%5Bderive%28Parser%2C%20Debug%29%5D%0A%23%5Bcommand%28version%2C%20about%2C%20long_about%20%3D%20None%29%5D%0Astruct%20Args%20%7B%0A%20%20%20%20///%20%E6%A4%9C%E7%B4%A2%E3%81%99%E3%82%8B%E6%96%87%E5%AD%97%E5%88%97%0A%20%20%20%20query%3A%20String%2C%0A%0A%20%20%20%20///%20%E6%A4%9C%E7%B4%A2%E5%AF%BE%E8%B1%A1%E3%81%AE%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%83%91%E3%82%B9%0A%20%20%20%20file_path%3A%20std%3A%3Apath%3A%3APathBuf%2C%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20args%20%3D%20Args%3A%3Aparse%28%29%3B%0A%0A%20%20%20%20println%21%28%22Searching%20for%20%27%7B%7D%27%22%2C%20args.query%29%3B%0A%20%20%20%20println%21%28%22In%20file%20%27%7B%7D%27%22%2C%20args.file_path.display%28%29%29%3B%0A%7D)
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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Ause%20clap%3A%3AParser%3B%0Ause%20std%3A%3Aerror%3A%3AError%3B%0A%0A//%20...%20Args%E6%A7%8B%E9%80%A0%E4%BD%93%E3%81%AE%E5%AE%9A%E7%BE%A9%E3%81%AF%E5%90%8C%E3%81%98%20...%0A%23%5Bderive%28Parser%2C%20Debug%29%5D%0A%23%5Bcommand%28version%2C%20about%2C%20long_about%20%3D%20None%29%5D%0Astruct%20Args%20%7B%0A%20%20%20%20///%20%E6%A4%9C%E7%B4%A2%E3%81%99%E3%82%8B%E6%96%87%E5%AD%97%E5%88%97%0A%20%20%20%20query%3A%20String%2C%0A%20%20%20%20///%20%E6%A4%9C%E7%B4%A2%E5%AF%BE%E8%B1%A1%E3%81%AE%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%83%91%E3%82%B9%0A%20%20%20%20file_path%3A%20std%3A%3Apath%3A%3APathBuf%2C%0A%7D%0A%0A%0Afn%20main%28%29%20-%3E%20Result%3C%28%29%2C%20Box%3Cdyn%20Error%3E%3E%20%7B%0A%20%20%20%20let%20args%20%3D%20Args%3A%3Aparse%28%29%3B%0A%0A%20%20%20%20println%21%28%22Searching%20for%20%27%7B%7D%27%22%2C%20args.query%29%3B%0A%20%20%20%20println%21%28%22In%20file%20%27%7B%7D%27%22%2C%20args.file_path.display%28%29%29%3B%0A%0A%20%20%20%20//%20%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%81%AE%E5%86%85%E5%AE%B9%E3%82%92%E8%AA%AD%E3%81%BF%E8%BE%BC%E3%82%80%0A%20%20%20%20let%20content%20%3D%20std%3A%3Afs%3A%3Aread_to_string%28%26args.file_path%29%3F%3B%0A%20%20%20%20%0A%20%20%20%20println%21%28%22With%20text%3A%5Cn%7B%7D%22%2C%20content%29%3B%0A%0A%20%20%20%20Ok%28%28%29%29%0A%7D)
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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/lib.rs%0Apub%20fn%20search%3C%27a%3E%28query%3A%20%26str%2C%20contents%3A%20%26%27a%20str%29%20-%3E%20Vec%3C%26%27a%20str%3E%20%7B%0A%20%20%20%20contents%0A%20%20%20%20%20%20%20%20.lines%28%29%0A%20%20%20%20%20%20%20%20.filter%28%7Cline%7C%20line.contains%28query%29%29%0A%20%20%20%20%20%20%20%20.collect%28%29%0A%7D%0A%0A%23%5Bcfg%28test%29%5D%0Amod%20tests%20%7B%0A%20%20%20%20use%20super%3A%3A%2A%3B%0A%0A%20%20%20%20%23%5Btest%5D%0A%20%20%20%20fn%20one_result%28%29%20%7B%0A%20%20%20%20%20%20%20%20let%20query%20%3D%20%22duct%22%3B%0A%20%20%20%20%20%20%20%20let%20contents%20%3D%20%22%5C%0ARust%3A%0Asafe%2C%20fast%2C%20productive.%0APick%20three.%22%3B%0A%20%20%20%20%20%20%20%20assert_eq%21%28vec%21%5B%22safe%2C%20fast%2C%20productive.%22%5D%2C%20search%28query%2C%20contents%29%29%3B%0A%20%20%20%20%7D%0A%7D)

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0A//%20...%20use%E6%96%87%E3%81%A8Args%E6%A7%8B%E9%80%A0%E4%BD%93%E5%AE%9A%E7%BE%A9%20...%0Ause%20clap%3A%3AParser%3B%0Ause%20std%3A%3Aerror%3A%3AError%3B%0A//%20mini_grep%E3%82%AF%E3%83%AC%E3%83%BC%E3%83%88%EF%BC%88src/lib.rs%EF%BC%89%E3%81%AEsearch%E9%96%A2%E6%95%B0%E3%82%92%E3%82%A4%E3%83%B3%E3%83%9D%E3%83%BC%E3%83%88%0Ause%20mini_grep%3A%3Asearch%3B%0A%0A%23%5Bderive%28Parser%2C%20Debug%29%5D%0A%23%5Bcommand%28version%2C%20about%2C%20long_about%20%3D%20None%29%5D%0Astruct%20Args%20%7B%0A%20%20%20%20///%20%E6%A4%9C%E7%B4%A2%E3%81%99%E3%82%8B%E6%96%87%E5%AD%97%E5%88%97%0A%20%20%20%20query%3A%20String%2C%0A%20%20%20%20///%20%E6%A4%9C%E7%B4%A2%E5%AF%BE%E8%B1%A1%E3%81%AE%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%83%91%E3%82%B9%0A%20%20%20%20file_path%3A%20std%3A%3Apath%3A%3APathBuf%2C%0A%7D%0A%0Afn%20main%28%29%20-%3E%20Result%3C%28%29%2C%20Box%3Cdyn%20Error%3E%3E%20%7B%0A%20%20%20%20let%20args%20%3D%20Args%3A%3Aparse%28%29%3B%0A%20%20%20%20let%20content%20%3D%20std%3A%3Afs%3A%3Aread_to_string%28%26args.file_path%29%3F%3B%0A%0A%20%20%20%20for%20line%20in%20search%28%26args.query%2C%20%26content%29%20%7B%0A%20%20%20%20%20%20%20%20println%21%28%22%7B%7D%22%2C%20line%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20Ok%28%28%29%29%0A%7D)

`cargo test` を実行してライブラリのテストがパスすることを確認し、`cargo run -- are poem.txt` を実行して、"Are you nobody, too?" が表示されることを確認しましょう。

## 25.6 まとめ

- `clap` の `derive` マクロは、構造体を定義するだけで堅牢なCLI引数パーサーを生成できる。
- `main` 関数から `Result<(), Box<dyn Error>>` を返すのは、アプリケーションレベルのエラーを扱うための実践的なイディオム。
- コアロジックを `src/lib.rs` に、CLIのI/Oやセットアップを `src/main.rs` に分離することで、テストが容易で再利用性の高いコードになる。

---

次の章では、もう一つの主要なアプリケーション分野であるWebアプリケーションに挑戦します。
