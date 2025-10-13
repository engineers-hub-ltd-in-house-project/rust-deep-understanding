# 第 19 章：モジュールによるプロジェクトの整理

## この章のゴール
- `mod` キーワードを使って、単一ファイル内のコードを整理したり、別ファイルに分割したりできるようになる。
- `pub` キーワードの必要性を、コンパイルエラー「function is private」を通して説明できる。
- `use` キーワードを使って、他のモジュールへのパスを短縮し、コードの可読性を上げられる。
- クレート、パッケージ、モジュールの違いを説明できる。

---

## 19.1 問題設定：`main.rs` がごちゃごちゃしてきた！

プロジェクトが少し大きくなると、関連する機能を一つのファイルにまとめて書きたくなります。しかし、すぐにファイルは長くなり、どこに何が書かれているのか見通しが悪くなります。

`cargo new my_library` でプロジェクトを作り、ネットワークライブラリの一部を単一ファイルに書く体験をしてみましょう。

### 試してみよう：単一ファイルでの実装

```rust
// src/main.rs

// --- client モジュール相当のコード ---
fn connect() {
    println!("client::connect() called");
}

// --- network モジュール相当のコード ---
mod network {
    fn connect() {
        println!("network::connect() called");
    }
}

fn main() {
    // client の connect を呼びたい
    connect();

    // network の connect を呼びたい
    // ???
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0A%0A//%20---%20client%20%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB%E7%9B%B8%E5%BD%93%E3%81%AE%E3%82%B3%E3%83%BC%E3%83%89%20---%0Afn%20connect%28%29%20%7B%0A%20%20%20%20println%21%28%22client%3A%3Aconnect%28%29%20called%22%29%3B%0A%7D%0A%0A//%20---%20network%20%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB%E7%9B%B8%E5%BD%93%E3%81%AE%E3%82%B3%E3%83%BC%E3%83%89%20---%0Amod%20network%20%7B%0A%20%20%20%20fn%20connect%28%29%20%7B%0A%20%20%20%20%20%20%20%20println%21%28%22network%3A%3Aconnect%28%29%20called%22%29%3B%0A%20%20%20%20%7D%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20//%20client%20%E3%81%AE%20connect%20%E3%82%92%E5%91%BC%E3%81%B3%E3%81%9F%E3%81%84%0A%20%20%20%20connect%28%29%3B%0A%0A%20%20%20%20//%20network%20%E3%81%AE%20connect%20%E3%82%92%E5%91%BC%E3%81%B3%E3%81%9F%E3%81%84%0A%20%20%20%20//%20%3F%3F%3F%0A%7D)
この時点でも、いくつか問題が見えてきます。
- トップレベルの `connect` と `network` 内の `connect` のように、名前の衝突が起きやすくなる。
- `network` の中の `connect` はどうやって呼べばいいのか？（実はこのままだとプライベートなので呼べない）
- 全体の構造が分かりにくく、どの関数がどの機能グループに属するのか不明瞭。

この問題を解決するのが モジュールシステム です。

## 19.2 解決策：`mod` でコードを整理・分割する

Rust では `mod` キーワードを使って、コードを論理的なグループ（モジュール）に分割できます。モジュールは、他の言語の名前空間やパッケージに似た機能を提供します。

### `pub` でモジュールの境界を定義する

デフォルトでは、モジュール内のすべてのアイテム（関数、構造体など）は プライベート です。つまり、モジュールの外からはアクセスできません。

モジュールの外からアクセスできるようにするには、`pub` キーword を付けて 公開 (public) する必要があります。

### 試してみよう：`mod`, `pub`, `use` を体験する

先ほどのコードを、モジュールを使って整理してみましょう。`client` 機能を別のファイルに分割し、`network` 機能をインラインモジュールとして定義します。

**ステップ 1: `src/client.rs` ファイルを作成**

```rust
// src/client.rs

// この関数を main.rs から呼べるようにするには `pub` が必要
pub fn connect() {
    println!("client::connect() called");
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/client.rs%0A%0A//%20%E3%81%93%E3%81%AE%E9%96%A2%E6%95%B0%E3%82%92%20main.rs%20%E3%81%8B%E3%82%89%E5%91%BC%E3%81%B9%E3%82%8B%E3%82%88%E3%81%86%E3%81%AB%E3%81%99%E3%82%8B%E3%81%AB%E3%81%AF%20%60pub%60%20%E3%81%8C%E5%BF%85%E8%A6%81%0Apub%20fn%20connect%28%29%20%7B%0A%20%20%20%20println%21%28%22client%3A%3Aconnect%28%29%20called%22%29%3B%0A%7D)

**ステップ 2: `src/main.rs` を修正**

```rust
// src/main.rs

// `src/client.rs` ファイルを `client` モジュールとして宣言する
mod client;

mod network {
    // この関数を `main` から呼べるようにするには `pub` が必要
    pub fn connect() {
        println!("network::connect() called");
    }
}

fn main() {
    // client モジュールの connect を呼び出す
    client::connect();
    
    // network モジュールの connect を呼び出す
    network::connect();
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0A%0A//%20%60src/client.rs%60%20%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%82%92%20%60client%60%20%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB%E3%81%A8%E3%81%97%E3%81%A6%E5%AE%A3%E8%A8%80%E3%81%99%E3%82%8B%0Amod%20client%3B%0A%0Amod%20network%20%7B%0A%20%20%20%20//%20%E3%81%93%E3%81%AE%E9%96%A2%E6%95%B0%E3%82%92%20%60main%60%20%E3%81%8B%E3%82%89%E5%91%BC%E3%81%B9%E3%82%8B%E3%82%88%E3%81%86%E3%81%AB%E3%81%99%E3%82%8B%E3%81%AB%E3%81%AF%20%60pub%60%20%E3%81%8C%E5%BF%85%E8%A6%81%0A%20%20%20%20pub%20fn%20connect%28%29%20%7B%0A%20%20%20%20%20%20%20%20println%21%28%22network%3A%3Aconnect%28%29%20called%22%29%3B%0A%20%20%20%20%7D%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20//%20client%20%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB%E3%81%AE%20connect%20%E3%82%92%E5%91%BC%E3%81%B3%E5%87%BA%E3%81%9B%E3%82%8B%0A%20%20%20%20client%3A%3Aconnect%28%29%3B%0A%20%20%20%20%0A%20%20%20%20//%20network%20%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB%E3%81%AE%20connect%20%E3%82%92%E5%91%BC%E3%81%B3%E5%87%BA%E3%81%9B%E3%82%8B%0A%20%20%20%20network%3A%3Aconnect%28%29%3B%0A%7D)
`main.rs` で `mod client;` と宣言することで、Rustコンパイラは `src/client.rs` という名前のファイルを探し、その内容を `client` モジュールとして取り込みます。

もし `client.rs` の `connect` 関数から `pub` を外して `cargo run` すると、`function `connect` is private` というコンパイルエラーが発生します。このエラーを体験することで、`pub` がモジュールの公開インターフェースを定義する上でいかに重要かを理解できます。

### `use` でパスを短くする

毎回 `client::connect()` のようにフルパスで書くのは少し面倒です。`use` キーワードを使うと、モジュール内のアイテムを現在のスコープに持ち込むことができます。

```rust
// src/main.rs
mod client;
mod network;

// `use` で `client` モジュールの `connect` 関数をスコープに導入
use crate::client::connect;
// `as` で別名を付けることもできる
use crate::network::connect as network_connect;

fn main() {
    // 短い名前で呼び出せる
    connect();
    network_connect();
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Amod%20client%3B%0Amod%20network%3B%0A%0A//%20%60use%60%20%E3%81%A7%20%60client%60%20%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB%E3%81%AE%20%60connect%60%20%E9%96%A2%E6%95%B0%E3%82%92%E3%82%B9%E3%82%B3%E3%83%BC%E3%83%97%E3%81%AB%E5%B0%8E%E5%85%A5%0Ause%20crate%3A%3Aclient%3A%3Aconnect%3B%0A//%20%60as%60%20%E3%81%A7%E5%88%A5%E5%90%8D%E3%82%92%E4%BB%98%E3%81%91%E3%82%8B%E3%81%93%E3%81%A8%E3%82%82%E3%81%A7%E3%81%8D%E3%82%8B%0Ause%20crate%3A%3Anetwork%3A%3Aconnect%20as%20network_connect%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20//%20%E7%9F%AD%E3%81%84%E5%90%8D%E5%89%8D%E3%81%A7%E5%91%BC%E3%81%B3%E5%87%BA%E3%81%9B%E3%82%8B%0A%20%20%20%20connect%28%29%3B%0A%20%20%20%20network_connect%28%29%3B%0A%7D)
`crate` はクレートのルート（この場合は `src/main.rs` がいる階層）を指すキーワードです。

## 19.3 パッケージとクレート

- クレート (Crate): ライブラリまたは実行可能ファイルを生成するための、コンパイルの最小単位です。
    - ライブラリクレート: `src/lib.rs` がクレートのルート。
    - バイナリクレート: `src/main.rs` がクレートのルート。
- パッケージ (Package): 1つ以上のクレートを含み、`Cargo.toml` ファイルで管理される単位です。
  ```sh
  cargo new
  ```
  で作られるのはパッケージです。
    - 1つのパッケージは、最大1つのライブラリクレートと、複数のバイナリクレートを含むことができます。

## 19.4 まとめ

- `mod` は、コードを整理・カプセル化するための名前空間を作成する。
- `mod my_module;` は、`src/my_module.rs` ファイルをモジュールとして読み込む宣言。
- デフォルトですべてのアイテムはプライベート。`pub` を付けることでモジュール外に公開される。
- `use` は、他のモジュールへのパスを現在のスコープに持ち込み、コードを簡潔にする。

---

コードをモジュールに分割して整理できるようになりました。しかし、コードが正しい振る舞いをすることを保証するにはどうすればよいでしょうか？ 次の章では、Rust に組み込まれたテスト機能を使って、コードの品質と信頼性を高めるためのテストの書き方について学びます。
