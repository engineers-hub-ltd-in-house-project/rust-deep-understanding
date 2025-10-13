# 第 20 章：テストの記述

## この章のゴール
- `cargo test` を実行し、テストの成功と失敗の出力を読み解ける。
- `#[test]` 属性と `assert_eq!` マクロを使って、関数の振る舞いを検証する単体テストを記述できる。
- 意図的にテストを失敗させ、Rust のテストランナーが出力する差分情報の見方を説明できる。
- `#[should_panic]` 属性を使って、エラー時に正しくパニックすることをテストできる。
- `tests` ディレクトリを作成し、ライブラリの公開APIをテストする統合テストを記述できる。

---

## 20.1 問題設定：そのコード、本当に動きますか？

これまでの章で、私たちは多くの関数や構造体を書いてきました。しかし、それらが本当に期待通りに動作するか、どうやって確認してきたでしょうか？多くの場合、`main` 関数の中で `println!` を使って結果を出力し、目で見て確認する、という方法を採ってきました。

```sh
cargo new adder
```
でプロジェクトを作り、この「手動テスト」の問題点を体験してみましょう。

```rust
// src/lib.rs
pub fn add_two(a: i32) -> i32 {
    a + 2
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/lib.rs%0Apub%20fn%20add_two%28a%3A%20i32%29%20-%3E%20i32%20%7B%0A%20%20%20%20a%20%2B%202%0A%7D)
```rust
// src/main.rs
use adder::add_two;

fn main() {
    let result = add_two(2);
    println!("Result of add_two(2) is: {}", result);
    // これが 4 になっているか、毎回目で見て確認する必要がある...
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Ause%20adder%3A%3Aadd_two%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20result%20%3D%20add_two%282%29%3B%0A%20%20%20%20println%21%28%22Result%20of%20add_two%282%29%20is%3A%20%7B%7D%22%2C%20result%29%3B%0A%20%20%20%20//%20%E3%81%93%E3%82%8C%E3%81%8C%204%20%E3%81%AB%E3%81%AA%E3%81%A3%E3%81%A6%E3%81%84%E3%82%8B%E3%81%8B%E3%80%81%E6%AF%8E%E5%9B%9E%E7%9B%AE%E3%81%A7%E8%A6%8B%E3%81%A6%E7%A2%BA%E8%AA%8D%E3%81%99%E3%82%8B%E5%BF%85%E8%A6%81%E3%81%8C%E3%81%82%E3%82%8B...%0A%7D)

この方法には問題があります。
- 面倒: 機能が増えるたびに `main` 関数が複雑になる。
- 見落とし: 目での確認には限界があり、小さな間違いを見逃す可能性がある。
- 回帰の恐怖: コードを修正した際、以前は動いていた他の機能が壊れていないか（デグレードしていないか）を確認するのが非常に困難。

この問題を解決するのが、自動テストです。

## 20.2 解決策：`cargo test` で自動検証する

Rust は、言語と `cargo` に強力なテスト機能を組み込んでいます。`#[test]` 属性を付けるだけで、その関数がテストであることを示し、`cargo test` コマンドで一括実行できます。

### 単体テスト (Unit Tests)

単体テストは、個々の関数などの小さな単位（ユニット）を、他の部分から隔離してテストするものです。通常、テスト対象コードと同じファイル内の `#[cfg(test)]` でアノテートされた `tests` モジュールに記述します。

### 試してみよう：テストを失敗させてみる

テストで最も重要なのは、テストが失敗したときの挙動です。テストが失敗したときに、何が、なぜ間違っているのかを明確に教えてくれることが、優れたテストフレームワークの条件です。

`src/lib.rs` にテストコードを追加してみましょう。

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
`assert_eq!(left, right)` は、`left` と `right` が等しいことを表明 (assert) するマクロです。もし等しくなければ、スレッドはパニックし、テストは失敗したとマークされます。

`cargo test` を実行してみましょう。

```text
running 2 tests
test tests::it_works ... ok
test tests::add_two_test ... FAILED

failures:

---- tests::add_two_test stdout ----
thread 'tests::add_two_test' panicked at 'assertion failed: `(left == right)`
  left: `4`,
 right: `5`', src/lib.rs:18:9
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace

failures:
    tests::add_two_test

test result: FAILED. 1 passed; 1 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```
出力は非常に明確です。`add_two_test` が失敗したこと、そしてその理由が `left` (実際の値) は `4` だったのに対し、`right` (期待した値) は `5` だった、という差分情報から一目瞭然です。これが自動テストの力です。

### `should_panic` でパニックをテストする

意図通りに `panic!` することも、プログラムの重要な振る舞いです。`#[should_panic]` 属性は、そのテスト関数内でパニックが起きたらテストを成功とみなします。

```rust
// src/lib.rs
// ...
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

## 20.3 統合テスト (Integration Tests)

統合テストは、ライブラリを外部の利用者の視点からテストするものです。ライブラリの公開APIが正しく組み合わさって機能するかを検証します。

### 試してみよう：統合テストファイルを作成する

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
`cargo test` を実行すると、Cargo は単体テストに加えて `tests` ディレクトリ内のテストも自動的に実行してくれます。

## 20.4 まとめ

- テストは、コードの振る舞いを自動で検証し、リファクタリングや機能追加の安全性を高める。
- 単体テスト は `src` 内の `#[cfg(test)] mod tests` に書き、個々の機能をテストする。
- 統合テスト は `tests` ディレクトリに書き、ライブラリの公開APIを外部からテストする。
- `assert_eq!` マクロは2つの値が等しいか検証し、違う場合は分かりやすい差分を表示して失敗する。
- `#[should_panic]` 属性は、コードが意図通りにパニックすることを検証する。

---

テストによってコードの正しさを保証する方法を学びました。コードの品質をさらに高めるには、他の開発者がコードを理解しやすくするためのドキュメントが重要です。次の章では、効果的なドキュメントの書き方と、それをテストする方法について学びます。

