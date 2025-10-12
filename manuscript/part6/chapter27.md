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

`cargo new csv-processor` でプロジェクトを作成し、必要なクレートを `Cargo.toml` に追加します。`csv` と `serde` はデータ処理の鉄板コンビです。

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

