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

この問題を解決するのが**非同期プログラミング**です。非同期モデルでは、あるタスクがI/O（ネットワークからの応答待ちなど）でブロックされている間、CPUを他のタスクの実行に充てることができます。これにより、ごく少数のOSスレッドで、非常に多くのタスクを効率的にさばくことが可能になります。

## 23.2 `Future`: まだ実行されていない「レシピ」

Rustの非同期プログラミングの核心は `Future` トレイトです。`async fn` で定義された関数は、呼び出されても即座にコードを実行しません。代わりに、**未来のある時点で完了する計算の「レシピ」**、つまり `Future` を返します。

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

実行しても、コンソールには `Let's start!` と `Finished.` しか表示されません。`say_hello` が返した `Future` という「レシピ」は、誰にも実行されることなく捨てられてしまったのです。

## 23.3 非同期ランタイム `tokio`: レシピの「料理人」

`Future` を実行するには、それを管理し、進捗させるための「料理人」が必要です。これが**非同期ランタイム**です。Rustコミュニティで最も広く使われているのが `tokio` です。

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

これを実行すると、2つのタスクが並行して実行され、合計時間は約1秒になります。`tokio::join!` は、渡された複数の `Future` を同時に実行し、すべてが完了するまで待機します。これは、`tokio`ランタイムが内部でタスクを切り替えながら、効率的に処理を進めているからです。

## 23.5 まとめ

- OSスレッドは高コストなため、大量のI/Oバウンドなタスクには非同期プログラミングが適している。
- `async fn` は、コードを直接実行せず、未来の計算を表す「レシピ（`Future`）」を返す。
- `Future` を実行するには `tokio` のような非同期ランタイム（料理人）が必要。`#[tokio::main]` で簡単に導入できる。
- `.await` は、`Future` が完了するのを非同期的に（CPUをブロックせずに）待機する。
- `tokio::spawn` や `tokio::join!` を使うと、複数の非同期タスクを非常に効率的に並行実行できる。

---

次の章では、`async`/`await` を使って、より実践的な非同期処理、例えば非同期ストリームや、タスク間の通信などを扱います。
