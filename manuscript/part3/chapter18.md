# 第 18 章：スマートポインタ：`Box<T>` でヒープにデータを置く

## この章のゴール
- スタックとヒープの違い、およびそれぞれのデータがどのような場合に適しているかを説明できる。
- `Box<T>` を使って、コンパイル時にサイズが確定しない再帰的なデータ構造（例：Cons リスト）を定義できる。
- `Deref` トレイトが、スマートポインタが通常の参照のように振る舞う仕組みの裏側にあることを説明できる。

---

## 18.1 所有権とポインタの復習

これまでの章で、所有権、借用、参照について学んできました。`&` で作成する参照は、データを指し示す一種のポインタですが、所有権は持ちません。

この章から数章にわたって、参照とは異なる種類のポインタである スマートポインタ を探求します。スマートポインタは、単にメモリアドレスを指すだけでなく、追加のメタデータや能力を持つ `struct` です。

## 18.2 なぜ `Box<T>` が必要なのか？：ヒープへのデータ配置

最もシンプルなスマートポインタは `Box<T>` です。`Box<T>` を使うと、データをスタックではなく ヒープ に配置することができます。

- スタック: 高速。関数呼び出しと共に確保され、関数から抜けると自動的に解放される。サイズが固定でなければならない。
- ヒープ: やや低速。OSに明示的にメモリ領域を要求し、使い終わったら解放する必要がある。サイズが可変、またはコンパイル時に不明なデータを置くことができる。

`Box<T>` は、コンパイル時にサイズが分からないデータを扱う場合や、大きなデータを所有権をムーブさせずに転送したい場合（コピーのコストを避けるため）に特に役立ちます。

### 試してみよう：`Box<T>` でヒープに `i32` を置く

```rust
// src/main.rs

fn main() {
    let a = Box::new(5);
    println!("Box 'a' created with value: {}", *a);

    let b = Box::new(10);
    println!("Box 'b' created with value: {}", *b);

    println!("--- End of main scope ---");
} // a と b はここでスコープを抜ける
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0A%0Afn%20main()%20%7B%0A%20%20%20%20let%20a%20%3D%20Box%3A%3Anew(5)%3B%0A%20%20%20%20println%21%28%22Box%20%27a%27%20created%20with%20value%3A%20%7B%7D%22%2C%20%2Aa%29%3B%0A%20%20%20%20%0A%20%20%20%20let%20b%20%3D%20Box%3A%3Anew(10)%3B%0A%20%20%20%20println%21%28%22Box%20%27b%27%20created%20with%20value%3A%20%7B%7D%22%2C%20%2Ab%29%3B%0A%0A%20%20%20%20println%21%28%22---%20End%20of%20main%20scope%20---%22%29%3B%0A%7D%20//%20a%20%E3%81%A8%20b%20%E3%81%AF%E3%81%93%E3%81%93%E3%81%A7%E3%82%B9%E3%82%B3%E3%83%BC%E3%83%97%E3%82%92%E6%8A%9C%E3%81%91%E3%82%8B)

これを実行すると、`main` 関数が終了するタイミングで、変数が作られたのとは 逆の順番 で `drop` が呼ばれることがわかります。

```text
Box 'a' created with value: 5
Box 'b' created with value: 10
--- End of main scope ---
```

このように、`Box<T>` はヒープ上のデータへの所有者として振る舞い、所有権のルールも通常通り適用されます。`b` がスコープを抜ける際に、`Box<T>` のデストラクタが呼ばれ、ヒープのメモリ（`5`が格納されている領域）が自動的に解放されます。これがRAII（Resource Acquisition Is Initialization）の力です。

## 18.3 `Box<T>` の具体的なユースケース：再帰的なデータ構造

`Box<T>` の最も重要なユースケースの一つが、再帰的なデータ構造 の定義です。

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

コンパイラは、`List` のサイズを計算できません。`List` は `Cons` を含み、その `Cons` は `List` を含み...というように、無限に続いてしまうためです。

ここで `Box<T>` の出番です。`Box<T>` はポインタなので、そのサイズは常に固定です（中身のデータサイズに関わらず、メモリアドレスを格納する分だけ）。`List` の定義を `Box<List>` に変更することで、コンパイラは `List` のサイズを計算できるようになります。

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

これで、ネストしたリスト構造をヒープ上に構築することができました。

## 18.4 `Deref` トレイトによる参照外し

スマートポインタが「ポインタのように」振る舞えるのは、`Deref` トレイトを実装しているからです。`Deref` トレイトを実装すると、`*` 演算子（参照外し演算子）の挙動をカスタマイズできます。

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

これを実行すると、`main` 関数が終了するタイミングで、変数が作られたのとは 逆の順番 で `drop` が呼ばれることがわかります。

```text
MySmartPointer 'a' created.
MySmartPointer 'b' created.
--- End of main scope ---
Dropping MySmartPointer with data `world`!
Dropping MySmartPointer with data `hello`!
```

このように、Rustは手動でメモリ解放コードを書かなくても、`Drop` トレイトを通じて安全かつ自動的にリソースをクリーンアップしてくれるのです。

## 18.5 まとめ

- スマートポインタは、単なるポインタ以上の機能を持つ `struct` である。
- `Box<T>` は、データをヒープに配置するための最もシンプルなスマートポインタ。
- `Box<T>` を使うことで、コンパイル時にサイズが確定しない再帰的なデータ構造を安全に作ることができる。
- `Box<T>` は `Drop` トレイトを実装しており、スコープを抜ける際にヒープメモリを自動で解放する。
- `Box<T>` は `Deref` トレイトを実装しており、`*` 演算子で中のデータにアクセスできる。

---

`Box<T>` は、単一の所有者を持つヒープ上のデータを扱うための基本ツールです。次の章では、複数の所有者を持つデータを扱うためのスマートポインタ、`Rc<T>` (参照カウント) と、不変な値の内部を変更可能にする `RefCell<T>` について学びます。
