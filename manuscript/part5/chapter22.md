# 第 22 章：スレッドを使った並行処理

## この章のゴール
- `thread::spawn` を使って新しいスレッドを生成し、コードを並行に実行できるようになる。
- メインスレッドの変数をスレッド内で使おうとした際に発生するコンパイルエラーを、`move` キーワードを使って解決できる。
- `JoinHandle` の `.join()` メソッドを使い、すべてのスレッドが終了するのを待機できる。
- `Arc<Mutex<T>>` を使い、複数のスレッドから共有データを安全に変更できるようになる。

---

## 22.1 なぜ並行処理が必要なのか？ 恐怖なき並行性

現代のCPUは複数のコアを持つのが当たり前です。並行処理は、時間がかかる処理を複数のコアに分担させることで、プログラムの実行時間を短縮し、パフォーマンスを向上させるための技術です。

しかし、伝統的に並行プログラミングは難しいとされてきました。「データ競合」や「デッドロック」といった、並行処理特有のバグに悩まされることが多かったからです。Rustは、所有権システムと型システムを駆使して、これらのバグの多くを**コンパイル時に**検出します。これにより、開発者は自信を持って並行コードを書くことができます。これが、Rustの掲げる**「恐怖なき並行性 (Fearless Concurrency)」**です。

### 試してみよう：逐次処理の遅さを体験する

`cargo new threads` でプロジェクトを作り、まずは逐次処理がどれだけ時間がかかるかを体験してみましょう。

```rust
// src/main.rs
use std::thread;
use std::time::Duration;

fn main() {
    let start = std::time::Instant::now();

    // 2つの重い処理を順番に実行する
    heavy_task("Task 1");
    heavy_task("Task 2");

    println!("Total elapsed: {:.2?}", start.elapsed());
}

fn heavy_task(name: &str) {
    println!("'{}' started.", name);
    thread::sleep(Duration::from_secs(1));
    println!("'{}' finished.", name);
}
```
実行すると、Task 1が1秒、Task 2が1秒で、合計約2秒かかることがわかります。これは非効率です。

## 22.2 `thread::spawn` で処理を並行化する

`std::thread::spawn` 関数にクロージャを渡すことで、新しいOSスレッドを生成し、処理を任せることができます。

### 試してみよう：`move` と `join` を体験する

先ほどのコードを並行化してみましょう。しかし、ここにはいくつかの「罠」があります。それをコンパイラのエラーと実行結果から学びましょう。

```rust
// src/main.rs
use std::thread;
use std::time::Duration;

fn main() {
    let start = std::time::Instant::now();

    // `move` がないとコンパイルエラーになる
    let handle1 = thread::spawn(move || {
        heavy_task("Task 1");
    });
    let handle2 = thread::spawn(move || {
        heavy_task("Task 2");
    });

    // `.join()` がないと、メインスレッドが先に終了してしまう
    handle1.join().unwrap();
    handle2.join().unwrap();

    println!("Total elapsed: {:.2?}", start.elapsed());
}

fn heavy_task(name: &str) {
    println!("'{}' started.", name);
    thread::sleep(Duration::from_secs(1));
    println!("'{}' finished.", name);
}
```

このコードでは、2つの重要な概念を体験できます。

1.  **`move` クロージャ**: `thread::spawn(move || ...)` の `move` は、クロージャが使用する外部の変数（この例では `"Task 1"` のようなリテラルには直接影響しませんが、`let task_name = String::from("Task 1");` のような変数を使った場合に必須）の所有権を、新しいスレッドに**移動 (move)** させます。これにより、メインスレッドが先に終了しても、スレッド内の変数が無効になることを防ぎます。`move` を外してコンパイルエラーを体験してみてください。
2.  **`JoinHandle`**: `spawn` は `JoinHandle` を返します。`handle.join().unwrap()` を呼び出すと、そのハンドルが対応するスレッドの処理が完了するまで、現在のスレッド（この場合は `main` スレッド）の実行をブロックします。`.join()` の呼び出しをコメントアウトして実行すると、合計経過時間がほぼゼロになり、タスクが完了する前にプログラムが終了してしまうことを確認できます。

正しく実行すると、2つのタスクが並行して実行されるため、合計実行時間は約1秒になります。

## 22.3 `Arc<Mutex<T>>` で状態を共有する

複数のスレッドから同じデータを安全に読み書きするにはどうすればよいでしょうか？ここで、第18章で少し触れた `Mutex<T>` (Mutual Exclusion) と、新しいスマートポインタ `Arc<T>` (Atomically Reference Counted) が登場します。

- **`Mutex<T>`**: データへのアクセスを一度に一つのスレッドに限定する「ロック」を提供します。
- **`Arc<T>`**: `Rc<T>` のスレッドセーフ版です。アトミックな参照カウントを使い、複数のスレッドが安全に同じデータへの所有権を共有できるようにします。

この2つを組み合わせた `Arc<Mutex<T>>` は、Rustでスレッド間の状態共有を行うための最も一般的なイディオムです。

### 試してみよう：スレッドセーフなカウンター

複数のスレッドからカウンターをインクリメントしてみましょう。

```rust
use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    // Arc<Mutex<...>> でカウンターをラップする
    let counter = Arc::new(Mutex::new(0));
    let mut handles = vec![];

    for _ in 0..10 {
        // カウンターの所有権を各スレッドで共有するために Arc をクローン
        let counter_clone = Arc::clone(&counter);
        
        let handle = thread::spawn(move || {
            // .lock() でロックを取得する
            // 他のスレッドがロックを保持している場合、解放されるまで待機する
            let mut num = counter_clone.lock().unwrap();

            *num += 1;
        }); // `num` (MutexGuard) がスコープを抜けるときにロックは自動的に解放される
        handles.push(handle);
    }

    // すべてのスレッドが終了するのを待つ
    for handle in handles {
        handle.join().unwrap();
    }

    println!("Result: {}", *counter.lock().unwrap()); // => 10
}
```

## 22.4 まとめ

- Rustは、所有権システムのおかげで、データ競合の多くをコンパイル時に防ぐことができる「恐怖なき並行性」を提供する。
- `thread::spawn` は新しいOSスレッドを生成する。
- `move` キーワードは、クロージャに外部変数の所有権を移動させ、ライフタイムの問題を防ぐ。
- `JoinHandle` の `.join()` メソッドは、スレッドの終了を待機するために使う。
- `Arc<Mutex<T>>` は、複数のスレッドからデータを安全に共有・変更するための定番パターン。

---

次の章では、OSスレッドよりも軽量な「非同期プログラミング」のモデルと、それを支える `async`/`await` 構文について学びます。
