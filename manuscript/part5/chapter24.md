# 第 24 章：非同期処理：`async`/`await`

## この章のゴール
- 同期処理と非同期処理の違い、特にI/Oバウンドなタスクにおける非同期処理の利点を説明できる。
- `async` ブロックや `async move` クロージャを使い、非同期なコードの断片を作成できる。
- `Future` トレイトが、非同期処理における「未来のある時点で完了する値」を表す核となる抽象化であることを理解する。

---

## 24.1 なぜ非同期処理が必要なのか？：スレッドモデルの限界

前章で学んだスレッドは、CPUバウンドなタスク（純粋な計算処理）を並列化するには非常に効果的です。しかし、ネットワーク通信やファイルI/Oのような I/Oバウンド なタスクでは、スレッドはリソースを効率的に使えないことがあります。

1.  生成コスト: OSスレッドの生成と切り替え（コンテキストスイッチ）には、CPU時間とメモリの両面で無視できないコストがかかります。
2.  数の限界: OSが同時に扱えるスレッドの数には上限があります。

例えば、数万のクライアントからの同時接続を処理するWebサーバーを考えてみましょう。クライアントごとに1つのOSスレッドを割り当てていては、あっという間にリソースを使い果たしてしまいます。

この問題を解決するのが非同期プログラミングです。非同期モデルでは、あるタスクがI/O（ネットワークからの応答待ちなど）でブロックされている間、CPUを他のタスクの実行に充てることができます。これにより、ごく少数のOSスレッドで、非常に多くのタスクを効率的にさばくことが可能になります。

非同期処理は、この「待機時間」を有効活用するためのプログラミングモデルです。単一のスレッド上で、あるタスクがI/Oでブロックされている間に、別のタスクを実行することができます。これにより、OSスレッドを大量に作ることなく、多数のI/Oバウンドなタスクを効率的に捌くことができます。

## 24.2 `async`/`await` の基本と Future

Rustの非同期処理は、`async` と `await` という2つのキーワードを中心に構築されています。

- `async`: 関数やブロックを非同期処理の単位としてマークします。`async` とマークされた関数は、通常の値を直接返す代わりに、`Future` と呼ばれる特別な型を返します。
- `.await`: `Future` の処理が完了するのを待ち、結果を取り出します。`.await` は `async` 関数または `async` ブロックの中でのみ使用できます。

### `Future`：未来の結果を表す計画書

`async` 関数を呼び出すだけでは、その中のコードは実行されません。`async` 関数は、その処理を実行するための「計画書」や「レシピ」のようなものである `Future` を返すだけです。

この `Future` という計画書を実際に実行し、処理を進めてくれるのが **非同期ランタイム** です。

### 試してみよう：`Future` の実行

実際に `Future` を実行してみましょう。

```sh
cargo new async-basics
```
でプロジェクトを作り、`Cargo.toml` に最も人気のある非同期ランタイムである `tokio` を追加します。

```toml
# Cargo.toml
[dependencies]
tokio = { version = "1", features = ["macros", "rt-multi-thread"] }
```

そして、`src/main.rs` を以下のように編集します。

```rust
// src/main.rs
use tokio::time::{sleep, Duration};

async fn say_hello() {
    // 10ミリ秒待つ非同期処理
    sleep(Duration::from_millis(10)).await;
    println!("Hello, async world!");
}

#[tokio::main]
async fn main() { // main 関数も async fn になっている
    println!("Let's start!");
    // say_hello() は Future を返す
    let future = say_hello(); 
    
    // .await で Future が完了するのを待つ
    future.await; 
    
    println!("Finished.");
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&tokio=true&code=use%20tokio%3A%3Atime%3A%3A%7Bsleep%2C%20Duration%7D%3B%0A%0Aasync%20fn%20say_hello%28%29%20%7B%0A%20%20%20%20%2F%2F%2010%E3%83%9F%E3%83%AA%E7%A7%92%E5%BE%85%E3%81%A4%E9%9D%9E%E5%90%8C%E6%9C%9F%E5%87%A6%E7%90%86%0A%20%20%20%20sleep%28Duration%3A%3Afrom_millis%2810%29%29.await%3B%0A%20%20%20%20println!%28%22Hello%2C%20async%20world%21%22%29%3B%0A%7D%0A%0A%23%5Btokio%3A%3Amain%5D%0Aasync%20fn%20main%28%29%20%7B%20%2F%2F%20main%20%E9%96%A2%E6%95%B0%E3%82%82%20async%20fn%20%E3%81%AB%E3%81%AA%E3%81%A3%E3%81%A6%E3%81%84%E3%82%8B%0A%20%20%20%20println!%28%22Let%27s%20start%21%22%29%3B%0A%20%20%20%20%2F%2F%20say_hello%28%29%20%E3%81%AF%20Future%20%E3%82%92%E8%BF%94%E3%81%99%0A%20%20%20%20let%20future%20%3D%20say_hello%28%29%3B%20%0A%20%20%20%20%0A%20%20%20%20%2F%2F%20.await%20%E3%81%A7%20Future%20%E3%81%8C%E5%AE%8C%E4%BA%86%E3%81%99%E3%82%8B%E3%81%AE%E3%82%92%E5%BE%85%E3%81%A4%0A%20%20%20%20future.await%3B%20%0A%20%20%20%20%0A%20%20%20%20println!%28%22Finished.%22%29%3B%0A%7D)

`#[tokio::main]` マクロが、`main` 関数を非同期ランタイムのエントリーポイントに変えてくれます。この中で `.await` を使うことで、初めて `Future` が実行され、その完了を待つことができます。もし `.await` を付け忘れると、`Future` は生成されるだけで実行されず、コンパイラが警告を出してくれます。

## 24.3 実践：複数URLのコンテンツを並行取得する

`async`/`await` の真価は、複数のI/Oバウンドなタスクを並行して実行することで発揮されます。

ここでは、複数のWebサイトのHTMLをダウンロードし、それぞれの`<title>`タグの中身を抽出する、という現実的な例を見てみましょう。

`Cargo.toml` に、HTTPリクエストを行うための `reqwest` クレートを追加します。

```toml
# Cargo.toml
[dependencies]
tokio = { version = "1", features = ["macros", "rt-multi-thread"] }
reqwest = "0.11"
```

### 直列実行（アンチパターン）

まずは、単純にループの中で `.await` を使ってみます。

```rust
use std::time::Instant;

async fn fetch_title(url: &str) -> Result<String, reqwest::Error> {
    let body = reqwest::get(url).await?.text().await?;
    // 簡単のため、単純な文字列検索でタイトルを抽出
    let title = body
        .lines()
        .find(|line| line.contains("<title>"))
        .map(|line| line.trim().to_string())
        .unwrap_or_else(|| "No title found".to_string());
    Ok(title)
}

#[tokio::main]
async fn main() {
    let urls = [
        "https://www.rust-lang.org/",
        "https://www.google.com/",
        "https://www.microsoft.com/",
    ];

    let start = Instant::now();

    for url in urls {
        match fetch_title(url).await {
            Ok(title) => println!("Title for {}: {}", url, title),
            Err(e) => eprintln!("Error fetching {}: {}", url, e),
        }
    }

    println!("Total time (serial): {:?}", start.elapsed());
}
```

このコードは正しく動作しますが、各リクエストが完了するのを `await` で待ってから次のリクエストを開始するため、処理は **直列に** 実行されます。全体の実行時間は、各リクエストの所要時間の合計とほぼ同じになります。

### 並行実行 (`tokio::spawn`)

これらのリクエストを並行して実行するには、各非同期処理を独立したタスクとして生成します。`tokio::spawn` は、`Future` を受け取ってバックグラウンドで実行を開始し、そのタスクを管理するための `JoinHandle` を返します。

```rust
use std::time::Instant;

// fetch_title関数は先ほどと同じ
async fn fetch_title(url: &'static str) -> Result<String, reqwest::Error> {
    let body = reqwest::get(url).await?.text().await?;
    let title = body
        .lines()
        .find(|line| line.contains("<title>"))
        .map(|line| line.trim().to_string())
        .unwrap_or_else(|| "No title found".to_string());
    Ok(title)
}

#[tokio::main]
async fn main() {
    let urls = [
        "https://www.rust-lang.org/",
        "https://www.google.com/",
        "https://www.microsoft.com/",
    ];

    let start = Instant::now();

    let mut handles = vec![];
    for url in urls {
        // 各Futureを独立したタスクとして生成
        let handle = tokio::spawn(async move {
            fetch_title(url).await
        });
        handles.push(handle);
    }

    for (i, handle) in handles.into_iter().enumerate() {
        // 各タスクの結果を待つ
        match handle.await.unwrap() {
            Ok(title) => println!("Title for {}: {}", urls[i], title),
            Err(e) => eprintln!("Error fetching {}: {}", urls[i], e),
        }
    }

    println!("Total time (concurrent): {:?}", start.elapsed());
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&tokio=true&code=use%20std%3A%3Atime%3A%3AInstant%3B%0A%0Aasync%20fn%20fetch_title%28url%3A%20%26%27static%20str%29%20-%3E%20Result%3CString%2C%20reqwest%3A%3AError%3E%20%7B%0A%20%20%20%20let%20body%20%3D%20reqwest%3A%3Aget%28url%29.await%3F.text%28%29.await%3F%3B%0A%20%20%20%20let%20title%20%3D%20body%0A%20%20%20%20%20%20%20%20.lines%28%29%0A%20%20%20%20%20%20%20%20.find%28%7Cline%7C%20line.contains%28%22%3Ctitle%3E%22%29%29%0A%20%20%20%20%20%20%20%20.map%28%7Cline%7C%20line.trim%28%29.to_string%28%29%29%0A%20%20%20%20%20%20%20%20.unwrap_or_else%28%7C%7C%20%22No%20title%20found%22.to_string%28%29%29%3B%0A%20%20%20%20Ok%28title%29%0A%7D%0A%0A%23%5Btokio%3A%3Amain%5D%0Aasync%20fn%20main%28%29%20%7B%0A%20%20%20%20let%20urls%20%3D%20%5B%0A%20%20%20%20%20%20%20%20%22https%3A%2F%2Fwww.rust-lang.org%2F%22%2C%0A%20%20%20%20%20%20%20%20%22https%3A%2F%2Fwww.google.com%2F%22%2C%0A%20%20%20%20%20%20%20%20%22https%3A%2F%2Fwww.microsoft.com%2F%22%2C%0A%20%20%20%20%5D%3B%0A%0A%20%20%20%20let%20start%20%3D%20Instant%3A%3Anow%28%29%3B%0A%0A%20%20%20%20let%20mut%20handles%20%3D%20vec!%5B%5D%3B%0A%20%20%20%20for%20url%20in%20urls%20%7B%0A%20%20%20%20%20%20%20%20let%20handle%20%3D%20tokio%3A%3Aspawn%28async%20move%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20fetch_title%28url%29.await%0A%20%20%20%20%20%20%20%20%7D%29%3B%0A%20%20%20%20%20%20%20%20handles.push%28handle%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20for%20%28i%2C%20handle%29%20in%20handles.into_iter%28%29.enumerate%28%29%20%7B%0A%20%20%20%20%20%20%20%20match%20handle.await.unwrap%28%29%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20Ok%28title%29%20%3D%3E%20println!%28%22Title%20for%20%7B%7D%3A%20%7B%7D%22%2C%20urls%5Bi%5D%2C%20title%29%2C%0A%20%20%20%20%20%20%20%20%20%20%20%20Err%28e%29%20%3D%3E%20eprintln!%28%22Error%20fetching%20%7B%7D%3A%20%7B%7D%22%2C%20urls%5Bi%5D%2C%20e%29%2C%0A%20%20%20%20%20%20%20%20%7D%0A%20%20%20%20%7D%0A%0A%20%20%20%20println!%28%22Total%20time%20%28concurrent%29%3A%20%7B%3A%3F%7D%22%2C%20start.elapsed%28%29%29%3B%0A%7D)

今度は、すべてのリクエストがほぼ同時に開始されるため、全体の実行時間は、最も時間のかかったリクエストの所要時間とほぼ同じになり、大幅に高速化されます。

## 24.4 非同期タスクの管理

先ほどの例では `tokio::spawn` を使って手動でタスクを管理しましたが、非同期エコシステムには、こうした定型的な処理をより簡単にするためのユーティリティが豊富に用意されています。

### `futures::future::join_all`

`futures` は、`Future` を扱うための基本的なユーティリティを提供する、Rust非同期エコシステムの中心的なクレートです。その中にある `join_all` 関数を使うと、複数の `Future` を一つの `Future` にまとめることができ、すべての `Future` が完了するのを一度に待つことができます。

`Cargo.toml` に `futures` を追加します。

```toml
# Cargo.toml
[dependencies]
tokio = { version = "1", features = ["macros", "rt-multi-thread"] }
reqwest = "0.11"
futures = "0.3"
```

`join_all` を使うと、先ほどの並行実行のコードは以下のように書き換えられます。

```rust
use futures::future;
use std::time::Instant;

// fetch_title関数は先ほどと同じ
async fn fetch_title(url: &'static str) -> Result<String, reqwest::Error> {
    let body = reqwest::get(url).await?.text().await?;
    let title = body
        .lines()
        .find(|line| line.contains("<title>"))
        .map(|line| line.trim().to_string())
        .unwrap_or_else(|| "No title found".to_string());
    Ok(title)
}

#[tokio::main]
async fn main() {
    let urls = [
        "https://www.rust-lang.org/",
        "https://www.google.com/",
        "https://www.microsoft.com/",
    ];

    let start = Instant::now();

    // 各URLに対するFutureのVecを作成
    let futures = urls.iter().map(|url| fetch_title(url));

    // すべてのFutureを並行して実行し、結果を待つ
    let results = future::join_all(futures).await;

    for (i, result) in results.into_iter().enumerate() {
        match result {
            Ok(title) => println!("Title for {}: {}", urls[i], title),
            Err(e) => eprintln!("Error fetching {}: {}", urls[i], e),
        }
    }

    println!("Total time (join_all): {:?}", start.elapsed());
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&tokio=true&code=use%20futures%3A%3Afuture%3B%0Ause%20std%3A%3Atime%3A%3AInstant%3B%0A%0Aasync%20fn%20fetch_title%28url%3A%20%26%27static%20str%29%20-%3E%20Result%3CString%2C%20reqwest%3A%3AError%3E%20%7B%0A%20%20%20%20let%20body%20%3D%20reqwest%3A%3Aget%28url%29.await%3F.text%28%29.await%3F%3B%0A%20%20%20%20let%20title%20%3D%20body%0A%20%20%20%20%20%20%20%20.lines%28%29%0A%20%20%20%20%20%20%20%20.find%28%7Cline%7C%20line.contains%28%22%3Ctitle%3E%22%29%29%0A%20%20%20%20%20%20%20%20.map%28%7Cline%7C%20line.trim%28%29.to_string%28%29%29%0A%20%20%20%20%20%20%20%20.unwrap_or_else%28%7C%7C%20%22No%20title%20found%22.to_string%28%29%29%3B%0A%20%20%20%20Ok%28title%29%0A%7D%0A%0A%23%5Btokio%3A%3Amain%5D%0Aasync%20fn%20main%28%29%20%7B%0A%20%20%20%20let%20urls%20%3D%20%5B%0A%20%20%20%20%20%20%20%20%22https%3A%2F%2Fwww.rust-lang.org%2F%22%2C%0A%20%20%20%20%20%20%20%20%22https%3A%2F%2Fwww.google.com%2F%22%2C%0A%20%20%20%20%20%20%20%20%22https%3A%2F%2Fwww.microsoft.com%2F%22%2C%0A%20%20%20%20%5D%3B%0A%0A%20%20%20%20let%20start%20%3D%20Instant%3A%3Anow%28%29%3B%0A%0A%20%20%20%20let%20futures%20%3D%20urls.iter%28%29.map%28%7Curl%7C%20fetch_title%28url%29%29%3B%0A%0A%20%20%20%20let%20results%20%3D%20future%3A%3Ajoin_all%28futures%29.await%3B%0A%0A%20%20%20%20for%20%28i%2C%20result%29%20in%20results.into_iter%28%29.enumerate%28%29%20%7B%0A%20%20%20%20%20%20%20%20match%20result%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20Ok%28title%29%20%3D%3E%20println!%28%22Title%20for%20%7B%7D%3A%20%7B%7D%22%2C%20urls%5Bi%5D%2C%20title%29%2C%0A%20%20%20%20%20%20%20%20%20%20%20%20Err%28e%29%20%3D%3E%20eprintln!%28%22Error%20fetching%20%7B%7D%3A%20%7B%7D%22%2C%20urls%5Bi%5D%2C%20e%29%2C%0A%20%20%20%20%20%20%20%20%7D%0A%20%20%20%20%7D%0A%0A%20%20%20%20println!%28%22Total%20time%20%28join_all%29%3A%20%7B%3A%3F%7D%22%2C%20start.elapsed%28%29%29%3B%0A%7D)

`tokio::spawn` と `join_all` のどちらを使うかは状況によりますが、`join_all` の方がコードがシンプルになることが多いです。このように、Rustの非同期エコシステムは、言語機能（`async`/`await`）と、それを支えるライブラリ（ランタイムやユーティリティ）が協調して成り立っています。

## 24.5 まとめ

- 非同期処理は、特にネットワークサービスのようなI/Oバウンドなタスクにおいて、スレッドよりも少ないリソースで高いパフォーマンスを発揮する。
- `async fn` は、それ自体では実行されない「計画書」である `Future` を返す。
- `Future` を実行するには、`tokio` のような非同期ランタイムが必要。`#[tokio::main]` マクロを使うと簡単にセットアップできる。
- `.await` キーワードは、`async` の世界の中で `Future` の完了を待ち、結果を取得するために使う。
- 複数のI/Oバウンドなタスクを高速化するには、`tokio::spawn` や `futures::future::join_all` を使ってタスクを並行実行する。

---

`async`/`await`