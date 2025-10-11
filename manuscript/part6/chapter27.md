# 第 27 章：データ処理とファイル I/O

## この章のゴール
- `std::fs` を使った同期的なファイル読み書きと、`tokio::fs` を使った非同期的なファイル読み書きの違いを理解する。
- バッファリングされた I/O (`BufReader`, `BufWriter`) を使って、パフォーマンスを向上させる方法を理解する。
- `serde` と `serde_json` / `serde_yaml` / `csv` などのクレートを使って、様々なフォーマットのデータをシリアライズ・デシリアライズできるようになる。
- イテレータやストリーム (`Stream`) を使って、大きなデータを効率的に処理する方法を理解する。

## 前章の復習
前の章では、`axum` を使って非同期 Web アプリケーションを構築する基本を学びました。ルーティング、リクエストの抽出、状態共有、JSON の扱い方などを習得しました。

## なぜこれが必要なのか？
多くのアプリケーションは、データを永続化したり、外部のデータソースと連携したりする必要があります。これは、設定ファイルを読み込んだり、ログを書き出したり、ユーザーがアップロードしたファイルを処理したり、CSV や JSON 形式のデータセットを解析したりといった形で現れます。Rust は、これらのタスクを効率的かつ安全に実行するための強力なツールとライブラリを提供しています。

## 同期 I/O vs 非同期 I/O

### 同期ファイル I/O: `std::fs`
CLI ツールや、I/O 処理がプログラムのボトルネックにならない場面では、標準ライブラリの `std::fs` を使うのが最もシンプルです。
```rust
use std::fs::File;
use std::io::{Read, Write};

fn main() -> std::io::Result<()> {
    // ファイルへの書き込み
    let mut file = File::create("foo.txt")?;
    file.write_all(b"Hello, world!")?;

    // ファイルからの読み込み
    let mut file = File::open("foo.txt")?;
    let mut contents = String::new();
    file.read_to_string(&mut contents)?;
    
    assert_eq!(contents, "Hello, world!");
    Ok(())
}
```
これらの操作は、完了するまで現在のスレッドをブロックします。

### 非同期ファイル I/O: `tokio::fs`
`tokio` を使った非同期アプリケーション（例: Web サーバー）内では、`std::fs` を使うとランタイム全体をブロックしてしまうため、`tokio::fs` を使う必要があります。API は `std::fs` と非常に似ていますが、すべての関数が `async` になっています。
```rust
use tokio::fs::File;
use tokio::io::{AsyncReadExt, AsyncWriteExt};

async fn main() -> std::io::Result<()> {
    let mut file = File::create("foo.txt").await?;
    file.write_all(b"Hello, tokio!").await?;

    let mut file = File::open("foo.txt").await?;
    let mut contents = Vec::new();
    file.read_to_end(&mut contents).await?;

    Ok(())
}
```

## データフォーマットのシリアライズとデシリアライズ
`serde` は、Rust のデータ構造と様々なデータフォーマットとの間で、シリアライズ（構造体 → データ）とデシリアライズ（データ → 構造体）を行うためのフレームワークです。
`serde` 自体はフォーマットに依存せず、`serde_json`, `serde_yaml`, `csv` といったクレートと組み合わせて使います。

### JSON の例
```rust
use serde::{Serialize, Deserialize};

#[derive(Serialize, Deserialize, Debug)]
struct Point {
    x: i32,
    y: i32,
}

fn main() -> serde_json::Result<()> {
    let point = Point { x: 1, y: 2 };

    // シリアライズ: Point -> JSON String
    let serialized = serde_json::to_string(&point)?;
    println!("serialized = {}", serialized);

    // デシリアライズ: JSON String -> Point
    let deserialized: Point = serde_json::from_str(&serialized)?;
    println!("deserialized = {:?}", deserialized);

    Ok(())
}
```
構造体に `#[derive(Serialize, Deserialize)]` を追加するだけで、あとはライブラリがよしなに処理してくれます。

### CSV の例
CSV のような行ベースのデータも簡単に扱うことができます。
```toml
# Cargo.toml
[dependencies]
csv = "1.1"
serde = { version = "1.0", features = ["derive"] }
```
```rust
// ... Point 構造体の定義は同じ ...

fn write_csv() -> Result<(), csv::Error> {
    let mut writer = csv::Writer::from_path("points.csv")?;
    writer.write_record(&["x", "y"])?;
    writer.serialize(Point { x: 1, y: 2 })?;
    writer.serialize(Point { x: 3, y: 4 })?;
    writer.flush()?;
    Ok(())
}
```

## 大きなデータの効率的な処理
数ギガバイトにもなるような巨大なファイルを一度にメモリに読み込むのは現実的ではありません。このような場合は、データを少しずつ（チャンクごとに）処理する必要があります。

### バッファリング
`std::io::BufReader` は、内部にバッファを持つことで、OS に対する `read` システムコールの回数を減らし、パフォーマンスを向上させます。特に、一行ずつ読み込むような場合に効果的です。
```rust
use std::fs::File;
use std::io::{BufRead, BufReader};

let file = File::open("large_file.txt")?;
let reader = BufReader::new(file);

for line in reader.lines() {
    // 1行ずつ処理する
    println!("{}", line?);
}
```

### ストリーム
非同期の世界では、イテレータに相当するものが**ストリーム (`Stream`)** です。`tokio_stream` クレートなどを使うと、非同期的に生成される値のシーケンスを扱うことができます。例えば、HTTP リクエストのボディをストリーミングで処理したり、データベースからの大量のレコードを一つずつ処理したりするのに使われます。

## この章のまとめ
- 同期 I/O (`std::fs`) はシンプルだがスレッドをブロックする。非同期 I/O (`tokio::fs`) は `async` アプリケーション内で使う。
- `serde` は、`serde_json` や `csv` といったクレートと組み合わせることで、様々なデータフォーマットのシリアライズ・デシリアライズを簡単に行える。
- `BufReader` は、バッファリングによってファイル読み込みのパフォーマンスを向上させる。
- 大きなデータを扱う際は、一度にすべてをメモリに読み込むのではなく、イテレータやストリームを使ってチャンクごとに処理するのが基本。

## 次の章へ
Rust でのアプリケーション開発にも慣れてきました。しかし、既存の Python や Go のコードベースをどう扱うか、という問題が残っています。次の章では、Rust を既存のシステムに組み込んだり、Python/Go で書かれたコードを Rust に移行したりするための戦略について議論します。

