# 第 17 章：スマートポインタ入門：`Box<T>` と `Drop` トレイト

## この章のゴール
- スマートポインタが、ポインタのように振る舞いつつ所有権などの追加機能を持つデータ構造であることを理解する。
- `Box<T>` を使ってデータをスタックではなくヒープに割り当てる理由と方法を説明できる。
- 再帰的なデータ構造を定義しようとした際に発生するコンパイルエラーを、`Box<T>` を使って解決できる。
- `Drop` トレイトを実装し、値がスコープを抜けるときに実行される処理を可視化できる。

---

## 17.1 スマートポインタとは？

これまでの章で `String` や `Vec<T>` を使ってきましたが、これらは内部的にヒープメモリを利用しており、「スマートポインタ」の一種です。

スマートポインタとは、通常の参照 (`&`) とは異なり、データそのものの所有権を持つことができるポインタのようなデータ構造です。そして、所有者がスコープを抜けたときに何らかの処理（多くはメモリの解放）を自動的に実行する機能を持っています。

この章では、最も基本的なスマートポインタである `Box<T>` と、その自動解放の仕組みを支える `Drop` トレイトに焦点を当てます。

## 17.2 `Box<T>`：データをヒープに置く

`Box<T>` は、データをスタックではなくヒープに確保するための最もシンプルな方法です。`Box<T>` 自体はスタック上に置かれ、ヒープ上のデータ `T` を指し示します。

なぜデータをヒープに置きたいのでしょうか？
1.  **コンパイル時にサイズが不明な型を扱いたい時**: 再帰的なデータ構造など。
2.  **大きなデータを所有権ごと移動する時**: スタック上で巨大なデータをコピーするのは高コストですが、ヒープに置いておけば、スタック上のポインタ（`Box`）の移動だけで済み、高速です。

### 試してみよう：再帰的な型とコンパイラのエラー

Lispのような言語でよく使われる `Cons` リストというデータ構造を `enum` で定義しようとすると、`Box<T>` の必要性がよくわかります。`Cons` リストは、「値」と「次のリストへの参照」で構成されます。

```sh
cargo new smart_pointers
```
でプロジェクトを作り、試してみましょう。

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0A%0A//%20%E3%81%93%E3%81%AE%E3%82%B3%E3%83%BC%E3%83%89%E3%81%AF%E3%82%B3%E3%83%B3%E3%83%91%E3%82%A4%E3%83%AB%E3%82%A8%E3%83%A9%E3%83%BC%E3%81%AB%E3%81%AA%E3%82%8B%EF%BC%81%0Aenum%20List%20%7B%0A%20%20%20%20Cons%28i32%2C%20List%29%2C%0A%20%20%20%20Nil%2C%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20list%20%3D%20List%3A%3ACons%281%2C%20List%3A%3ACons%282%2C%20List%3A%3ACons%283%2C%20List%3A%3ANil%29%29%29%3B%0A%7D)

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0A%0Aenum%20List%20%7B%0A%20%20%20%20Cons%28i32%2C%20Box%3CList%3E%29%2C%20//%20List%20%E3%81%AE%E4%BB%A3%E3%82%8F%E3%82%8A%E3%81%AB%20Box%3CList%3E%20%E3%82%92%E4%BD%BF%E3%81%86%0A%20%20%20%20Nil%2C%0A%7D%0A%0Ause%20List%3A%3A%7BCons%2C%20Nil%7D%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20list%20%3D%20Cons%281%2C%20Box%3A%3Anew%28Cons%282%2C%20Box%3A%3Anew%28Cons%283%2C%20Box%3A%3Anew%28Nil%29%29%29%29%29%29%3B%0A%7D)
これでエラーは解決します。`Box<T>` はヒープ上のデータへの唯一の所有者として振る舞い、所有権のルールも通常通り適用されます。

## 17.3 `Drop` トレイトと RAII パターン

スマートポインタが強力なのは、`Drop` トレイト のおかげです。値がスコープを抜けるときに、`Drop` トレイトに実装された `drop` メソッドが自動的に呼び出されます。`Box<T>` の場合、`drop` メソッドがヒープメモリを解放してくれます。

この「インスタンスの寿命が終わるときにリソースを解放する」というパターンは RAII (Resource Acquisition Is Initialization) と呼ばれ、Rustの安全性とメモリ管理の根幹をなす考え方です。

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0A%0Astruct%20MySmartPointer%20%7B%0A%20%20%20%20data%3A%20String%2C%0A%7D%0A%0A//%20Drop%20%E3%83%88%E3%83%AC%E3%82%A4%E3%83%88%E3%82%92%E5%AE%9F%E8%A3%85%0Aimpl%20Drop%20for%20MySmartPointer%20%7B%0A%20%20%20%20fn%20drop%28%26mut%20self%29%20%7B%0A%20%20%20%20%20%20%20%20println%21%28%22Dropping%20MySmartPointer%20with%20data%20%60%7B%7D%60%21%22%2C%20self.data%29%3B%0A%20%20%20%20%7D%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20a%20%3D%20MySmartPointer%20%7B%20data%3A%20%22hello%22.to_string%28%29%20%7D%3B%0A%20%20%20%20println%21%28%22MySmartPointer%20%27a%27%20created.%22%29%3B%0A%20%20%20%20%0A%20%20%20%20let%20b%20%3D%20MySmartPointer%20%7B%20data%3A%20%22world%22.to_string%28%29%20%7D%3B%0A%20%20%20%20println%21%28%22MySmartPointer%20%27b%27%20created.%22%29%3B%0A%0A%20%20%20%20println%21%28%22---%20End%20of%20main%20scope%20---%22%29%3B%0A%7D%20//%20a%20%E3%81%A8%20b%20%E3%81%AF%E3%81%93%E3%81%93%E3%81%A7%E3%82%B9%E3%82%B3%E3%83%BC%E3%83%97%E3%82%92%E6%8A%9C%E3%81%91%E3%82%8B)

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
