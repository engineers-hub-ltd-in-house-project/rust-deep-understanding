# 第29章 実践：CSV パーサーライブラリの作成

## この章のゴール
- `csv` クレートを使い、CSV データの読み込み（デシリアライズ）と書き込み（シリアライズ）ができる。
- `anyhow` と `thiserror` クレートを組み合わせ、ライブラリのエラー報告とアプリケーションのエラー処理を分離する、実践的なエラーハンドリング戦略を実装できる。
- `clap` を使った CLI のフロントエンドと、`csv` を使ったコアロジックを分離し、テスト可能で再利用性の高いライブラリを設計できる。

---

## 29.1 なぜこれが必要か？ データ処理と堅牢なエラーハンドリング
CSV (Comma-Separated Values) は、表形式のデータを交換するための最も一般的なフォーマットの一つです。Rust のエコシステムには、`csv` クレートという非常に強力でパフォーマンスの良い CSV パーサーがあります。

この章では、`csv` クレートを使って CSV を Rust の構造体に変換する CLI ツールを構築すると同時に、`anyhow` と `thiserror` という2つのクレートを使って、より実践的でメンテナンス性の高いエラーハンドリングの方法を探求します。

## 29.2 プロジェクトの準備
`cargo new csv-processor` でプロジェクトを作成し、`Cargo.toml` に必要なクレートを追加します。

```toml
# Cargo.toml
[dependencies]
csv = "1.1"
serde = { version = "1.0", features = ["derive"] }
anyhow = "1.0"
thiserror = "1.0"
clap = { version = "4.0", features = ["derive"] }
```

今回のプロジェクトは、入力ファイルから特定の条件でデータを抽出し、出力ファイルに書き込む CLI ツールです。

## 29.3 `csv` クレートによるデシリアライズとシリアライズ
`csv` クレートの基本的な使い方は `serde` と非常によく似ています。

```rust
// src/main.rs
use serde::{Deserialize, Serialize};
use std::error::Error;

// CSV の行に対応する構造体を定義
#[derive(Debug, Deserialize, Serialize)]
struct Record {
    city: String,
    country: String,
    population: u64,
}

fn process_csv(input_path: &str, output_path: &str) -> Result<(), Box<dyn Error>> {
    let mut rdr = csv::Reader::from_path(input_path)?;
    let mut wtr = csv::Writer::from_path(output_path)?;

    wtr.write_record(&["city", "country", "population"])?;

    for result in rdr.deserialize() {
        let record: Record = result?;
        if record.population > 10_000_000 {
            wtr.serialize(record)?;
        }
    }
    
    wtr.flush()?;
    Ok(())
}
```
`deserialize` メソッドはイテレータを返し、一行ずつストリーミング処理するため、巨大なファイルでもメモリを効率的に使えます。

## 29.4 `anyhow` と `thiserror` によるエラーハンドリング
第11章では `Box<dyn Error>` を使いましたが、これではエラーの種類に関する情報が失われます。`thiserror` と `anyhow` を使うと、この問題を解決できます。

- **`thiserror`**: ライブラリ側で使う。カスタムエラー型を簡単に作成できます。
- **`anyhow`**: アプリケーション側で使う。様々なエラーを `anyhow::Error` 型に統一的に扱えます。

```rust
// src/lib.rs
use thiserror::Error;

#[derive(Error, Debug)]
pub enum MyError {
    #[error("CSV processing error: {0}")]
    CsvError(#[from] csv::Error),
    #[error("I/O error: {0}")]
    IoError(#[from] std::io::Error),
}

// main.rs では anyhow を使う
fn main() -> anyhow::Result<()> {
    // ... clap で引数をパース ...
    if let Err(e) = csv_processor::process(config) {
        eprintln!("Application error: {}", e);
        std::process::exit(1);
    }
    Ok(())
}
```
このパターンにより、「ライブラリは詳細なエラーを返し、アプリケーションはそれを簡単に扱う」という関心の分離が実現できます。

## 29.5 まとめ
- `csv` クレートは、`serde` と連携し、Rust の構造体と CSV データを効率的に相互変換します。
- `thiserror` は、ライブラリ側で構造化されたカスタムエラー型を定義するのに役立ちます。
- `anyhow` は、アプリケーション側で様々なエラー型を統一的に扱い、エラー伝播を簡潔にします。
- CLI (`main.rs`) とコアロジック (`lib.rs`) を分離するのは、堅牢なアプリケーションを構築するための優れたパターンです。

---
次の章では、ターミナル上で動作するインタラクティブなアプリケーション、TUI (Text User Interface) の作成に挑戦します。
