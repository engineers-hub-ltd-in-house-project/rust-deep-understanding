# 第 11 章：Result とエラーハンドリング

## この章のゴール
- 回復不能なエラー (`panic!`) と回復可能なエラー (`Result`) の違いを説明できる。
- `Result<T, E>` 型を `match` を使って安全に処理し、成功時と失敗時の両方のケースを記述できる。
- Go の `if err != nil` パターンと比較し、`?` 演算子がどのようにエラー処理を簡潔にするかを体験する。

---

## 11.1 エラーをどう表現するか？

Python と Go では、エラーの扱い方が大きく異なりました。

- Python: `try...except` ブロックで例外を捕捉していました。
- Go: `if err != nil` で、関数の戻り値 `error` を都度チェックする必要がありました。

Rust のアプローチは、Go のように戻り値でエラーを表現しつつも、`Option<T>` と同じように **型システム** と **`match`** を使って、エラー処理のし忘れをコンパイル時に防ぐ、というものです。

## 11.2 回復不能なエラー：`panic!`

まず、本当にどうしようもないエラーについて話しましょう。プログラムのバグなど、続行が不可能かつ無意味な状況で使われるのが `panic!` マクロです。

### 試してみよう：プログラムをパニックさせる

`cargo new errors` でプロジェクトを作り、`src/main.rs` に以下を記述してください。

```rust
// src/main.rs

fn main() {
    // この行のコメントを外すとプログラムがクラッシュする
    // panic!("crash and burn");

    let v = vec![1, 2, 3];
    // 範囲外のインデックスにアクセスするとパニックが起きる
    v[99];
}
```
`cargo run` を実行すると、プログラムは途中で停止し、エラーメッセージが表示されます。

```text
thread 'main' panicked at 'index out of bounds: the len is 3 but the index is 99'
```

これがパニックです。`panic!` は、問題が発生した地点でプログラムを即座に停止させます。Go で `panic()` を呼び出すのと同じです。
一般的に、アプリケーションコードで `panic!` を直接呼び出すのは、よほどのことがない限り避けるべきです。特に、ライブラリのコードでは決して使うべきではありません。

## 11.3 回復可能なエラー：`Result<T, E>`

ファイルが見つからないなど、プログラムが対処して処理を続行できる可能性のあるエラーは、**回復可能なエラー** として扱います。そのための道具が `Result<T, E>` です。

`Result<T, E>` は、`Option<T>` と同様に `enum` として定義されています。

```rust
enum Result<T, E> {
    Ok(T),    // 処理が成功した場合、T 型の値を保持
    Err(E),   // 処理が失敗した場合、E 型のエラー値を保持
}
```
`<T>` は成功時の値の型 (Type)、`<E>` は失敗時のエラーの型 (Error) を表します。

### `match` で `Result` を処理する

ファイルを開くという、失敗する可能性のある一般的な操作を例に見てみましょう。`File::open` 関数は `Result<std::fs::File, std::io::Error>` を返します。

`Option<T>` の時と同様に、`Result` は `match` で安全に処理します。

```rust
use std::fs::File;

fn main() {
    let f_result = File::open("hello.txt");

    let f = match f_result {
        Ok(file) => {
            println!("ファイルが開きました。");
            file // file ハンドルを返す
        },
        Err(error) => {
            panic!("ファイルを開けませんでした: {:?}", error);
        },
    };
}
```
`match` を使うことで、成功した場合 (`Ok`) と失敗した場合 (`Err`) の両方の処理を記述することをコンパイラが強制します。これにより、「エラーケースの考慮漏れ」というバグを未然に防ぎます。

## 11.4 エラー処理のショートカット：`?` 演算子

`match` は安全ですが、エラーが発生した場合に即座に関数から `Err` を返したい、という処理は頻繁に発生します。Go の `if err != nil { return err }` がまさにそれです。

毎回 `match` を書くのは少し冗長です。そこで Rust には、このパターンを簡潔に書くための **`?` 演算子** が用意されています。

### 試してみよう：`?` でコードを綺麗にする

ファイルからユーザ名を読み出す関数を例に、`?` の威力を体験しましょう。

```rust
use std::fs::File;
use std::io::{self, Read};

// `match` を使った冗長なバージョン
fn read_username_from_file_long() -> Result<String, io::Error> {
    let f_result = File::open("username.txt");

    let mut f = match f_result {
        Ok(file) => file,
        Err(e) => return Err(e), // エラーの場合は即座に return
    };

    let mut s = String::new();

    match f.read_to_string(&mut s) {
        Ok(_) => Ok(s),
        Err(e) => Err(e), // ここでもエラーを伝播
    }
}

// `?` 演算子を使った簡潔なバージョン
fn read_username_from_file_short() -> Result<String, io::Error> {
    let mut f = File::open("username.txt")?;
    let mut s = String::new();
    f.read_to_string(&mut s)?;
    Ok(s)
}

fn main() {
    // 存在しないファイルなので Err が表示されるはず
    println!("{:?}", read_username_from_file_short());
}
```
`read_username_from_file_long` 関数と `read_username_from_file_short` 関数は、全く同じ処理を行っています。

`?` 演算子は、`Result` 型の値に対してのみ使え、以下のように動作します。
- `Result` の値が `Ok(T)` であれば、`T` の値をそのまま取り出して式の評価を続ける。
- `Result` の値が `Err(E)` であれば、その `Err(E)` を関数全体からの戻り値として即座に `return` する。

これにより、Go で頻出する `if err != nil` の定型句を排除し、成功した場合の処理（ハッピーパス）だけを一直線に記述できるため、コードが非常に読みやすくなります。

**注意**: `?` 演算子が使えるのは、戻り値の型が `Result` または `Option` の関数内に限られます。

## 11.5 まとめ

- ✓ Rust のエラーは、`panic!` で処理する「回復不能なエラー」と、`Result` で処理する「回復可能なエラー」に大別される。
- ✓ `Result<T, E>` は、成功 (`Ok(T)`) と失敗 (`Err(E)`) の 2 つの状態を表す `enum`。
- ✓ `match` 式は、`Result` の `Ok` と `Err` の両方を網羅的に処理するための最も基本的な方法。
- ✓ `?` 演算子は、`Result` が `Err` の場合に即座に関数からエラーを返す処理を簡潔に書くためのショートカット。

---

`Result<T, E>` と `?` 演算子は、堅牢でかつ読みやすいエラーハンドリングを実現する、Rust の強力な機能です。

次の章では、`Vec` や `HashMap` といった、より実践的なデータを扱うためのコレクション型について学びます。
