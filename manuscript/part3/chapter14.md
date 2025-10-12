# 第 14 章：トレイト入門

## この章のゴール
- コードの重複という具体的な問題に対し、トレイトがどのように解決策となるかを説明できる。
- `trait` を定義し、複数の異なる構造体に実装できる。
- `impl Trait` 構文を使い、トレイトを実装したあらゆる型を引数に取るジェネリックな関数を書ける。
- デフォルト実装を持つトレイトメソッドを定義・利用できる。

---

## 14.1 問題設定：異なる型に共通の振る舞いをさせたい

これまでに、`struct` や `enum` を使って独自の型を定義してきました。しかし、異なる型だけれども、どこか似たような振る舞いをさせたい場合はどうすればよいでしょうか？

例えば、SNSアプリケーションを考えてみましょう。タイムラインには、ニュース記事と個人のツイートの両方が流れてくるとします。どちらも「要約を表示する」という共通の機能が必要です。

`cargo new traits` でプロジェクトを作り、この問題を素朴に実装してみましょう。

```rust
// src/main.rs

pub struct NewsArticle {
    pub headline: String,
    pub author: String,
}

pub struct Tweet {
    pub username: String,
    pub content: String,
}

// ニュース記事を要約する関数
fn summarize_article(article: &NewsArticle) -> String {
    format!("{}, by {}", article.headline, article.author)
}

// ツイートを要約する関数
fn summarize_tweet(tweet: &Tweet) -> String {
    format!("{}: {}", tweet.username, tweet.content)
}

fn main() {
    let tweet = Tweet {
        username: String::from("horse_ebooks"),
        content: String::from("of course, as you probably already know, people"),
    };
    println!("1 new tweet: {}", summarize_tweet(&tweet));
}
```
これでも動きますが、大きな問題があります。
- **コードの重複**: `summarize_...` という似たような関数が型の数だけ増えていきます。
- **拡張性の欠如**: タイムラインに新しい種類のアイテム（例えばブログ投稿）を追加するたびに、新しい `summarize_...` 関数を作り、タイムラインを表示するロジックも修正しなければなりません。

この問題をエレガントに解決するのが **トレイト (Trait)** です。

## 14.2 解決策：`trait` で振る舞いを定義する

トレイトは、Goのインターフェースのように、特定の型が持つべき共通の振る舞いを定義します。

まず、`Summary` というトレイトを定義しましょう。これには `summarize` というメソッドのシグネチャ（型定義）が含まれます。

```rust
pub trait Summary {
    fn summarize(&self) -> String;
}
```

次に、このトレイトを各構造体に実装 (`impl`) します。

```rust
impl Summary for NewsArticle {
    fn summarize(&self) -> String {
        format!("{}, by {}", self.headline, self.author)
    }
}

impl Summary for Tweet {
    fn summarize(&self) -> String {
        format!("{}: {}", self.username, self.content)
    }
}
```
`impl Summary for NewsArticle` という構文は、「`NewsArticle` 型のために `Summary` トレイトを実装します」と読んでください。

これで、`NewsArticle` と `Tweet` は、どちらも `Summary` トレイトの振る舞いを持つ型となりました。

### 試してみよう：トレイトの力

トレイトを実装したことで、これらの型を抽象的に扱えるようになります。`impl Trait` 構文を使うと、「`Summary` トレイトを実装している任意の型」を引数に取る関数を定義できます。

```rust
// `item` は `Summary` トレイトを実装した任意の型の不変参照
pub fn notify(item: &impl Summary) {
    println!("Breaking news! {}", item.summarize());
}

fn main() {
    let tweet = Tweet {
        username: String::from("horse_ebooks"),
        content: String::from("of course, as you probably already know, people"),
    };

    let article = NewsArticle {
        headline: String::from("Penguins win the Stanley Cup Championship!"),
        author: String::from("Iceburgh"),
    };

    // notify 関数は Tweet と NewsArticle の両方を受け取れる！
    notify(&tweet);
    notify(&article);
}
```
`summarize_tweet` と `summarize_article` という個別の関数はもう必要ありません。`notify` 関数は、`summarize` メソッドを持つ型なら何でも受け取ってくれます。これがトレイトによる **ポリモーフィズム（多態性）** です。新しいアイテム（ブログ投稿など）を追加したくなったら、その構造体に `Summary` トレイトを実装するだけで、`notify` 関数はそのまま再利用できます。

## 14.3 デフォルト実装

トレイトのメソッドには、デフォルトの振る舞いを実装しておくこともできます。これにより、実装する型側でメソッドをオーバーライドしない限り、このデフォルトの振る舞いが使われます。

```rust
pub trait Summary {
    fn summarize_author(&self) -> String;

    // デフォルト実装を持つメソッド
    fn summarize(&self) -> String {
        format!("(Read more from {}...)", self.summarize_author())
    }
}

// Tweet に実装する場合、summarize_author を実装するだけでよい
impl Summary for Tweet {
    fn summarize_author(&self) -> String {
        format!("@{}", self.username)
    }
}

// main 関数で tweet.summarize() を呼ぶと、デフォルト実装が使われる
// -> "(Read more from @horse_ebooks...)"
```
`Tweet` の `impl` ブロックでは `summarize` を定義していませんが、トレイトのデフォルト実装が利用できます。ただし、デフォルト実装は、同じトレイト内の他のメソッド（この場合は`summarize_author`）を呼び出すことができますが、その逆はできません。

## 14.4 まとめ

- トレイトは、異なる型に共通の振る舞いを定義するための仕組みで、Go のインターフェースに似ている。
- `trait` キーワードで定義し、`impl Trait for Type` で明示的に実装する。
- `impl Trait` 構文を使うと、トレイトを実装した型を関数の引数として抽象的に受け取れる。これにより、コードの再利用性が劇的に向上する。
- デフォルト実装を提供することで、トレイト実装の手間を省いたり、共通の振る舞いを提供したりできる。

---

トレイトは、Rustの型システムの心臓部とも言える機能です。次の章では、トレイトと並ぶもう一つの重要な抽象化機能、「ジェネリクス」と、この2つを組み合わせた「トレイト境界」について学びます。
