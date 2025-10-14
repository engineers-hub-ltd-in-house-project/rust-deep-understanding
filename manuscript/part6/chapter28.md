# 第28章 実践：Webアプリケーション (Axum)

## この章のゴール
- `axum` を使って、基本的な HTTP サーバーを起動できる。
- `Json<T>`, `Path<T>`, `State<T>` などのエクストラクタを使い、リクエストから型安全にデータを抽出できる。
- `Arc<Mutex<T>>` パターンを使い、複数のリクエスト間でアプリケーションの状態を安全に共有できる。
- 簡単な JSON API を実装し、`curl` を使って動作確認ができる。

---

## 28.1 なぜこれが必要か？ Rust と Web の今
Web アプリケーションは、現代のソフトウェア開発の中心です。Rust のエコシステムには、パフォーマンスと安全性を両立させた優れた Web フレームワークが数多く存在します。中でも `axum` は、`tokio` プロジェクトチームによって開発されており、以下の特徴から人気を集めています。

- **モジュール性**: 小さなコンポーネントの組み合わせでアプリケーションを構築します。
- **表現力豊かなハンドラ**: 関数のシグネチャ（引数と戻り値の型）を定義するだけで、`axum` がリクエストの解析やレスポンスの生成を自動的に行ってくれます。
- **エコシステムとの統合**: `tokio` 上に構築されており、`tower` や `hyper` といった実績あるライブラリの恩恵を最大限に受けることができます。

この章では、`axum` を使って、インメモリのデータベースを持つシンプルな JSON API を構築します。

## 28.2 プロジェクトの準備
`cargo new web-api` でプロジェクトを作成し、`Cargo.toml` に必要なクレートを追加します。

```toml
# Cargo.toml
[dependencies]
axum = "0.7"
tokio = { version = "1", features = ["full"] }
serde = { version = "1.0", features = ["derive"] }
```

- `axum`: Web フレームワーク本体。
- `tokio`: Rust の非同期ランタイム。`axum` は `tokio` 上で動作します。
- `serde`: JSON のシリアライズ・デシリアライズに使います。

## 28.3 ルーティングとハンドラ
まず、最もシンプルなサーバーを起動してみましょう。

```rust
// src/main.rs
use axum::{routing::get, Router};

#[tokio::main]
async fn main() {
    // ルーターを定義
    let app = Router::new().route("/", get(root));

    // サーバーを起動
    let listener = tokio::net::TcpListener::bind("127.0.0.1:3000").await.unwrap();
    println!("listening on {}", listener.local_addr().unwrap());
    axum::serve(listener, app).await.unwrap();
}

// "GET /" に対応するハンドラ関数
async fn root() -> &'static str {
    "Hello, World!"
}
```

`cargo run` を実行し、ブラウザや `curl` で `http://127.0.0.1:3000/` にアクセスすると、"Hello, World!" と表示されます。

## 28.4 エクストラクタと状態共有
`axum` の真価は、ハンドラ関数の引数に「エクストラクタ」を指定するだけで、リクエストの様々な部分を型安全に抽出できる点にあります。

- `Path(T)`: `/users/:id` のようなパスパラメータを抽出します。
- `Json(T)`: リクエストボディの JSON をデシリアライズします。
- `State(T)`: アプリケーションで共有される状態にアクセスします。

これらを使い、ユーザーを作成・取得できるインメモリの API を実装します。

```rust
// src/main.rs
use axum::{
    extract::{Path, State},
    http::StatusCode,
    response::Json,
    routing::{get, post},
    Router,
};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::sync::{Arc, Mutex};

// 共有する状態の型エイリアス
type Db = Arc<Mutex<HashMap<u64, User>>>;

#[derive(Debug, Serialize, Clone)]
struct User {
    id: u64,
    username: String,
}

#[derive(Debug, Deserialize)]
struct CreateUser {
    username: String,
}

#[tokio::main]
async fn main() {
    // アプリケーションの状態：インメモリのシンプルな DB
    let db = Arc::new(Mutex::new(HashMap::new()));

    let app = Router::new()
        .route("/users", post(create_user))
        .route("/users/:id", get(get_user))
        // .with_state() でルーターに状態を渡す
        .with_state(db);

    let listener = tokio::net::TcpListener::bind("127.0.0.1:3000").await.unwrap();
    println!("listening on {}", listener.local_addr().unwrap());
    axum::serve(listener, app).await.unwrap();
}

// POST /users: ユーザーを作成するハンドラ
async fn create_user(
    State(db): State<Db>,
    Json(payload): Json<CreateUser>,
) -> (StatusCode, Json<User>) {
    let mut db_lock = db.lock().unwrap();
    let id = db_lock.keys().max().unwrap_or(&0) + 1;
    let user = User {
        id,
        username: payload.username,
    };
    db_lock.insert(id, user.clone());

    (StatusCode::CREATED, Json(user))
}

// GET /users/:id: ユーザーを取得するハンドラ
async fn get_user(
    State(db): State<Db>,
    Path(id): Path<u64>,
) -> Result<Json<User>, StatusCode> {
    let db_lock = db.lock().unwrap();
    if let Some(user) = db_lock.get(&id) {
        Ok(Json(user.clone()))
    } else {
        Err(StatusCode::NOT_FOUND)
    }
}
```

### API のテスト
`cargo run` でサーバーを起動し、`curl` で動作を確認してみましょう。

```bash
# ユーザー "alice" を作成
$ curl -X POST -H "Content-Type: application/json" -d '{"username": "alice"}' http://127.0.0.1:3000/users
# {"id":1,"username":"alice"}

# ID 1 のユーザーを取得
$ curl http://127.0.0.1:3000/users/1
# {"id":1,"username":"alice"}

# 存在しないユーザーを取得
$ curl -i http://127.0.0.1:3000/users/99
# HTTP/1.1 404 Not Found
# ...
```

## 28.5 まとめ
- `axum` は、ハンドラ関数のシグネチャ（引数と戻り値の型）を見るだけで API の仕様が理解できる、直感的で型安全な Web フレームワークです。
- `Json<T>`、`Path<T>`、`State<T>` などのエクストラクタが、リクエスト処理の定型コードを大幅に削減します。
- `Arc<Mutex<T>>` パターンは、複数のリクエストをまたいでアプリケーションの状態を安全に共有するための標準的な方法です。

---
`axum` を使うことで、Rust の持つ安全性とパフォーマンスを Web API 開発に活かすことができます。次の章では、CSV データを効率的に扱うパーサーライブラリの作成に挑戦します。
