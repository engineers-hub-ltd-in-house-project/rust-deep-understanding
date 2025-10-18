# 第 23 章：並行処理：スレッドとメッセージパッシング

## この章のゴール
- `thread::spawn` を使って新しいスレッドを生成し、メインスレッドと並行にコードを実行できる。
- メインスレッドの変数をスレッド内で使おうとした際に発生するコンパイルエラーを、`move` キーワードを使って解決できる。
- `JoinHandle` の `.join()` メソッドを使い、すべてのスレッドが終了するのを待機できる。
- `Arc<Mutex<T>>` を使い、複数のスレッドから共有データを安全に変更できるようになる。

---

## 23.1 並行処理 vs 並列処理

- 並行 (Concurrency): 複数のタスクが 見かけ上 同時に進行している状態。実際には、単一のCPUコアがタスクを高速に切り替えながら実行しているかもしれない。
- 並列 (Parallelism): 複数のタスクが 物理的に 同時に実行されている状態。マルチコアCPUの各コアが、それぞれ別のタスクを実行する。

Rustは、OSが提供するスレッド機能を利用して、並行処理と並列処理の両方を実現します。

### なぜ並行処理が必要か？

現代のコンピュータは複数のCPUコアを搭載しているのが当たり前です。例えば、大量の画像のリサイズや、巨大なデータの集計といった重い処理を行う際、処理を複数のコアに分担させることができれば、全体の処理時間を劇的に短縮できます。

しかし、複数のスレッドから同じデータにアクセスしようとすると、「データ競合」などの厄介なバグが発生しやすくなります。これこそが、多くのプログラミング言語で並行処理が難しいとされる所以です。

### Rustの強み：恐れるに足らない並行性

Rustは、所有権システムとコンパイラによる厳しいチェックのおかげで、これらの並行処理にまつわるバグの多くをコンパイル時に検出・防止できます。これにより、開発者は実行時エラーの恐怖に怯えることなく、自信を持って並行コードを書くことができます。これを「恐れるに足らない並行性 (Fearless Concurrency)」と呼びます。

## 23.2 スレッドの生成：`thread::spawn`

`std::thread::spawn` 関数は、新しいOSスレッドを生成し、その上でクロージャ（引数を取らない匿名関数）を実行します。

```sh
cargo new threads-example
```
でプロジェクトを作り、スレッドの基本的な動きを見てみましょう。

```rust
// src/main.rs
use std::thread;
use std::time::Duration;

fn main() {
    // 新しいスレッドを生成
    thread::spawn(|| {
        for i in 1..=5 {
            println!("Spawned thread: {}", i);
            thread::sleep(Duration::from_millis(1));
        }
    });

    // メインスレッドの処理
    for i in 1..=3 {
        println!("Main thread: {}", i);
        thread::sleep(Duration::from_millis(1));
    }
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Athread%3B%0Ause%20std%3A%3Atime%3A%3ADuration%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20%2F%2F%20%E6%96%B0%E3%81%97%E3%81%84%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%82%92%E7%94%9F%E6%88%90%0A%20%20%20%20thread%3A%3Aspawn%28%7C%7C%20%7B%0A%20%20%20%20%20%20%20%20for%20i%20in%201..%3D5%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20println!%28%22Spawned%20thread%3A%20%7B%7D%22%2C%20i%29%3B%0A%20%20%20%20%20%20%20%20%20%20%20%20thread%3A%3Asleep%28Duration%3A%3Afrom_millis%281%29%29%3B%0A%20%20%20%20%20%20%20%20%7D%0A%20%20%20%20%7D%29%3B%0A%0A%20%20%20%20%2F%2F%20%E3%83%A1%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%81%AE%E5%87%A6%E7%90%86%0A%20%20%20%20for%20i%20in%201..%3D3%20%7B%0A%20%20%20%20%20%20%20%20println!%28%22Main%20thread%3A%20%7B%7D%22%2C%20i%29%3B%0A%20%20%20%20%20%20%20%20thread%3A%3Asleep%28Duration%3A%3Afrom_millis%281%29%29%3B%0A%20%20%20%20%7D%0A%7D)

このコードを実行すると、多くの場合、生成されたスreadが5まで数え終わる前にプログラムが終了してしまいます。これは、メインスレッドが終了すると、すべての子スレッドも強制的に終了されるためです。

### `join` でスレッドの終了を待つ

生成したスレッドが処理を完了するのを保証するには、`spawn` が返す `JoinHandle` の `join()` メソッドを使います。

```rust
// src/main.rs
use std::thread;
use std::time::Duration;

fn main() {
    let handle = thread::spawn(|| {
        for i in 1..=5 {
            println!("Spawned thread: {}", i);
            thread::sleep(Duration::from_millis(1));
        }
    });

    // join() を呼び出すと、スレッドが終了するまでこの場所で待機する
    handle.join().unwrap();

    // メインスレッドの処理（待機後）
    for i in 1..=3 {
        println!("Main thread: {}", i);
        thread::sleep(Duration::from_millis(1));
    }
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Athread%3B%0Ause%20std%3A%3Atime%3A%3ADuration%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20handle%20%3D%20thread%3A%3Aspawn%28%7C%7C%20%7B%0A%20%20%20%20%20%20%20%20for%20i%20in%201..%3D5%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20println!%28%22Spawned%20thread%3A%20%7B%7D%22%2C%20i%29%3B%0A%20%20%20%20%20%20%20%20%20%20%20%20thread%3A%3Asleep%28Duration%3A%3Afrom_millis%281%29%29%3B%0A%20%20%20%20%20%20%20%20%7D%0A%20%20%20%20%7D%29%3B%0A%0A%20%20%20%20%2F%2F%20join%28%29%20%E3%82%92%E5%91%BC%E3%81%B3%E5%87%A6%E3%81%99%E3%81%A8%E3%80%81%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%81%8C%E7%B5%82%E4%BA%86%E3%81%99%E3%82%8B%E3%81%BE%E3%81%A7%E3%81%93%E3%81%AE%E5%A0%B4%E6%89%80%E3%81%A7%E5%BE%85%E6%A9%9F%E3%81%99%E3%82%8B%0A%20%20%20%20handle.join%28%29.unwrap%28%29%3B%0A%0A%20%20%20%20%2F%2F%20%E3%83%A1%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%81%AE%E5%87%A6%E7%90%86%EF%BC%88%E5%BE%85%E6%A9%9F%E5%BE%8C%EF%BC%89%0A%20%20%20%20for%20i%20in%201..%3D3%20%7B%0A%20%20%20%20%20%20%20%20println!%28%22Main%20thread%3A%20%7B%7D%22%2C%20i%29%3B%0A%20%20%20%20%20%20%20%20thread%3A%3Asleep%28Duration%3A%3Afrom_millis%281%29%29%3B%0A%20%20%20%20%7D%0A%7D)

### `move` クロージャで所有権を渡す

メインスレッドの変数を新しいスレッドで使いたい場合、所有権の問題が発生します。

```rust
// コンパイルエラーになる例
use std::thread;

fn main() {
    let v = vec![1, 2, 3];

    // `v` の所有権は main にあるが、スレッド内で参照しようとしている
    let handle = thread::spawn(|| {
        println!("Here's a vector: {:?}", v);
    });

    handle.join().unwrap();
}
```

このコードはコンパイルエラーになります。Rustコンパイラは、`v` がいつ解放されるか分からないため、スレッド内で安全に参照できるかを保証できないからです。

この問題を解決するのが `move` キーワードです。クロージャの前に `move` を付けると、クロージャが使用する変数の所有権が強制的にクロージャ内へムーブされます。

```rust
// `move` を使って修正
use std::thread;

fn main() {
    let v = vec![1, 2, 3];

    // `move` で `v` の所有権をクロージャに移動
    let handle = thread::spawn(move || {
        println!("Here's a vector: {:?}", v);
    });

    handle.join().unwrap();
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Athread%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20v%20%3D%20vec!%5B1%2C%202%2C%203%5D%3B%0A%0A%20%20%20%20%2F%2F%20%60move%60%20%E3%81%A7%20%60v%60%20%E3%81%AE%E6%89%80%E6%9C%89%E6%A8%A9%E3%82%92%E3%82%AF%E3%83%AD%E3%83%BC%E3%82%B8%E3%83%A3%E3%81%AB%E7%A7%BB%E5%8B%95%0A%20%20%20%20let%20handle%20%3D%20thread%3A%3Aspawn%28move%20%7C%7C%20%7B%0A%20%20%20%20%20%20%20%20println!%28%22Here's%20a%20vector%3A%20%7B%3A%3F%7D%22%2C%20v%29%3B%0A%20%20%20%20%7D%29%3B%0A%0A%20%20%20%20handle.join%28%29.unwrap%28%29%3B%0A%7D)

## 23.3 メッセージパッシングによるスレッド間通信

「共有メモリによる通信はするな、代わりに通信によってメモリを共有せよ」(Do not communicate by sharing memory; instead, share memory by communicating.) というのは、Go言語などの並行処理モデルで有名な考え方です。

Rustでは、チャネル (Channel) を使ってこのメッセージパッシングを実現します。チャネルは、送信側 (`Sender`) と受信側 (`Receiver`) のペアで構成されます。

### チャネルを使った並行計算

1から1,000,000までの数値を合計する処理を考えてみましょう。この処理を4つのスレッドに分割し、高速化することを試みます。各スレッドは計算結果をチャネル経由でメインスレッドに送信します。

```rust
use std::thread;
use std::sync::mpsc; // mpsc: multiple producer, single consumer

fn main() {
    let data = (1..=1_000_000).collect::<Vec<u64>>();
    // 送信側(tx)と受信側(rx)を作成
    let (tx, rx) = mpsc::channel();

    let chunk_size = data.len() / 4;
    let mut handles = vec![];

    for i in 0..4 {
        let start = i * chunk_size;
        let end = if i == 3 { data.len() } else { (i + 1) * chunk_size };
        
        // データを所有権ごとスライスして各スレッドに渡す
        let chunk = data[start..end].to_vec();
        let tx_clone = tx.clone(); // 送信側はクローンできる

        let handle = thread::spawn(move || {
            let sum = chunk.iter().sum::<u64>();
            // 計算結果を送信
            tx_clone.send(sum).unwrap();
        });
        handles.push(handle);
    }

    // すべてのスレッドが終了するのを待つ
    for handle in handles {
        handle.join().unwrap();
    }

    // 受信側は4つの結果を受け取るまで待機し、合計する
    let total_sum: u64 = rx.iter().take(4).sum();
    println!("Total sum: {}", total_sum);
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Athread%3B%0Ause%20std%3A%3Async%3A%3Ampsc%3B%20%2F%2F%20mpsc%3A%20multiple%20producer%2C%20single%20consumer%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20data%20%3D%20%281..%3D1_000_000%29.collect%3A%3AVec%3Cu64%3E%3E%28%29%3B%0A%20%20%20%20%2F%2F%20%E9%80%81%E4%BF%A1%E5%81%B4%28tx%29%E3%81%A8%E5%8F%97%E4%BF%A1%E5%81%B4%28rx%29%E3%82%92%E4%BD%9C%E6%88%90%0A%20%20%20%20let%20%28tx%2C%20rx%29%20%3D%20mpsc%3A%3Achannel%28%29%3B%0A%0A%20%20%20%20let%20chunk_size%20%3D%20data.len%28%29%20%2F%204%3B%0A%20%20%20%20let%20mut%20handles%20%3D%20vec!%5B%5D%3B%0A%0A%20%20%20%20for%20i%20in%200..4%20%7B%0A%20%20%20%20%20%20%20%20let%20start%20%3D%20i%20*%20chunk_size%3B%0A%20%20%20%20%20%20%20%20let%20end%20%3D%20if%20i%20%3D%3D%203%20%7B%20data.len%28%29%20%7D%20else%20%7B%20%28i%20%2B%201%29%20*%20chunk_size%20%7D%3B%0A%20%20%20%20%20%20%20%20%0A%20%20%20%20%20%20%20%20%2F%2F%20%E3%83%87%E3%83%BC%E3%82%BF%E3%82%92%E6%89%80%E6%9C%89%E6%A8%A9%E3%81%94%E3%81%A8%E3%82%B9%E3%83%A9%E3%82%A4%E3%82%B9%E3%81%97%E3%81%A6%E5%90%84%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%81%AB%E6%B8%A1%E3%81%99%0A%20%20%20%20%20%20%20%20let%20chunk%20%3D%20data%5Bstart..end%5D.to_vec%28%29%3B%0A%20%20%20%20%20%20%20%20let%20tx_clone%20%3D%20tx.clone%28%29%3B%20%2F%2F%20%E9%80%81%E4%E4%BF%A1%E5%81%B4%E3%81%AF%E3%82%AF%E3%83%AD%E3%83%BC%E3%83%B3%E3%81%A7%E3%81%8D%E3%82%8B%0A%0A%20%20%20%20%20%20%20%20let%20handle%20%3D%20thread%3A%3Aspawn%28move%20%7C%7C%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20let%20sum%20%3D%20chunk.iter%28%29.sum%3A%3Cu64%3E%28%29%3B%0A%20%20%20%20%20%20%20%20%20%20%20%20%2F%2F%20%E8%A8%88%E7%AE%97%E7%B5%90%E6%9E%9C%E3%82%92%E9%80%81%E4%BE%A1%0A%20%20%20%20%20%20%20%20%20%20%20%20tx_clone.send%28sum%29.unwrap%28%29%3B%0A%20%20%20%20%20%20%20%20%7D%29%3B%0A%20%20%20%20%20%20%20%20handles.push%28handle%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20%2F%2F%20%E3%81%99%E3%81%B9%E3%81%A6%E3%81%AE%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%81%8C%E7%B5%82%E4%86%A9%E3%81%99%E3%82%8B%E3%81%AE%E3%82%92%E5%BE%85%E3%81%A4%0A%20%20%20%20for%20handle%20in%20handles%20%7B%0A%20%20%20%20%20%20%20%20handle.join%28%29.unwrap%28%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20%2F%2F%20%E5%8F%97%E4%BF%A1%E5%81%B4%E3%81%AF4%E3%81%A4%E3%81%AE%E7%B5%90%E6%9E%9C%E3%82%92%E5%8F%97%E3%81%91%E5%8F%96%E3%82%8B%E3%81%BE%E3%81%A7%E5%BE%85%E6%A9%9F%E3%81%97%E3%80%81%E5%90%88%E8%A8%88%E3%81%99%E3%82%8B%0A%20%20%20%20let%20total_sum%3A%20u64%20%3D%20rx.iter%28%29.take%284%29.sum%28%29%3B%0A%20%20%20%20println!%28%22Total%20sum%3A%20%7B%7D%22%2C%20total_sum%29%3B%0A%7D)

`mpsc` は "multiple producer, single consumer" (複数の送信者、単一の受信者) の略です。`tx` (送信側) は `clone()` することで複数のスレッドに渡せますが、`rx` (受信側) は一つしか存在できません。これにより、複数のスレッドからの結果を一つの場所で安全に集約できます。

## 23.4 `Mutex<T>` による共有メモリ

伝統的な並行処理モデルは、ミューテックス (Mutex: Mutual Exclusion) を使った共有メモリです。`Mutex<T>` は、一度に一つのスレッドしかデータにアクセスできないようにロックをかける仕組みを提供します。

Rustの `Mutex<T>` は、ロックの取得 (`lock()`) に失敗するとスレッドをブロックし、ロックが解放されるのを待ちます。また、`lock()` が返す `MutexGuard` というスマートポインタがスコープを抜けるときに自動的にロックを解放するため、ロックのかけ忘れによるデッドロックのリスクを低減します。

### `Arc<T>`：スレッドセーフな参照カウント

`Mutex<T>` を複数のスレッドで共有しようとすると、所有権の問題が再び発生します。`Rc<T>` はスレッドセーフではないため、コンパイルエラーになります。

`Arc<T>` (Atomic Reference Counting) は、`Rc<T>` のスレッドセーフ版です。参照カウントの増減をアトミックに行うため、複数のスレッドから安全に所有権を共有できます。

### `Arc<Mutex<T>>` を使った並行カウンター

`Arc<T>` と `Mutex<T>` の組み合わせは、Rustの並行処理における非常に一般的なパターンです。複数のスレッドから、あるデータを安全に共有し、かつ変更したい場合に利用します。

10個のスレッドを生成し、各スレッドが共有カウンターを1回ずつインクリメントする例を見てみましょう。

```rust
use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    // Arc<Mutex<i32>> でカウンターをラップする
    let counter = Arc::new(Mutex::new(0));
    let mut handles = vec![];

    for _ in 0..10 {
        // カウンターへの参照をクローンして各スレッドに渡す
        let counter = Arc::clone(&counter);
        let handle = thread::spawn(move || {
            // lock() を呼び出してMutexのロックを取得する
            // ロックが取得できるまでこのスレッドはブロックされる
            let mut num = counter.lock().unwrap();

            *num += 1;
            // `num` (MutexGuard) がスコープを抜けるときに自動的にロックが解放される
        });
        handles.push(handle);
    }

    // すべてのスレッドの終了を待つ
    for handle in handles {
        handle.join().unwrap();
    }

    // 最終的なカウンターの値を表示
    println!("Result: {}", *counter.lock().unwrap());
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Async%3A%3A%7BArc%2C%20Mutex%7D%3B%0Ause%20std%3A%3Athread%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20%2F%2F%20Arc%3CMutex%3Ci32%3E%3E%20%E3%81%A7%E3%82%AB%E3%82%A6%E3%83%B3%E3%82%BF%E3%83%BC%E3%82%92%E3%83%A9%E3%83%83%E3%83%97%E3%81%99%E3%82%8B%0A%20%20%20%20let%20counter%20%3D%20Arc%3A%3Anew%28Mutex%3A%3Anew%280%29%29%3B%0A%20%20%20%20let%20mut%20handles%20%3D%20vec!%5B%5D%3B%0A%0A%20%20%20%20for%20_%20in%200..10%20%7B%0A%20%20%20%20%20%20%20%20%2F%2F%20%E3%82%AB%E3%82%A6%E3%83%B3%E3%82%BF%E3%83%BC%E3%81%B8%E3%81%AE%E5%8F%82%E7%85%A7%E3%82%92%E3%82%AF%E3%83%AD%E3%83%BC%E3%83%B3%E3%81%97%E3%81%A6%E5%90%84%E3%82%B8%E3%83%A3%E3%83%B3%E3%83%AB%E3%81%AB%E6%B8%A1%E3%81%99%0A%20%20%20%20%20%20%20%20let%20counter%20%3D%20Arc%3A%3Aclone%28%26counter%29%3B%0A%20%20%20%20%20%20%20%20let%20handle%20%3D%20thread%3A%3Aspawn%28move%20%7C%7C%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20%2F%2F%20lock%28%29%20%E3%82%92%E5%91%BC%E3%81%B3%E5%87%A6%E3%81%97%E3%81%A6Mutex%E3%81%AE%E3%83%AD%E3%83%83%E3%82%AF%E3%82%92%E5%8F%96%E5%BE%97%E3%81%99%E3%82%8B%0A%20%20%20%20%20%20%20%20%20%20%20%20%2F%2F%20%E3%83%AD%E3%83%83%E3%82%AF%E3%81%8C%E5%8F%96%E5%BE%97%E3%81%A7%E3%81%8D%E3%82%8B%E3%81%BE%E3%81%A7%E3%81%93%E3%81%AE%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%81%AF%E3%83%96%E3%83%AD%E3%83%83%E3%82%AF%E3%81%95%E3%82%8C%E3%82%8B%0A%20%20%20%20%20%20%20%20%20%20%20%20let%20mut%20num%20%3D%20counter.lock%28%29.unwrap%28%29%3B%0A%0A%20%20%20%20%20%20%20%20%20%20%20%20*num%20%2B%3D%201%3B%0A%20%20%20%20%20%20%20%20%20%20%20%20%2F%2F%20%60num%60%20%28MutexGuard%29%20%E3%81%8C%E3%82%B9%E3%82%B3%E3%83%BC%E3%83%97%E3%82%92%E6%8A%9C%E3%81%91%E3%82%8B%E3%81%A8%E3%81%8D%E3%81%AB%E8%87%AA%E5%8B%95%E7%9A%84%E3%81%AB%E3%83%AD%E3%83%83%E3%82%AF%E3%81%8C%E8%A7%A3%E6%94%BE%E3%81%95%E3%82%8C%E3%82%8B%0A%20%20%20%20%20%20%20%20%7D%29%3B%0A%20%20%20%20%20%20%20%20handles.push%28handle%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20%2F%2F%20%E3%81%99%E3%81%B9%E3%81%A6%E3%81%AE%E3%82%B9%E3%83%AC%E3%83%83%E3%83%89%E3%81%AE%E7%B5%82%E4%BA%86%E3%82%92%E5%BE%85%E3%81%A4%0A%20%20%20%20for%20handle%20in%20handles%20%7B%0A%20%20%20%20%20%20%20%20handle.join%28%29.unwrap%28%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20%2F%2F%20%E6%9C%80%E7%B5%82%E7%9A%84%E3%81%AA%E3%82%AB%E3%82%A6%E3%83%B3%E3%82%BF%E3%83%BC%E3%81%AE%E5%80%A4%E3%82%92%E8%A1%A8%E7%A4%BA%0A%20%20%20%20println!%28%22Result%3A%20%7B%7D%22%2C%20*counter.lock%28%29.unwrap%28%29%29%3B%0A%7D)

## 23.5 まとめ

- Rust は `thread::spawn` を通じて OS ネイティブのスレッドを生成し、並行・並列処理を実現する。
- `spawn` が返す `JoinHandle` の `join()` メソッドを呼び出すことで、スレッドが終了するのを待機できる。
- `move` キーワードをクロージャに付けることで、変数の所有権をスレッド内に移動させることができる。
- スレッド間の通信には、主に2つのアプローチがある。
    1.  **メッセージパッシング**: `mpsc::channel` を作成し、送信側(`Sender`)から受信側(`Receiver`)へデータを送る方法。送信側はクローン可能で、複数のスレッドから一つの受信側にデータを集約するのに適している。
    2.  **状態共有**: `Arc<Mutex<T>>` を使い、複数のスレッドから同じデータを安全に読み書きする方法。`Mutex`がデータへのアクセスを一度に一つに制限し、`Arc`がスレッド間で安全な参照の共有を可能にする。
- Rustの所有権システムとコンパイラのチェックにより、これらの並行処理パターンを安全に利用でき、データ競合のようなバグをコンパイル時に防ぐことができる（恐れるに足らない並行性）。

---

これで、Rust がいかにして「恐怖なき並行性 (Fearless Concurrency)」を実現しているかの基本を学びました。次の章では、より現代的で高レベルな非同期処理の仕組みである `async`/`await` について探求します。
