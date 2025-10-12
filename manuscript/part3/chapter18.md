# 第 18 章：共有所有と内部可変性 `Rc<T>` と `RefCell<T>`

## この章のゴール
- `Box<T>` の単一所有権では表現できないデータ構造（例：グラフ）を示し、`Rc<T>` による共有所有の必要性を説明できる。
- `Rc::clone` と `Rc::strong_count` を使い、参照カウントがどのように機能するかを追体験する。
- 共有されたデータを変更しようとするとコンパイルエラーになることを体験し、「内部可変性」の必要性を説明できる。
- `RefCell<T>` の `.borrow_mut()` を使って共有データを変更し、意図的に借用ルールを破って実行時パニックを引き起こせる。

---

## 18.1 問題設定(1)：所有者が一人では困る場面

前の章で学んだ `Box<T>` は、データに対する **唯一の** 所有権を表現するのに非常に便利です。しかし、時には複数の所有者が必要になる場合があります。

例えば、`Cons` リストで、2つの異なるリストが同じ後続リストを共有するケースを考えてみましょう。

`cargo new rc_refcell` でプロジェクトを作り、試してみましょう。

```rust
// src/main.rs
enum List {
    Cons(i32, Box<List>),
    Nil,
}

use List::{Cons, Nil};

fn main() {
    let a = Cons(5, Box::new(Cons(10, Box::new(Nil))));
    // `a` の一部を共有したい
    let b = Cons(3, Box::new(a)); // `a` は `b` にムーブされる
    let c = Cons(4, Box::new(a)); // エラー！ `a` は既にムーブされている
}
```
`a` は `b` を作った時点で所有権がムーブされてしまうため、`c` で再び `a` を使おうとするとコンパイルエラーになります。これを解決するのが **`Rc<T>` (Reference Counting)** です。

## 18.2 解決策(1)：`Rc<T>` で所有権を共有する

`Rc<T>` は、単一スレッド内で、あるデータに対する複数の所有者を可能にするスマートポインタです。`Rc<T>` は、自身が何回クローンされたかを常にカウントしており（参照カウント）、このカウントがゼロになった時に初めてヒープ上のデータを解放します。

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
`Rc<T>` を使うことで、`a` の所有権をムーブさせることなく、複数のリストで安全に共有することができました。

## 18.3 問題設定(2)：共有しているデータを変更したい！

`Rc<T>` でデータの共有はできましたが、`Rc<T>` が保持する値は不変です。もし共有している値を後から変更したくなったらどうなるでしょうか？

```rust
use std::rc::Rc;
let shared_value = Rc::new(5);
let mut mut_ref = shared_value.as_mut(); // エラー！
*mut_ref = 10;
```
`Rc<T>` は複数の不変参照 (`&T`) を配ることはできますが、可変参照 (`&mut T`) を得ることはできません。もしできてしまうと、複数の場所から同時にデータを変更できてしまい、借用規則に違反するからです。

ここで登場するのが **「内部可変性 (Interior Mutability)」** というデザインパターンと、それを実現する **`RefCell<T>`** です。

## 18.4 解決策(2)：`RefCell<T>` で内部可変性を実現する

`RefCell<T>` は、コンパイル時ではなく **実行時** に借用規則をチェックするスマートポインタです。これにより、不変に見える値の「内部」を可変にすることができます。

- `borrow()`: 不変参照 `&T` に相当する `Ref<T>` を返す。
- `borrow_mut()`: 可変参照 `&mut T` に相当する `RefMut<T>` を返す。

`Rc<T>` と `RefCell<T>` を組み合わせた `Rc<RefCell<T>>` は、**複数の所有者を持ち、かつ内部の値を変更できる** オブジェクトを作るための非常に強力なパターンです。

### 試してみよう：実行時パニックを体験する

`RefCell<T>` は借用規則をコンパイル時に素通ししますが、実行時に監視しています。もしルールを破れば、プログラムは即座に **パニック** します。

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
`b_mut` という可変借用が存在するスコープで、`a.borrow_mut()` を呼び出したため、実行時エラーが発生しました。これは、コンパイラが見逃した借用規則違反を、`RefCell` が実行時に捕まえてくれた結果です。

## 18.5 まとめ

- `Rc<T>` は、単一スレッドでデータへの複数の所有権（共有所有）を可能にする。参照カウントで寿命を管理する。
- 内部可変性は、不変な参照の裏でデータを変更するためのデザインパターン。
- `RefCell<T>` は、実行時に借用チェックを行うことで内部可変性を提供する。
- `RefCell<T>` の借用規則を破ると、コンパイルエラーの代わりに **実行時パニック** が発生する。
- `Rc<RefCell<T>>` は、複数の所有者がいて、かつ値を変更できるデータ構造を作るための一般的なイディオム。

---

次の部では、視点を変えて、プロジェクトが大きくなるにつれて重要になる「プロジェクト管理」と「テスト」について学びます。まずは、コードを整理するためのモジュールシステムから始めましょう。
