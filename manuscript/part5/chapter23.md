# 第 23 章：async/await による非同期プログラミングの基礎

## この章のゴール
- `async fn` が即時実行されるコードではなく、「未来の計算を表すレシピ (`Future`)」を返すことを体験する。
- `Future` を実行するために `tokio` のような非同期ランタイムが必要な理由を理解し、`#[tokio::main]` を使えるようになる。
- `.await` キーワードが `Future` の完了を待ち、処理を進行させる役割であることを体験する。
- `tokio::spawn` を使って、軽量な非同期タスクを複数並行実行できるようになる。

---

## 23.1 なぜこれが必要か？ OSスレッドの限界

前の章では、OSスレッドを使って処理を並行化し、実行時間を短縮しました。しかし、スレッドには限界があります。

1.  **生成コスト**: OSスレッドの生成と切り替え（コンテキストスイッチ）には、CPU時間とメモリの両面で無視できないコストがかかります。
2.  **数の限界**: OSが同時に扱えるスレッドの数には上限があります。

例えば、数万のクライアントからの同時接続を処理するWebサーバーを考えてみましょう。クライアントごとに1つのOSスレッドを割り当てていては、あっという間にリソースを使い果たしてしまいます。

この問題を解決するのが非同期プログラミングです。非同期モデルでは、あるタスクがI/O（ネットワークからの応答待ちなど）でブロックされている間、CPUを他のタスクの実行に充てることができます。これにより、ごく少数のOSスレッドで、非常に多くのタスクを効率的にさばくことが可能になります。

## 23.2 `Future`: まだ実行されていない「レシピ」

Rustの非同期プログラミングの核心は `Future` トレイトです。`async fn` で定義された関数は、呼び出されても即座にコードを実行しません。代わりに、未来のある時点で完了する計算の「レシピ」、つまり `Future` を返します。

### 試してみよう：`async` 関数を呼び出しても何も起こらないことを体験する

`cargo new async-basics` でプロジェクトを作り、`Cargo.toml` に `tokio` を追加します。

```toml
# Cargo.toml
[dependencies]
tokio = { version = "1", features = ["full"] }
```

そして、`async` 関数を定義して、普通の関数と同じように呼び出してみましょう。

```rust
// src/main.rs
async fn say_hello() {
    println!("Hello, async world!");
}

fn main() {
    println!("Let's start!");
    say_hello(); // これを呼び出しても、"Hello, async world!" は表示されない！
    println!("Finished.");
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Aasync%20fn%20say_hello%28%29%20%7B%0A%20%20%20%20println%21%28%22Hello%2C%20async%20world%21%22%29%3B%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20println%21%28%22Let%27s%20start%21%22%29%3B%0A%20%20%20%20say_hello%28%29%3B%20//%20%E3%81%93%E3%82%8C%E3%82%92%E5%91%BC%E3%81%B3%E5%87%BA%E3%81%97%E3%81%A6%E3%82%82%E3%80%81%22Hello%2C%20async%20world%21%22%20%E3%81%AF%E8%A1%A8%E7%A4%BA%E3%81%95%E3%82%8C%E3%81%AA%E3%81%84%EF%BC%81%0A%20%20%20%20println%21%28%22Finished.%22%29%3B%0A%7D)

実行しても、コンソールには `Let's start!` と `Finished.` しか表示されません。`say_hello` が返した `Future` という「レシピ」は、誰にも実行されることなく捨てられてしまったのです。

## 23.3 非同期ランタイム `tokio`: レシピの「料理人」

`Future` を実行するには、それを管理し、進捗させるための「料理人」が必要です。これが非同期ランタイムです。Rustコミュニティで最も広く使われているのが `tokio` です。

`tokio` は、`main` 関数を簡単に非同期化するための `#[tokio::main]` マクロを提供します。

### 試してみよう：`.await` でレシピを実行する

先ほどのコードに `#[tokio::main]` を付け、`say_hello()` の呼び出しに `.await` を付けてみましょう。

```rust
// src/main.rs
async fn say_hello() {
    println!("Hello, async world!");
}

#[tokio::main]
async fn main() { // main 関数も async fn になっていることに注意
    println!("Let's start!");
    say_hello().await; // .await で Future が完了するのを待つ
    println!("Finished.");
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Aasync%20fn%20say_hello%28%29%20%7B%0A%20%20%20%20println%21%28%22Hello%2C%20async%20world%21%22%29%3B%0A%7D%0A%0A%23%5Btokio%3A%3Amain%5D%0Aasync%20fn%20main%28%29%20%7B%20//%20main%20%E9%96%A2%E6%95%B0%E3%82%82%20async%20fn%20%E3%81%AB%E3%81%AA%E3%81%A3%E3%81%A6%E3%81%84%E3%82%8B%E3%81%93%E3%81%A8%E3%81%AB%E6%B3%A8%E6%84%8F%0A%20%20%20%20println%21%28%22Let%27s%20start%21%22%29%3B%0A%20%20%20%20say_hello%28%29.await%3B%20//%20.await%20%E3%81%A7%20Future%20%E3%81%8C%E5%AE%8C%E4%BA%86%E3%81%99%E3%82%8B%E3%81%AE%E3%82%92%E5%BE%85%E3%81%A4%0A%20%20%20%20println%21%28%22Finished.%22%29%3B%0A%7D)
これを実行すると、今度は "Hello, async world!" が表示されます。

- `#[tokio::main]`: `main` 関数の上で `tokio` ランタイムを起動し、`main` 関数自体を `Future` として実行する定型コードを裏側で生成します。
- `.await`: `say_hello()` が返す `Future` をランタイムに渡し、「このレシピが完成するまで待ってください」とお願いします。待っている間、CPUは他のタスクを実行できます（この例では他にタスクはありませんが）。

## 23.4 `tokio::spawn` でタスクを並行実行する

`async`/`await` の真価は、複数のタスクを並行実行するときに発揮されます。`tokio::spawn` は `thread::spawn` に似ていますが、OSスレッドではなく、はるかに軽量な非同期タスクを生成します。

### 試してみよう：複数の非同期タスクを同時に動かす

第22章で使った `heavy_task` を `async` 版にしてみましょう。

```rust
// src/main.rs
use tokio::time::{sleep, Duration};

async fn heavy_task(name: &str) {
    println!("'{}' started.", name);
    // std::thread::sleep ではなく、tokio::time::sleep を使う
    sleep(Duration::from_secs(1)).await;
    println!("'{}' finished.", name);
}

#[tokio::main]
async fn main() {
    let start = std::time::Instant::now();

    // 2つのタスクを生成するが、まだ実行はされない
    let task1 = heavy_task("Task 1");
    let task2 = heavy_task("Task 2");

    // join! マクロで複数の Future を同時に待つ
    tokio::join!(task1, task2);

    println!("Total elapsed: {:.2?}", start.elapsed());
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Ause%20tokio%3A%3Atime%3A%3A%7Bsleep%2C%20Duration%7D%3B%0A%0Aasync%20fn%20heavy_task%28name%3A%20%26str%29%20%7B%0A%20%20%20%20println%21%28%22%27%7B%7D%27%20started.%22%2C%20name%29%3B%0A%20%20%20%20//%20std%3A%3Athread%3A%3Asleep%20%E3%81%A7%E3%81%AF%E3%81%AA%E3%81%8F%E3%80%81tokio%3A%3Atime%3A%3Asleep%20%E3%82%92%E4%BD%BF%E3%81%86%0A%20%20%20%20sleep%28Duration%3A%3Afrom_secs%281%29%29.await%3B%0A%20%20%20%20println%21%28%22%27%7B%7D%27%20finished.%22%2C%20name%29%3B%0A%7D%0A%0A%23%5Btokio%3A%3Amain%5D%0Aasync%20fn%20main%28%29%20%7B%0A%20%20%20%20let%20start%20%3D%20std%3A%3Atime%3A%3AInstant%3A%3Anow%28%29%3B%0A%0A%20%20%20%20//%202%E3%81%A4%E3%81%AE%E3%82%BF%E3%82%B9%E3%82%AF%E3%82%92%E7%94%9F%E6%88%90%E3%81%99%E3%82%8B%E3%81%8C%E3%80%81%E3%81%BE%E3%81%A0%E5%AE%9F%E8%A1%8C%E3%81%AF%E3%81%95%E3%82%8C%E3%81%AA%E3%81%84%0A%20%20%20%20let%20task1%20%3D%20heavy_task%28%22Task%201%22%29%3B%0A%20%20%20%20let%20task2%20%3D%20heavy_task%28%22Task%202%22%29%3B%0A%0A%20%20%20%20//%20join%21%20%E3%83%9E%E3%82%AF%E3%83%AD%E3%81%A7%E8%A4%87%E6%95%B0%E3%81%AE%20Future%20%E3%82%92%E5%90%8C%E6%99%82%E3%81%AB%E5%BE%85%E3%81%A4%0A%20%20%20%20tokio%3A%3Ajoin%21%28task1%2C%20task2%29%3B%0A%0A%20%20%20%20println%21%28%22Total%20elapsed%3A%20%7B%3A.2%3F%7D%22%2C%20start.elapsed%28%29%29%3B%0A%7D)

これを実行すると、2つのタスクが並行して実行され、合計時間は約1秒になります。`tokio::join!` は、渡された複数の `Future` を同時に実行し、すべてが完了するまで待機します。これは、`tokio`ランタイムが内部でタスクを切り替えながら、効率的に処理を進めているからです。

## 23.5 まとめ

- OSスレッドは高コストなため、大量のI/Oバウンドなタスクには非同期プログラミングが適している。
- `async fn` は、コードを直接実行せず、未来の計算を表す「レシピ（`Future`）」を返す。
- `Future` を実行するには `tokio` のような非同期ランタイム（料理人）が必要。`#[tokio::main]` で簡単に導入できる。
- `.await` は、`Future` が完了するのを非同期的に（CPUをブロックせずに）待機する。
- `tokio::spawn` や `tokio::join!` を使うと、複数の非同期タスクを非常に効率的に並行実行できる。

---

次の章では、`