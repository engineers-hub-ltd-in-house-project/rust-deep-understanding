# 第 21 章：テストの書き方

## この章のゴール
- `#[test]` 属性を使って、関数の正常系をテストする単体テストが書ける。
- `cargo test -- --test-threads=1` や `cargo test -- --show-output` などのコマンドラインオプションを使い、テストの実行を制御できる。
- `tests` ディレクトリに統合テストを配置し、ライブラリの公開APIをテストできる。

---

## 21.1 なぜテストを書くのか？

テストは、コードが期待通りに動作することを保証するための自動化されたプログラムです。テストを書くことで、以下のようなメリットがあります。
- リファクタリングへの自信: コードの内部実装を変更（リファクタリング）した際に、既存の機能が壊れていないことをすぐに確認できます。

Rustのテスト機能は言語に組み込まれており、外部のテスティングフレームワークを追加する必要なく、すぐにテストを書き始めることができます。

## 21.2 単体テスト：実践的なビジネスロジックの例

`#[test]` 属性を使った最も基本的なテストから見ていきましょう。ここでは、単純な足し算ではなく、ECサイトで使われるような、もう少し現実的な割引率を計算する関数を例に取ります。

```sh
cargo new discount_calculator
```
でプロジェクトを作り、`src/lib.rs` にコードを書いていきます。

### ビジネスロジックとテストケース

プレミアム会員かどうかで割引率が変わるロジックを考えます。

```rust
// src/lib.rs
pub struct User {
    is_premium: bool,
}

// プレミアム会員は20%割引、通常会員は10%割引
pub fn calculate_discount(user: &User, price: u32) -> u32 {
    let rate = if user.is_premium { 0.8 } else { 0.9 };
    apply_percentage(price, rate)
}

// 割引率を適用するプライベート関数
fn apply_percentage(price: u32, rate: f64) -> u32 {
    (price as f64 * rate) as u32
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn premium_user_gets_20_percent_discount() {
        let user = User { is_premium: true };
        assert_eq!(calculate_discount(&user, 1000), 800);
    }

    #[test]
    fn normal_user_gets_10_percent_discount() {
        let user = User { is_premium: false };
        assert_eq!(calculate_discount(&user, 1000), 900);
    }
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=pub%20struct%20User%20%7B%0A%20%20%20%20is_premium%3A%20bool%2C%0A%7D%0A%0A%2F%2F%20%E3%83%97%E3%83%AC%E3%83%9F%E3%82%A2%E3%83%A0%E4%BC%9A%E5%93%A1%E3%81%AF20%25%E5%89%B2%E5%BC%95%E3%80%81%E9%80%9A%E5%B8%B8%E4%BC%9A%E5%93%A1%E3%81%AF10%25%E5%89%B2%E5%BC%95%0Apub%20fn%20calculate_discount%28user%3A%20%26User%2C%20price%3A%20u32%29%20-%3E%20u32%20%7B%0A%20%20%20%20let%20rate%20%3D%20if%20user.is_premium%20%7B%200.8%20%7D%20else%20%7B%200.9%20%7D%3B%0A%20%20%20%20apply_percentage%28price%2C%20rate%29%0A%7D%0A%0A%2F%2F%20%E5%89%B2%E5%BC%95%E7%8E%87%E3%82%92%E9%81%A9%E7%94%A8%E3%81%99%E3%82%8B%E3%83%97%E3%83%A9%E3%82%A4%E3%83%99%E3%83%BC%E3%83%88%E9%96%A2%E6%95%B0%0Afn%20apply_percentage%28price%3A%20u32%2C%20rate%3A%20f64%29%20-%3E%20u32%20%7B%0A%20%20%20%20%28price%20as%20f64%20*%20rate%29%20as%20u32%0A%7D%0A%0A%23%5Bcfg%28test%29%5D%0Amod%20tests%20%7B%0A%20%20%20%20use%20super%3A%3A*%3B%0A%0A%20%20%20%20%23%5Btest%5D%0A%20%20%20%20fn%20premium_user_gets_20_percent_discount%28%29%20%7B%0A%20%20%20%20%20%20%20%20let%20user%20%3D%20User%20%7B%20is_premium%3A%20true%20%7D%3B%0A%20%20%20%20%20%20%20%20assert_eq%21%28calculate_discount%28%26user%2C%201000%29%2C%20800%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20%23%5Btest%5D%0A%20%20%20%20fn%20normal_user_gets_10_percent_discount%28%29%20%7B%0A%20%20%20%20%20%20%20%20let%20user%20%3D%20User%20%7B%20is_premium%3A%20false%20%7D%3B%0A%20%20%20%20%20%20%20%20assert_eq%21%28calculate_discount%28%26user%2C%201000%29%2C%20900%29%3B%0A%20%20%20%20%7D%0A%7D)

`cargo test` を実行すると、`tests` モジュール内の2つのテストが実行されます。`assert_eq!` マクロは、左辺と右辺の値が等しいことを期待します。もし異なれば、テストは失敗（`panicked`）し、どこで何が違ったのかが表示されます。

この例では、公開APIである `calculate_discount` 関数だけでなく、その内部で使われているプライベート関数 `apply_percentage` もテストできます。これが、単体テストが「ユニット（単位）」テストと呼ばれる所以です。

### 異常系テスト：`#[should_panic]`

特定の条件下でコードが意図通りにパニックするかどうかをテストすることも重要です。例えば、「価格は0より大きくなければならない」という制約を追加してみましょう。

```rust
// src/lib.rs

// ... User構造体の定義など ...

pub fn calculate_discount(user: &User, price: u32) -> u32 {
    if price == 0 {
        panic!("Price must be greater than 0");
    }
    let rate = if user.is_premium { 0.8 } else { 0.9 };
    apply_percentage(price, rate)
}

// ... apply_percentage関数 ...

#[cfg(test)]
mod tests {
    use super::*;
    // ... 正常系テスト ...

    #[test]
    #[should_panic(expected = "Price must be greater than 0")]
    fn panic_if_price_is_zero() {
        let user = User { is_premium: true };
        calculate_discount(&user, 0);
    }
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=pub%20struct%20User%20%7B%0A%20%20%20%20is_premium%3A%20bool%2C%0A%7D%0A%0Apub%20fn%20calculate_discount%28user%3A%20%26User%2C%20price%3A%20u32%29%20-%3E%20u32%20%7B%0A%20%20%20%20if%20price%20%3D%3D%200%20%7B%0A%20%20%20%20%20%20%20%20panic%21%28%22Price%20must%20be%20greater%20than%200%22%29%3B%0A%20%20%20%20%7D%0A%20%20%20%20let%20rate%20%3D%20if%20user.is_premium%20%7B%200.8%20%7D%20else%20%7B%200.9%20%7D%3B%0A%20%20%20%20apply_percentage%28price%2C%20rate%29%0A%7D%0A%0Afn%20apply_percentage%28price%3A%20u32%2C%20rate%3A%20f64%29%20-%3E%20u32%20%7B%0A%20%20%20%20%28price%20as%20f64%20*%20rate%29%20as%20u32%0A%7D%0A%0A%23%5Bcfg%28test%29%5D%0Amod%20tests%20%7B%0A%20%20%20%20use%20super%3A%3A*%3B%0A%0A%20%20%20%20%23%5Btest%5D%0A%20%20%20%20%23%5Bshould_panic%28expected%20%3D%20%22Price%20must%20be%20greater%20than%200%22%29%5D%0A%20%20%20%20fn%20panic_if_price_is_zero%28%29%20%7B%0A%20%20%20%20%20%20%20%20let%20user%20%3D%20User%20%7B%20is_premium%3A%20true%20%7D%3B%0A%20%20%20%20%20%20%20%20calculate_discount%28%26user%2C%200%29%3B%0A%20%20%20%20%7D%0A%7D)

`#[should_panic]` 属性を使うと、そのテスト関数がパニックすれば成功、パニックしなければ失敗とみなされます。さらに `expected` 引数を追加することで、パニックメッセージに特定の部分文字列が含まれているかも検証でき、より正確なテストになります。

このテストは、`calculate_discount` に `0` を渡したときに正しくパニックするので成功します。

## 21.3 `Result` を返す関数のテスト

`panic!` は回復不能なエラーに使われますが、多くの場合は `Result` 型でエラーを返す方がより堅牢です。`Result` を返す関数はどのようにテストすればよいでしょうか。

`calculate_discount` を、価格が0の場合に `Err` を返すように変更してみましょう。

```rust
// src/lib.rs
// ...

#[derive(Debug, PartialEq)] // テストで比較できるよう PartialEq を追加
pub enum DiscountError {
    InvalidPrice,
}

pub fn calculate_discount_result(
    user: &User,
    price: u32,
) -> Result<u32, DiscountError> {
    if price == 0 {
        return Err(DiscountError::InvalidPrice);
    }
    let rate = if user.is_premium { 0.8 } else { 0.9 };
    Ok(apply_percentage(price, rate))
}

// ...

#[cfg(test)]
mod tests {
    use super::*;
    // ...

    #[test]
    fn returns_err_if_price_is_zero() {
        let user = User { is_premium: true };
        let result = calculate_discount_result(&user, 0);
        assert!(result.is_err());
        assert_eq!(result.unwrap_err(), DiscountError::InvalidPrice);
    }
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=pub%20struct%20User%20%7B%0A%20%20%20%20is_premium%3A%20bool%2C%0A%7D%0A%0Afn%20apply_percentage%28price%3A%20u32%2C%20rate%3A%20f64%29%20-%3E%20u32%20%7B%0A%20%20%20%20%28price%20as%20f64%20*%20rate%29%20as%20u32%0A%7D%0A%0A%23%5Bderive%28Debug%2C%20PartialEq%29%5D%0Apub%20enum%20DiscountError%20%7B%0A%20%20%20%20InvalidPrice%2C%0A%7D%0A%0Apub%20fn%20calculate_discount_result%28%0A%20%20%20%20user%3A%20%26User%2C%0A%20%20%20%20price%3A%20u32%2C%0A%29%20-%3E%20Result%3Cu32%2C%20DiscountError%3E%20%7B%0A%20%20%20%20if%20price%20%3D%3D%200%20%7B%0A%20%20%20%20%20%20%20%20return%20Err%28DiscountError%3A%3AInvalidPrice%29%3B%0A%20%20%20%20%7D%0A%20%20%20%20let%20rate%20%3D%20if%20user.is_premium%20%7B%200.8%20%7D%20else%20%7B%200.9%20%7D%3B%0A%20%20%20%20Ok%28apply_percentage%28price%2C%20rate%29%29%0A%7D%0A%0A%23%5Bcfg%28test%29%5D%0Amod%20tests%20%7B%0A%20%20%20%20use%20super%3A%3A*%3B%0A%0A%20%20%20%20%23%5Btest%5D%0A%20%20%20%20fn%20returns_err_if_price_is_zero%28%29%20%7B%0A%20%20%20%20%20%20%20%20let%20user%20%3D%20User%20%7B%20is_premium%3A%20true%20%7D%3B%0A%20%20%20%20%20%20%20%20let%20result%20%3D%20calculate_discount_result%28%26user%2C%200%29%3B%0A%20%20%20%20%20%20%20%20assert%21%28result.is_err%28%29%29%3B%0A%20%20%20%20%20%20%20%20assert_eq%21%28result.unwrap_err%28%29%2C%20DiscountError%3A%3AInvalidPrice%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20%23%5Btest%5D%0A%20%20%20%20fn%20premium_user_discount_with_result%28%29%20%7B%0A%20%20%20%20%20%20%20%20let%20user%20%3D%20User%20%7B%20is_premium%3A%20true%20%7D%3B%0A%20%20%20%20%20%20%20%20let%20result%20%3D%20calculate_discount_result%28%26user%2C%201000%29%3B%0A%20%20%20%20%20%20%20%20assert%21%28result.is_ok%28%29%29%3B%0A%20%20%20%20%20%20%20%20assert_eq%21%28result.unwrap%28%29%2C%20800%29%3B%0A%20%20%20%20%7D%0A%7D)

このように、`Result` を返す関数のテストでは、`Ok` と `Err` 両方のケースを網羅することが重要です。

## 21.4 `cargo test` コマンドのオプション

`cargo test` には、テストの実行を細かく制御するための便利なオプションがあります。
- `cargo test -- --test-threads=1`: テストを並列実行せず、1スレッドで実行します。テストが標準出力に何かを出力する場合に、表示が混ざらないようにするのに便利です。
- `cargo test -- --show-output`: 成功したテストも含め、すべてのテストの標準出力を表示します。
- `cargo test premium_user`: `premium_user` という名前が含まれるテストのみを実行します。
- `cargo test -- --ignored`: `#[ignore]` 属性が付いたテストのみを実行します。重いテストなどを普段はスキップするのに便利です。

## 21.5 統合テスト

- 単体テスト (Unit Tests): 小さな単位（個々の関数など）を隔離してテストする。プライベートな関数もテストできる。`src` 内に `#[cfg(test)]` を付けた `mod tests` を作るのが慣例。
- 統合テスト (Integration Tests): ライブラリを外部の利用者の視点からテストする。公開されたAPIのみをテストできる。パッケージルート（`Cargo.toml` と同じ階層）に `tests` ディレクトリを作成し、その中に `.rs` ファイルを置くのが慣例。

### 試してみよう：`tests` ディレクトリの作成

1.  プロジェクトのルートディレクトリに `tests` という名前のディレクトリを作成します。
2.  `tests/integration_test.rs` というファイルを作成します。

```rust
// tests/integration_test.rs

// クレート名を `use` でインポートする
use discount_calculator::{calculate_discount, User};

#[test]
fn integration_test_normal_user_discount() {
    let user = User { is_premium: false };
    assert_eq!(900, calculate_discount(&user, 1000));
}
```
`cargo test` を実行すると、単体テストと統合テストの両方が実行されます。

### 統合テストとモッキング

ライブラリが外部のサービス（データベース、Web APIなど）に依存している場合、統合テストの実行は困難になります。テストを実行するたびにAPIを呼び出していたら、テストは不安定で遅くなり、余計なコストもかかります。

こうした問題を解決するのが「モッキング」です。モッキングとは、依存するオブジェクトを、テスト用に作られた偽物（モック）に置き換えるテクニックです。

#### `trait` による依存関係の抽象化

Rust でモッキングを実現する一般的な方法は、`trait` を使って依存関係を抽象化することです。

例えば、割引後にユーザーに通知を送る機能を考えます。通知サービスのインターフェースを `trait` として定義します。

```rust
// src/lib.rs
// ...

pub trait Notifier {
    fn notify(&self, message: &str);
}

// `Notifier` トレイトを実装する任意の型 `N` を受け取る
pub fn discounted_price_and_notify<N: Notifier>(
    user: &User,
    price: u32,
    notifier: &N,
) -> u32 {
    let discounted = calculate_discount(user, price);
    notifier.notify("Your discount has been applied!");
    discounted
}
```

これで、`discounted_price_and_notify` 関数は、`Notifier` トレイトを実装してさえいれば、どんな型の通知サービスでも受け取れるようになりました。

#### テストでモックを使う

統合テストでは、この `Notifier` トレイトを実装したモック用の構造体を用意します。

```rust
// tests/integration_test.rs
use discount_calculator::{
    discounted_price_and_notify, Notifier, User
};
use std::cell::RefCell;

// モック用の通知サービス
struct MockNotifier {
    // 呼び出されたメッセージを記録する
    messages: RefCell<Vec<String>>,
}

impl Notifier for MockNotifier {
    fn notify(&self, message: &str) {
        // `&self` が不変参照なので、内部可変性パターンの `RefCell` を使う
        self.messages.borrow_mut().push(String::from(message));
    }
}

#[test]
fn it_notifies_when_discount_applied() {
    let mock_notifier = MockNotifier {
        messages: RefCell::new(vec![]),
    };
    let user = User { is_premium: true };

    discounted_price_and_notify(&user, 1000, &mock_notifier);

    // `notify` メソッドが期待通りに呼ばれたか検証
    assert_eq!(mock_notifier.messages.borrow().len(), 1);
    assert_eq!(
        mock_notifier.messages.borrow()[0],
        "Your discount has been applied!"
    );
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Acell%3A%3ARefCell%3B%0A%0A%2F%2F%20---%20Library%20code%20from%20src%2Flib.rs%20---%0Apub%20struct%20User%20%7B%0A%20%20%20%20is_premium%3A%20bool%2C%0A%7D%0Afn%20apply_percentage%28price%3A%20u32%2C%20rate%3A%20f64%29%20-%3E%20u32%20%7B%0A%20%20%20%20%28price%20as%20f64%20*%20rate%29%20as%20u32%0A%7D%0Apub%20fn%20calculate_discount%28user%3A%20%26User%2C%20price%3A%20u32%29%20-%3E%20u32%20%7B%0A%20%20%20%20if%20price%20%3D%3D%200%20%7B%0A%20%20%20%20%20%20%20%20panic%21%28%22Price%20must%20be%20greater%20than%200%22%29%3B%0A%20%20%20%20%7D%0A%20%20%20%20let%20rate%20%3D%20if%20user.is_premium%20%7B%200.8%20%7D%20else%20%7B%200.9%20%7D%3B%0A%20%20%20%20apply_percentage%28price%2C%20rate%29%0A%7D%0A%0Apub%20trait%20Notifier%20%7B%0A%20%20%20%20fn%20notify%28%26self%2C%20message%3A%20%26str%29%3B%0A%7D%0A%0Apub%20fn%20discounted_price_and_notify%3CN%3A%20Notifier%3E%28%0A%20%20%20%20user%3A%20%26User%2C%0A%20%20%20%20price%3A%20u32%2C%0A%20%20%20%20notifier%3A%20%26N%2C%0A%29%20-%3E%20u32%20%7B%0A%20%20%20%20let%20discounted%20%3D%20calculate_discount%28user%2C%20price%29%3B%0A%20%20%20%20notifier.notify%28%22Your%20discount%20has%20been%20applied%21%22%29%3B%0A%20%20%20%20discounted%0A%7D%0A%0A%2F%2F%20---%20Test%20code%20from%20tests%2Fintegration_test.rs%20---%0A%23%5Bcfg%28test%29%5D%0Amod%20tests%20%7B%0A%20%20%20%20use%20super%3A%3A*%3B%0A%20%20%20%20use%20std%3A%3Acell%3A%3ARefCell%3B%0A%0A%20%20%20%20struct%20MockNotifier%20%7B%0A%20%20%20%20%20%20%20%20messages%3A%20RefCell%3CVec%3CString%3E%3E%2C%0A%20%20%20%20%7D%0A%0A%20%20%20%20impl%20Notifier%20for%20MockNotifier%20%7B%0A%20%20%20%20%20%20%20%20fn%20notify%28%26self%2C%20message%3A%20%26str%29%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20self.messages.borrow_mut%28%29.push%28String%3A%3Afrom%28message%29%29%3B%0A%20%20%20%20%20%20%20%20%7D%0A%20%20%20%20%7D%0A%0A%20%20%20%20%23%5Btest%5D%0A%20%20%20%20fn%20it_notifies_when_discount_applied%28%29%20%7B%0A%20%20%20%20%20%20%20%20let%20mock_notifier%20%3D%20MockNotifier%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20messages%3A%20RefCell%3A%3Anew%28vec%21%5B%5D%29%2C%0A%20%20%20%20%20%20%20%20%7D%3B%0A%20%20%20%20%20%20%20%20let%20user%20%3D%20User%20%7B%20is_premium%3A%20true%20%7D%3B%0A%0A%20%20%20%20%20%20%20%20discounted_price_and_notify%28%26user%2C%201000%2C%20%26mock_notifier%29%3B%0A%0A%20%20%20%20%20%20%20%20assert_eq%21%28mock_notifier.messages.borrow%28%29.len%28%29%2C%201%29%3B%0A%20%20%20%20%20%20%20%20assert_eq%21%28%0A%20%20%20%20%20%20%20%20%20%20%20%20mock_notifier.messages.borrow%28%29%5B0%5D%2C%0A%20%20%20%20%20%20%20%20%20%20%20%20%22Your%20discount%20has%20been%20applied%21%22%0A%20%20%20%20%20%20%20%20%29%3B%0A%20%20%20%20%7D%0A%7D)

このテストでは、`MockNotifier` を `discounted_price_and_notify` 関数に渡しています。`MockNotifier` は実際に通知を送る代わりに、呼び出されたメッセージを自身の `messages` フィールドに記録します。テストの最後で、この `messages` の中身を検証することで、「`notify` メソッドが期待通りに呼ばれたか」を外部サービスなしでテストできるのです。

このように、`trait` を使って依存関係を抽象化し、テスト時にモックオブジェクトを注入する手法は、Rustで堅牢かつテストしやすいコードを書くための非常に重要なパターンです。

## 21.6 まとめ

- Rustのテストは、`#[test]` 属性を関数に付けるだけで簡単に書ける。
- `assert!`, `assert_eq!`, `assert_ne!` マクロは、テストの条件を検証するための基本的なツール。
- `#[should_panic]` 属性は、コードが意図通りに回復不能なエラー（パニック）を起こすことを検証する。
- `Result` を返す関数は、`Ok` の場合と `Err` の場合の両方をテストする必要がある。
- `cargo test` は、単体テストと統合テストの両方を実行するコマンド。特定のテストだけを実行したり、出力を制御したりするオプションがある。
- 単体テストは `src` 内に、統合テストはルートの `tests` ディレクトリに配置するのが慣例。
- `trait` を使って依存関係を抽象化することで、モックを使ったテストがしやすくなる。

---

テストを書く習慣は、堅牢で信頼性の高いソフトウェアを開発するための基礎です。次の章では、Rust の `Result` 型と `?` 演算子を使った、より実践的なエラーハンドリングの方法について学びます。

