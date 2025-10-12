# 第 17 章：スマートポインタ入門：`Box<T>` と `Drop` トレイト

## この章のゴール
- スマートポインタが、ポインタのように振る舞いつつ所有権などの追加機能を持つデータ構造であることを理解する。
- `Box<T>` を使ってデータをスタックではなくヒープに割り当てる理由と方法を説明できる。
- 再帰的なデータ構造を定義しようとした際に発生するコンパイルエラーを、`Box<T>` を使って解決できる。
- `Drop` トレイトを実装し、値がスコープを抜けるときに実行される処理を可視化できる。

---

## 17.1 スマートポインタとは？

これまでの章で `String` や `Vec<T>` を使ってきましたが、これらは内部的にヒープメモリを利用しており、「スマートポインタ」の一種です。

**スマートポインタ**とは、通常の参照 (`&`) とは異なり、データそのものの**所有権を持つ**ことができるポインタのようなデータ構造です。そして、所有者がスコープを抜けたときに何らかの処理（多くはメモリの解放）を自動的に実行する機能を持っています。

この章では、最も基本的なスマートポインタである `Box<T>` と、その自動解放の仕組みを支える `Drop` トレイトに焦点を当てます。

## 17.2 `Box<T>`：データをヒープに置く

`Box<T>` は、データをスタックではなくヒープに確保するための最もシンプルな方法です。`Box<T>` 自体はスタック上に置かれ、ヒープ上のデータ `T` を指し示します。

なぜデータをヒープに置きたいのでしょうか？
1.  **コンパイル時にサイズが不明な型を扱いたい時**: 再帰的なデータ構造など。
2.  **大きなデータを所有権ごと移動する時**: スタック上で巨大なデータをコピーするのは高コストですが、ヒープに置いておけば、スタック上のポインタ（`Box`）の移動だけで済み、高速です。

### 試してみよう：再帰的な型とコンパイラのエラー

Lispのような言語でよく使われる `Cons` リストというデータ構造を `enum` で定義しようとすると、`Box<T>` の必要性がよくわかります。`Cons` リストは、「値」と「次のリストへの参照」で構成されます。

`cargo new smart_pointers` でプロジェクトを作り、試してみましょう。

```rust
// src/main.rs

// このコードはコンパイルエラーになる！
enum List {
    Cons(i32, List),
    Nil,
}

fn main() {
    let list = List::Cons(1, List::Cons(2, List::Cons(3, List::Nil)));
}
```

これをコンパイルすると、コンパイラは混乱してしまいます。

```text
error[E0072]: recursive type `List` has infinite size
 --> src/main.rs:2:1
  |
2 | enum List {
  | ^^^^^^^^^ recursive type has infinite size
3 |     Cons(i32, List),
  |               ---- recursive without indirection
  |
help: insert some indirection (e.g., a `Box`, `Rc`, or `&`) to break the cycle
  |
3 |     Cons(i32, Box<List>),
  |               ++++    +
```

コンパイラは `List` 型のサイズを計算しようとします。`List` は `Cons` を含み、その `Cons` はまた `List` を含み...と、サイズが無限になってしまい、スタック上にどれだけのメモリを確保すればよいか判断できません。

### 解決策：`Box<T>` で間接的に参照する

ここで `Box<T>` の出番です。`List` を直接 `Cons` に含める代わりに、`Box<List>` を含めるように変更します。`Box<T>` はポインタであり、そのサイズはコンパイル時に確定しているため、コンパイラは `List` 型のサイズを計算できるようになります。

```rust
// src/main.rs

enum List {
    Cons(i32, Box<List>), // List の代わりに Box<List> を使う
    Nil,
}

use List::{Cons, Nil};

fn main() {
    let list = Cons(1, Box::new(Cons(2, Box::new(Cons(3, Box::new(Nil))))));
}
```
これでエラーは解決します。`Box<T>` はヒープ上のデータへの唯一の所有者として振る舞い、所有権のルールも通常通り適用されます。

## 17.3 `Drop` トレイトと RAII パターン

スマートポインタが強力なのは、**`Drop` トレイト** のおかげです。値がスコープを抜けるときに、`Drop` トレイトに実装された `drop` メソッドが自動的に呼び出されます。`Box<T>` の場合、`drop` メソッドがヒープメモリを解放してくれます。

この「インスタンスの寿命が終わるときにリソースを解放する」というパターンは **RAII (Resource Acquisition Is Initialization)** と呼ばれ、Rustの安全性とメモリ管理の根幹をなす考え方です。

### 試してみよう：`drop` の呼び出しを可視化する

`drop` がいつ呼ばれるのかを、`println!` を使って可視化してみましょう。

```rust
// src/main.rs

struct MySmartPointer {
    data: String,
}

// Drop トレイトを実装
impl Drop for MySmartPointer {
    fn drop(&mut self) {
        println!("Dropping MySmartPointer with data `{}`!", self.data);
    }
}

fn main() {
    let a = MySmartPointer { data: "hello".to_string() };
    println!("MySmartPointer 'a' created.");
    
    let b = MySmartPointer { data: "world".to_string() };
    println!("MySmartPointer 'b' created.");

    println!("--- End of main scope ---");
} // a と b はここでスコープを抜ける
```

これを実行すると、`main` 関数が終了するタイミングで、変数が作られたのとは **逆の順番** で `drop` が呼ばれることがわかります。

```text
MySmartPointer 'a' created.
MySmartPointer 'b' created.
--- End of main scope ---
Dropping MySmartPointer with data `world`!
Dropping MySmartPointer with data `hello`!
```
このように、Rustは手動でメモリ解放コードを書かなくても、`Drop` トレイトを通じて安全かつ自動的にリソースをクリーンアップしてくれるのです。

## 17.4 まとめ

- `Box<T>` は、データをヒープに割り当てるための最も基本的なスマートポインタ。唯一の所有権を持つ。
- コンパイル時にサイズが確定できない再帰的な型などを定義する際に `Box<T>` が必要になる。
- `Drop` トレイトは、値がスコープを抜けるときに自動的に呼ばれる `drop` メソッドを定義する。
- `Drop` トレイトによる自動的なリソース解放（RAII）が、Rustのメモリ安全性の鍵となっている。

---

次の章では、`Box<T>` が持つ「唯一の所有権」という制約を超えて、データを共有するためのスマートポインタ、`Rc<T>` (参照カウント) と、それに伴う「内部可変性」という新しい概念について学びます。
