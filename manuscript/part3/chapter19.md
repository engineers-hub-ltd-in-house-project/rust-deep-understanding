# 第 19 章：共有所有権 `Rc<T>` と内部可変性 `RefCell<T>`

## この章のゴール
- `Box<T>` が持つ「単一所有権」の制約と、`Rc<T>` が解決する「複数所有権」の必要性を説明できる。
- `Rc::clone` と `Rc::strong_count` を使い、参照カウントがどのように機能するかを追体験する。
- 共有されたデータを変更しようとするとコンパイルエラーになることを体験し、「内部可変性」の必要性を説明できる。
- `RefCell<T>` の `.borrow_mut()` を使って共有データを変更し、意図的に借用ルールを破って実行時パニックを引き起こせる。
- `RefCell<T>` を使って、不変参照 (`&T`) を通して値を変更する方法と、その際の実行時借用チェックの仕組みを説明できる。
- `Rc<RefCell<T>>` の組み合わせが、複数の所有者からデータを可変に借用するための一般的なパターンであることを理解する。

---

## 19.1 問題設定：データを複数の所有者で共有したい

前の章で学んだ `Box<T>` は、ヒープ上のデータに対して 単一の所有権 を持ちます。しかし、プログラムによっては、あるデータを複数の所有者で共有したい場合があります。

例えば、`Cons` リストで、2つの異なるリストが同じ後続リストを共有するケースを考えてみましょう。

```sh
cargo new rc_refcell
```
でプロジェクトを作り、試してみましょう。

```rust
// src/main.rs
enum List {
    Cons(i32, Box<List>),
    Nil,
}

use List::{Cons, Nil};

fn main() {
    let a = Cons(5, Box::new(Cons(10, Box::new(Nil)))));
    // `a` の一部を共有したい
    let b = Cons(3, Box::new(a)); // `a` は `b` にムーブされる
    let c = Cons(4, Box::new(a)); // エラー！ `a` は既にムーブされている
}
```
`Box::new(a)` を呼び出した時点で、`a` の所有権は `b` にムーブされてしまいます。そのため、`c` を作成しようとすると、すでにムーブされた後の値を使おうとしたとしてコンパイルエラーになります。

## 19.2 解決策：`Rc<T>` で参照をカウントする

この問題を解決するのが、`Rc<T>` (Reference Counting) です。`Rc<T>` は、ある値への参照の数を追跡し、参照が一つもなくなった時に初めてその値を解放します。

### 試してみよう：参照カウントを可視化する

`Rc::clone()` は、データのディープコピーではなく、ポインタをコピーして参照カウントを1増やすだけの軽量な操作です。その様子を `Rc::strong_count` で観察してみましょう。

```rust
use std::rc::Rc;

enum List {
    Cons(i32, Rc<List>),
    Nil,
}

use List::{Cons, Nil};

fn main() {
    let a = Rc::new(Cons(5, Rc::new(Cons(10, Rc::new(Nil)))));
    println!("Count after creating a = {}", Rc::strong_count(&a)); // => 1

    // `Rc::clone` を使う。`a.clone()` と書いても同じ。
    let b = Cons(3, Rc::clone(&a));
    println!("Count after creating b = {}", Rc::strong_count(&a)); // => 2

    {
        let c = Cons(4, Rc::clone(&a));
        println!("Count after creating c = {}", Rc::strong_count(&a)); // => 3
    } // `c` がスコープを抜けるとカウントが減る

    println!("Count after c goes out of scope = {}", Rc::strong_count(&a)); // => 2
}
```

[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Arc%3A%3ARc%3B%0A%0Aenum%20List%20%7B%0A%20%20%20%20Cons(i32%2C%20Rc%3CList%3E)%2C%0A%20%20%20%20Nil%2C%0A%7D%0A%0Ause%20List%3A%3A%7BCons%2C%20Nil%7D%3B%0A%0Afn%20main()%20%7B%0A%20%20%20%20let%20a%20%3D%20Rc%3A%3Anew(Cons(5%2C%20Rc%3A%3Anew(Cons(10%2C%20Rc%3A%3Anew(Nil))))))%3B%0A%20%20%20%20println!(%22Count%20after%20creating%20a%20%3D%20%7B%7D%22%2C%20Rc%3A%3Astrong_count(%26a))%3B%20%2F%2F%20%3D%3E%201%0A%0A%20%20%20%20%2F%2F%20%60Rc%3A%3Aclone%60%20%E3%82%92%E4%BD%BF%E3%81%86%E3%80%82%60a.clone%28%29%60%20%E3%81%A8%E6%9B%B8%E3%81%84%E3%81%A6%E3%82%82%E5%90%8C%E3%81%98%E3%80%82%0A%20%20%20%20let%20b%20%3D%20Cons(3%2C%20Rc%3A%3Aclone(%26a))%3B%0A%20%20%20%20println!(%22Count%20after%20creating%20b%20%3D%20%7B%7D%22%2C%20Rc%3A%3Astrong_count(%26a))%3B%20%2F%2F%20%3D%3E%202%0A%0A%20%20%20%20%7B%0A%20%20%20%20%20%20%20%20let%20c%20%3D%20Cons(4%2C%20Rc%3A%3Aclone(%26a))%3B%0A%20%20%20%20%20%20%20%20println!(%22Count%20after%20creating%20c%20%3D%20%7B%7D%22%2C%20Rc%3A%3Astrong_count(%26a))%3B%20%2F%2F%20%3D%3E%203%0A%20%20%20%20%7D%20%2F%2F%20%60c%60%20%E3%81%8C%E3%82%B9%E3%82%B3%E3%83%BC%E3%83%97%E3%82%92%E6%8A%9C%E3%81%91%E3%82%8B%E3%81%A8%E3%82%AB%E3%82%A6%E3%83%B3%E3%83%88%E3%81%8C%E6%B8%9B%E3%82%8B%0A%0A%20%20%20%20println!(%22Count%20after%20c%20goes%20out%20of%20scope%20%3D%20%7B%7D%22%2C%20Rc%3A%3Astrong_count(%26a))%3B%20%2F%2F%20%3D%3E%202%0A%7D)

`Rc::clone(&a)` は、`a` が指すデータへの新しい参照を作り、参照カウントをインクリメントします（この場合は 1 から 2 へ）。これはディープコピーではなく、非常に低コストな操作です。`b` と `c` がスコープを抜けるたびに参照カウントはデクリメントされ、最終的に 0 になった時点で `a` のデータが解放されます。

注意: `Rc<T>` はシングルスレッドでの利用に限定されています。スレッド間で所有権を共有する場合は、アトミックな参照カウントを持つ `Arc<T>` (Atomic Rc) を使います（第23章で詳述）。

## 19.3 `RefCell<T>` による内部可変性

`Rc<T>` は複数の所有者がデータを 不変に 共有することを可能にしますが、共有したデータを変更したい場合はどうすればよいでしょうか？

この「不変な値の内部にあるデータを可変にする」デザインパターンを 内部可変性 (Interior Mutability) と呼びます。

このパターンを実現するのが `RefCell<T>` です。`RefCell<T>` は、借用規則（複数の不変参照 or 一つの可変参照）のチェックを、コンパイル時ではなく 実行時 に行います。

- 通常の参照: 借用規則違反はコンパイルエラーになる。
- `RefCell<T>`: 借用規則違反は実行時にパニックを引き起こす。

`RefCell<T>` は、値の不変参照 (`&self`) を受け取って可変参照 (`&mut self`) を返す `borrow_mut()` というメソッドを提供します。

### 試してみよう：`RefCell<T>` の実行時パニック

`RefCell<T>` がどのように借用規則を実行時にチェックするかを見てみましょう。

```rust
use std::rc::Rc;
use std::cell::RefCell;

fn main() {
    let shared_value = Rc::new(RefCell::new(5));

    let a = Rc::clone(&shared_value);
    let b = Rc::clone(&shared_value);

    // 最初の可変借用は成功する
    let mut b_mut = b.borrow_mut();
    *b_mut += 10;
    println!("b_mut: {}", b_mut);

    // 同じスコープ内で、さらに可変借用しようとすると...
    let mut a_mut = a.borrow_mut(); // 💥 パニック！
    // thread 'main' panicked at 'already borrowed: BorrowMutError'

    *a_mut += 5;
    println!("a_mut: {}", a_mut);
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Arc%3A%3ARc%3B%0Ause%20std%3A%3Acell%3A%3ARefCell%3B%0A%0Afn%20main()%20%7B%0A%20%20%20%20let%20shared_value%20%3D%20Rc%3A%3Anew(RefCell%3A%3Anew(5))%3B%0A%0A%20%20%20%20let%20a%20%3D%20Rc%3A%3Aclone(%26shared_value)%3B%0A%20%20%20%20let%20b%20%3D%20Rc%3A%3Aclone(%26shared_value)%3B%0A%0A%20%20%20%20%2F%2F%20%E6%9C%80%E5%88%9D%E3%81%AE%E5%8F%AF%E5%A4%89%E5%80%9F%E7%94%A8%E3%81%AF%E6%88%90%E5%8A%9F%E3%81%99%E3%82%8B%0A%20%20%20%20let%20mut%20b_mut%20%3D%20b.borrow_mut()%3B%0A%20%20%20%20*b_mut%20%2B%3D%2010%3B%0A%20%20%20%20println!(%22b_mut%3A%20%7B%7D%22%2C%20b_mut)%3B%0A%0A%20%20%20%20%2F%2F%20%E5%90%8C%E3%81%98%E3%82%B9%E3%82%B3%E3%83%BC%E3%83%97%E5%86%85%E3%81%A7%E3%80%81%E3%81%95%E3%82%89%E3%81%AB%E5%8F%AF%E5%A4%89%E5%80%9F%E7%94%A8%E3%81%97%E3%82%88%E3%81%86%E3%81%A8...%0A%20%20%20%20let%20mut%20a_mut%20%3D%20a.borrow_mut()%3B%20%2F%2F%20%F0%9F%92%A5%20%E3%83%91%E3%83%8B%E3%83%83%E3%82%AF%EF%BC%81%0A%20%20%20%20%2F%2F%20thread%20%27main%27%20panicked%20at%20%27already%20borrowed%3A%20BorrowMutError%27%0A%0A%20%20%20%20*a_mut%20%2B%3D%205%3B%0A%20%20%20%20println!(%22a_mut%3A%20%7B%7D%22%2C%20a_mut)%3B%0A%7D)
`b_mut` という可変借用が存在するスコープで、`a.borrow_mut()` を呼び出したため、実行時エラーが発生しました。これは、コンパイラが見逃した借用規則違反を、`RefCell` が実行時に捕まえてくれた結果です。

## 19.4 `Rc<RefCell<T>>` の組み合わせ

`Rc<T>` と `RefCell<T>` を組み合わせることで、「複数の所有者がいて、かつその値を変更したい」という複雑な要求に応えることができます。

- `Rc<T>`: 複数の所有者がいることを可能にする。
- `RefCell<T>`: 不変な `Rc<T>` の中身を変更可能にする。

このパターンは、グラフ構造や、GUIアプリケーションのコンポーネント管理など、複雑な所有権関係を持つ場面で頻繁に利用されます。

## 19.5 まとめ

- `Rc<T>` (Reference Counting) は、単一のデータに対して複数の所有者を持つことを可能にするスマートポインタ。
- `Rc::clone` は参照カウントを増やすだけで、ディープコピーは行わない。
- `RefCell<T>` は、コンパイル時の借用チェックを回避し、実行時にチェックを行うことで 内部可変性 を提供する。
- 借用規則に違反した場合、`RefCell<T>` は実行時にパニックする。
- `Rc<RefCell<T>>` は、複数の所有者がデータを可変に借用するための一般的なデザインパターン。

---

スマートポインタは、Rust の所有権システムをより柔軟に活用するための強力なツールです。次の章からは、大規模なプロジェクトを管理するための「モジュールシステム」について学んでいきます。
