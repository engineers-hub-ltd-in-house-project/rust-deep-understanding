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

## 17.4 まとめ

- Rust で独自に定義した `struct` や `enum` は、デフォルトでは比較もコピーも表示もできない。
- `#[derive]` 属性は、`Debug`, `Clone` といった標準的なトレイトの実装をコンパイラに自動生成させるための便利な機能。
- `Clone` は明示的な複製、`Copy` は暗黙的なコピー。`Copy` は `Clone` を実装している必要がある。
- `Debug` は開発者向け、`Display` はユーザー向けの文字列表現であり、`Display` は手動での実装が必要。

---

これで、`struct` や `enum` に基本的な振る舞いを簡単に追加する方法を学びました。次の章では、所有権システムのより高度な使い方を可能にする「スマートポインタ」について探求していきます。
