# 第 9 章：構造体とメソッド

## この章のゴール
- `struct` キーワードを使って、関連するデータをまとめた独自のデータ型を定義できる。
- フィールドを持つ通常の構造体、タプル構造体、ユニット様構造体の 3 種類を使い分けられる。
- `impl` ブロックを使って、構造体にメソッドや関連関数を定義できる。
- メソッドの第一引数 `&self`, `&mut self`, `self` の違いを、所有権の観点からエラーを通して体験する。

---

## 9.1 データと振る舞いを組み合わせる

Python や Go では、複数のデータを一つにまとめるための仕組みがありました。

- Python: `class` を使って、データ (属性) とそのデータを操作するメソッドを一緒にカプセル化していました。
- Go: `struct` を使って、異なる型のフィールドをまとめたデータ構造を定義していました。メソッドは、`func` キーワードとレシーバ引数を使って、`struct` とは別に定義するスタイルでした。

Rust のアプローチは、**データ構造の定義 (`struct`)** と、その **振る舞いの定義 (`impl`)** を分離するという点で、Go に似ています。この分離が、後の章で学ぶ「トレイト」という概念において重要な役割を果たします。

## 9.2 構造体 (`struct`) の定義

構造体は、関連する複数の値をまとめて意味のあるグループを作るためのカスタムデータ型です。`cargo new structs` でプロジェクトを作り、試していきましょう。

### 1. 通常の構造体
フィールドに名前を付けて定義する、最も一般的な構造体です。

```rust
// src/main.rs

struct User {
    active: bool,
    username: String,
    email: String,
    sign_in_count: u64,
}

fn main() {
    // 構造体のインスタンスを作成
    let mut user1 = User {
        email: String::from("someone@example.com"),
        username: String::from("someusername123"),
        active: true,
        sign_in_count: 1,
    };

    // ドット記法でフィールドにアクセス
    user1.email = String::from("anotheremail@example.com");

    println!("User email: {}", user1.email);
}
```
インスタンス全体を可変 (`mut`) にすれば、フィールドの値を変更できます (フィールドがプライベートでない場合)。

### 試してみよう：フィールド更新構文

既存のインスタンスのいくつかの値を変更して、新しいインスタンスを作りたい場合があります。その際に便利なのが `..` 構文です。

```rust
// main 関数の続き

let user2 = User {
    email: String::from("another@example.com"),
    // username は user1 と同じ値を使う
    ..user1
};

// ここで user1 の username を使おうとするとどうなる？
// println!("user1's username: {}", user1.username);
```
`..user1` は、`user2` で明示的に設定されていないフィールドの値を `user1` から持ってくる、という意味です。

ここで最後の `println!` のコメントを外して `cargo run` してみましょう。
`borrow of partially moved value: user1` というエラーが出ます。`username` フィールドの `String` は `Copy` トレイトを持たないため、`user2` の作成時に所有権が `user1` から `user2` に **ムーブ** してしまいました。そのため、`user1.username` にはもうアクセスできません。一方で、`active` や `sign_in_count` のような `Copy` トレイトを持つフィールドはコピーされるだけなので、`user1.active` にはアクセスできます。この挙動は所有権のルールそのものです。

### 2. タプル構造体とユニット様構造体

- **タプル構造体**: フィールド名はなく、型の定義だけを持つ構造体です。タプル全体に名前を付けたい場合に便利です。
  ```rust
  struct Color(i32, i32, i32);
  let black = Color(0, 0, 0);
  ```
- **ユニット様構造体**: フィールドを一切持たない構造体です。ある型に対してトレイトを実装したいが、格納すべきデータがない場合に役立ちます。
  ```rust
  struct AlwaysEqual;
  let subject = AlwaysEqual;
  ```

## 9.3 メソッド (`impl`) で振る舞いを定義する

メソッドは関数と似ていますが、構造体 (や enum) の文脈で定義される点が異なります。メソッドは、その構造体のインスタンスが何を行えるか、という「振る舞い」を定義します。メソッドは `impl` (implementation の略) ブロックの中に定義します。

```mermaid
graph TD
    A[struct Rectangle { ... }] -- データ --> C{Rectangle 型};
    B[impl Rectangle { ... }] -- 振る舞い --> C;
```

```rust
struct Rectangle {
    width: u32,
    height: u32,
}

// Rectangle 構造体のための impl ブロック
impl Rectangle {
    // メソッド `area` の定義
    // 第一引数は常に `self` のいずれかの形式
    fn area(&self) -> u32 {
        self.width * self.height
    }
}

fn main() {
    let rect1 = Rectangle {
        width: 30,
        height: 50,
    };
    println!("The area is {}", rect1.area());
}
```
### 試してみよう：`&self`, `&mut self`, `self` の違い

`impl` ブロック内のメソッドの第一引数は、常に `self` であり、これは `impl` の対象となっている型のインスタンスを指します。`self` には3つの主要な形式があり、所有権の扱いが異なります。これを体験的に学びましょう。

```rust
struct Point {
    x: f64,
    y: f64,
}

impl Point {
    // インスタンスを不変で借用する
    // データを読み取るだけ
    fn distance_from_origin(&self) -> f64 {
        (self.x.powi(2) + self.y.powi(2)).sqrt()
    }

    // インスタンスを可変で借用する
    // インスタンスの状態を変更する
    fn translate(&mut self, dx: f64, dy: f64) {
        self.x += dx;
        self.y += dy;
    }

    // インスタンスの所有権を奪う
    // インスタンスを消費して別のものを生成する
    fn to_tuple(self) -> (f64, f64) {
        (self.x, self.y)
    }
}

fn main() {
    let mut p = Point { x: 3.0, y: 4.0 };

    // &self を受け取るメソッドを呼ぶ
    println!("Distance from origin: {}", p.distance_from_origin());
    // 呼び出し後も p は有効
    println!("Point is at ({}, {})", p.x, p.y);

    // &mut self を受け取るメソッドを呼ぶ
    p.translate(1.0, 1.0);
    // 呼び出し後も p は有効（中身は変わっている）
    println!("Point is now at ({}, {})", p.x, p.y);

    // self を受け取るメソッドを呼ぶ
    let t = p.to_tuple();
    println!("Point as tuple: {:?}", t);

    // 呼び出し後、p はムーブしているのでもう使えない
    // 以下の行のコメントを外すとコンパイルエラー！
    // println!("Point is at ({}, {})", p.x, p.y);
}
```
最後の `println!` のコメントを外して `cargo run` すると、`borrow of moved value: p` エラーが発生します。`to_tuple` メソッドが `p` の所有権を奪った（消費した）ため、`main` 関数ではもう `p` にアクセスできなくなるのです。このように、メソッドのシグネチャを見るだけで、そのメソッドがインスタンスに対して何をする（読み取るだけか、変更するか、消費するか）のかが明確にわかるのがRustの大きな利点です。

## 9.4 関連関数 (Associated Functions)

`impl` ブロックの中には、`self` を第一引数として取らない関数も定義できます。これを **関連関数 (associated function)** と呼びます。これは、他の言語における「静的メソッド (static method)」と似ています。

関連関数は、構造体名と `::` 構文を使って呼び出します。構造体の新しいインスタンスを返すコンストラクタとしてよく使われます。

```rust
// Point の impl ブロックに追加
impl Point {
    // self を取らないので関連関数
    fn origin() -> Point {
        Point { x: 0.0, y: 0.0 }
    }
}

// main 関数内で呼び出す
let origin_point = Point::origin();
```
## 9.5 まとめ

- ✓ 構造体 (`struct`) は、関連するデータをまとめたカスタムデータ型。
- ✓ Rust では、データの定義 (`struct`) と振る舞いの定義 (`impl`) を分離する。
- ✓ フィールド更新構文 `..` は、一部のフィールドが `Copy` でない場合、所有権のムーブを引き起こす。
- ✓ メソッドの第一引数は、所有権の扱い方を決定する:
  - `&self`: 不変の借用 (読み取り)
  - `&mut self`: 可変の借用 (変更)
  - `self`: 所有権のムーブ (消費)
- ✓ `self` を引数に取らない `impl` 内の関数は、`::` で呼び出す関連関数となる。

---

構造体とメソッドは、プログラムを構成する基本的な部品です。特に `self` の扱いは、所有権システムがコードの隅々まで浸透していることを示しています。

次の章では、もう一つの重要なデータ構造である「enum (列挙型)」と、Go エンジニアには馴染み深い `nil` の問題を解決する `Option` 型について学びます。
