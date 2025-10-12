# 第 26 章：実践プロジェクト：Web API サーバーの開発

## この章のゴール
- `axum` フレームワークを使い、基本的なJSON APIサーバーをゼロから構築できる。
- `Router` を使って、HTTPメソッドとパスをリクエスト処理関数（ハンドラ）に紐付けることができる。
- `Json`, `Path`, `State` といった `axum` のエクストラクタを使い、リクエストから型安全にデータを抽出できる。
- `serde` を使って、リクエストボディのJSONをRustの構造体にデシリアライズし、レスポンスとして構造体をJSONにシリアライズできる。
- `Arc<Mutex<T>>` を使って、複数のリクエストにまたがるアプリケーションの状態を安全に共有・変更できる。

---

## 26.1 なぜこれが必要か？ Rustによる高速・安全なWebサービス

CLIツールと並び、Webアプリケーション（特にバックエンドAPI）は、Rustの主要なユースケースの一つです。Rustの安全性、パフォーマンス、そして優れた非同期エコシステムは、高負荷なリクエストを効率的にさばき、メモリ安全性のバグをコンパイル時に排除できるため、信頼性の高いWebサービスを構築するのに非常に適しています。

この章では、`tokio`エコシステムの一部であるモダンなWebフレームワーク `axum` を使って、ユーザー情報を管理するシンプルなJSON APIサーバーを構築します。

## 26.2 プロジェクトの準備と「Hello, World」

`cargo new web-api-server` でプロジェクトを作成し、必要なクレートを `Cargo.toml` に追加します。

```toml
# Cargo.toml
[dependencies]
axum = "0.7"
tokio = { version = "1", features = ["full"] }
serde = { version = "1.0", features = ["derive"] }
serde_json = "1.0"
```

### 試してみよう：最初のサーバーを起動する

まずは、最もシンプルなサーバーを起動してみましょう。`src/main.rs` を以下のように編集します。

```rust
// src/main.rs
use axum::{
    routing::get,
    Router,
};

#[tokio::main]
async fn main() {
    // 1. ルーターを定義する
    let app = Router::new().route("/", get(root));

    // 2. サーバーを起動する
    let listener = tokio::net::TcpListener::bind("127.0.0.1:3000").await.unwrap();
    println!("listening on {}", listener.local_addr().unwrap());
    axum::serve(listener, app).await.unwrap();
}

// 3. ルートパス ("/") へのGETリクエストを処理するハンドラ
async fn root() -> &'static str {
    "Hello, World!"
}
```
`cargo run` で実行し、`curl http://127.0.0.1:3000` を別のターミナルで実行すると、"Hello, World!" が返ってきます。

## 26.3 JSON APIの構築

次に、ユーザーを作成・取得するAPIを実装していきます。

### 試してみよう：JSONの送受信と状態共有

まず、ユーザーを表す `User` 構造体と、ユーザー作成時のリクエストボディを表す `CreateUser` 構造体を定義します。`serde` の `Serialize`/`Deserialize` を `derive` することに注意してください。

次に、ユーザーデータを保存するための「データベース」として、インメモリの `HashMap` を使います。これを複数のリクエストで共有するために `Arc<Mutex<...>>` でラップし、`axum` の `State` エクストラクタでハンドラに渡します。

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

#[tokio::main]
async fn main() {
    // アプリケーションの状態：インメモリのシンプルなDB
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

// POST /users: ユーザーを作成するハンドラ
async fn create_user(
    // Stateエクストラクタで共有状態を受け取る
    State(db): State<Db>,
    // Jsonエクストラクタでリクエストボディをデシリアライズ
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
    // Pathエクストラクタでパスパラメータを受け取る
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

### APIのテスト

`cargo run` でサーバーを起動し、`curl` で動作を確認してみましょう。

```bash
# ユーザー "alice" を作成
$ curl -X POST -H "Content-Type: application/json" -d '{"username": "alice"}' http://127.0.0.1:3000/users
# {"id":1,"username":"alice"}

# ユーザー "bob" を作成
$ curl -X POST -H "Content-Type: application/json" -d '{"username": "bob"}' http://127.0.0.1:3000/users
# {"id":2,"username":"bob"}

# ID 1 のユーザーを取得
$ curl http://127.0.0.1:3000/users/1
# {"id":1,"username":"alice"}

# 存在しないユーザーを取得
$ curl -i http://127.0.0.1:3000/users/99
# HTTP/1.1 404 Not Found
# ...
```

## 26.4 まとめ

- `axum` は、ハンドラ関数のシグネチャ（引数と戻り値の型）を見るだけで、リクエストからデータを抽出し、レスポンスを構築する、直感的で型安全なWebフレームワーク。
- `Json<T>` エクストラクタと `serde` の組み合わせにより、JSONのシリアライズ・デシリアライズが簡単に行える。
- `Path<T>` エクストラクタは、URLのパスから動的な値を型安全に抽出する。
- `State<T>` エクストラクタと `Arc<Mutex<T>>` パターンは、複数のリクエストをまたいでアプリケーションの状態を安全に共有するための標準的な方法。

---

次の章では、Rustでのデータ処理とファイルI/Oについて、より詳しく掘り下げていきます。
