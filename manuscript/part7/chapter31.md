# 第 31 章：本番環境へのデプロイ

## この章のゴール
- `--release` フラグを使って、本番用に最適化されたバイナリをビルドできるようになる。
- 静的リンクと動的リンクの違いを理解し、Rust がデフォルトで静的リンクに近い形で依存関係を扱うことの利点を説明できる。
- Docker を使って、Rust アプリケーションをコンテナ化し、ポータビリティを高める基本的な方法を理解する。
- クロスコンパイルを使って、開発環境とは異なるターゲットアーキテクチャ（例: `x86_64` から `aarch64`）向けのバイナリをビルドできるようになる。
- CI/CD パイプライン（例: GitHub Actions）に、ビルド、テスト、デプロイのステップを組み込む基本的な考え方を理解する。

## 前章の復習
前の章では、`unsafe` コードの世界を探求しました。プログラマの責任において Rust の安全保証を一部無効にし、低レベルな操作や FFI を実現する方法と、それを安全にカプセル化する重要性について学びました。

## アプリケーションのビルド
開発サイクル中は `cargo run` や `cargo build` を使ってきましたが、これらはデバッグ情報を含み、最適化がほとんどかかっていない開発用のビルドです。本番環境にデプロイする際は、必ず `--release` フラグをつけます。
```sh
cargo build --release
```
これにより、`target/release/` ディレクトリに、最大限に最適化された実行可能ファイルが生成されます。このバイナリは、開発ビルドに比べて実行速度が大幅に向上し、ファイルサイズも小さくなることがよくあります。

## 静的リンクとポータビリティ
Rust の大きな利点の一つは、生成されるバイナリのポータビリティ（可搬性）の高さです。`cargo build` でビルドすると、`Cargo.toml` にリストされている Rust の依存関係（クレート）はすべて静的にリンクされます。これは、すべての必要なコードが単一の実行可能ファイルにまとめられることを意味します。

これにより、Python や Node.js のアプリケーションのように、実行環境に特定のバージョンのランタイムやライブラリをインストールする必要がありません。生成されたバイナリをサーバーにコピーし、実行権限を与えれば、それだけで動きます（ただし、libc のようなシステムライブラリは動的リンクされることが多いです）。

## Docker によるコンテナ化
本番環境でのデプロイにおいては、アプリケーションとその実行環境をまとめてパッケージ化できる Docker が広く使われています。

### マルチステージビルド
Rust のコンパイルには `rustc` や `cargo`、そして多くのビルド時依存が必要ですが、実行時には生成されたバイナリだけで十分です。Docker のマルチステージビルドを使うと、最終的な Docker イメージのサイズを劇的に小さくできます。

```dockerfile
# ---- ビルドステージ ----
# Rust の公式イメージをビルド環境として使用
FROM rust:1.70 AS builder

# 作業ディレクトリを作成
WORKDIR /usr/src/myapp

# まずは依存関係だけをコピーしてビルド
# これにより、コードの変更がない限り、依存関係の再ダウンロードと再コンパイルがスキップされる
COPY Cargo.toml Cargo.lock ./
RUN mkdir src && echo "fn main() {}" > src/main.rs
RUN cargo build --release

# アプリケーションのソースコードをコピー
COPY src ./src
# 再度ビルド（キャッシュが効いて高速）
RUN cargo build --release

# ---- 実行ステージ ----
# 非常に軽量な "distroless" イメージを実行環境として使用
FROM gcr.io/distroless/cc-debian11 AS runtime

# ビルドステージから、コンパイル済みのバイナリだけをコピー
COPY --from=builder /usr/src/myapp/target/release/myapp /usr/local/bin/myapp

# アプリケーションを実行
CMD ["myapp"]
```
この `Dockerfile` を使ってビルドすると、Rust のツールチェーンを含まない、非常に軽量でセキュアなコンテナイメージが作成できます。

## クロスコンパイル
開発マシン（例: macOS on Apple Silicon）から、本番サーバー（例: Linux on x86_64）向けのバイナリをビルドすることも可能です。これを**クロスコンパイル**と呼びます。
```sh
# 1. ターゲットのツールチェインを追加
rustup target add x86_64-unknown-linux-musl

# 2. ターゲットを指定してビルド
# musl を使うと、libc も静的リンクされ、完全にポータブルなバイナリが作れる
cargo build --release --target x86_64-unknown-linux-musl
```

## CI/CD パイプライン
継続的インテグレーション/継続的デプロイメント (CI/CD) は、コードの変更を自動的にテストし、問題がなければ本番環境にデプロイする仕組みです。GitHub Actions は、これを実現するための一般的なプラットフォームです。

` .github/workflows/rust.yml`
```yaml
name: Rust CI/CD

on:
  push:
    branches: [ "main" ]
  pull_request:
    branches: [ "main" ]

jobs:
  build_and_test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    - name: Build
      run: cargo build --verbose
    - name: Run tests
      run: cargo test --verbose

  deploy:
    # main ブランチへのプッシュ時のみ実行
    if: github.ref == 'refs/heads/main' && github.event_name == 'push'
    needs: build_and_test
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    - name: Build release binary
      run: cargo build --release --verbose
    - name: Deploy to server
      # ここに SCP や rsync、あるいは Docker イメージをプッシュするコマンドなどを記述する
      run: echo "Deploying..."
```

## この章のまとめ
- 本番用のビルドには必ず `cargo build --release` を使う。
- Rust が生成するバイナリは、依存クレートが静的リンクされるためポータビリティが高い。
- Docker のマルチステージビルドは、軽量でセキュアなコンテナイメージを作成するためのベストプラクティス。
- `rustup target add` と `cargo build --target` で、クロスコンパイルが可能。
- GitHub Actions のような CI/CD ツールを使うことで、ビルド、テスト、デプロイのプロセスを自動化できる。

## 旅の終わり、そして始まり
この本を通して、Rust の基本的な文法から、所有権、データ構造、抽象化、並行性、そして実践的なアプリケーション開発とデプロイまで、長い旅をしてきました。あなたは今、自信を持って Rust のプロジェクトに参加し、貢献できるだけの知識とスキルを身につけました。

Rust のエコシステムは日々成長しており、学ぶべきことは常にあります。公式ドキュメント、コミュニティ、そして何よりも自分自身でコードを書き続けることが、さらなる成長への鍵となります。

幸運を祈ります。Happy Hacking!

