# 第 24 章：実践的な非同期プログラミング

## この章のゴール
- `tokio::spawn` を使って、複数の非同期タスクを並行に実行できる。
- `tokio` が提供する非同期版の `Mutex` やチャネルを使いこなせる。
- `tokio::time` を使って、非同期コード内でスリープやタイムアウト処理を実装できる。
- `futures` クレートの `join!` や `select!` マクロの基本的な使い方を理解する。
- 非同期コードにおけるエラーハンドリングの基本を理解する。

## 前章の復習
前の章では `async`/`await` の基礎を学びました。`async fn` が `Future` を返し、`.await` でその完了を待つこと、そしてこれらを実行するためには `tokio` のような非同期ランタイムが必要であることを理解しました。

## なぜこれが必要なのか？
`async`/`await` の構文を学んだだけでは、Web サーバーのような実用的なアプリケーションを構築することはできません。実際の非同期アプリケーションでは、複数のタスクを同時に実行し（例: 複数のクライアントからのリクエストを同時に処理する）、タスク間で通信し、タイムアウトのような時間に関連する処理を扱う必要があります。この章では、`tokio` ランタイムが提供するツールを使って、これらの実践的な問題を解決する方法を学びます。

## 複数タスクの並行実行
OS スレッドを `thread::spawn` で生成したように、`tokio` には軽量なグリーンスレッド（タスク）を生成するための `tokio::spawn` があります。
```rust
use tokio::time::{sleep, Duration};

async fn task_one() {
    println!("Task one START");
    sleep(Duration::from_secs(2)).await;
    println!("Task one END");
}

async fn task_two() {
    println!("Task two START");
    sleep(Duration::from_secs(1)).await;
    println!("Task two END");
}

#[tokio::main]
async fn main() {
    // 2つのタスクを生成する
    // これらはバックグラウンドで並行に実行される
    let handle1 = tokio::spawn(task_one());
    let handle2 = tokio::spawn(task_two());

    // 両方のタスクが完了するのを待つ
    handle1.await.unwrap();
    handle2.await.unwrap();
}
```
このコードを実行すると、`task_one` の2秒間のスリープと `task_two` の1秒間のスリープが同時に進行します。そのため、プログラム全体は約2秒で終了します。もし `task_one().await` の後に `task_two().await` を実行すると、逐次実行となり、合計で3秒かかります。

## 非同期の同期プリミティブ
`std::sync::Mutex` は、ロックを待つ間にスレッドをブロックしてしまうため、非同期コード内では使えません（使うとランタイム全体が停止してしまう可能性があります）。`tokio` は、非同期コードで安全に使える代替品を提供しています。

### `tokio::sync::Mutex`
`.lock()` メソッドが `async` になっており、ロックを待っている間、現在のタスクを中断して他のタスクに実行を譲ります。
```rust
use tokio::sync::Mutex;
use std::sync::Arc;

let data = Arc::new(Mutex::new(0));

// ... 複数のタスクからこの data を共有して変更する ...
let mut lock = data.lock().await; // ロックを非同期に待機
*lock += 1;
```

### `tokio::sync::mpsc`
`std::sync::mpsc` の非同期版です。`send` と `recv` メソッドが `async` になっています。

## 時間の扱い
`std::thread::sleep` もスレッドをブロックするため、非同期コードでは使えません。代わりに `tokio::time` を使います。
- `tokio::time::sleep(duration)`: 指定された時間、現在のタスクをスリープさせます。
- `tokio::time::timeout(duration, future)`: `future` が指定時間内に完了しない場合、`Err(Elapsed)` を返します。

## `futures` クレートのユーティリティ
`futures` クレートは、`Future` を扱うための多くの便利なユーティリティを提供しており、`tokio` と共に広く使われています。
```toml
# Cargo.toml
[dependencies]
futures = "0.3"
```
### `join!`
複数のフューチャーを並行に実行し、すべてが完了するのを待ちます。
```rust
use futures::join;

async fn get_user() -> User { /* ... */ }
async fn get_permissions() -> Permissions { /* ... */ }

// get_user と get_permissions が並行に実行される
let (user, permissions) = join!(get_user(), get_permissions());
```

### `select!`
複数のフューチャーのうち、**最初に完了したもの**の結果を返します。レースコンディション（競合状態）の実装に便利です。

## エラーハンドリング
非同期関数も `Result` を返すことができます。`?` 演算子も通常通り使えます。`anyhow` や `thiserror` といったクレートは、非同期コードでも同様に役立ちます。

## この章のまとめ
- `tokio::spawn` を使うことで、OS スレッドよりもはるかに軽量なタスクを多数生成し、並行に実行できる。
- 非同期コード内では、スレッドをブロックしない `tokio::sync` の `Mutex` やチャネルを使う必要がある。
- `tokio::time` は、非同期なスリープやタイムアウト処理を提供する。
- `futures` クレートの `join!` は、複数の非同期処理を並行に実行するのに便利。

## 次の章へ
第5部では、Rust の並行・非同期プログラミングの能力を探求しました。これで、高パフォーマンスなネットワークサービスなどを構築するための理論的な基礎が整いました。次の第6部では、これまでに学んだ知識を総動員して、より実践的なプロジェクトの開発に取り組みます。まずは、多くの Rust プロジェクトが最初に作る定番アプリケーション、CLI ツールから始めましょう。
