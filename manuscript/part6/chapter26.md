# 第 26 章：Web アプリケーション (Actix/Axum)

## この章のゴール
- `actix-web` または `axum` を使って、基本的な HTTP サーバーを起動できるようになる。
- ルーティングを定義し、リクエストハンドラ関数を作成できる。
- リクエストからパスパラメータ、クエリパラメータ、JSON ボディを抽出できるようになる。
- JSON レスポンスをクライアントに返せるようになる。
- アプリケーションの状態（例: DB 接続プール）をハンドラ間で共有する方法を理解する。

## 前章の復習
前の章では、`clap` クレートを使って実践的な CLI ツールを開発しました。引数の解析、ファイル I/O、エラー処理といった、アプリケーション開発の基本を学びました。

## なぜこれが必要なのか？
Web サービスや API は、現代のソフトウェア開発の中心的な要素です。Rust は、そのパフォーマンス、安全性、そして優れた非同期エコシステムにより、高速で信頼性の高い Web アプリケーションを構築するための強力な選択肢となっています。この章では、`tokio` 上に構築された人気の非同期 Web フレームワークである `actix-web` と `axum` を紹介し、簡単な Web API を作成するプロセスを体験します。

## `axum` vs `actix-web`
- **`axum`:** `tokio` プロジェクトによってメンテナンスされている、比較的新しいフレームワーク。ミニマルでモジュール性が高く、特に `tower` エコシステムとの統合が強力です。関数のシグネチャを使ってリクエストを抽出する方法が非常に直感的です。
- **`actix-web`:** 長い実績があり、非常に高速なことで知られるフレームワーク。アクターモデルに触発された設計が特徴ですが、必ずしもアクターを意識する必要はありません。

この章では、よりモダンで学習しやすい `axum` を中心に解説します。

## はじめての `axum` アプリケーション
```toml
# Cargo.toml
[dependencies]
axum = "0.7"
tokio = { version = "1", features = ["full"] }
```

```rust
use axum::{
    routing::get,
    Router,
};
use std::net::SocketAddr;

#[tokio::main]
async fn main() {
    // ルーターを定義
    let app = Router::new().route("/", get(handler));

    // サーバーのアドレス
    let addr = SocketAddr::from(([127, 0, 0, 1], 3000));
    println!("listening on {}", addr);

    // サーバーを起動
    let listener = tokio::net::TcpListener::bind(addr).await.unwrap();
    axum::serve(listener, app).await.unwrap();
}

// "GET /" へのリクエストを処理するハンドラ
async fn handler() -> &'static str {
    "Hello, World!"
}
```
`cargo run` を実行し、ブラウザや `curl` で `http://127.0.0.1:3000` にアクセスすると、"Hello, World!" と表示されます。

## ルーティングとハンドラ
`Router::new().route("/path", method(handler))` の形式で、特定のパスと HTTP メソッドにハンドラ関数を紐付けます。

### パスパラメータの抽出
```rust
use axum::extract::Path;

async fn user_handler(Path(user_id): Path<u32>) -> String {
    format!("Hello, user {}", user_id)
}
// Router::new().route("/users/:id", get(user_handler));
```
`axum` は、ハンドラの引数の型 (`Path<u32>`) を見て、リクエストのパスから `:id` の部分を `u32` として抽出し、`user_id` に束縛してくれます。この仕組みを **Extractor** と呼びます。

### JSON の扱い
`axum::Json` Extractor を使うと、JSON のリクエストボディをデシリアライズしたり、構造体を JSON レスポンスとしてシリアライズしたりすることが簡単にできます。これには `serde` クレートが必要です。
```toml
# Cargo.toml
[dependencies]
# ...
serde = { version = "1.0", features = ["derive"] }
serde_json = "1.0"
```

```rust
use axum::{extract::Json, http::StatusCode, response::IntoResponse};
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize)]
struct User {
    id: u64,
    name: String,
}

// JSON ボディを受け取り、JSON を返すハンドラ
async fn create_user(Json(payload): Json<User>) -> impl IntoResponse {
    let new_user = User {
        id: 1337,
        name: payload.name,
    };
    (StatusCode::CREATED, Json(new_user))
}
```

## 状態の共有
データベース接続プールや設定情報など、複数のハンドラで共有したい状態は、`State` Extractor を使って渡します。
```rust
use axum::extract::State;
use std::sync::Arc;

// 共有したい状態
struct AppState {
    // ...
}

#[tokio::main]
async fn main() {
    let shared_state = Arc::new(AppState { /* ... */ });
    
    let app = Router::new()
        .route("/", get(handler))
        .with_state(shared_state); // ルーターに状態を登録
    
    // ... サーバー起動 ...
}

async fn handler(State(state): State<Arc<AppState>>) {
    // ハンドラ内で共有状態にアクセス
}
```
状態は `Arc` でラップするのが一般的です。`axum` がリクエストごとに状態をクローンするためです。

## エラーハンドリング
ハンドラから `Result` を返すことで、エラー処理を簡単に行うことができます。`Result` の `Err` ヴァリアントは、`IntoResponse` トレイトを実装している必要があります。これにより、エラーを適切な HTTP ステータスコードとレスポンスボディに変換できます。

## 練習問題
### 問題 1: TODO リスト API
インメモリの `HashMap` を状態として共有し、以下のエンドポイントを持つ簡単な TODO リスト API を作成してください。
- `GET /todos`: すべての TODO をリストする
- `POST /todos`: 新しい TODO を作成する

## この章のまとめ
- `axum` は、`tokio` 上に構築されたモダンで使いやすい Web フレームワーク。
- ルーターを使って、パスと HTTP メソッドをハンドラ関数に紐付ける。
- Extractor (`Path`, `Query`, `Json`, `State`) は、関数のシグネチャを元にリクエストからデータを直感的に抽出する強力な仕組み。
- `Arc` を使ってアプリケーションの状態を定義し、`with_state` でルーターに登録することで、ハンドラ間で安全に共有できる。

## 次の章へ
Web アプリケーションでは、データベースとのやり取りだけでなく、ファイルシステムからのデータの読み書きも頻繁に発生します。次の章では、Rust でのデータ処理とファイル I/O について、より詳しく掘り下げていきます。
