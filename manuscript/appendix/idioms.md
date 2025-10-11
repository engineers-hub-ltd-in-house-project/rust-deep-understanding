# 付録 A：Rust のイディオム集

この付録では、Rust コミュニティで広く使われている、または Rust の特性を活かした効果的なコーディングパターン（イディオム）をいくつか紹介します。

## `new` と `default`
- **`new()`:** 構造体のインスタンスを生成するための関連関数として、`new` という名前を使うのが一般的です。引数を取ることもよくあります。
- **`Default` トレイト:** 引数なしで「デフォルト」のインスタンスを生成できる型には、`Default` トレイトを実装するのが良い習慣です。これにより、`MyStruct::default()` のように呼び出せるほか、`..Default::default()` 構文で構造体の一部だけを初期化する際に便利です。

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

## `AsRef` と `Into`
- **`AsRef<T>`:** 所有権を消費せず、ある型から別の型への安価な参照変換を提供します。関数が `String` も `&str` も両方受け入れたい場合に、ジェネリックなトレイト境界 `T: AsRef<str>` を使うと便利です。
- **`Into<T>`:** 所有権を消費して、ある型から別の型へ変換します。`AsRef` よりも適用範囲が広いですが、値の所有権が移動します。

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

## RAII (Resource Acquisition Is Initialization)
Rust の所有権とドロップセマンティクスは、RAII パターンを自然に実現します。`File` や `MutexGuard` のように、オブジェクトが生成されたときにリソース（ファイルハンドルやロック）を獲得し、オブジェクトがスコープを抜けるときに自動的にリソースを解放します。これにより、リソースの解放し忘れを防ぎます。
