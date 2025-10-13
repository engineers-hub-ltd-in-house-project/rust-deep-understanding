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

Rust のアプローチは、Go のように戻り値でエラーを表現しつつも、`Option<T>` と同じように型システムと `match` を使って、エラー処理のし忘れをコンパイル時に防ぐ、というものです。

## 11.2 回復不能なエラー：`panic!`

まず、本当にどうしようもないエラーについて話しましょう。プログラムのバグなど、続行が不可能かつ無意味な状況で使われるのが `panic!` マクロです。

### 試してみよう：プログラムをパニックさせる

```sh
cargo new errors
```
でプロジェクトを作り、`src/main.rs` に以下を記述してください。

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20//%20%E3%81%93%E3%81%AE%E8%A1%8C%E3%81%AE%E3%82%B3%E3%83%A1%E3%83%B3%E3%83%88%E3%82%92%E5%A4%96%E3%81%99%E3%81%A8%E3%83%97%E3%83%AD%E3%82%B0%E3%83%A9%E3%83%A0%E3%81%8C%E3%82%AF%E3%83%A9%E3%83%83%E3%82%B7%E3%83%A5%E3%81%99%E3%82%8B%0A%20%20%20%20//%20panic%21%28%22crash%20and%20burn%22%29%3B%0A%0A%20%20%20%20let%20v%20%3D%20vec%21%5B1%2C%202%2C%203%5D%3B%0A%20%20%20%20//%20%E7%AF%84%E5%9B%B2%E5%A4%96%E3%81%AE%E3%82%A4%E3%83%B3%E3%83%87%E3%83%83%E3%82%AF%E3%82%B9%E3%81%AB%E3%82%A2%E3%82%AF%E3%82%BB%E3%82%B9%E3%81%99%E3%82%8B%E3%81%A8%E3%83%91%E3%83%8B%E3%83%83%E3%82%AF%E3%81%8C%E8%B5%B7%E3%81%8D%E3%82%8B%0A%20%20%20%20v%5B99%5D%3B%0A%7D)
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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=enum%20Result%3CT%2C%20E%3E%20%7B%0A%20%20%20%20Ok%28T%29%2C%20%20%20%20//%20%E5%87%A6%E7%90%86%E3%81%8C%E6%88%90%E5%8A%9F%E3%81%97%E3%81%9F%E5%A0%B4%E5%90%88%E3%80%81T%20%E5%9E%8B%E3%81%AE%E5%80%A4%E3%82%92%E4%BF%9D%E6%8C%81%0A%20%20%20%20Err%28E%29%2C%20%20%20//%20%E5%87%A6%E7%90%86%E3%81%8C%E5%A4%B1%E6%95%97%E3%81%97%E3%81%9F%E5%A0%B4%E5%90%88%E3%80%81E%20%E5%9E%8B%E3%81%AE%E3%82%A8%E3%83%A9%E3%83%BC%E5%80%A4%E3%82%92%E4%BF%9D%E6%8C%81%0A%7D)
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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Afs%3A%3AFile%3B%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20let%20f_result%20%3D%20File%3A%3Aopen%28%22hello.txt%22%29%3B%0A%0A%20%20%20%20let%20f%20%3D%20match%20f_result%20%7B%0A%20%20%20%20%20%20%20%20Ok%28file%29%20%3D%3E%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20println%21%28%22%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%81%8C%E9%96%8B%E3%81%8D%E3%81%BE%E3%81%97%E3%81%9F%E3%80%82%22%29%3B%0A%20%20%20%20%20%20%20%20%20%20%20%20file%20//%20file%20%E3%83%8F%E3%83%B3%E3%83%89%E3%83%AB%E3%82%92%E8%BF%94%E3%81%99%0A%20%20%20%20%20%20%20%20%7D%2C%0A%20%20%20%20%20%20%20%20Err%28error%29%20%3D%3E%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20panic%21%28%22%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%82%92%E9%96%8B%E3%81%91%E3%81%BE%E3%81%9B%E3%82%93%E3%81%A7%E3%81%97%E3%81%9F%3A%20%7B%3A%3F%7D%22%2C%20error%29%3B%0A%20%20%20%20%20%20%20%20%7D%2C%0A%20%20%20%20%7D%3B%0A%7D)
`match` を使うことで、成功した場合 (`Ok`) と失敗した場合 (`Err`) の両方の処理を記述することをコンパイラが強制します。これにより、「エラーケースの考慮漏れ」というバグを未然に防ぎます。

## 11.4 エラー処理のショートカット：`?` 演算子

`match` は安全ですが、エラーが発生した場合に即座に関数から `Err` を返したい、という処理は頻繁に発生します。Go の `if err != nil { return err }` がまさにそれです。

毎回 `match` を書くのは少し冗長です。そこで Rust には、このパターンを簡潔に書くための `?` 演算子が用意されています。

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=use%20std%3A%3Afs%3A%3AFile%3B%0Ause%20std%3A%3Aio%3A%3A%7Bself%2C%20Read%7D%3B%0A%0A//%20%60match%60%20%E3%82%92%E4%BD%BF%E3%81%A3%E3%81%9F%E5%86%97%E9%95%B7%E3%81%AA%E3%83%90%E3%83%BC%E3%82%B8%E3%83%A7%E3%83%B3%0Afn%20read_username_from_file_long%28%29%20-%3E%20Result%3CString%2C%20io%3A%3AError%3E%20%7B%0A%20%20%20%20let%20f_result%20%3D%20File%3A%3Aopen%28%22username.txt%22%29%3B%0A%0A%20%20%20%20let%20mut%20f%20%3D%20match%20f_result%20%7B%0A%20%20%20%20%20%20%20%20Ok%28file%29%20%3D%3E%20file%2C%0A%20%20%20%20%20%20%20%20Err%28e%29%20%3D%3E%20return%20Err%28e%29%2C%20//%20%E3%82%A8%E3%83%A9%E3%83%BC%E3%81%AE%E5%A0%B4%E5%90%88%E3%81%AF%E5%8D%B3%E5%BA%A7%E3%81%AB%20return%0A%20%20%20%20%7D%3B%0A%0A%20%20%20%20let%20mut%20s%20%3D%20String%3A%3Anew%28%29%3B%0A%0A%20%20%20%20match%20f.read_to_string%28%26mut%20s%29%20%7B%0A%20%20%20%20%20%20%20%20Ok%28_%29%20%3D%3E%20Ok%28s%29%2C%0A%20%20%20%20%20%20%20%20Err%28e%29%20%3D%3E%20Err%28e%29%2C%20//%20%E3%81%93%E3%81%93%E3%81%A7%E3%82%82%E3%82%A8%E3%83%A9%E3%83%BC%E3%82%92%E4%BC%9D%E6%92%AD%0A%20%20%20%20%7D%0A%7D%0A%0A//%20%60%3F%60%20%E6%BC%94%E7%AE%97%E5%AD%90%E3%82%92%E4%BD%BF%E3%81%A3%E3%81%9F%E7%B0%A1%E6%BD%94%E3%81%AA%E3%83%90%E3%83%BC%E3%82%B8%E3%83%A7%E3%83%B3%0Afn%20read_username_from_file_short%28%29%20-%3E%20Result%3CString%2C%20io%3A%3AError%3E%20%7B%0A%20%20%20%20let%20mut%20f%20%3D%20File%3A%3Aopen%28%22username.txt%22%29%3F%3B%0A%20%20%20%20let%20mut%20s%20%3D%20String%3A%3Anew%28%29%3B%0A%20%20%20%20f.read_to_string%28%26mut%20s%29%3F%3B%0A%20%20%20%20Ok%28s%29%0A%7D%0A%0Afn%20main%28%29%20%7B%0A%20%20%20%20//%20%E5%AD%98%E5%9C%A8%E3%81%97%E3%81%AA%E3%81%84%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%81%AA%E3%81%AE%E3%81%A7%20Err%20%E3%81%8C%E8%A1%A8%E7%A4%BA%E3%81%95%E3%82%8C%E3%82%8B%E3%81%AF%E3%81%9A%0A%20%20%20%20println%21%28%22%7B%3A%3F%7D%22%2C%20read_username_from_file_short%28%29%29%3B%0A%7D)
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
