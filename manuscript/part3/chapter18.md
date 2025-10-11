# 第 18 章：内部可変性 (RefCell, Mutex)

## この章のゴール
- 「内部可変性」が、不変な参照 (`&T`) を通してデータを変更するデザインパターンであることを理解する。
- `RefCell<T>` を使って、単一スレッド内で実行時に借用規則をチェックし、内部可変性を実現できるようになる。
- `Mutex<T>` を使って、複数スレッド間で排他制御を行い、共有データを安全に変更できるようになる。
- `RefCell` と `Mutex` の使い分けを説明できる。

## 前章の復習
前の章では、`Box`, `Rc`, `Arc` といったスマートポインタを学びました。これらにより、ヒープへのデータ確保や、複数の所有者を持つといった、より柔軟な所有権のパターンを安全に扱えるようになりました。

## なぜこれが必要なのか？
Rust の借用規則は非常に厳格です。「一つの可変参照 (`&mut T`) か、複数の不変参照 (`&T`) のどちらか一方しか存在できない」というルールがコンパイル時に強制されることで、多くのバグを防いでいます。

しかし、時にはこのルールが厳しすぎることがあります。例えば、あるデータ構造に対して不変の参照しか持っていないけれど、その内部の一部だけをロギングやキャッシュのために変更したい、といったケースです。

このような場合に、「コンパイル時の借用チェックを回避し、実行時にチェックする」ことで、不変参照の裏でデータを可変にするパターン、それが**内部可変性 (Interior Mutability)** です。

## `RefCell<T>`: シングルスレッドでの内部可変性
`RefCell<T>` は、単一スレッド内で内部可変性を実現するためのスマートポインタです。`RefCell<T>` は、コンパイル時ではなく**実行時**に借用規則をチェックします。もし規則違反（例えば、既に可変借用されているのに、さらに可変借用しようとする）があれば、プログラムはパニックします。

`RefCell` は、不変な `&self` を受け取るメソッド内で、フィールドの値を変更したい場合などによく使われます。
```rust
use std::cell::RefCell;

pub trait Messenger {
    fn send(&self, msg: &str);
}

pub struct LimitTracker<'a, T: Messenger> {
    messenger: &'a T,
    value: usize,
    max: usize,
    // RefCell を使って、send メソッドが &self でも内部のベクタを変更できるようにする
    sent_messages: RefCell<Vec<String>>,
}

impl<'a, T> LimitTracker<'a, T>
    where T: Messenger,
{
    // ... new ...

    pub fn set_value(&mut self, value: usize) {
        self.value = value;
        // ...
        if percentage >= 1.0 {
            // send は &self を取るが、内部で sent_messages を変更したい
            self.messenger.send("Error: You are over your quota!");
        }
        // ...
    }
}

// テスト用のモックオブジェクト
struct MockMessenger {
    // RefCell を使って、send メソッドが &self でも内部のベクタを変更できるようにする
    sent_messages: RefCell<Vec<String>>,
}

impl MockMessenger {
    fn new() -> MockMessenger {
        MockMessenger { sent_messages: RefCell::new(vec![]) }
    }
}

impl Messenger for MockMessenger {
    fn send(&self, message: &str) {
        // .borrow_mut() で可変参照を "借りる"
        // 実行時に借用規則がチェックされる
        self.sent_messages.borrow_mut().push(String::from(message));
    }
}
```
`Rc<T>` と `RefCell<T>` を組み合わせる (`Rc<RefCell<T>>`) ことで、複数の所有者を持ち、かつ内部の値を変更できるオブジェクトを作ることができます。これは非常に強力なパターンです。

## `Mutex<T>`: マルチスレッドでの内部可変性
`Mutex<T>` (Mutual Exclusion) は、複数スレッド間で共有されたデータを安全に変更するための仕組みです。`Mutex` は、一度に一つのスレッドしかデータへのアクセス（ロック）を許可しません。他のスレッドがアクセスしようとした場合、ロックが解放されるまで待機（ブロック）させられます。

前章で見た `Arc<T>` と組み合わせる (`Arc<Mutex<T>>`) のが、スレッド間で状態を共有するための最も一般的な方法です。
```rust
use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    // Arc<Mutex<i32>> でカウンターをラップする
    let counter = Arc::new(Mutex::new(0));
    let mut handles = vec![];

    for _ in 0..10 {
        let counter = Arc::clone(&counter);
        let handle = thread::spawn(move || {
            // .lock() でロックを取得する
            // ロックが取得できるまでこのスレッドは待機する
            // .unwrap() はロック取得に失敗した場合（他のスレッドがパニックしたなど）にパニックする
            let mut num = counter.lock().unwrap();

            *num += 1;
        }); // MutexGuard (num) はここでスコープを抜けるので、ロックが自動的に解放される
        handles.push(handle);
    }

    for handle in handles {
        handle.join().unwrap();
    }

    println!("Result: {}", *counter.lock().unwrap()); // => 10
}
```
`.lock()` は `MutexGuard` というスマートポインタを返します。このガードがスコープを抜けるときに自動的にロックが解放されるため（RAII）、ロックの解放し忘れを防ぐことができます。

## `RefCell` vs `Mutex`
| 特徴 | `RefCell<T>` | `Mutex<T>` |
|---|---|---|
| **利用シーン** | シングルスレッド | マルチスレッド |
| **オーバーヘッド** | 低い（スレッド同期なし） | 高い（アトミック操作、スレッド待機） |
| **違反時の挙動** | 実行時にパニック | スレッドをブロック（待機）させる |
| **スレッドセーフ** | No (`Send` を実装しない) | Yes (`Send` を実装する) |

## よくあるエラーと対処法
### エラー 1: `already borrowed: BorrowMutError`
**原因:** `RefCell` に対して、既に借用（不変または可変）が存在する状態で、互換性のない新しい借用（例: 可変借用中にさらに可変借用）をしようとしました。
**解決法:** コードのロジックを見直し、`borrow()` や `borrow_mut()` で得られたガードのスコープが、必要以上に長くなっていないか確認してください。

## この章のまとめ
- 内部可変性は、不変な参照の裏でデータを変更するためのデザインパターン。
- `RefCell<T>` は、単一スレッドで、実行時借用チェックによる内部可変性を提供する。
- `Mutex<T>` は、マルチスレッドで、ロックによる排他制御を通じて内部可変性を提供する。
- `Arc<Mutex<T>>` は、スレッド間で共有状態を安全に変更するためのイディオム。

## 次の章へ
第3部では、Rust の抽象化機能について深く見てきました。これにより、再利用可能で柔軟なコードを書くための強力なツールを手に入れました。次の第4部では、視点を変えて、プロジェクトが大きくなるにつれて重要になる「プロジェクト管理」と「テスト」について学びます。まずは、コードを整理するためのモジュールシステムから始めましょう。
