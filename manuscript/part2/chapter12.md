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

これらのコレクションはすべてヒープにデータを格納し、コンパイル時にはサイズが分からなくても、実行時にサイズを拡大・縮小できます。
```sh
cargo new collections
```
でプロジェクトを作り、挙動を見ていきましょう。

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20//%20vec%21%20%E3%83%9E%E3%82%AF%E3%83%AD%E3%81%A7%E5%88%9D%E6%9C%9F%E5%80%A4%E3%82%92%E6%8C%81%E3%81%A4%E3%83%99%E3%82%AF%E3%82%BF%E3%82%92%E4%BD%9C%E6%88%90%0A%20%20%20%20let%20mut%20v%20%3D%20vec%21%5B100%2C%2032%2C%2057%5D%3B%0A%0A%20%20%20%20//%20%E8%A6%81%E7%B4%A0%E3%81%AE%E8%BF%BD%E5%8A%A0%0A%20%20%20%20v.push%2877%29%3B%0A%0A%20%20%20%20//%20%E8%A6%81%E7%B4%A0%E3%81%B8%E3%81%AE%E3%82%A2%E3%82%AF%E3%82%BB%E3%82%B9%0A%20%20%20%20//%20%E6%B7%BB%E5%AD%97%E3%81%A7%E3%82%A2%E3%82%AF%E3%82%BB%E3%82%B9%E3%81%99%E3%82%8B%E3%81%A8%E3%80%81%E7%AF%84%E5%9B%B2%E5%A4%96%E3%81%AE%E5%A0%B4%E5%90%88%E3%83%91%E3%83%8B%E3%83%83%E3%82%AF%E3%82%92%E8%B5%B7%E3%81%93%E3%81%99%0A%20%20%20%20let%20third%3A%20%26i32%20%3D%20%26v%5B2%5D%3B%0A%20%20%20%20println%21%28%22The%20third%20element%20is%20%7B%7D%22%2C%20third%29%3B%0A%0A%20%20%20%20//%20.get%20%E3%82%92%E4%BD%BF%E3%81%86%E3%81%A8%20Option%3C%26T%3E%20%E3%81%8C%E8%BF%94%E3%82%8B%E3%81%9F%E3%82%81%E5%AE%89%E5%85%A8%0A%20%20%20%20match%20v.get%282%29%20%7B%0A%20%20%20%20%20%20%20%20Some%28third%29%20%3D%3E%20println%21%28%22The%20third%20element%20is%20%7B%7D%22%2C%20third%29%2C%0A%20%20%20%20%20%20%20%20None%20%3D%3E%20println%21%28%22There%20is%20no%20third%20element.%22%29%2C%0A%20%20%20%20%7D%0A%7D)

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=fn%20main%28%29%20%7B%0A%20%20%20%20let%20mut%20v%20%3D%20vec%21%5B1%2C%202%2C%203%2C%204%2C%205%5D%3B%0A%0A%20%20%20%20let%20first%20%3D%20%26v%5B0%5D%3B%20//%20v%20%E3%81%AE%E6%9C%80%E5%88%9D%E3%81%AE%E8%A6%81%E7%B4%A0%E3%81%B8%E3%81%AE%E4%B8%8D%E5%A4%89%E3%81%AE%E5%8F%82%E7%85%A7%E3%82%92%E5%8F%96%E5%BE%97%0A%0A%20%20%20%20v.push%286%29%3B%20//%20v%20%E3%81%AB%E8%A6%81%E7%B4%A0%E3%82%92%E8%BF%BD%E5%8A%A0%E3%81%97%E3%82%88%E3%81%86%E3%81%A8%E3%81%99%E3%82%8B%EF%BC%88%E5%8F%AF%E5%A4%89%E3%81%AE%E5%80%9F%E7%94%A8%E3%81%8C%E5%BF%85%E8%A6%81%EF%BC%89%0A%0A%20%20%20%20println%21%28%22The%20first%20element%20is%3A%20%7B%7D%22%2C%20first%29%3B%0A%7D)

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
- `String`: ヒープ上に確保された、伸長可能で可変な文字列。所有権を持つ。
- `&str`: 文字列スライス。`String`や文字列リテラルの一部を不変で借用する。所有権を持たない。

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=fn%20main%28%29%20%7B%0A%20%20%20%20let%20mut%20s%20%3D%20String%3A%3Anew%28%29%3B%20//%20%E7%A9%BA%E3%81%AEString%E3%82%92%E4%BD%9C%E6%88%90%0A%20%20%20%20%0A%20%20%20%20let%20data%20%3D%20%22initial%20contents%22%3B%0A%20%20%20%20let%20s%20%3D%20data.to_string%28%29%3B%20//%20%26str%E3%81%8B%E3%82%89String%E3%82%92%E4%BD%9C%E6%88%90%0A%0A%20%20%20%20let%20mut%20s%20%3D%20String%3A%3Afrom%28%22foo%22%29%3B%0A%20%20%20%20s.push_str%28%22bar%22%29%3B%20//%20%E6%96%87%E5%AD%97%E5%88%97%E3%82%B9%E3%83%A9%E3%82%A4%E3%82%B9%E3%82%92%E8%BF%BD%E5%8A%A0%0A%20%20%20%20println%21%28%22%7B%7D%22%2C%20s%29%3B%20//%20%22foobar%22%0A%7D)
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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Acollections%3A%3AHashMap%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20mut%20scores%20%3D%20HashMap%3A%3Anew%28%29%3B%0A%0A%20%20%20%20scores.insert%28String%3A%3Afrom%28%22Blue%22%29%2C%2010%29%3B%0A%20%20%20%20scores.insert%28String%3A%3Afrom%28%22Yellow%22%29%2C%2050%29%3B%0A%0A%20%20%20%20//%20%E5%80%A4%E3%81%AE%E5%8F%96%E5%BE%97%0A%20%20%20%20let%20team_name%20%3D%20String%3A%3Afrom%28%22Blue%22%29%3B%0A%20%20%20%20//%20get%20%E3%81%AF%20Option%3C%26V%3E%20%E3%82%92%E8%BF%94%E3%81%99%0A%20%20%20%20let%20score%20%3D%20scores.get%28%26team_name%29.copied%28%29.unwrap_or%280%29%3B%0A%20%20%20%20println%21%28%22Score%20for%20Blue%3A%20%7B%7D%22%2C%20score%29%3B%0A%0A%20%20%20%20//%20%E3%82%A4%E3%83%86%E3%83%AC%E3%83%BC%E3%83%88%0A%20%20%20%20for%20%28key%2C%20value%29%20in%20%26scores%20%7B%0A%20%20%20%20%20%20%20%20println%21%28%22%7B%7D%3A%20%7B%7D%22%2C%20key%2C%20value%29%3B%0A%20%20%20%20%7D%0A%7D)

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Acollections%3A%3AHashMap%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20field_name%20%3D%20String%3A%3Afrom%28%22Favorite%20color%22%29%3B%0A%20%20%20%20let%20field_value%20%3D%20String%3A%3Afrom%28%22Blue%22%29%3B%0A%0A%20%20%20%20let%20mut%20map%20%3D%20HashMap%3A%3Anew%28%29%3B%0A%20%20%20%20map.insert%28field_name%2C%20field_value%29%3B%0A%0A%20%20%20%20//%20%E4%BB%A5%E4%B8%8B%E3%81%AE%E8%A1%8C%E3%81%AE%E3%82%B3%E3%83%A1%E3%83%B3%E3%83%88%E3%82%92%E5%A4%96%E3%81%99%E3%81%A8%E3%82%B3%E3%83%B3%E3%83%91%E3%82%A4%E3%83%AB%E3%82%A8%E3%83%A9%E3%83%BC%EF%BC%81%0A%20%20%20%20//%20field_name%20%E3%81%A8%20field_value%20%E3%81%AF%20map%20%E3%81%AB%E3%83%A0%E3%83%BC%E3%83%96%E3%81%95%E3%82%8C%E3%81%9F%E5%BE%8C%E3%81%AA%E3%81%AE%E3%81%A7%E4%BD%BF%E3%81%88%E3%81%AA%E3%81%84%0A%20%20%20%20//%20println%21%28%22%7B%7D%2C%20%7B%7D%22%2C%20field_name%2C%20field_value%29%3B%0A%7D)
`Copy`トレイトを実装する`i32`のような型であれば値はコピーされますが、`String`のようなヒープにデータを持つ型はムーブされます。これも所有権システムの基本ルールがそのまま適用されています。

## 12.5 まとめ

- `Vec<T>`, `String`, `HashMap<K, V>` は、ヒープ上にデータを格納する主要なコレクション。
- コレクションを操作する際は、所有権と借用のルールが厳密に適用される。特に、不変の借用がある間は、コレクションを変更（可変の借用）することはできない。
- `String` は所有権を持つ文字列、`&str` はその借用。関数に渡す際は、所有権が不要なら `&str` を使うのが一般的。
- `HashMap` に `String` などの所有権を持つ型を挿入すると、所有権は `HashMap` にムーブされる。

---

コレクションは、実際のアプリケーションでデータを管理するための基本的な部品です。所有権システムが、これらのデータ構造をいかに安全に扱わせてくれるかを理解することが重要です。

次の章では、Rust の強力な「イテレータ」機能を使って、これらのコレクションをより効率的かつ宣言的に操作する方法について学びます。

