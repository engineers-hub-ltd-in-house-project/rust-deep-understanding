# 付録 D：参考資料とコミュニティ

Rust を学び続ける上で役立つ、質の高い情報源とコミュニティを紹介します。

## 公式ドキュメント
Rust プロジェクトが提供する公式の資料は、常に最も正確で最新の情報源です。
- **The Rust Programming Language (TRPL):** 通称 "the book"。この本で学んだ内容を、より深く、体系的に解説しています。Rust の基本的な考え方を理解するための最良の出発点です。
  - [https://doc.rust-lang.org/book/](https://doc.rust-lang.org/book/)
- **Rust by Example:** 具体的なコード例を通じて、Rust の文法や標準ライブラリの使い方を実践的に学べるリソースです。
  - [https://doc.rust-lang.org/rust-by-example/](https://doc.rust-lang.org/rust-by-example/)
- **The Rustonomicon:** `unsafe` Rust の深淵を覗くためのガイド。安全な抽象化を構築するために `unsafe` を正しく使う方法について解説しています。
  - [https://doc.rust-lang.org/nomicon/](https://doc.rust-lang.org/nomicon/)
- **The Cargo Book:** パッケージマネージャである Cargo の詳細な使い方、設定、ベストプラクティスについて解説しています。
  - [https://doc.rust-lang.org/cargo/](https://doc.rust-lang.org/cargo/)
- **Standard Library API Reference:** 標準ライブラリのすべての型、関数、マクロについてのリファレンスドキュメント。具体的な使い方を知りたいときに非常に役立ちます。
  - [https://doc.rust-lang.org/std/](https://doc.rust-lang.org/std/)

## 主要なクレートとエコシステム
- **crates.io:** Rust の公式パッケージレジストリ。あらゆる種類のライブラリ（クレート）をここから見つけることができます。
- **Tokio:** 非同期ランタイムのデファクトスタンダード。Web サーバー、ネットワークサービスなど、高パフォーマンスな非同期アプリケーションを構築するためのエコシステムです。
- **Serde:** Rust のデータ構造を様々なフォーマット（JSON, YAML, CSV など）にシリアライズ・デシリアライズするためのフレームワーク。
- **actix-web, axum, rocket:** 人気のある Web アプリケーションフレームワーク。
- **clap, structopt:** 高機能な CLI アプリケーションを簡単に作成するためのライブラリ。
- **Rayon:** データ並列処理を簡単に行うためのライブラリ。イテレータの `.iter()` を `.par_iter()` に変えるだけで、処理を並列化できます。

## コミュニティ
- **Rust Users Forum:** Rust に関する質問や議論を行うための公式フォーラム。初心者からエキスパートまで、多くの Rustacean（Rust ユーザーの愛称）が参加しています。
  - [https://users.rust-lang.org/](https://users.rust-lang.org/)
- **Rust Discord/Zulip:** よりリアルタイムなチャット形式でのコミュニケーションが行われています。
- **Reddit (/r/rust):** Rust に関するニュース、ブログ記事、プロジェクトの紹介などが投稿されるコミュニティ。
- **This Week in Rust:** 毎週発行されるニュースレター。Rust プロジェクトやエコシステムの最新動向を追うのに最適です。
  - [https://this-week-in-rust.org/](https://this-week-in-rust.org/)

これらのリソースを活用し、コミュニティに参加することで、あなたの Rust の旅はさらに豊かで実りあるものになるでしょう。
