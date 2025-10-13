# 第 27 章：実践プロジェクト：データ処理とファイル I/O

## この章のゴール
- `csv` クレートと `serde` クレートを組み合わせ、CSVファイルのデータを型安全なRustの構造体にデシリアライズできる。
- `BufReader` を使い、ファイルからの読み込みを効率化する方法を理解する。
- ファイル全体をメモリに読み込むのではなく、イテレータを使って一行ずつ処理する（ストリーミング処理）ことができる。
- Rustの構造体を `csv::Writer` を使ってCSVファイルにシリアライズ（書き込み）できる。

---

## 27.1 なぜこれが必要か？ 現実世界のデータと向き合う

これまでのプロジェクトでは、`std::fs::read_to_string` でファイル全体をメモリに読み込んでいました。これは手軽ですが、数ギガバイトにもなる巨大なログファイルやデータセットを扱う際には、メモリを使い果たしてしまい現実的ではありません。

また、データはJSONやプレーンテキストだけでなく、CSV（Comma-Separated Values）のような表形式で提供されることも非常に多いです。

この章では、巨大なCSVファイルを効率的かつ安全に処理する、という現実的な課題を通して、Rustでの実践的なデータ処理とファイルI/Oのテクニックを学びます。

## 27.2 プロジェクト：CSVデータのフィルタリング

```sh
cargo new csv-processor
```
でプロジェクトを作成し、必要なクレートを `Cargo.toml` に追加します。`csv` と `serde` はデータ処理の鉄板コンビです。

```toml
# Cargo.toml
[dependencies]
csv = "1.1"
serde = { version = "1.0", features = ["derive"] }
```

**今回のプロジェクトの仕様：**
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

## 27.3 `serde`と`csv`によるデータ処理

### 試してみよう：CSVを読んでフィルタリングし、書き出す

この処理を一つの`main`関数の中に実装してみましょう。ここでも `main` が `Result` を返すパターンを使います。

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

    println!("\nProcessing complete! Check '{}'", output_path);

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

## 27.4 コードの解説

- **`#[derive(Deserialize, Serialize)]`**: `serde` の魔法です。この一行で、`Record` 構造体とCSVの行（や他の多くのフォーマット）とを相互に変換できるようになります。
- **`csv::Reader::from_path(...)`**: ファイルパスを受け取り、CSVパーサーを初期化します。内部で `std::fs::File::open` と `std::io::BufReader` を使っており、効率的な読み込みを行います。
- **`rdr.deserialize()`**: これがストリーミング処理の心臓部です。CSVの行を一つずつ読み込み、`Record` 構造体にデシリアライズしようと試みる**イテレータ**を返します。ファイル全体を一度にメモリに読み込むのではなく、必要な分だけを処理するため、メモリ使用量を非常に低く抑えることができます。
- **`csv::Writer::from_path(...)`**: 書き込み用のCSVライターを作成します。
- **`wtr.serialize(record)?`**: `Record` 構造体を受け取り、CSVの一行としてシリアライズし、内部のバッファに書き込みます。
- **`wtr.flush()?`**: バッファに残っているデータをディスクに書き込みます。プログラム終了時に自動的にフラッシュされますが、明示的に呼び出すのが良い習慣です。

## 27.5 まとめ

- `csv` クレートと `serde` クレートを組み合わせることで、CSVデータを型安全かつ宣言的に扱うことができる。
- `csv::Reader` が返すイテレータを処理することで、巨大なファイルでもメモリ効率の良いストリーミング処理を簡単に実装できる。
- `csv::Writer` を使えば、Rustの構造体からCSVファイルへの書き込みも簡単に行える。
- このパターンは、CSVだけでなく、JSON Lines (`.jsonl`) やその他の行ベースのデータフォーマット処理にも広く応用できる。

---

次の章では、PythonやGoで書かれた既存のコードベースをRustに移行したり、Rustを組み込んだりするための戦略について議論します。

