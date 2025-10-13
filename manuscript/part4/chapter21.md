# 第 21 章：テストの書き方

## この章のゴール
- `#[test]` 属性を使って、関数の正常系をテストする単体テストが書ける。
- `cargo test -- --test-threads=1` や `cargo test -- --show-output` などのコマンドラインオプションを使い、テストの実行を制御できる。
- `tests` ディレクトリに統合テストを配置し、ライブラリの公開APIをテストできる。

---

## 21.1 なぜテストを書くのか？

テストは、コードが期待通りに動作することを保証するための自動化されたプログラムです。テストを書くことで、以下のようなメリットがあります。
- **リファクタリングへの自信**: コードの内部実装を変更（リファクタリング）した際に、既存の機能が壊れていないことをすぐに確認できます。

Rustのテスト機能は言語に組み込まれており、外部のテスティングフレームワークを追加する必要なく、すぐにテストを書き始めることができます。

## 21.2 単体テスト：`#[test]` 属性

最も基本的なテストは、`#[test]` 属性を付けた関数です。この関数は、`cargo test` を実行したときにテストランナーによって呼び出されます。

```sh
cargo new adder
```
でプロジェクトを作り、簡単な足し算ライブラリのテストを書いてみましょう。

### 正常系テスト

```rust
// src/lib.rs
pub fn add_two(a: i32) -> i32 {
    a + 2
}

// この属性は、`cargo test` を実行した時にだけコンパイルされることを示す
#[cfg(test)]
mod tests {
    use super::*; // 親モジュール（このファイルの上半分）のアイテムをインポート

    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }

    #[test]
    fn add_two_test() {
        // わざと失敗させてみよう！
        assert_eq!(add_two(2), 5); 
    }
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/lib.rs%0Apub%20fn%20add_two%28a%3A%20i32%29%20-%3E%20i32%20%7B%0A%20%20%20%20a%20%2B%202%0A%7D%0A%0A//%20%E3%81%93%E3%81%AE%E5%B1%9E%E6%80%A7%E3%81%AF%E3%80%81%60cargo%20test%60%20%E3%82%92%E5%AE%9F%E8%A1%8C%E3%81%97%E3%81%9F%E6%99%82%E3%81%AB%E3%81%A0%E3%81%91%E3%82%B3%E3%83%B3%E3%83%91%E3%82%A4%E3%83%AB%E3%81%95%E3%82%8C%E3%82%8B%E3%81%93%E3%81%A8%E3%82%92%E7%A4%BA%E3%81%99%0A%23%5Bcfg%28test%29%5D%0Amod%20tests%20%7B%0A%20%20%20%20use%20super%3A%3A%2A%3B%20//%20%E8%A6%AA%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB%EF%BC%88%E3%81%93%E3%81%AE%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%81%AE%E4%B8%8A%E5%8D%8A%E5%88%86%EF%BC%89%E3%81%AE%E3%82%A2%E3%82%A4%E3%83%86%E3%83%A0%E3%82%92%E3%82%A4%E3%83%B3%E3%83%9D%E3%83%BC%E3%83%88%0A%0A%20%20%20%20%23%5Btest%5D%0A%20%20%20%20fn%20it_works%28%29%20%7B%0A%20%20%20%20%20%20%20%20assert_eq%21%282%20%2B%202%2C%204%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20%23%5Btest%5D%0A%20%20%20%20fn%20add_two_test%28%29%20%7B%0A%20%20%20%20%20%20%20%20//%20%E3%82%8F%E3%81%96%E3%81%A8%E5%A4%B1%E6%95%97%E3%81%95%E3%81%9B%E3%81%A6%E3%81%BF%E3%82%88%E3%81%86%EF%BC%81%0A%20%20%20%20%20%20%20%20assert_eq%21%28add_two%282%29%2C%205%29%3B%20%0A%20%20%20%20%7D%0A%7D)
`cargo test` を実行すると、`tests` モジュール内の `it_works` 関数が実行され、`assert_eq!` がパニックしなければテストは成功（`ok`）と表示されます。

### 異常系テスト：`#[should_panic]`

特定の条件下でコードが意図通りにパニックするかどうかをテストすることも重要です。`#[should_panic]` 属性を使うと、そのテスト関数がパニックすれば成功、パニックしなければ失敗とみなされます。

```rust
// src/lib.rs
pub struct Guess {
    value: i32,
}

impl Guess {
    pub fn new(value: i32) -> Guess {
        if value < 1 || value > 100 {
            panic!("Guess value must be between 1 and 100, got {}.", value);
        }
        Guess { value }
    }
}

#[cfg(test)]
mod tests {
    // ...
    #[test]
    #[should_panic]
    fn greater_than_100() {
        Guess::new(200);
    }
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/lib.rs%0A//%20...%0Apub%20struct%20Guess%20%7B%0A%20%20%20%20value%3A%20i32%2C%0A%7D%0A%0Aimpl%20Guess%20%7B%0A%20%20%20%20pub%20fn%20new%28value%3A%20i32%29%20-%3E%20Guess%20%7B%0A%20%20%20%20%20%20%20%20if%20value%20%3C%201%20%7C%7C%20value%20%3E%20100%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20panic%21%28%22Guess%20value%20must%20be%20between%201%20and%20100%2C%20got%20%7B%7D.%22%2C%20value%29%3B%0A%20%20%20%20%20%20%20%20%7D%0A%20%20%20%20%20%20%20%20Guess%20%7B%20value%20%7D%0A%20%20%20%20%7D%0A%7D%0A%0A%23%5Bcfg%28test%29%5D%0Amod%20tests%20%7B%0A%20%20%20%20//%20...%0A%20%20%20%20%23%5Btest%5D%0A%20%20%20%20%23%5Bshould_panic%5D%0A%20%20%20%20fn%20greater_than_100%28%29%20%7B%0A%20%20%20%20%20%20%20%20Guess%3A%3Anew%28200%29%3B%0A%20%20%20%20%7D%0A%7D)
このテストは、`Guess::new(200)` がパニックするので成功します。

## 21.3 `cargo test` の使い方

`cargo test` には、テストの実行を細かく制御するための便利なオプションがあります。
- `cargo test -- --test-threads=1`: テストを並列実行せず、1スレッドで実行します。テストが標準出力に何かを出力する場合に、表示が混ざらないようにするのに便利です。
- `cargo test -- --show-output`: 成功したテストも含め、すべてのテストの標準出力を表示します。

## 21.4 統合テスト

- **単体テスト (Unit Tests)**: 小さな単位（個々の関数など）を隔離してテストする。プライベートな関数もテストできる。`src` 内に `#[cfg(test)]` を付けた `mod tests` を作るのが慣例。
- **統合テスト (Integration Tests)**: ライブラリを外部の利用者の視点からテストする。公開されたAPIのみをテストできる。パッケージルート（`Cargo.toml` と同じ階層）に `tests` ディレクトリを作成し、その中に `.rs` ファイルを置くのが慣例。

### 試してみよう：`tests` ディレクトリの作成

1.  プロジェクトのルートディレクトリに `tests` という名前のディレクトリを作成します。
2.  `tests/integration_test.rs` というファイルを作成します。

```rust
// tests/integration_test.rs

// クレート名を `use` でインポートする
use adder;

#[test]
fn it_adds_two_integration() {
    assert_eq!(4, adder::add_two(2));
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20tests/integration_test.rs%0A%0A//%20%E3%82%AF%E3%83%AC%E3%83%BC%E3%83%88%E5%90%8D%E3%82%92%20%60use%60%20%E3%81%A7%E3%82%A4%E3%83%B3%E3%83%9D%E3%83%BC%E3%83%88%E3%81%99%E3%82%8B%0Ause%20adder%3B%0A%0A%23%5Btest%5D%0Afn%20it_adds_two_integration%28%29%20%7B%0A%20%20%20%20assert_eq%21%284%2C%20adder%3A%3Aadd_two%282%29%29%3B%0A%7D)
`cargo test` を実行すると、単体テストと統合テストの両方が実行されます。

## 21.5 まとめ

- Rustのテストは、`#[test]` 属性を関数に付けるだけで簡単に書ける。
- `assert!`, `assert_eq!`, `assert_ne!` マクロは、テストの条件を検証するための基本的なツール。
- `#[should_panic]` 属性は、コードが意図通りにエラー（パニック）を起こすことを検証する。
- `cargo test` は、単体テストと統合テストの両方を実行するコマンド。特定のテストだけを実行したり、出力を制御したりするオプションがある。
- 単体テストは `src` 内に、統合テストはルートの `tests` ディレクトリに配置するのが慣例。

---

テストを書く習慣は、堅牢で信頼性の高いソフトウェアを開発するための基礎です。次の章では、Rust の `Result` 型と `?` 演算子を使った、より実践的なエラーハンドリングの方法について学びます。

