# 第 15 章：ジェネリクスとトレイト境界

## この章のゴール
- 具体的な型の関数に存在するコードの重複問題を認識し、ジェネリクスで解決できるようになる。
- ジェネリックな関数を書いた際に発生するコンパイルエラーを読み解き、トレイト境界の必要性を説明できる。
- `T: PartialOrd + Copy` のようなトレイト境界をジェネリックな関数に追加し、エラーを解決できる。
- `where` 句を使って、複雑なトレイト境界を読みやすく記述できる。

---

## 15.1 問題設定：ロジックは同じなのに、型が違う

`i32` のスライスから最大の数を見つける関数と、`char` のスライスから最大の文字を見つける関数を考えてみましょう。ロジックは全く同じですが、型が違うため、2つの関数を書く必要があります。

`cargo new generics` でプロジェクトを作り、この重複を体験してみましょう。

```rust
// src/main.rs

// i32 のスライスから最大値を見つける
fn largest_i32(list: &[i32]) -> &i32 {
    let mut largest = &list[0];
    for item in list {
        if item > largest {
            largest = item;
        }
    }
    largest
}

// char のスライスから最大値を見つける
fn largest_char(list: &[char]) -> &char {
    let mut largest = &list[0];
    for item in list {
        if item > largest {
            largest = item;
        }
    }
    largest
}

fn main() {
    let number_list = vec![34, 50, 25, 100, 65];
    println!("The largest number is {}", largest_i32(&number_list));

    let char_list = vec!['y', 'm', 'a', 'q'];
    println!("The largest char is {}", largest_char(&char_list));
}
```
関数の中身が完全に同一であることに注目してください。これは非効率ですし、将来ロジックを修正する必要が出てきた場合に、両方の関数を修正しなければならず、バグの元になります。

この問題を解決するのが **ジェネリクス (Generics)** です。

## 15.2 解決策：ジェネリクスでコードを抽象化する

ジェネリクスを使うと、具体的な型をプレースホルダー（型パラメータ）に置き換えて、型に対して汎用的な関数やデータ構造を定義できます。

### 試してみよう：ジェネリック関数とコンパイラのエラー

先ほどの2つの関数を、ジェネリックな型パラメータ `<T>` を使って一つにまとめてみましょう。

```rust
fn largest<T>(list: &[T]) -> &T {
    let mut largest = &list[0];

    for item in list {
        if item > largest { // 💥 ここでエラーが発生する！
            largest = item;
        }
    }

    largest
}
```

このコードをコンパイルしようとすると、コンパイラに怒られます。

```text
error[E0369]: binary operation `>` cannot be applied to type `&T`
 --> src/main.rs:6:17
  |
6 |         if item > largest {
  |            ---- ^ ------- &T
  |            |
  |            &T
  |
help: consider restricting type parameter `T`
  |
1 | fn largest<T: std::cmp::PartialOrd>(list: &[T]) -> &T {
  |             ++++++++++++++++++++++
```

コンパイラは非常に親切です。エラーメッセージは「`>` という二項演算子は、型 `&T` には適用できません」と言っています。なぜなら、ジェネリックな型パラメータ `T` は、この時点では **あらゆる型** の可能性があります。`>` 演算子で比較できない型（例えば、前の章で定義した `NewsArticle` 構造体など）が `list` に渡される可能性もあるため、コンパイラは安全のためにコンパイルを中断します。

### トレイト境界 (Trait Bounds) で制約を追加する

コンパイラのエラーを解決するには、「`T` はただの型ではなく、比較が可能な型でなければならない」という **制約** をコンパイラに教える必要があります。これが **トレイト境界 (Trait Bounds)** です。

エラーメッセージの `help` にある通り、`std::cmp::PartialOrd` トレイト（`>` や `<` などの比較演算子を実装するトレイト）を型パラメータ `T` に束縛します。

```rust
// T は PartialOrd トレイトを実装した型でなければならない
fn largest<T: std::cmp::PartialOrd>(list: &[T]) -> &T {
    let mut largest = &list[0];

    for item in list {
        if item > largest {
            largest = item;
        }
    }

    largest
}
```
これで、比較できない型を `largest` 関数に渡そうとすると、コンパイル時にエラーで弾かれるようになり、関数の安全性が保証されます。`i32` や `char` は `PartialOrd` を実装しているので、この関数を呼び出すことができます。

### ジェネリックな構造体とメソッド

ジェネリクスは関数だけでなく、構造体やメソッドにも使えます。

```rust
struct Point<T> {
    x: T,
    y: T,
}

// Point<T> のための impl ブロック
impl<T> Point<T> {
    fn x(&self) -> &T {
        &self.x
    }
}

// f32 型に限定したメソッド
impl Point<f32> {
    fn distance_from_origin(&self) -> f32 {
        (self.x.powi(2) + self.y.powi(2)).sqrt()
    }
}
```
`Point<T>` は任意の型の `x` と `y` を持てますが、`distance_from_origin` メソッドは `f32` 型の `Point` インスタンスでのみ呼び出すことができます。

## 15.3 `where` 句でトレイト境界を整理する

トレイト境界が複雑になると、関数のシグネチャが読みにくくなることがあります。`where` 句を使うと、これらをきれいに整理できます。

```rust
use std::fmt::{Display, Debug};

// where句を使わない場合
fn some_function<T: Display + Clone, U: Clone + Debug>(t: &T, u: &U) { /* ... */ }

// where句を使った場合（可読性が向上する）
fn some_function_pretty<T, U>(t: &T, u: &U)
    where T: Display + Clone,
          U: Clone + Debug
{
    /* ... */
}
```

## 15.4 まとめ

- ジェネリクスは、関数やデータ構造を特定の型に縛られずに定義するための仕組みで、コードの重複を削減する。
- ジェネリックな型パラメータ `<T>` には、あらゆる型が入る可能性があるため、コンパイラは安全のためにその型でできる操作を制限する。
- トレイト境界 (`T: Trait`) は、ジェネリックな型が満たすべき振る舞い（実装すべきトレイト）をコンパイラに伝え、コードの安全性を保証する。
- `where` 句を使うと、複雑なトレイト境界を読みやすく整理できる。

---

ジェネリクスとトレイトの組み合わせは、Rustの持つ「ゼロコスト抽象化」の核心です。これにより、高い柔軟性とパフォーマンスを両立したコードを書くことができます。

次の章では、Rust には `Debug`, `Clone`, `PartialEq` など、非常に便利で広く使われている「標準トレイト」が多数用意されています。これらの標準トレイトを使いこなし、`derive` 属性で簡単に実装する方法について学びます。
