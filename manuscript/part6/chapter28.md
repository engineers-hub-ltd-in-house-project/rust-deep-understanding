# 第 28 章：実践：CSVパーサーライブラリの作成

## この章のゴール
- `csv` クレートを使い、CSVデータの読み込み（デシリアライズ）と書き込み（シリアライズ）ができる。
- `anyhow` と `thiserror` クレートを組み合わせ、ライブラリのエラー報告とアプリケーションのエラー処理を分離する、実践的なエラーハンドリング戦略を実装できる。
- `clap` を使ったCLIのフロントエンドと、`csv` を使ったコアロジックを分離し、テスト可能で再利用性の高いライブラリを設計できる。

---

## 28.1 なぜこれが必要か？ データ処理と堅牢なエラーハンドリング

CSV (Comma-Separated Values) は、表形式のデータを交換するための最も一般的なフォーマットの一つです。Rustのエコシステムには、`csv` クレートという非常に強力でパフォーマンスの良いCSVパーサーがあります。

この章では、`csv` クレートを使ってCSVをRustの構造体に変換するCLIツールを構築すると同時に、`anyhow` と `thiserror` という2つのクレートを使って、より実践的でメンテナンス性の高いエラーハンドリングの方法を探求します。

## 28.2 プロジェクトの準備

```sh
cargo new csv-processor
```
でプロジェクトを作成し、`Cargo.toml` に必要なクレートを追加します。

```toml
# Cargo.toml
[dependencies]
csv = "1.1"
serde = { version = "1.0", features = ["derive"] }
```

今回のプロジェクトの仕様：
1.  世界の都市の人口データが書かれた `input.csv` を読み込む。
2.  人口が1000万人以上の都市だけをフィルタリングする。
3.  結果を `output.csv` に書き出す。

まず、プロジェクトのルートに `input.csv` を作成しましょう。

```csv
# input.csv
city,country,population
Tokyo,Japan,37435191
Delhi,India,29399141
Shanghai,China,26317104
Sao Paulo,Brazil,21846507
Mumbai,India,20185064
Kyoto,Japan,1475183
```

## 28.3 `csv` クレートによるデシリアライズとシリアライズ

`csv` クレートの基本的な使い方は `serde` と非常によく似ています。

```rust
// src/main.rs
use serde::{Deserialize, Serialize};
use std::error::Error;

// 1. CSVの行に対応する構造体を定義
#[derive(Debug, Deserialize, Serialize)]
struct Record {
    city: String,
    country: String,
    // u64にパースできない行があった場合のためにOptionで受けることもできる
    population: u64,
}

fn main() -> Result<(), Box<dyn Error>> {
    let input_path = "input.csv";
    let output_path = "output.csv";
    
    // 2. CSV Readerを作成
    // `from_path` は内部でファイルを開き、BufReaderでラップしてくれる
    let mut rdr = csv::Reader::from_path(input_path)?;
    
    // 3. CSV Writerを作成
    let mut wtr = csv::Writer::from_path(output_path)?;
    
    // ヘッダーを書き込む (オプション)
    wtr.write_record(&["city", "country", "population"])?;

    // 4. 一行ずつ処理する (ストリーミング)
    for result in rdr.deserialize() {
        let record: Record = result?;
        
        // 5. フィルタリング
        if record.population > 10_000_000 {
            // 6. レコードを書き込む
            println!("Writing record: {:?}", record);
            wtr.serialize(record)?;
        }
    }
    
    // Writerのバッファをフラッシュして、すべてのデータが書き込まれるのを保証
    wtr.flush()?;

    println!("nProcessing complete! Check '{}'", output_path);

    Ok(())
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Ause%20serde%3A%3A%7BDeserialize%2C%20Serialize%7D%3B%0Ause%20std%3A%3Aerror%3A%3AError%3B%0A%0A//%201.%20CSV%E3%81%AE%E8%A1%8C%E3%81%AB%E5%AF%BE%E5%BF%9C%E3%81%99%E3%82%8B%E6%A7%8B%E9%80%A0%E4%BD%93%E3%82%92%E5%AE%9A%E7%BE%A9%0A%23%5Bderive%28Debug%2C%20Deserialize%2C%20Serialize%29%5D%0Astruct%20Record%20%7B%0A%20%20%20%20city%3A%20String%2C%0A%20%20%20%20country%3A%20String%2C%0A%20%20%20%20//%20u64%E3%81%AB%E3%83%91%E3%83%BC%E3%82%B9%E3%81%A7%E3%81%8D%E3%81%AA%E3%81%84%E8%A1%8C%E3%81%8C%E3%81%82%E3%81%A3%E3%81%9F%E5%A0%B4%E5%90%88%E3%81%AE%E3%81%9F%E3%82%81%E3%81%ABOption%E3%81%A7%E5%8F%97%E3%81%91%E3%82%8B%E3%81%93%E3%81%A8%E3%82%82%E3%81%A7%E3%81%8D%E3%82%8B%0A%20%20%20%20population%3A%20u64%2C%0A%7D%0A%0Afn%20main%28%29%20-%3E%20Result%3C%28%29%2C%20Box%3Cdyn%20Error%3E%3E%20%7B%0A%20%20%20%20let%20input_path%20%3D%20%22input.csv%22%3B%0A%20%20%20%20let%20output_path%20%3D%20%22output.csv%22%3B%0A%20%20%20%20%0A%20%20%20%20//%202.%20CSV%20Reader%E3%82%92%E4%BD%9C%E6%88%90%0A%20%20%20%20//%20%60from_path%60%20%E3%81%AF%E5%86%85%E9%83%A8%E3%81%A7%E3%83%95%E3%82%A1%E3%82%A4%E3%83%AB%E3%82%92%E9%96%8B%E3%81%8D%E3%80%81BufReader%E3%81%A7%E3%83%A9%E3%83%83%E3%83%97%E3%81%97%E3%81%A6%E3%81%8F%E3%82%8C%E3%82%8B%0A%20%20%20%20let%20mut%20rdr%20%3D%20csv%3A%3AReader%3A%3Afrom_path%28input_path%29%3F%3B%0A%20%20%20%20%0A%20%20%20%20//%203.%20CSV%20Writer%E3%82%92%E4%BD%9C%E6%88%90%0A%20%20%20%20let%20mut%20wtr%20%3D%20csv%3A%3AWriter%3A%3Afrom_path%28output_path%29%3F%3B%0A%20%20%20%20%0A%20%20%20%20//%20%E3%83%98%E3%83%83%E3%83%80%E3%83%BC%E3%82%92%E6%9B%B8%E3%81%8D%E8%BE%BC%E3%82%80%20%28%E3%82%AA%E3%83%97%E3%82%B7%E3%83%A7%E3%83%B3%29%0A%20%20%20%20wtr.write_record%28%26%5B%22city%22%2C%20%22country%22%2C%20%22population%22%5D%29%3F%3B%0A%0A%20%20%20%20//%204.%20%E4%B8%80%E8%A1%8C%E3%81%9A%E3%81%A4%E5%87%A6%E7%90%86%E3%81%99%E3%82%8B%20%28%E3%82%B9%E3%83%88%E3%83%AA%E3%83%BC%E3%83%9F%E3%83%B3%E3%82%B0%29%0A%20%20%20%20for%20result%20in%20rdr.deserialize%28%29%20%7B%0A%20%20%20%20%20%20%20%20let%20record%3A%20Record%20%3D%20result%3F%3B%0A%20%20%20%20%20%20%20%20%0A%20%20%20%20%20%20%20%20//%205.%20%E3%83%95%E3%82%A3%E3%83%AB%E3%82%BF%E3%83%AA%E3%83%B3%E3%82%B0%0A%20%20%20%20%20%20%20%20if%20record.population%20%3E%2010_000_000%20%7B%0A%20%20%20%20%20%20%20%20%20%20%20%20//%206.%20%E3%83%AC%E3%82%B3%E3%83%BC%E3%83%89%E3%82%92%E6%9B%B8%E3%81%8D%E8%BE%BC%E3%82%80%0A%20%20%20%20%20%20%20%20%20%20%20%20println%21%28%22Writing%20record%3A%20%7B%3A%3F%7D%22%2C%20record%29%3B%0A%20%20%20%20%20%20%20%20%20%20%20%20wtr.serialize%28record%29%3F%3B%0A%20%20%20%20%20%20%20%20%7D%0A%20%20%20%20%7D%0A%20%20%20%20%0A%20%20%20%20//%20Writer%E3%81%AE%E3%83%90%E3%83%83%E3%83%95%E3%82%A1%E3%82%92%E3%83%95%E3%83%A9%E3%83%83%E3%82%B7%E3%83%A5%E3%81%97%E3%81%A6%E3%80%81%E3%81%99%E3%81%B9%E3%81%A6%E3%81%AE%E3%83%87%E3%83%BC%E3%82%BF%E3%81%8C%E6%9B%B8%E3%81%8D%E8%BE%BC%E3%81%BE%E3%82%8C%E3%82%8B%E3%81%AE%E3%82%92%E4%BF%9D%E8%A8%BC%0A%20%20%20%20wtr.flush%28%29%3F%3B%0A%0A%20%20%20%20println%21%28%22%5CnProcessing%20complete%21%20Check%20%27%7B%7D%27%22%2C%20output_path%29%3B%0A%0A%20%20%20%20Ok%28%28%29%29%0A%7D)

`cargo run` で実行すると、コンソールに処理中のレコードが表示され、プロジェクトルートに `output.csv` が生成されます。中身を確認すると、人口が1000万人以上の都市だけが書き出されているはずです。

```csv
# output.csv
city,country,population
Tokyo,Japan,37435191
Delhi,India,29399141
Shanghai,China,26317104
Sao Paulo,Brazil,21846507
Mumbai,India,20185064
```

## 28.4 `anyhow` と `thiserror` によるエラーハンドリング

第22章では、`Box<dyn Error>` を使ってエラーを処理しました。これは手軽ですが、エラーの種類に関する情報が失われてしまいます。

- `thiserror`: ライブラリ側で使う。`#[derive(Error)]` を使って、カスタムエラー型を簡単に作成できる。ライブラリの利用者がエラーの種類に応じて `match` できるように、詳細で構造化されたエラーを定義するのに適している。
- `anyhow`: アプリケーション側（`main.rs` など）で使う。様々な種類のエラーを `anyhow::Result` や `anyhow::Error` という単一の型にまとめ、`?` 演算子で簡単に伝播させることができる。コンテキストを追加する (`.context()`) のも容易。

この2つを組み合わせることで、「ライブラリは詳細なエラーを返し、アプリケーションはそれを簡単に扱う」という、理想的な関心の分離が実現できます。

## 28.5 まとめ

- `csv` クレートは、`serde` と連携し、Rustの構造体とCSVデータを効率的に相互変換（シリアライズ・デシリアライズ）するためのデファクトスタンダード。
- `csv::Reader` はCSVデータを構造体のイテレータに、`csv::Writer` は構造体をCSVデータに変換する。
- `thiserror` は、ライブラリ 側で構造化されたカスタムエラー型を定義するためのクレート。`#[derive(Error)]` が定型コードを削減する。
- `anyhow` は、アプリケーション 側で様々なエラー型を統一的に扱い、コンテキストを追加してエラー伝播を簡潔にするためのクレート。
- `clap` でCLIを、`thiserror` を使ったコアロジックをライブラリとして `lib.rs` に分離し、`main.rs` では `anyhow` を使ってそれらを統合するのは、堅牢なアプリケーションを構築するための優れたパターン。

---

これで、Rustを使った実践的なデータ処理と、高度なエラーハンドリングのパターンを学びました。次の部では、Rustの表現力をさらに高める「マクロ」と、パフォーマンスを極限まで追求するための「unsafe」な世界について探求します。

