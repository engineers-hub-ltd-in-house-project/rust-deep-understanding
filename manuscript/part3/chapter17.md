# 第 17 章：標準トレイトと `derive` 属性

## この章のゴール
- `Debug`, `Clone`, `Copy`, `PartialEq`, `Eq`, `PartialOrd`, `Ord`, `Default` といった、Rust の基本的な標準トレイトの役割を説明できる。
- `#[derive]` 属性を使い、これらの標準トレイトを独自の `struct` や `enum` に自動的に実装できる。
- `Clone` と `Copy` の違い、`PartialEq` と `Eq` の違いなど、似たトレイト間の使い分けを説明できる。

---

## 17.1 問題設定：定義しただけの `struct` は何もできない

これまでに `struct` を使って独自の型を定義してきましたが、実は、定義しただけの「素の」`struct` は、ほとんど何もできません。デバッグのためにプリントすることも、等しいか比較することも、コピーすることすらできないのです。

```sh
cargo new standard_traits
```
でプロジェクトを作り、この「何もできない」状態を体験してみましょう。

### 試してみよう：「素の」`struct` でエラーを起こす

```rust
// src/main.rs

struct Point {
    x: i32,
    y: i32,
}

fn main() {
    let p1 = Point { x: 1, y: 2 };
    let p2 = Point { x: 1, y: 2 };
    let p3 = p1;

    // 1. プリントしようとすると...？
    // println!("p1: {:?}", p1);

    // 2. 比較しようとすると...？
    // if p1 == p2 {
    //     println!("p1 and p2 are equal");
    // }

    // 3. コピー（のつもり）の後、元の変数を使おうとすると...？
    // println!("p3: {:?}", p3);
    // println!("p1 is moved: {:?}", p1);
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0A%0Astruct%20Point%20%7B%0A%20%20%20%20x%3A%20i32%2C%0A%20%20%20%20y%3A%20i32%2C%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20p1%20%3D%20Point%20%7B%20x%3A%201%2C%20y%3A%202%20%7D%3B%0A%20%20%20%20let%20p2%20%3D%20Point%20%7B%20x%3A%201%2C%20y%3A%202%20%7D%3B%0A%20%20%20%20let%20p3%20%3D%20p1%3B%0A%0A%20%20%20%20//%201.%20%E3%83%97%E3%83%AA%E3%83%B3%E3%83%88%E3%81%97%E3%82%88%E3%81%86%E3%81%A8%E3%81%99%E3%82%8B%E3%81%A8...%EF%BC%9F%0A%20%20%20%20//%20println%21%28%22p1%3A%20%7B%3A%3F%7D%22%2C%20p1%29%3B%0A%0A%20%20%20%20//%202.%20%E6%AF%94%E8%BC%83%E3%81%97%E3%82%88%E3%81%86%E3%81%A8%E3%81%99%E3%82%8B%E3%81%A8...%EF%BC%9F%0A%20%20%20%20//%20if%20p1%20%3D%3D%20p2%20%7B%0A%20%20%20%20//%20%20%20%20%20println%21%28%22p1%20and%20p2%20are%20equal%22%29%3B%0A%20%20%20%20//%20%7D%0A%0A%20%20%20%20//%203.%20%E3%82%B3%E3%83%94%E3%83%BC%EF%BC%88%E3%81%AE%E3%81%A4%E3%82%82%E3%82%8A%EF%BC%89%E3%81%AE%E5%BE%8C%E3%80%81%E5%85%83%E3%81%AE%E5%A4%89%E6%95%B0%E3%82%92%E4%BD%BF%E3%81%8A%E3%81%86%E3%81%A8%E3%81%99%E3%82%8B%E3%81%A8...%EF%BC%9F%0A%20%20%20%20//%20println%21%28%22p3%3A%20%7B%3A%3F%7D%22%2C%20p3%29%3B%0A%20%20%20%20//%20println%21%28%22p1%20is%20moved%3A%20%7B%3A%3F%7D%22%2C%20p1%29%3B%0A%7D)
このコードは、コメントアウトされた行を一つでも有効にすると、様々なコンパイルエラーを引き起こします。

1.  プリントエラー: `Point` doesn't implement `Debug` (`Point`は`Debug`トレイトを実装していません)
2.  比較エラー: binary operation `==` cannot be applied to type `Point` (`==`演算子は`Point`型に適用できません)
3.  ムーブエラー: `p1` の値は `p3` にムーブされているため、`p1` はもう使えません (`use of moved value: p1`)

これらの基本的な操作を可能にするのが、Rustの 標準トレイト と、それを簡単に実装するための `#[derive]` マクロです。

## 17.2 解決策：`derive` 属性で振る舞いを追加する

これらの基本的な振る舞いを `Point` に追加するには、トレイトを手動で `impl` することもできますが、Rust にはもっと簡単な方法が用意されています。それが `#[derive]` 属性です。

```rust
#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
struct Point {
    x: i32,
    y: i32,
}

fn main() {
    let p1 = Point { x: 1, y: 2 };
    let p2 = Point { x: 4, y: -3 };
    let p3 = p1;

    // 1. Debug を derive したので、プリントできる！
    println!("p1: {:?}", p1);

    // 2. PartialEq を derive したので、比較できる！
    if p1 == p2 {
        println!("p1 and p2 are equal");
    } else {
        println!("p1 and p2 are NOT equal");
    }

    // 3. Copy を derive したので、p1 はムーブされずコピーされる
    println!("p3: {:?}", p3);
    println!("p1 is still available: {:?}", p1);
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=%23%5Bderive%28Debug%2C%20Clone%2C%20Copy%2C%20PartialEq%2C%20Eq%2C%20Default%29%5D%0Astruct%20Point%20%7B%0A%20%20%20%20x%3A%20i32%2C%0A%20%20%20%20y%3A%20i32%2C%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20p1%20%3D%20Point%20%7B%20x%3A%201%2C%20y%3A%202%20%7D%3B%0A%20%20%20%20let%20p2%20%3D%20Point%20%7B%20x%3A%204%2C%20y%3A%20-3%20%7D%3B%0A%20%20%20%20let%20p3%20%3D%20p1%3B%0A%0A%20%20%20%20//%201.%20Debug%20%E3%82%92%20derive%20%E3%81%97%E3%81%9F%E3%81%AE%E3%81%A7%E3%80%81%E3%83%97%E3%83%AA%E3%83%B3%E3%83%88%E3%81%A7%E3%81%8D%E3%82%8B%EF%BC%81%0A%20%20%20%20println%21%28%22p1%3A%20%7B%3A%3F%7D%22%2C%20p1%29%3B%0A%0A%20%20%20%20//%202.%20PartialEq%20%E3%82%92%20derive%20%E3%81%97%E3%81%9F%E3%81%AE%E3%81%A7%E3%80%81%E6%AF%94%E8%BC%83%E3%81%A7%E3%81%8D%E3%82%8B%EF%BC%81%0A%20%20%20%20if%20p1%20%3D%3D%20p2%20%7B%0A%20%20%20%20%20%20%20%20println%21%28%22p1%20and%20p2%20are%20equal%22%29%3B%0A%20%20%20%20%7D%20else%20%7B%0A%20%20%20%20%20%20%20%20println%21%28%22p1%20and%20p2%20are%20NOT%20equal%22%29%3B%0A%20%20%20%20%7D%0A%0A%20%20%20%20//%203.%20Copy%20%E3%82%92%20derive%20%E3%81%97%E3%81%9F%E3%81%AE%E3%81%A7%E3%80%81p1%20%E3%81%AF%E3%83%A0%E3%83%BC%E3%83%96%E3%81%95%E3%82%8C%E3%81%9A%E3%82%B3%E3%83%94%E3%83%BC%E3%81%95%E3%82%8C%E3%82%8B%0A%20%20%20%20println%21%28%22p3%3A%20%7B%3A%3F%7D%22%2C%20p3%29%3B%0A%20%20%20%20println%21%28%22p1%20is%20still%20available%3A%20%7B%3A%3F%7D%22%2C%20p1%29%3B%0A%7D)
この一行を追加するだけで、先ほどのエラーはすべて解決します。

## 17.3 主要な標準トレイト

`derive` でよく使われる主要な標準トレイトを見ていきましょう。

| トレイト | 説明 | derive 可能か |
|----------|------|--------------|
| `Debug` | `{:?}` や `{:#?}` で使われる開発者向けのデバッグ出力。`derive` で簡単に実装できる。 | ✓ |
| `PartialEq`, `Eq` | `==` 演算子による等価比較を可能にします。ほとんどの場合、`PartialEq` を `derive` すれば十分です。(`Eq` はより厳密な等価性を示しますが、今は気にする必要はありません) | ✓ |
| `Clone`, `Copy` | 
- `Clone`: `.clone()` メソッドで明示的に値を複製する。ヒープデータを持つ型（`String`, `Vec`）などは `Clone` を実装する。
- `Copy`: `Copy` トレイトを実装した型は、代入時に暗黙的に値がコピーされる（ムーブにならない）。`Copy` を実装できるのは、その型のすべてのフィールドが `Copy` を実装している場合のみ。したがって、ヒープにデータを持つ `String` や `Vec` を持つ `struct` は `Copy` になれない。

### `PartialEq` vs `Eq`, `PartialOrd` vs `Ord`

- `PartialEq`, `PartialOrd`: `f32` の `NaN` (Not a Number) のように、自分自身と比較しても等しくならない値が存在する場合のためのトレイト。ほとんどの場合は `Eq` や `Ord` まで実装して問題ない。
- `Eq`, `Ord`: 完全な等価性・順序性を持つ型のためのトレイト。

---

ここまでは `derive` で自動的に実装できるトレイトを見てきました。しかし、実際の開発では、手動で実装することでコードの柔軟性や表現力を大きく向上させる、重要な標準トレイトが数多く存在します。

## 17.4 型変換の達人：`From` と `Into`

`From` と `Into` は、ある型から別の型への変換を担う、非常に重要なトレイトです。

- `From<T>`: 型 `T` から自分自身の型 (`Self`) へ変換するロジックを実装します。
- `Into<T>`: 自分自身の型 (`Self`) から型 `T` へ変換します。

この二つには面白い関係があります。`impl From<T> for U` を実装すると、標準ライブラリが自動的に `impl Into<U> for T` を実装してくれるのです。つまり、私たちは `From` を実装するだけで、逆方向の `Into` も手に入れることができます。

### 試してみよう：タプルから `Point` を作る

タプル `(i32, i32)` から `Point` 型を生成できるように、`From` トレイトを実装してみましょう。

```rust
#[derive(Debug)]
struct Point {
    x: i32,
    y: i32,
}

// (i32, i32) 型から Point 型への変換を実装
impl From<(i32, i32)> for Point {
    fn from(tuple: (i32, i32)) -> Self {
        Point { x: tuple.0, y: tuple.1 }
    }
}

fn main() {
    let tuple = (10, 20);

    // From を使った変換
    let p1 = Point::from(tuple);
    println!("p1 from From: {:?}", p1);

    // Into を使った変換 (型推論が必要な場合がある)
    // From を実装したので、into() も自動的に使えるようになっている
    let p2: Point = tuple.into();
    println!("p2 from Into: {:?}", p2);
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=%23%5Bderive%28Debug%29%5D%0Astruct%20Point%20%7B%0A%20%20%20%20x%3A%20i32%2C%0A%20%20%20%20y%3A%20i32%2C%0A%7D%0A%0A%2F%2F%20%28i32%2C%20i32%29%20%E5%9E%8B%E3%81%8B%E3%82%89%20Point%20%E5%9E%8B%E3%81%B8%E3%81%AE%E5%A4%89%E6%8F%9B%E3%82%92%E5%AE%9F%E8%A3%85%0Aimpl%20From%3C%28i32%2C%20i32%29%3E%20for%20Point%20%7B%0A%20%20%20%20fn%20from%28tuple%3A%20%28i32%2C%20i32%29%29%20-%3E%20Self%20%7B%0A%20%20%20%20%20%20%20%20Point%20%7B%20x%3A%20tuple.0%2C%20y%3A%20tuple.1%20%7D%0A%20%20%20%20%7D%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20tuple%20%3D%20%2810%2C%2020%29%3B%0A%0A%20%20%20%20%2F%2F%20From%20%E3%82%92%E4%BD%BF%E3%81%A3%E3%81%9F%E5%A4%89%E6%8F%9B%0A%20%20%20%20let%20p1%20%3D%20Point%3A%3Afrom%28tuple%29%3B%0A%20%20%20%20println!%28%22p1%20from%20From%3A%20%7B%3A%3F%7D%22%2C%20p1%29%3B%0A%0A%20%20%20%20%2F%2F%20Into%20%E3%82%92%E4%BD%BF%E3%81%A3%E3%81%9F%E5%A4%89%E6%8F%9B%20%28%E5%9E%8B%E6%8E%A8%E8%AB%96%E3%81%8C%E5%BF%85%E8%A6%81%E3%81%AA%E5%A0%B4%E5%90%88%E3%81%8C%E3%81%82%E3%82%8B%29%0A%20%20%20%20%2F%2F%20From%20%E3%82%92%E5%AE%9F%E8%A3%85%E3%81%97%E3%81%9F%E3%81%AE%E3%81%A7%E3%80%81into%28%29%20%E3%82%82%E8%87%AA%E5%8B%95%E7%9A%84%E3%81%AB%E4%BD%BF%E3%81%88%E3%82%8B%E3%82%88%E3%81%86%E3%81%AB%E3%81%AA%E3%81%A3%E3%81%A6%E3%81%84%E3%82%8B%0A%20%20%20%20let%20p2%3A%20Point%20%3D%20tuple.into%28%29%3B%0A%20%20%20%20println!%28%22p2%20from%20Into%3A%20%7B%3A%3F%7D%22%2C%20p2%29%3B%0A%7D)
このように、`From` を実装することで、型変換のロジックをカプセル化し、コードの可読性を高めることができます。

## 17.5 柔軟な借用：`AsRef` と `AsMut`

`AsRef` と `AsMut` は、所有権を奪うことなく、ある型から別の型への「安価な参照変換」を提供するためのトレイトです。

特によく使われるのが `AsRef<str>` です。`String` と `&str` の両方を透過的に扱いたい関数を書く際に絶大な効果を発揮します。

### 試してみよう：`String` も `&str` も受け取る関数

```rust
// ジェネリックな引数 T を取り、T は AsRef<str> を実装している必要がある、という制約
fn greet<T: AsRef<str>>(name: T) {
    // .as_ref() を呼ぶことで、&str 型の参照を取得できる
    println!("Hello, {}!", name.as_ref());
}

fn main() {
    let name_string = String::from("Alice");
    let name_str = "Bob";

    // String 型を渡しても...
    greet(name_string);

    // &str 型を渡しても...
    greet(name_str);
    // どちらも同じように動作する！
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=%2F%2F%20%E3%82%B8%E3%82%A7%E3%83%8D%E3%83%AA%E3%83%83%E3%82%AF%E3%81%AA%E5%BC%95%E6%95%B0%20T%20%E3%82%92%E5%8F%96%E3%82%8A%E3%80%81T%20%E3%81%AF%20AsRef%3Cstr%3E%20%E3%82%92%E5%AE%9F%E8%A3%85%E3%81%97%E3%81%A6%E3%81%84%E3%82%8B%E5%BF%85%E8%A6%81%E3%81%8C%E3%81%82%E3%82%8B%E3%80%81%E3%81%A8%E3%81%84%E3%81%86%E5%88%B6%E7%B4%84%0Afn%20greet%3CT%3A%20AsRef%3Cstr%3E%3E%28name%3A%20T%29%20%7B%0A%20%20%20%20%2F%2F%20.as_ref%28%29%20%E3%82%92%E5%91%BC%E3%81%B6%E3%81%93%E3%81%A8%E3%81%A7%E3%80%81%26str%20%E5%9E%8B%E3%81%AE%E5%8F%82%E7%85%A7%E3%82%92%E5%8F%96%E5%BE%97%E3%81%A7%E3%81%8D%E3%82%8B%0A%20%20%20%20println!%28%22Hello%2C%20%7B%7D!%22%2C%20name.as_ref%28%29%29%3B%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20name_string%20%3D%20String%3A%3Afrom%28%22Alice%22%29%3B%0A%20%20%20%20let%20name_str%20%3D%20%22Bob%22%3B%0A%0A%20%20%20%20%2F%2F%20String%20%E5%9E%8B%E3%82%92%E6%B8%A1%E3%81%97%E3%81%A6%E3%82%82...%0A%20%20%20%20greet%28name_string%29%3B%0A%0A%20%20%20%20%2F%2F%20%26str%20%E5%9E%8B%E3%82%92%E6%B8%A1%E3%81%97%E3%81%A6%E3%82%82...%0A%20%20%20%20greet%28name_str%29%3B%0A%20%20%20%20%2F%2F%20%E3%81%A9%E3%81%A1%E3%82%89%E3%82%82%E5%90%8C%E3%81%98%E3%82%88%E3%81%86%E3%81%AB%E5%8B%95%E4%BD%9C%E3%81%99%E3%82%8B%EF%BC%81%0A%7D)
`AsRef` を使うことで、APIの利用者に無駄な型変換を強いることなく、より柔軟で使いやすい関数を設計できます。

## 17.6 パターン：ゲッターメソッド

トレイトではありませんが、実際の開発で非常によく使われるパターンが「ゲッター (Getters)」です。これは、`struct` のプライベートなフィールドの値を外部に公開するための、読み取り専用のメソッドです。

### なぜ直接アクセスしないのか？

フィールドを `pub` にして直接アクセスさせることもできますが、ゲッターメソッドを介することで「カプセル化」が実現できます。将来的に、フィールドの値を返す前に何らかの計算やフォーマットを挟む必要が出たとしても、関数のシグネチャさえ変えなければ、APIの利用者に影響を与えることなく内部実装を変更できます。

```rust
#[derive(Debug)]
struct User {
    username: String,
    age: u32,
}

impl User {
    // username のゲッター
    pub fn username(&self) -> &str {
        &self.username
    }

    // age のゲッター
    pub fn age(&self) -> u32 {
        self.age
    }
}

fn main() {
    let user = User {
        username: "Goto".to_string(),
        age: 42,
    };
    println!("User {} is {} years old.", user.username(), user.age());
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=%23%5Bderive%28Debug%29%5D%0Astruct%20User%20%7B%0A%20%20%20%20username%3A%20String%2C%0A%20%20%20%20age%3A%20u32%2C%0A%7D%0A%0Aimpl%20User%20%7B%0A%20%20%20%20%2F%2F%20username%20%E3%81%AE%E3%82%B2%E3%83%83%E3%82%BF%E3%83%BC%0A%20%20%20%20pub%20fn%20username%28%26self%29%20-%3E%20%26str%20%7B%0A%20%20%20%20%20%20%20%20%26self.username%0A%20%20%20%20%7D%0A%0A%20%20%20%20%2F%2F%20age%20%E3%81%AE%E3%82%B2%E3%83%83%E3%82%BF%E3%83%BC%0A%20%20%20%20pub%20fn%20age%28%26self%29%20-%3E%20u32%20%7B%0A%20%20%20%20%20%20%20%20self.age%0A%20%20%20%20%7D%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20user%20%3D%20User%20%7B%0A%20%20%20%20%20%20%20%20username%3A%20%22Goto%22.to_string%28%29%2C%0A%20%20%20%20%20%20%20%20age%3A%2042%2C%0A%20%20%20%20%7D%3B%0A%20%20%20%20println!%28%22User%20%7B%7D%20is%20%7B%7D%20years%20old.%22%2C%20user.username%28%29%2C%20user.age%28%29%29%3B%0A%7D)

このような定型的なコードは、`getset` のようなコミュニティクレートを使うことで自動生成することも可能です。これは、マクロの強力な一例です。

## 17.7 まとめ

- Rust で独自に定義した `struct` や `enum` は、デフォルトでは比較もコピーも表示もできない。
- `#[derive]` 属性は、`Debug`, `Clone` といった標準的なトレイトの実装をコンパイラに自動生成させるための便利な機能。
- `From` を実装することで、柔軟な型変換が可能になり、`Into` が自動的に提供される。
- `AsRef` を使うことで、`String` と `&str` のように、所有権の有無が異なる型を透過的に扱える、柔軟な関数を設計できる。
- ゲッターメソッドは、カプセル化を維持しつつ、内部のデータへの読み取りアクセスを提供する一般的なパターン。
- `Clone` は明示的な複製、`Copy` は暗黙的なコピー。`Copy` は `Clone` を実装している必要がある。
- `Debug` は開発者向け、`Display` はユーザー向けの文字列表現であり、`Display` は手動での実装が必要。

---

これで、`struct` や `enum` に基本的な振る舞いを簡単に追加する方法を学びました。次の章では、所有権システムのより高度な使い方を可能にする「スマートポインタ」について探求していきます。
