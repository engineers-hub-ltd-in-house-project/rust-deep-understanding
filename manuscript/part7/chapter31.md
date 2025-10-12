# 第 31 章：本番環境へのデプロイ：世界への公開

## この章のゴール
- `cargo build --release` で本番用に最適化されたバイナリをビルドすることの重要性を再確認する。
- Dockerのマルチステージビルドを使い、軽量でセキュアなRustアプリケーションのコンテナイメージを構築できるようになる。
- GitHub Actionsを使い、コードのテストとビルドを自動化する基本的なCIパイプラインを構築できるようになる。
- これまで学んだ知識を統合し、Rustアプリケーションを開発からデプロイまで導く流れを体験する。

---

## 31.1 なぜこれが必要か？ 開発から運用へ

これまでに、私たちはCLIツールやWeb APIサーバーなど、様々なRustアプリケーションを構築してきました。しかし、`cargo run`でローカルマシンで動かすことと、それを24時間365日安定して稼働させる**本番環境（Production）**で運用することは、全く別の挑戦です。

この最終章では、開発したアプリケーションを、再現可能でポータブルな形でパッケージングし、継続的にデプロイするための現代的な手法を体験します。

## 31.2 Step 1: リリースビルドの作成

本番環境にデプロイするバイナリは、必ず最適化された**リリースビルド**でなければなりません。これは、`--release` フラグを付けてビルドすることで作成できます。

```bash
# プロジェクトのルートディレクトリで実行
cargo build --release
```

これにより、`target/release/` ディレクトリに、最大限に最適化された実行可能ファイルが生成されます。Rustの大きな利点の一つは、多くの依存関係がこの単一のバイナリに静的リンクされるため、PythonやNode.jsのように実行環境にランタイムを別途インストールする必要がない点です。

## 31.3 Step 2: Dockerによるコンテナ化を体験する

「自分のPCでは動いたのに、サーバー上では動かない...」という問題を解決するのが**コンテナ化**です。Dockerを使うことで、アプリケーションとそれが動くのに必要なOSライブラリなどをひとまとめにした「コンテナイメージ」を作成できます。

ここでは、第26章で作成した `web-api-server` をコンテナ化してみましょう。

### 試してみよう：Dockerfileでマルチステージビルド

Rustのコンパイルにはツールチェーンが必要ですが、実行時にはコンパイル済みのバイナリさえあれば十分です。**マルチステージビルド**という手法を使うと、最終的なイメージサイズを劇的に小さくできます。

プロジェクトのルートディレクトリに、`Dockerfile` という名前で以下のファイルを作成してください。

```dockerfile
# Dockerfile

# --- 1. ビルドステージ ---
# ビルド用のイメージ。Rustの公式イメージを使用。
FROM rust:1-slim AS builder

# 作業ディレクトリを作成
WORKDIR /app

# まず依存関係のみをコピーしてビルドキャッシュを効かせる
COPY Cargo.toml Cargo.lock ./
# ダミーのmain.rsを作成して依存関係をビルド
RUN mkdir src && echo "fn main(){}" > src/main.rs && cargo build --release

# 次にソースコードをコピー
COPY src ./src

# アプリケーションをビルド
RUN cargo build --release

# --- 2. 実行ステージ ---
# 実行用のイメージ。Debianベースの非常に軽量なイメージ。
FROM debian:12-slim AS runtime

# ビルダーからコンパイル済みのバイナリをコピー
COPY --from=builder /app/target/release/web-api-server /usr/local/bin/

# コンテナ起動時に実行するコマンド
CMD ["web-api-server"]
```

この`Dockerfile`を使って、以下のコマンドでコンテナイメージをビルドし、実行してみましょう。

```bash
# Dockerイメージをビルド
# docker build -t <イメージ名> <Dockerfileがあるディレクトリ>
docker build -t web-api-server .

# ビルドしたイメージからコンテナを実行
# docker run -p <ホストのポート>:<コンテナのポート> <イメージ名>
docker run -p 3000:3000 web-api-server
```

これで、`localhost:3000` にアクセスすれば、コンテナ内で実行されているAPIサーバーに接続できるはずです。Rustコンパイラすら入っていない軽量な環境で、私たちのアプリが動いていることを確認してください。

## 31.4 Step 3: GitHub ActionsによるCI/CDを体験する

**CI/CD (Continuous Integration/Continuous Deployment)** は、コードの変更を自動的にテストし、デプロイする仕組みです。これにより、手作業によるミスを防ぎ、高速で信頼性の高いリリースサイクルを実現します。

### 試してみよう：GitHub ActionsでCIパイプラインを構築

GitHubリポジトリにコードをプッシュするたびに、自動でテストとビルドが実行されるようにしてみましょう。

プロジェクトのルートに `.github/workflows/` というディレクトリを作成し、その中に `rust.yml` という名前で以下のファイルを作成します。

```yaml
# .github/workflows/rust.yml

name: Rust CI

# mainブランチへのpushまたはpull_request時に実行
on:
  push:
    branches: [ "main" ]
  pull_request:
    branches: [ "main" ]

jobs:
  build_and_test:
    # Ubuntuの最新版で実行
    runs-on: ubuntu-latest

    steps:
      # 1. リポジトリのコードをチェックアウト
      - uses: actions/checkout@v4

      # 2. Rustツールチェーンのキャッシュを設定
      - name: Cache Rust dependencies
        uses: Swatinem/rust-cache@v2

      # 3. Clippyでリントチェック
      - name: Run Clippy
        run: cargo clippy -- -D warnings

      # 4. テストを実行
      - name: Run tests
        run: cargo test --verbose

      # 5. リリースビルドを実行
      - name: Build release binary
        run: cargo build --release --verbose

      # 6. (オプション) ビルドしたバイナリをアーティファクトとして保存
      - name: Upload artifact
        uses: actions/upload-artifact@v4
        with:
          name: web-api-server-binary
          path: target/release/web-api-server
```

このファイルをリポジトリに追加してGitHubにプッシュすると、"Actions"タブでワークフローが自動的に実行されるのが確認できます。これにより、あなたのプロジェクトはコード品質を常に高く保つための第一歩を踏み出しました。

## 31.5 旅の終わり、そして始まり

この本を通して、あなたはRustの基本的な文法から、その魂である所有権システム、高度な抽象化機能、並行処理、そして実践的なアプリケーション開発とデプロイまで、長い旅をしてきました。あなたは今、自信を持ってRustのプロジェクトに参加し、貢献できるだけの知識とスキルを身につけています。

Rustのエコシステムは日々成長しており、学ぶべきことは常にあります。

- **The Rust Programming Language (The Book)**: さらに深く学ぶための公式ドキュメント。
- **crates.io**: Rustのパッケージレジストリ。
- **This Week in Rust**: Rustコミュニティの最新情報を得るためのニュースレター。

何よりも、自分自身でコードを書き続けることが、さらなる成長への鍵となります。

幸運を祈ります。Happy Hacking!

