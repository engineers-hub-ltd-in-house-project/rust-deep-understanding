# 付録 A：Rust のイディオム集

この付録では、Rust コミュニティで広く使われている、または Rust の特性を活かした効果的なコーディングパターン（イディオム）をいくつか紹介します。

## `new` と `default`
- `new()`: 構造体のインスタンスを生成するための関連関数として、`new` という名前を使うのが一般的です。引数を取ることもよくあります。
- `Default` トレイト: 引数なしで「デフォルト」のインスタンスを生成できる型には、`Default` トレイトを実装するのが良い習慣です。これにより、`MyStruct::default()` のように呼び出せるほか、`..Default::default()` 構文で構造体の一部だけを初期化する際に便利です。

## `Result` と `?` 演算子
エラーを返す可能性のある関数では、`Result<T, E>` を返すのが標準です。関数内で他の `Result` を返す関数を呼び出す際は、`?` 演算子を使ってエラーの早期リターンを簡潔に記述します。
```rust
use std::fs;
use std::io;

fn read_username_from_file() -> Result<String, io::Error> {
    let username = fs::read_to_string("hello.txt")?;
    Ok(username)
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Afs%3B%0Ause%20std%3A%3Aio%3B%0A%0Afn%20read_username_from_file%28%29%20-%3E%20Result%3CString%2C%20io%3A%3AError%3E%20%7B%0A%20%20%20%20let%20username%20%3D%20fs%3A%3Aread_to_string%28%22hello.txt%22%29%3F%3B%0A%20%20%20%20Ok%28username%29%0A%7D)

## `AsRef` と `Into`
- `AsRef<T>`: 所有権を消費せず、ある型から別の型への安価な参照変換を提供します。関数が `String` も `&str` も両方受け入れたい場合に、ジェネリックなトレイト境界 `T: AsRef<str>` を使うと便利です。
- `Into<T>`: 所有権を消費して、ある型から別の型へ変換します。`AsRef` よりも適用範囲が広いですが、値の所有権が移動します。

## `Builder` パターン
多くの設定オプションを持つ構造体を生成する際に、`Builder` パターンが役立ちます。これにより、型安全性を保ちつつ、可読性の高い方法でインスタンスを構築できます。
```rust
struct Food {
    name: String,
    calories: u32,
}

struct FoodBuilder {
    name: String,
    calories: Option<u32>,
}

impl FoodBuilder {
    pub fn new(name: impl Into<String>) -> Self {
        FoodBuilder { name: name.into(), calories: None }
    }
    
    pub fn calories(mut self, calories: u32) -> Self {
        self.calories = Some(calories);
        self
    }
    
    pub fn build(self) -> Food {
        Food {
            name: self.name,
            calories: self.calories.unwrap_or(0),
        }
    }
}

// 使用例
let pizza = FoodBuilder::new("Pizza").calories(800).build();
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=struct%20Food%20%7B%0A%20%20%20%20name%3A%20String%2C%0A%20%20%20%20calories%3A%20u32%2C%0A%7D%0A%0Astruct%20FoodBuilder%20%7B%0A%20%20%20%20name%3A%20String%2C%0A%20%20%20%20calories%3A%20Option%3Cu32%3E%2C%0A%7D%0A%0Aimpl%20FoodBuilder%20%7B%0A%20%20%20%20pub%20fn%20new%28name%3A%20impl%20Into%3CString%3E%29%20-%3E%20Self%20%7B%0A%20%20%20%20%20%20%20%20FoodBuilder%20%7B%20name%3A%20name.into%28%29%2C%20calories%3A%20None%20%7D%0A%20%20%20%20%7D%0A%20%20%20%20%0A%20%20%20%20pub%20fn%20calories%28mut%20self%2C%20calories%3A%20u32%29%20-%3E%20Self%20%7B%0A%20%20%20%20%20%20%20%20self.calories%20%3D%20Some%28calories%29%3B%0A%20%20%20%20%20%20%20%20self%0A%20%20%20%20%7D%0A%20%20%20%20%0A%20%20%20%20pub%20fn%20build%28self%29%20-%3E%20Food%20%7B%0A%20%20%20%20%20%20%20%20Food%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20name%3A%20self.name%2C%0A%20%20%20%20%20%20%20%20%20%20%20%20calories%3A%20self.calories.unwrap_or%280%29%2C%0A%20%20%20%20%20%20%20%20%7D%0A%20%20%20%20%7D%0A%7D%0A%0A//%20%E4%BD%BF%E7%94%A8%E4%BE%8B%0Alet%20pizza%20%3D%20FoodBuilder%3A%3Anew%28%22Pizza%22%29.calories%28800%29.build%28%29%3B)

## `Type State` パターン
型システムを使って、オブジェクトの状態遷移をコンパイル時に保証するパターンです。例えば、「下書き」状態のブログ投稿は公開できず、「レビュー済み」状態のものだけが公開できる、といったルールを型で表現します。
```rust
struct Post { content: String }
struct DraftPost { content: String }

impl DraftPost {
    // レビューをリクエストすると、レビュー待ちの投稿になる
    pub fn request_review(self) -> PendingReviewPost { /* ... */ }
}

struct PendingReviewPost { content: String }

impl PendingReviewPost {
    // 承認されると、公開可能な投稿になる
    pub fn approve(self) -> Post { /* ... */ }
}

impl Post {
    // この状態でのみ content を取得できる
    pub fn content(&self) -> &str { &self.content }
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=struct%20Post%20%7B%20content%3A%20String%20%7D%0Astruct%20DraftPost%20%7B%20content%3A%20String%20%7D%0A%0Aimpl%20DraftPost%20%7B%0A%20%20%20%20//%20%E3%83%AC%E3%83%93%E3%83%A5%E3%83%BC%E3%82%92%E3%83%AA%E3%82%AF%E3%82%A8%E3%82%B9%E3%83%88%E3%81%99%E3%82%8B%E3%81%A8%E3%80%81%E3%83%AC%E3%83%93%E3%83%A5%E3%83%BC%E5%BE%85%E3%81%A1%E3%81%AE%E6%8A%95%E7%A8%BF%E3%81%AB%E3%81%AA%E3%82%8B%0A%20%20%20%20pub%20fn%20request_review%28self%29%20-%3E%20PendingReviewPost%20%7B%20/%2A%20...%20%2A/%20%7D%0A%7D%0A%0Astruct%20PendingReviewPost%20%7B%20content%3A%20String%20%7D%0A%0Aimpl%20PendingReviewPost%20%7B%0A%20%20%20%20//%20%E6%89%BF%E8%AA%8D%E3%81%95%E3%82%8C%E3%82%8B%E3%81%A8%E3%80%81%E5%85%AC%E9%96%8B%E5%8F%AF%E8%83%BD%E3%81%AA%E6%8A%95%E7%A8%BF%E3%81%AB%E3%81%AA%E3%82%8B%0A%20%20%20%20pub%20fn%20approve%28self%29%20-%3E%20Post%20%7B%20/%2A%20...%20%2A/%20%7D%0A%7D%0A%0Aimpl%20Post%20%7B%0A%20%20%20%20//%20%E3%81%93%E3%81%AE%E7%8A%B6%E6%85%8B%E3%81%A7%E3%81%AE%E3%81%BF%20content%20%E3%82%92%E5%8F%96%E5%BE%97%E3%81%A7%E3%81%8D%E3%82%8B%0A%20%20%20%20pub%20fn%20content%28%26self%29%20-%3E%20%26str%20%7B%20%26self.content%20%7D%0A%7D)

## RAII (Resource Acquisition Is Initialization)
Rust の所有権とドロップセマンティクスは、RAII パターンを自然に実現します。`File` や `MutexGuard` のように、オブジェクトが生成されたときにリソース（ファイルハンドルやロック）を獲得し、オブジェクトがスコープを抜けるときに自動的にリソースを解放します。これにより、リソースの解放し忘れを防ぎます。
