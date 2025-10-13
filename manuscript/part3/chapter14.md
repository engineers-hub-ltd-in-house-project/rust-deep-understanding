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

```sh
cargo new traits
```
でプロジェクトを作り、この問題を素朴に実装してみましょう。

```rust
// src/main.rs

struct NewsArticle {
    headline: String,
    author: String,
}

struct Tweet {
    username: String,
    content: String,
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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0A%0Astruct%20NewsArticle%20%7B%0A%20%20%20%20headline%3A%20String%2C%0A%20%20%20%20author%3A%20String%2C%0A%7D%0A%0Astruct%20Tweet%20%7B%0A%20%20%20%20username%3A%20String%2C%0A%20%20%20%20content%3A%20String%2C%0A%7D%0A%0A//%20%E3%83%8B%E3%83%A5%E3%83%BC%E3%82%B9%E8%A8%98%E4%BA%8B%E3%82%92%E8%A6%81%E7%B4%84%E3%81%99%E3%82%8B%E9%96%A2%E6%95%B0%0Afn%20summarize_article%28article%3A%20%26NewsArticle%29%20-%3E%20String%20%7B%0A%20%20%20%20format%21%28%22%7B%7D%2C%20by%20%7B%7D%22%2C%20article.headline%2C%20article.author%29%0A%7D%0A%0A//%20%E3%83%84%E3%82%A4%E3%83%BC%E3%83%88%E3%82%92%E8%A6%81%E7%B4%84%E3%81%99%E3%82%8B%E9%96%A2%E6%95%B0%0Afn%20summarize_tweet%28tweet%3A%20%26Tweet%29%20-%3E%20String%20%7B%0A%20%20%20%20format%21%28%22%7B%7D%3A%20%7B%7D%22%2C%20tweet.username%2C%20tweet.content%29%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20tweet%20%3D%20Tweet%20%7B%0A%20%20%20%20%20%20%20%20username%3A%20String%3A%3Afrom%28%22horse_ebooks%22%29%2C%0A%20%20%20%20%20%20%20%20content%3A%20String%3A%3Afrom%28%22of%20course%2C%20as%20you%20probably%20already%20know%2C%20people%22%29%2C%0A%20%20%20%20%7D%3B%0A%20%20%20%20println%21%28%221%20new%20tweet%3A%20%7B%7D%22%2C%20summarize_tweet%28%26tweet%29%29%3B%0A%7D)
これでも動きますが、大きな問題があります。
- **コードの重複**: `summarize_...` という似たような関数が型の数だけ増えていきます。
- **拡張性の欠如**: タイムラインに新しい種類のアイテム（例えばブログ投稿）を追加するたびに、新しい `summarize_...` 関数を作り、タイムラインを表示するロジックも修正しなければなりません。

この問題をエレガントに解決するのが トレイト (Trait) です。

## 14.2 解決策：`trait` で振る舞いを定義する

トレイトは、Goのインターフェースのように、特定の型が持つべき共通の振る舞いを定義します。

まず、`Summary` というトレイトを定義しましょう。これには `summarize` というメソッドのシグネチャ（型定義）が含まれます。

```rust
trait Summary {
    fn summarize(&self) -> String;
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=trait%20Summary%20%7B%0A%20%20%20%20fn%20summarize%28%26self%29%20-%3E%20String%3B%0A%7D)

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=impl%20Summary%20for%20NewsArticle%20%7B%0A%20%20%20%20fn%20summarize%28%26self%29%20-%3E%20String%20%7B%0A%20%20%20%20%20%20%20%20format%21%28%22%7B%7D%2C%20by%20%7B%7D%22%2C%20self.headline%2C%20self.author%29%0A%20%20%20%20%7D%0A%7D%0A%0Aimpl%20Summary%20for%20Tweet%20%7B%0A%20%20%20%20fn%20summarize%28%26self%29%20-%3E%20String%20%7B%0A%20%20%20%20%20%20%20%20format%21%28%22%7B%7D%3A%20%7B%7D%22%2C%20self.username%2C%20self.content%29%0A%20%20%20%20%7D%0A%7D)
`impl Summary for NewsArticle` という構文は、「`NewsArticle` 型のために `Summary` トレイトを実装します」と読んでください。

これで、`NewsArticle` と `Tweet` は、どちらも `Summary` トレイトの振る舞いを持つ型となりました。

### 試してみよう：トレイトの力

トレイトを実装したことで、これらの型を抽象的に扱えるようになります。`impl Trait` 構文を使うと、「`Summary` トレイトを実装している任意の型」を引数に取る関数を定義できます。

```rust
// `item` は `Summary` トレイトを実装した任意の型の不変参照
fn notify(item: &impl Summary) {
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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=pub%20trait%20Summary%20%7B%0A%20%20%20%20fn%20summarize%28%26self%29%20-%3E%20String%3B%0A%7D%0A%0Apub%20struct%20NewsArticle%20%7B%0A%20%20%20%20pub%20headline%3A%20String%2C%0A%20%20%20%20pub%20author%3A%20String%2C%0A%7D%0A%0Aimpl%20Summary%20for%20NewsArticle%20%7B%0A%20%20%20%20fn%20summarize%28%26self%29%20-%3E%20String%20%7B%0A%20%20%20%20%20%20%20%20format!%28%22%7B%7D%2C%20by%20%7B%7D%22%2C%20self.headline%2C%20self.author%29%0A%20%20%20%20%7D%0A%7D%0A%0Apub%20struct%20Tweet%20%7B%0A%20%20%20%20pub%20username%3A%20String%2C%0A%20%20%20%20pub%20content%3A%20String%2C%0A%7D%0A%0Aimpl%20Summary%20for%20Tweet%20%7B%0A%20%20%20%20fn%20summarize%28%26self%29%20-%3E%20String%20%7B%0A%20%20%20%20%20%20%20%20format!%28%22%7B%7D%3A%20%7B%7D%22%2C%20self.username%2C%20self.content%29%0A%20%20%20%20%7D%0A%7D%0A//%20%60item%60%20%E3%81%AF%20%60Summary%60%20%E3%83%88%E3%83%AC%E3%82%A4%E3%83%88%E3%82%92%E5%AE%9F%E8%A3%85%E3%81%97%E3%81%9F%E4%BB%BB%E6%84%8F%E3%81%AE%E5%9E%8B%E3%81%AE%E4%B8%8D%E5%A4%89%E5%8F%82%E7%85%A7%0Afn%20notify%28item%3A%20%26impl%20Summary%29%20%7B%0A%20%20%20%20println!%28%22Breaking%20news!%20%7B%7D%22%2C%20item.summarize%28%29%29%3B%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20tweet%20%3D%20Tweet%20%7B%0A%20%20%20%20%20%20%20%20username%3A%20String%3A%3Afrom%28%22horse_ebooks%22%29%2C%0A%20%20%20%20%20%20%20%20content%3A%20String%3A%3Afrom%28%22of%20course%2C%20as%20you%20probably%20already%20know%2C%20people%22%29%2C%0A%20%20%20%20%7D%3B%0A%0A%20%20%20%20let%20article%20%3D%20NewsArticle%20%7B%0A%20%20%20%20%20%20%20%20headline%3A%20String%3A%3Afrom%28%22Penguins%20win%20the%20Stanley%20Cup%20Championship!%22%29%2C%0A%20%20%20%20%20%20%20%20author%3A%20String%3A%3Afrom%28%22Iceburgh%22%29%2C%0A%20%20%20%20%7D%3B%0A%0A%20%20%20%20//%20notify%20%E9%96%A2%E6%95%B0%E3%81%AF%20Tweet%20%E3%81%A8%20NewsArticle%20%E3%81%AE%E4%B8%A1%E6%96%B9%E3%82%92%E5%8F%97%E3%81%91%E5%8F%96%E3%82%8C%E3%82%8B%EF%BC%81%0A%20%20%20%20notify%28%26tweet%29%3B%0A%20%20%20%20notify%28%26article%29%3B%0A%7D)
`summarize_tweet` と `summarize_article` という個別の関数はもう必要ありません。`notify` 関数は、`summarize` メソッドを持つ型なら何でも受け取ってくれます。これがトレイトによる ポリモーフィズム（多態性） です。新しいアイテム（ブログ投稿など）を追加したくなったら、その構造体に `Summary` トレイトを実装するだけで、`notify` 関数はそのまま再利用できます。

## 14.3 デフォルト実装

トレイトのメソッドには、デフォルトの振る舞いを実装しておくこともできます。これにより、実装する型側でメソッドをオーバーライドしない限り、このデフォルトの振る舞いが使われます。

```rust
trait Summary {
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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=trait%20Summary%20%7B%0A%20%20%20%20fn%20summarize_author%28%26self%29%20-%3E%20String%3B%0A%0A%20%20%20%20//%20%E3%83%87%E3%83%95%E3%82%A9%E3%83%AB%E3%83%88%E5%AE%9F%E8%A3%85%E3%82%92%E6%8C%81%E3%81%A4%E3%83%A1%E3%82%BD%E3%83%83%E3%83%89%0A%20%20%20%20fn%20summarize%28%26self%29%20-%3E%20String%20%7B%0A%20%20%20%20%20%20%20%20format!%28%22%28Read%20more%20from%20%7B%7D...%29%22%2C%20self.summarize_author%28%29%29%0A%20%20%20%20%7D%0A%7D%0A%0A//%20Tweet%20%E3%81%AB%E5%AE%9F%E8%A3%85%E3%81%99%E3%82%8B%E5%A0%B4%E5%90%88%E3%80%81summarize_author%20%E3%82%92%E5%AE%9F%E8%A3%85%E3%81%99%E3%82%8B%E3%81%A0%E3%81%91%E3%81%A7%E3%82%88%E3%81%84%0Aimpl%20Summary%20for%20Tweet%20%7B%0A%20%20%20%20fn%20summarize_author%28%26self%29%20-%3E%20String%20%7B%0A%20%20%20%20%20%20%20%20format!%28%22%40%7B%7D%22%2C%20self.username%29%0A%20%20%20%20%7D%0A%7D%0A%0A//%20main%20%E9%96%A2%E6%95%B0%E3%81%A7%20tweet.summarize%28%29%20%E3%82%92%E5%91%BC%E3%81%B6%E3%81%A8%E3%80%81%E3%83%87%E3%83%95%E3%82%A9%E3%83%AB%E3%83%88%E5%AE%9F%E8%A3%85%E3%81%8C%E4%BD%BF%E3%82%8F%E3%82%8C%E3%82%8B%0A//%20-%3E%20%22%28Read%20more%20from%20%40horse_ebooks...%29%22)
`Tweet` の `impl` ブロックでは `summarize` を定義していませんが、トレイトのデフォルト実装が利用できます。ただし、デフォルト実装は、同じトレイト内の他のメソッド（この場合は`summarize_author`）を呼び出すことができますが、その逆はできません。

## 14.4 まとめ

- トレイトは、異なる型に共通の振る舞いを定義するための仕組みで、Go のインターフェースに似ている。
- `trait` キーワードで定義し、`impl Trait for Type` で明示的に実装する。
- `impl Trait` 構文を使うと、トレイトを実装した型を関数の引数として抽象的に受け取れる。これにより、コードの再利用性が劇的に向上する。
- デフォルト実装を提供することで、トレイト実装の手間を省いたり、共通の振る舞いを提供したりできる。

---

トレイトは、Rustの型システムの心臓部とも言える機能です。次の章では、トレイトと並ぶもう一つの重要な抽象化機能、「ジェネリクス」と、この2つを組み合わせた「トレイト境界」について学びます。
