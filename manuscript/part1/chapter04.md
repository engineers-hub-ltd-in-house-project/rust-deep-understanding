# 第 4 章：制御構文とパターンマッチング

## この章のゴール
- `if` が文ではなく式であることを理解し、`let` と組み合わせて使える。
- `loop`, `while`, `for` の基本的な使い方と、それぞれの使い分けを説明できる。
- `match` を使った網羅的なパターンマッチングを実装できる。
- `if let` を使って、特定のパターンにのみ一致する場合の処理を簡潔に書ける。

---

## 4.1 Python/Go ではこうやっていた

Python や Go における制御構文は、主に 文 (Statement) として機能しました。

- Python: `if`/`elif`/`else` 文や `for`/`while` ループを使って、プログラムの流れを制御していました。これらが値を返すことはありませんでした。
- Go: `if`/`else` 文、`switch` 文、そして `for` 文 (Go には `while` がありません) を使っていました。Go の `if` 文は、条件式の前に簡単な文を置けるという特徴がありました。

Rust の制御構文も見た目は似ていますが、一つ大きな違いがあります。それは、`if` や `match` が 式 (Expression) である、ということです。

## 4.2 `if` は文ではなく「式」

Python や Go との大きな違いは、Rust の `if` が文 (Statement) ではなく、値を返すことができる **式 (Expression)** である点です。

### 試してみよう：`if` の結果を変数に入れる

`if` が値を返すとはどういうことか、`cargo new controlflow` で新しいプロジェクトを作って試してみましょう。`src/main.rs` に以下を記述して `cargo run` してください。

```rust
fn main() {
    let condition = true;
    let number = if condition { 5 } else { 6 };

    println!("The value of number is: {}", number);
}
```

結果として `5` が表示されたはずです。`if condition { 5 } else { 6 }` の部分全体が評価され、`condition` が `true` だったので `5` という値を返しました。その結果が `number` という変数に束縛されたのです。

これは、Python の三項演算子 (`5 if condition else 6`) のように使えますが、より読みやすく、複雑なロジックも記述できます。

### 試してみよう：コンパイラに型を怒られてみる

`if` 式が値を返すということは、その値には型がなければなりません。コンパイラは、`if` の両方のブロックから返される値の型が同じであることを要求します。

もし違う型の値を返そうとするとどうなるでしょうか？試してみましょう。

```rust
fn main() {
    let condition = true;
    // ❌ コンパイルエラー！
    // `if` ブロックは整数を、`else` ブロックは文字列を返そうとしている
    let number = if condition { 5 } else { "six" };

    println!("The value of number is: {}", number);
}
```

これを実行すると、`if` と `else` に `mismatched types` (型の不一致) があるとコンパイラが教えてくれます。

```text
error[E0308]: `if` and `else` have incompatible types
 --> src/main.rs:5:44
  |
5 |     let number = if condition { 5 } else { "six" };
  |                                 -          ^^^^^ expected integer, found `&str`
  |                                 |
  |                                 expected because of this
```

この制約のおかげで、私たちは `number` という変数の型が常に一つに定まることを確信でき、意図しない型のエラーを実行時まで持ち越すことがありません。

## 4.3 用途で使い分ける3種類のループ

Rust には 3 種類のループがあります。それぞれの目的と使い分けを理解することが重要です。

```mermaid
graph TD
    subgraph "ループの種類"
        A[loop] --> B(無限ループ<br>break で値を返す);
        C[while] --> D(条件付きループ<br>条件が true の間繰り返す);
        E[for] --> F(コレクションループ<br>イテレータの各要素を処理);
    end
```

### 1. `loop`: 無限ループ
意図的に無限ループを作りたい場合や、ループからの脱出条件が複雑な場合に使用します。`break` キーワードでループを抜け、値を返すこともできます。

```rust
fn main() {
    let mut counter = 0;

    let result = loop {
        counter += 1;

        if counter == 10 {
            break counter * 2; // ループを抜け、値を返す
        }
    };

    println!("The result is {}", result); // => 20
}
```

### 2. `while`: 条件付きループ
特定の条件が `true` である間、ループを継続します。Go には `while` がありませんでしたが、Python 経験者にはお馴染みでしょう。

```rust
fn main() {
    let mut number = 3;

    while number != 0 {
        println!("{}!", number);
        number -= 1;
    }

    println!("LIFTOFF!!!");
}
```

### 3. `for`: コレクションループ
Python の `for item in collection:` のように、配列やベクタなどのコレクションの各要素を順番に処理するのに最も適しています。安全でエラーが起きにくいため、最も一般的に使われるループです。

```rust
fn main() {
    let a = [10, 20, 30, 40, 50];

    for element in a {
        println!("the value is: {}", element);
    }
}
```

`for` ループは、`.rev()` メソッドを使って逆順にイテレートすることもできます。

```rust
// 3, 2, 1, LIFTOFF!!! と出力
for number in (1..4).rev() {
    println!("{}!", number);
}
```

## 4.4 Rust のスーパーパワー：`match`

`match` は、Go の `switch` 文を遥かに強力にしたような機能です。値を取り、その値が成り立ちうる一連のパターンと比較し、一致したパターンのコードブロックを実行します。

### 試してみよう：コンパイラの網羅性チェック

`match` の最も重要な性質は、**網羅性チェック (Exhaustiveness Checking)** です。コンパイラは、ありとあらゆる可能性が `match` のアーム (各 `=>` の分岐) で網羅されているかをチェックします。

`enum` (列挙型、後の章で詳しく学びます) を使って、この機能を体験してみましょう。

```rust
enum Coin {
    Penny,
    Nickel,
    Dime,
    Quarter,
}

fn value_in_cents(coin: Coin) -> u8 {
    match coin {
        Coin::Penny => 1,
        Coin::Nickel => 5,
        Coin::Dime => 10,
        // わざと Quarter をコメントアウトしてみる
        // Coin::Quarter => 25,
    }
}

fn main() {
    let my_coin = Coin::Penny;
    println!("Value: {}", value_in_cents(my_coin));
}
```

このコードで `cargo run` を実行すると、コンパイルエラーになります。

```text
error[E0004]: non-exhaustive patterns: `Coin::Quarter` not covered
   --> src/main.rs:10:11
    |
10  |     match coin {
    |           ^^^^ pattern `Coin::Quarter` not covered
    |
    = note: the matched value is of type `Coin`
help: ensure that all possible cases are being handled by adding a match arm with a wildcard pattern or an arm for `Coin::Quarter`
```

「`non-exhaustive patterns` (網羅的でないパターン)」、つまり `Coin::Quarter` の可能性がカバーされていませんよ、とコンパイラが教えてくれました。

これにより、Go の `switch` で `default` を書き忘れたり、Python の `if`/`elif` で条件分岐が漏れたりするようなバグを、コンパイル段階で完全に防ぐことができます。

コメントアウトを外せば、正しくコンパイルできます。

```rust
fn value_in_cents(coin: Coin) -> u8 {
    match coin {
        Coin::Penny => 1,
        Coin::Nickel => 5,
        Coin::Dime => 10,
        Coin::Quarter => 25,
    }
}
```

## 4.5 `if let`：一つのパターンに注目する

`match` は網羅的で強力ですが、たくさんの可能性のうち、たった一つのパターンにだけ興味がある場合には、少し冗長になることがあります。

```rust
let config_max = Some(3u8);

// match を使った場合
match config_max {
    Some(max) => println!("The maximum is configured to be {}", max),
    _ => (), // 他のケースは何もしない
}
```
`_ => ()` は「その他のすべてのケースにマッチし、何もしない」という意味ですが、少し回りくどいですね。

このような場合に、`if let` 構文が役立ちます。`if let` は `match` の糖衣構文 (シンタックスシュガー) で、値が特定のパターンに一致した場合にのみコードブロックを実行します。

```rust
// if let を使った場合
if let Some(max) = config_max {
    println!("The maximum is configured to be {}", max);
}
```

`match` で行っていた処理を、より簡潔に記述できました。`else` を組み合わせることも可能です。

## 4.6 まとめ

- ✓ Rust の `if` は 式 であり、値を返すことができる。各分岐の戻り値は同じ型でなければならない。
- ✓ ループには `loop` (無限)、`while` (条件付き)、`for` (コレクション) の 3 種類があり、目的に応じて使い分ける。
- ✓ `match` は、網羅性チェックを備えた非常に強力なパターンマッチング機能。
- ✓ `if let` は、単一のパターンにのみマッチさせたい場合に `match` を簡潔に書くための構文。

---

Rust の制御構文、特に `if` や `match` が式であるという性質は、関数型プログラミングから影響を受けており、より宣言的でバグの少ないコードを書くのに役立ちます。

次の章では、これらの制御構文を使ってロジックをカプセル化する「関数」と、さらに柔軟な処理を可能にする「クロージャ」について学びます。
