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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Aenum%20List%20%7B%0A%20%20%20%20Cons%28i32%2C%20Box%3CList%3E%29%2C%0A%20%20%20%20Nil%2C%0A%7D%0A%0Ause%20List%3A%3A%7BCons%2C%20Nil%7D%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20a%20%3D%20Cons%285%2C%20Box%3A%3Anew%28Cons%2810%2C%20Box%3A%3Anew%28Nil%29%29%29%29%3B%0A%20%20%20%20//%20%60a%60%20%E3%81%AE%E4%B8%80%E9%83%A8%E3%82%92%E5%85%B1%E6%9C%89%E3%81%97%E3%81%9F%E3%81%84%0A%20%20%20%20let%20b%20%3D%20Cons%283%2C%20Box%3A%3Anew%28a%29%29%3B%20//%20%60a%60%20%E3%81%AF%20%60b%60%20%E3%81%AB%E3%83%A0%E3%83%BC%E3%83%96%E3%81%95%E3%82%8C%E3%82%8B%0A%20%20%20%20let%20c%20%3D%20Cons%284%2C%20Box%3A%3Anew%28a%29%29%3B%20//%20%E3%82%A8%E3%83%A9%E3%83%BC%EF%BC%81%20%60a%60%20%E3%81%AF%E6%97%A2%E3%81%AB%E3%83%A0%E3%83%BC%E3%83%96%E3%81%95%E3%82%8C%E3%81%A6%E3%81%84%E3%82%8B%0A%7D)
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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Arc%3A%3ARc%3B%0A%0Aenum%20List%20%7B%0A%20%20%20%20Cons%28i32%2C%20Rc%3CList%3E%29%2C%0A%20%20%20%20Nil%2C%0A%7D%0A%0Ause%20List%3A%3A%7BCons%2C%20Nil%7D%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20a%20%3D%20Rc%3A%3Anew%28Cons%285%2C%20Rc%3A%3Anew%28Cons%2810%2C%20Rc%3A%3Anew%28Nil%29%29%29%29%29%3B%0A%20%20%20%20println%21%28%22Count%20after%20creating%20a%20%3D%20%7B%7D%22%2C%20Rc%3A%3Astrong_count%28%26a%29%29%3B%20//%20%3D%3E%201%0A%0A%20%20%20%20//%20%60Rc%3A%3Aclone%60%20%E3%82%92%E4%BD%BF%E3%81%86%E3%80%82%60a.clone%28%29%60%20%E3%81%A8%E6%9B%B8%E3%81%84%E3%81%A6%E3%82%82%E5%90%8C%E3%81%98%E3%80%82%0A%20%20%20%20let%20b%20%3D%20Cons%283%2C%20Rc%3A%3Aclone%28%26a%29%29%3B%0A%20%20%20%20println%21%28%22Count%20after%20creating%20b%20%3D%20%7B%7D%22%2C%20Rc%3A%3Astrong_count%28%26a%29%29%3B%20//%20%3D%3E%202%0A%0A%20%20%20%20%7B%0A%20%20%20%20%20%20%20%20let%20c%20%3D%20Cons%284%2C%20Rc%3A%3Aclone%28%26a%29%29%3B%0A%20%20%20%20%20%20%20%20println%21%28%22Count%20after%20creating%20c%20%3D%20%7B%7D%22%2C%20Rc%3A%3Astrong_count%28%26a%29%29%3B%20//%20%3D%3E%203%0A%20%20%20%20%7D%20//%20%60c%60%20%E3%81%8C%E3%82%B9%E3%82%B3%E3%83%BC%E3%83%97%E3%82%92%E6%8A%9C%E3%81%91%E3%82%8B%E3%81%A8%E3%82%AB%E3%82%A6%E3%83%B3%E3%83%88%E3%81%8C%E6%B8%9B%E3%82%8B%0A%0A%20%20%20%20println%21%28%22Count%20after%20c%20goes%20out%20of%20scope%20%3D%20%7B%7D%22%2C%20Rc%3A%3Astrong_count%28%26a%29%29%3B%20//%20%3D%3E%202%0A%7D)
`Rc<T>` を使うことで、`a` の所有権をムーブさせることなく、複数のリストで安全に共有することができました。

## 18.3 問題設定(2)：共有しているデータを変更したい！

`Rc<T>` でデータの共有はできましたが、`Rc<T>` が保持する値は不変です。もし共有している値を後から変更したくなったらどうなるでしょうか？

```rust
use std::rc::Rc;
let shared_value = Rc::new(5);
let mut mut_ref = shared_value.as_mut(); // エラー！
*mut_ref = 10;
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Arc%3A%3ARc%3B%0Alet%20shared_value%20%3D%20Rc%3A%3Anew%285%29%3B%0Alet%20mut%20mut_ref%20%3D%20shared_value.as_mut%28%29%3B%20//%20%E3%82%A8%E3%83%A9%E3%83%BC%EF%BC%81%0A%2Amut_ref%20%3D%2010%3B)
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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Arc%3A%3ARc%3B%0Ause%20std%3A%3Acell%3A%3ARefCell%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20shared_value%20%3D%20Rc%3A%3Anew%28RefCell%3A%3Anew%285%29%29%3B%0A%0A%20%20%20%20let%20a%20%3D%20Rc%3A%3Aclone%28%26shared_value%29%3B%0A%20%20%20%20let%20b%20%3D%20Rc%3A%3Aclone%28%26shared_value%29%3B%0A%0A%20%20%20%20//%20%E6%9C%80%E5%88%9D%E3%81%AE%E5%8F%AF%E5%A4%89%E5%80%9F%E7%94%A8%E3%81%AF%E6%88%90%E5%8A%9F%E3%81%99%E3%82%8B%0A%20%20%20%20let%20mut%20b_mut%20%3D%20b.borrow_mut%28%29%3B%0A%20%20%20%20%2Ab_mut%20%2B%3D%2010%3B%0A%20%20%20%20println%21%28%22b_mut%3A%20%7B%7D%22%2C%20b_mut%29%3B%0A%0A%20%20%20%20//%20%E5%90%8C%E3%81%98%E3%82%B9%E3%82%B3%E3%83%BC%E3%83%97%E5%86%85%E3%81%A7%E3%80%81%E3%81%95%E3%82%89%E3%81%AB%E5%8F%AF%E5%A4%89%E5%80%9F%E7%94%A8%E3%81%97%E3%82%88%E3%81%86%E3%81%A8%E3%81%99%E3%82%8B%E3%81%A8...%0A%20%20%20%20let%20mut%20a_mut%20%3D%20a.borrow_mut%28%29%3B%20//%20%F0%9F%92%A5%20%E3%83%91%E3%83%8B%E3%83%83%E3%82%AF%EF%BC%81%0A%20%20%20%20//%20thread%20%27main%27%20panicked%20at%20%27already%20borrowed%3A%20BorrowMutError%27%0A%0A%20%20%20%20%2Aa_mut%20%2B%3D%205%3B%0A%20%20%20%20println%21%28%22a_mut%3A%20%7B%7D%22%2C%20a_mut%29%3B%0A%7D)
`b_mut` という可変借用が存在するスコープで、`a.borrow_mut()` を呼び出したため、実行時エラーが発生しました。これは、コンパイラが見逃した借用規則違反を、`RefCell` が実行時に捕まえてくれた結果です。

## 18.5 まとめ

- `Rc<T>` は、単一スレッドでデータへの複数の所有権（共有所有）を可能にする。参照カウントで寿命を管理する。
- 内部可変性は、不変な参照の裏でデータを変更するためのデザインパターン。
- `RefCell<T>` は、実行時に借用チェックを行うことで内部可変性を提供する。
- `RefCell<T>` の借用規則を破ると、コンパイルエラーの代わりに **実行時パニック** が発生する。
- `Rc<RefCell<T>>` は、複数の所有者がいて、かつ値を変更できるデータ構造を作るための一般的なイディオム。

---

次の部では、視点を変えて、プロジェクトが大きくなるにつれて重要になる「プロジェクト管理」と「テスト」について学びます。まずは、コードを整理するためのモジュールシステムから始めましょう。
