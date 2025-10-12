# 第 12 章：コレクション (Vec, HashMap, String)

## この章のゴール
- `Vec`, `HashMap`, `String` の基本的な操作 (追加, 取得, 更新) を行える。
- `Vec` のイテレーション中にその `Vec` を変更しようとすると、なぜコンパイルエラーになるのかを説明できる。
- `String` が所有権を持つ文字列、`&str` がその借用であることを、所有権の観点から説明できる。
- `HashMap` への挿入時に、キーと値の所有権がムーブする場合があることを体験する。

---

## 12.1 よく使うデータの集まり

ほとんどのプログラムでは、複数の値のリストや、キーと値のペアといったデータの集まりを扱う必要があります。Rustの標準ライブラリが提供する主要なコレクションは以下の3つです。

- `Vec<T>`: 可変長の配列。Pythonの`list`やGoの`slice`に相当します。
- `String`: 文字列。内部的には`Vec<u8>`に近く、UTF-8として保証されています。
- `HashMap<K, V>`: ハッシュマップ。Pythonの`dict`やGoの`map`に相当します。

これらのコレクションはすべてヒープにデータを格納し、コンパイル時にはサイズが分からなくても、実行時にサイズを拡大・縮小できます。`cargo new collections` でプロジェクトを作り、挙動を見ていきましょう。

## 12.2 `Vec<T>`：値のシーケンス

`Vec<T>` (ベクタ) は、同じ型の値を連続したメモリ領域に格納します。

```rust
// src/main.rs

fn main() {
    // vec! マクロで初期値を持つベクタを作成
    let mut v = vec![100, 32, 57];

    // 要素の追加
    v.push(77);

    // 要素へのアクセス
    // 添字でアクセスすると、範囲外の場合パニックを起こす
    let third: &i32 = &v[2];
    println!("The third element is {}", third);

    // .get を使うと Option<&T> が返るため安全
    match v.get(2) {
        Some(third) => println!("The third element is {}", third),
        None => println!("There is no third element."),
    }
}
```

### 試してみよう：借用ルールとベクタ

ベクタへの参照を持っている間に、そのベクタに要素を追加しようとするとどうなるでしょうか？試してみましょう。

```rust
fn main() {
    let mut v = vec![1, 2, 3, 4, 5];

    let first = &v[0]; // v の最初の要素への不変の参照を取得

    v.push(6); // v に要素を追加しようとする（可変の借用が必要）

    println!("The first element is: {}", first);
}
```

これを `cargo run` するとコンパイルエラーになります。

```text
error[E0502]: cannot borrow `v` as mutable because it is also borrowed as immutable
 --> src/main.rs:6:5
  |
4 |     let first = &v[0];
  |                  - immutable borrow occurs here
5 |
6 |     v.push(6);
  |     ^^^^^^^^^ mutable borrow occurs here
7 |
8 |     println!("The first element is: {}", first);
  |                                          ----- immutable borrow later used here
```

これは借用のルールそのものです。`first` が `v` の一部を不変で借用している間は、`v.push(6)` で `v` 全体を可変で借用することはできません。なぜなら、`push` によってベクタのメモリが足りなくなった場合、全要素が新しいメモリ位置にコピーされて再配置される可能性があります。そうなると、`first` が指していたメモリ位置は無効になってしまい、ダングリングポインタが生まれてしまうからです。Rustのコンパイラは、この危険な操作をコンパイル時に防いでくれます。

## 12.3 `String`：所有権を持つ文字列

Rustの文字列には、主に2つの型があります。
- `String`: ヒープ上に確保された、伸長可能で可変な文字列。**所有権を持つ**。
- `&str`: 文字列スライス。`String`や文字列リテラルの一部を不変で借用する。**所有権を持たない**。

`String`は、実質的にはUTF-8エンコードされたバイトの`Vec<u8>`をラップしたものです。

```rust
fn main() {
    let mut s = String::new(); // 空のStringを作成
    
    let data = "initial contents";
    let s = data.to_string(); // &strからStringを作成

    let mut s = String::from("foo");
    s.push_str("bar"); // 文字列スライスを追加
    println!("{}", s); // "foobar"
}
```
関数に文字列を渡す際は、その関数の目的によって`String`と`&str`を使い分けることが重要です。単に文字列を読み取りたいだけなら、所有権を奪わない`&str`を引数に取る方が、柔軟性が高く効率的です。

## 12.4 `HashMap<K, V>`：キーと値のマッピング

`HashMap<K, V>` は、キー `K` と値 `V` のペアを格納します。

```rust
use std::collections::HashMap;

fn main() {
    let mut scores = HashMap::new();

    scores.insert(String::from("Blue"), 10);
    scores.insert(String::from("Yellow"), 50);

    // 値の取得
    let team_name = String::from("Blue");
    // get は Option<&V> を返す
    let score = scores.get(&team_name).copied().unwrap_or(0);
    println!("Score for Blue: {}", score);

    // イテレート
    for (key, value) in &scores {
        println!("{}: {}", key, value);
    }
}
```

### 試してみよう：HashMap と所有権

`HashMap` に値を挿入すると、そのキーと値の所有権は `HashMap` にムーブします。

```rust
use std::collections::HashMap;

fn main() {
    let field_name = String::from("Favorite color");
    let field_value = String::from("Blue");

    let mut map = HashMap::new();
    map.insert(field_name, field_value);

    // 以下の行のコメントを外すとコンパイルエラー！
    // field_name と field_value は map にムーブされた後なので使えない
    // println!("{}, {}", field_name, field_value);
}
```
`Copy`トレイトを実装する`i32`のような型であれば値はコピーされますが、`String`のようなヒープにデータを持つ型はムーブされます。これも所有権システムの基本ルールがそのまま適用されています。

## 12.5 まとめ

- `Vec<T>`, `String`, `HashMap<K, V>` は、ヒープ上にデータを格納する主要なコレクション。
- コレクションを操作する際は、所有権と借用のルールが厳密に適用される。特に、不変の借用がある間は、コレクションを変更（可変の借用）することはできない。
- `String` は所有権を持つ文字列、`&str` はその借用。関数に渡す際は、所有権が不要なら `&str` を使うのが一般的。
- `HashMap` に `String` などの所有権を持つ型を挿入すると、所有権は `HashMap` にムーブされる。

---

コレクションは、実際のアプリケーションでデータを管理するための基本的な部品です。所有権システムが、これらのデータ構造をいかに安全に扱わせてくれるかを理解することが重要です。

次の章では、Rust の強力な「イテレータ」機能を使って、これらのコレクションをより効率的かつ宣言的に操作する方法について学びます。

