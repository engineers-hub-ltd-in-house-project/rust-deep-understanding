# 第 22 章：スレッドを使った並行処理

## この章のゴール
- `thread::spawn` を使って新しいスレッドを生成し、コードを並行に実行できるようになる。
- メインスレッドの変数をスレッド内で使おうとした際に発生するコンパイルエラーを、`move` キーワードを使って解決できる。
- `JoinHandle` の `.join()` メソッドを使い、すべてのスレッドが終了するのを待機できる。
- `Arc<Mutex<T>>` を使い、複数のスレッドから共有データを安全に変更できるようになる。

---

## 22.1 なぜ並行処理が必要なのか？ 恐怖なき並行性

現代のCPUは複数のコアを持つのが当たり前です。並行処理は、時間がかかる処理を複数のコアに分担させることで、プログラムの実行時間を短縮し、パフォーマンスを向上させるための技術です。

しかし、伝統的に並行プログラミングは難しいとされてきました。「データ競合」や「デッドロック」といった、並行処理特有のバグに悩まされることが多かったからです。Rustは、所有権システムと型システムを駆使して、これらのバグの多くをコンパイル時に検出します。これにより、開発者は自信を持って並行コードを書くことができます。これが、Rustの掲げる「恐怖なき並行性 (Fearless Concurrency)」です。

### 試してみよう：逐次処理の遅さを体験する

```sh
cargo new threads
```
でプロジェクトを作り、まずは逐次処理がどれだけ時間がかかるかを体験してみましょう。

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Ause%20std%3A%3Athread%3B%0Ause%20std%3A%3Atime%3A%3ADuration%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20start%20%3D%20std%3A%3Atime%3A%3AInstant%3A%3Anow%28%29%3B%0A%0A%20%20%20%20//%202%E3%81%A4%E3%81%AE%E9%87%8D%E3%81%84%E5%87%A6%E7%90%86%E3%82%92%E9%A0%86%E7%95%AA%E3%81%AB%E5%AE%9F%E8%A1%8C%E3%81%99%E3%82%8B%0A%20%20%20%20heavy_task%28%22Task%201%22%29%3B%0A%20%20%20%20heavy_task%28%22Task%202%22%29%3B%0A%0A%20%20%20%20println%21%28%22Total%20elapsed%3A%20%7B%3A.2%3F%7D%22%2C%20start.elapsed%28%29%29%3B%0A%7D%0A%0Afn%20heavy_task%28name%3A%20%26str%29%20%7B%0A%20%20%20%20println%21%28%22%27%7B%7D%27%20started.%22%2C%20name%29%3B%0A%20%20%20%20thread%3A%3Asleep%28Duration%3A%3Afrom_secs%281%29%29%3B%0A%20%20%20%20println%21%28%22%27%7B%7D%27%20finished.%22%2C%20name%29%3B%0A%7D)
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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Ause%20std%3A%3Athread%3B%0Ause%20std%3A%3Atime%3A%3ADuration%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20start%20%3D%20std%3A%3Atime%3A%3AInstant%3A%3Anow%28%29%3B%0A%0A%20%20%20%20//%20%60move%60%20%E3%81%8C%E3%81%AA%E3%81%84%E3%81%A8%E3%82%B3%E3%83%B3%E3%83%91%E3%82%A4%E3%83%AB%E3%82%A8%E3%83%A9%E3%83%BC%E3%81%AB%E3%81%AA%E3%82%8B%0A%20%20%20%20let%20handle1%20%3D%20thread%3A%3Aspawn%28move%20%7C%7C%20%7B%0A%20%20%20%20%20%20%20%20heavy_task%28%22Task%201%22%29%3B%0A%20%20%20%20%7D%29%3B%0A%20%20%20%20let%20handle2%20%3D%20thread%3A%3Aspawn%28move%20%7C%7C%20%7B%0A%20%20%20%20%20%20%20%20heavy_task%28%22Task%202%22%29%3B%0A%20%20%20%20%7D%29%3B%0A%0A%20%20%20%20//%20%60.join%28%29%60%20%E3%81%8C%E3%81%AA%E3%81%84%E3%81%A8%E3%80%81%E3%83%A1%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%81%8C%E5%85%88%E3%81%AB%E7%B5%82%E4%BA%86%E3%81%97%E3%81%A6%E3%81%97%E3%81%BE%E3%81%86%0A%20%20%20%20handle1.join%28%29.unwrap%28%29%3B%0A%20%20%20%20handle2.join%28%29.unwrap%28%29%3B%0A%0A%20%20%20%20println%21%28%22Total%20elapsed%3A%20%7B%3A.2%3F%7D%22%2C%20start.elapsed%28%29%29%3B%0A%7D%0A%0Afn%20heavy_task%28name%3A%20%26str%29%20%7B%0A%20%20%20%20println%21%28%22%27%7B%7D%27%20started.%22%2C%20name%29%3B%0A%20%20%20%20thread%3A%3Asleep%28Duration%3A%3Afrom_secs%281%29%29%3B%0A%20%20%20%20println%21%28%22%27%7B%7D%27%20finished.%22%2C%20name%29%3B%0A%7D)

このコードでは、2つの重要な概念を体験できます。

1.  `move` クロージャ: `thread::spawn(move || ...)` の `move` は、クロージャが使用する外部の変数（この例では `"Task 1"` のようなリテラルには直接影響しませんが、`let task_name = String::from("Task 1");` のような変数を使った場合に必須）の所有権を、新しいスレッドに移動 (move) させます。これにより、メインスレッドが先に終了しても、スレッド内の変数が無効になることを防ぎます。`move` を外してコンパイルエラーを体験してみてください。
2.  `JoinHandle`: `spawn` は `JoinHandle` を返します。`handle.join().unwrap()` を呼び出すと、そのハンドルが対応するスレッドの処理が完了するまで、現在のスレッド（この場合は `main` スレッド）の実行をブロックします。`.join()` の呼び出しをコメントアウトして実行すると、合計経過時間がほぼゼロになり、タスクが完了する前にプログラムが終了してしまうことを確認できます。

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Async%3A%3A%7BArc%2C%20Mutex%7D%3B%0Ause%20std%3A%3Athread%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20//%20Arc%3CMutex%3C...%3E%3E%20%E3%81%A7%E3%82%AB%E3%82%A6%E3%83%B3%E3%82%BF%E3%83%BC%E3%82%92%E3%83%A9%E3%83%83%E3%83%97%E3%81%99%E3%82%8B%0A%20%20%20%20let%20counter%20%3D%20Arc%3A%3Anew%28Mutex%3A%3Anew%280%29%29%3B%0A%20%20%20%20let%20mut%20handles%20%3D%20vec%21%5B%5D%3B%0A%0A%20%20%20%20for%20_%20in%200..10%20%7B%0A%20%20%20%20%20%20%20%20//%20%E3%82%AB%E3%82%A6%E3%83%B3%E3%82%BF%E3%83%BC%E3%81%AE%E6%89%80%E6%9C%89%E6%A8%A9%E3%82%92%E5%90%84%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%81%A7%E5%85%B1%E6%9C%89%E3%81%99%E3%82%8B%E3%81%9F%E3%82%81%E3%81%AB%20Arc%20%E3%82%92%E3%82%AF%E3%83%AD%E3%83%BC%E3%83%B3%0A%20%20%20%20%20%20%20%20let%20counter_clone%20%3D%20Arc%3A%3Aclone%28%26counter%29%3B%0A%20%20%20%20%20%20%20%20%0A%20%20%20%20%20%20%20%20let%20handle%20%3D%20thread%3A%3Aspawn%28move%20%7C%7C%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20//%20.lock%28%29%20%E3%81%A7%E3%83%AD%E3%83%83%E3%82%AF%E3%82%92%E5%8F%96%E5%BE%97%E3%81%99%E3%82%8B%0A%20%20%20%20%20%20%20%20%20%20%20%20//%20%E4%BB%96%E3%81%AE%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%81%8C%E3%83%AD%E3%83%83%E3%82%AF%E3%82%92%E4%BF%9D%E6%8C%81%E3%81%97%E3%81%A6%E3%81%84%E3%82%8B%E5%A0%B4%E5%90%88%E3%80%81%E8%A7%A3%E6%94%BE%E3%81%95%E3%82%8C%E3%82%8B%E3%81%BE%E3%81%A7%E5%BE%85%E6%A9%9F%E3%81%99%E3%82%8B%0A%20%20%20%20%20%20%20%20%20%20%20%20let%20mut%20num%20%3D%20counter_clone.lock%28%29.unwrap%28%29%3B%0A%0A%20%20%20%20%20%20%20%20%20%20%20%20%2Anum%20%2B%3D%201%3B%0A%20%20%20%20%20%20%20%20%7D%29%3B%20//%20%60num%60%20%28MutexGuard%29%20%E3%81%8C%E3%82%B9%E3%82%B3%E3%83%BC%E3%83%97%E3%82%92%E6%8A%9C%E3%81%91%E3%82%8B%E3%81%A8%E3%81%8D%E3%81%AB%E3%83%AD%E3%83%83%E3%82%AF%E3%81%AF%E8%87%AA%E5%8B%95%E7%9A%84%E3%81%AB%E8%A7%A3%E6%94%BE%E3%81%95%E3%82%8C%E3%82%8B%0A%20%20%20%20%20%20%20%20handles.push%28handle%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20//%20%E3%81%99%E3%81%B9%E3%81%A6%E3%81%AE%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%81%8C%E7%B5%82%E4%BA%86%E3%81%99%E3%82%8B%E3%81%AE%E3%82%92%E5%BE%85%E3%81%A4%0A%20%20%20%20for%20handle%20in%20handles%20%7B%0A%20%20%20%20%20%20%20%20handle.join%28%29.unwrap%28%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20println%21%28%22Result%3A%20%7B%7D%22%2C%20%2Acounter.lock%28%29.unwrap%28%29%29%3B%20//%20%3D%3E%2010%0A%7D)

## 22.4 まとめ

- Rustは、所有権システムのおかげで、データ競合の多くをコンパイル時に防ぐことができる「恐怖なき並行性」を提供する。
- `thread::spawn` は新しいOSスレッドを生成する。
- `move` キーワードは、クロージャに外部変数の所有権を移動させ、ライフタイムの問題を防ぐ。
- `JoinHandle` の `.join()` メソッドは、スレッドの終了を待機するために使う。
- `Arc<Mutex<T>>` は、複数のスレッドからデータを安全に共有・変更するための定番パターン。

---

次の章では、OSスレッドよりも軽量な「非同期プログラミング」のモデルと、それを支える `async`/`await` 構文について学びます。
