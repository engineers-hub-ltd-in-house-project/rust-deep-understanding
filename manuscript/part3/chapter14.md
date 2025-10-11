# 第 14 章：トレイトシステム入門

## この章のゴール
- トレイトが共有の振る舞いを定義するためのものであることを理解する。
- 構造体や列挙型にトレイトを実装できるようになる。
- デフォルト実装を持つトレイトメソッドを定義・利用できる。
- 関数や構造体のジェネリクスでトレイト境界を指定できるようになる。

## 前章の復習
前の章ではイテレータについて学び、コレクションを効率的かつ宣言的に操作する方法を習得しました。`map` や `filter` といったメソッドチェーンがいかにコードの可読性を高めるかを見てきました。

## なぜこれが必要なのか？
異なる型であっても、共通の振る舞い（例えば、画面に表示する、要約を生成するなど）を持たせたい場合があります。トレイトは、このような共有の振る舞いを定義するための Rust の仕組みです。これにより、コードの再利用性が高まり、柔軟な設計が可能になります。

## Python/Go ではこうやっていた
- **Python:** ダックタイピングが主流で、特定のメソッド（例: `__str__`）を持っていれば、そのオブジェクトは特定の振る舞いを持つとみなされました。ABC (抽象基底クラス) や `Protocol` を使って、より明示的にインターフェースを定義することもできました。
- **Go:** `interface` を使ってメソッドのシグネチャを定義しました。ある型がインターフェースのすべてのメソッドを実装していれば、その型は暗黙的にそのインターフェースを満たすと見なされました。

## Rust ではこう書く: トレイト
Rust のトレイトは Go のインターフェースに似ていますが、実装は**明示的**に行う必要があります。

### トレイトの定義
`trait` キーワードを使って、共有したいメソッドのシグネチャを定義します。
```rust
pub trait Summary {
    fn summarize(&self) -> String;
}
```
ここでは `summarize` というメソッドを定義しました。このメソッドは `&self` を引数に取り、`String` を返します。

### 型へのトレイトの実装
`impl Trait for Type` という構文で、特定の型に対してトレイトを実装します。
```rust
pub struct NewsArticle {
    pub headline: String,
    pub location: String,
    pub author: String,
    pub content: String,
}

impl Summary for NewsArticle {
    fn summarize(&self) -> String {
        format!("{}, by {} ({})", self.headline, self.author, self.location)
    }
}

pub struct Tweet {
    pub username: String,
    pub content: String,
    pub reply: bool,
    pub retweet: bool,
}

impl Summary for Tweet {
    fn summarize(&self) -> String {
        format!("{}: {}", self.username, self.content)
    }
}
```
これで `NewsArticle` と `Tweet` の両方が `summarize` メソッドを持つようになり、`Summary` トレイトを満たす型として扱えるようになりました。

### デフォルト実装
トレイトのメソッドには、デフォルトの振る舞いを実装しておくことができます。これにより、実装する型側でメソッドをオーバーライドしない限り、このデフォルトの振る舞いが使われます。
```rust
pub trait Summary {
    fn summarize_author(&self) -> String;

    fn summarize(&self) -> String {
        // デフォルト実装
        format!("(Read more from {}...)", self.summarize_author())
    }
}

// Tweet に実装する場合、summarize_author を実装するだけでよい
impl Summary for Tweet {
    fn summarize_author(&self) -> String {
        format!("@{}", self.username)
    }
}
// この Tweet インスタンスはデフォルトの summarize メソッドを使える
```

### トレイトを引数に取る
トレイトを関数の引数として使うことで、そのトレイトを実装している任意の型を受け取ることができます。これを**トレイト境界**と呼びます。
```rust
// `item` は Summary トレイトを実装した任意の型
pub fn notify(item: &impl Summary) {
    println!("Breaking news! {}", item.summarize());
}
```
この `notify` 関数は、`NewsArticle` のインスタンスも `Tweet` のインスタンスも、どちらも引数として受け取ることができます。

## よくあるエラーと対処法
### エラー 1: `the trait bound ... is not satisfied`
**原因:** トレイト境界を持つ関数に、そのトレイトを実装していない型の値を渡そうとしました。
**解決法:** 渡そうとしている型に、必要なトレイトを `impl` ブロックを使って実装してください。

## 練習問題
### 問題 1: `Display` トレイトの実装
`Point` という構造体を定義し、座標を `(x, y)` の形式で表示できるように、標準ライブラリの `std::fmt::Display` トレイトを実装してください。
```rust
struct Point {
    x: i32,
    y: i32,
}
// ここに Display トレイトを実装する
```

## この章のまとめ
- トレイトは、異なる型に共通の振る舞いを定義するための仕組み。
- `trait` キーワードで定義し、`impl Trait for Type` で実装する。
- デフォルト実装を提供することで、実装の手間を省ける場合がある。
- `impl Trait` 構文を使って、トレイトを実装した型を関数の引数として受け取れる (トレイト境界)。

## 次の章へ
トレイトは、特定の型だけでなく、あらゆる型に対して汎用的なコードを書くための「ジェネリクス」と組み合わせることで、その真価を発揮します。次の章では、ジェネリクスと、より複雑なトレイト境界について学びます。
