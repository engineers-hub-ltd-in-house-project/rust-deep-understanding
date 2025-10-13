# 第 27 章：イテレータとクロージャの活用

## この章のゴール
- `mini-grep` の実装を、従来のループベースから、イテレータとクロージャを使ったより宣言的で効率的なスタイルにリファクタリングできる。
- `filter` や `map` などのイテレータアダプタをメソッドチェーンで繋ぎ、複雑なデータ変換処理を構築できる。
- クロージャが環境の変数をキャプチャする仕組み（`Fn`, `FnMut`, `FnOnce`）の概要を説明できる。

---

## 27.1 なぜリファクタリングが必要か？

前章で作成した `mini-grep` は正しく動作しますが、コードには改善の余地があります。特に、設定のパース部分と検索ロジックの部分です。

- 効率: 現在の実装では、引数の `Vec` をクローンしており、小さなオーバーヘッドがあります。
- 表現力: `for` ループを使った検索ロジックは手続き的です。イテレータを使えば、より宣言的に「何がしたいか」を表現できます。

この章では、クロージャとイテレータという Rust の強力な機能を活用して、`mini-grep` をよりプロフェッショナルなコードにリファクタリングします。

## 27.2 クロージャによる設定パースの改善

現在、`Config::build` 関数は、引数スライスのインデックスに直接アクセスしており、`panic!` する可能性があります。これを、イテレータを返す `std::env::Args` を直接消費するように変更します。

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
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20src/main.rs%0Ause%20axum%3A%3A%7B%0A%20%20%20%20extract%3A%3A%7BPath%2C%20State%7D%2C%0A%20%20%20%20http%3A%3AStatusCode%2C%0A%20%20%20%20response%3A%3AJson%2C%0A%20%20%20%20routing%3A%3A%7Bget%2C%20post%7D%2C%0A%20%20%20%20Router%2C%0A%7D%3B%0Ause%20serde%3A%3A%7BDeserialize%2C%20Serialize%7D%3B%0Ause%20std%3A%3Acollections%3A%3AHashMap%3B%0Ause%20std%3A%3Async%3A%3A%7BArc%2C%20Mutex%7D%3B%0A%0A%23%5Btokio%3A%3Amain%5D%0Aasync%20fn%20main%28%29%20%7B%0A%20%20%20%20//%20%E3%82%A2%E3%83%97%E3%83%AA%E3%82%B1%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3%E3%81%AE%E7%8A%B6%E6%85%8B%EF%BC%9A%E3%82%A4%E3%83%B3%E3%83%A1%E3%83%A2%E3%83%AA%E3%81%AE%E3%82%B7%E3%83%B3%E3%83%97%E3%83%AB%E3%81%AADB%0A%20%20%20%20let%20db%20%3D%20Arc%3A%3Anew%28Mutex%3A%3Anew%28HashMap%3A%3Anew%28%29%29%29%3B%0A%0A%20%20%20%20let%20app%20%3D%20Router%3A%3Anew%28%29%0A%20%20%20%20%20%20%20%20.route%28%22/users%22%2C%20post%28create_user%29%29%0A%20%20%20%20%20%20%20%20.route%28%22/users/%3Aid%22%2C%20get%28get_user%29%29%0A%20%20%20%20%20%20%20%20//%20.with_state%28%29%20%E3%81%A7%E3%83%AB%E3%83%BC%E3%82%BF%E3%83%BC%E3%81%AB%E7%8A%B6%E6%85%8B%E3%82%92%E6%B8%A1%E3%81%99%0A%20%20%20%20%20%20%20%20.with_state%28db%29%3B%0A%0A%20%20%20%20let%20listener%20%3D%20tokio%3A%3Anet%3A%3ATcpListener%3A%3Abind%28%22127.0.0.1%3A3000%22%29.await.unwrap%28%29%3B%0A%20%20%20%20println%21%28%22listening%20on%20%7B%7D%22%2C%20listener.local_addr%28%29.unwrap%28%29%29%3B%0A%20%20%20%20axum%3A%3Aserve%28listener%2C%20app%29.await.unwrap%28%29%3B%0A%7D%0A%0A//%20%E5%85%B1%E6%9C%89%E3%81%99%E3%82%8B%E7%8A%B6%E6%85%8B%E3%81%AE%E5%9E%8B%E3%82%A8%E3%82%A4%E3%83%AA%E3%82%A2%E3%82%B9%0Atype%20Db%20%3D%20Arc%3CMutex%3CHashMap%3Cu64%2C%20User%3E%3E%3E%3B%0A%0A%23%5Bderive%28Debug%2C%20Serialize%2C%20Clone%29%5D%0Astruct%20User%20%7B%0A%20%20%20%20id%3A%20u64%2C%0A%20%20%20%20username%3A%20String%2C%0A%7D%0A%0A%23%5Bderive%28Debug%2C%20Deserialize%29%5D%0Astruct%20CreateUser%20%7B%0A%20%20%20%20username%3A%20String%2C%0A%7D%0A%0A//%20POST%20/users%3A%20%E3%83%A6%E3%83%BC%E3%82%B6%E3%83%BC%E3%82%92%E4%BD%9C%E6%88%90%E3%81%99%E3%82%8B%E3%83%8F%E3%83%B3%E3%83%89%E3%83%A9%0Aasync%20fn%20create_user%28%0A%20%20%20%20//%20State%E3%82%A8%E3%82%AF%E3%82%B9%E3%83%88%E3%83%A9%E3%82%AF%E3%82%BF%E3%81%A7%E5%85%B1%E6%9C%89%E7%8A%B6%E6%85%8B%E3%82%92%E5%8F%97%E3%81%91%E5%8F%96%E3%82%8B%0A%20%20%20%20State%28db%29%3A%20State%3CDb%3E%2C%0A%20%20%20%20//%20Json%E3%82%A8%E3%82%AF%E3%82%B9%E3%83%88%E3%83%A9%E3%82%AF%E3%82%BF%E3%81%A7%E3%83%AA%E3%82%AF%E3%82%A8%E3%82%B9%E3%83%88%E3%83%9C%E3%83%87%E3%82%A3%E3%82%92%E3%83%87%E3%82%B7%E3%83%AA%E3%82%A2%E3%83%A9%E3%82%A4%E3%82%BA%0A%20%20%20%20Json%28payload%29%3A%20Json%3CCreateUser%3E%2C%0A%29%20-%3E%20%28StatusCode%2C%20Json%3CUser%3E%29%20%7B%0A%20%20%20%20let%20mut%20db_lock%20%3D%20db.lock%28%29.unwrap%28%29%3B%0A%20%20%20%20let%20id%20%3D%20db_lock.keys%28%29.max%28%29.unwrap_or%28%260%29%20%2B%201%3B%0A%20%20%20%20let%20user%20%3D%20User%20%7B%0A%20%20%20%20%20%20%20%20id%2C%0A%20%20%20%20%20%20%20%20username%3A%20payload.username%2C%0A%20%20%20%20%7D%3B%0A%20%20%20%20db_lock.insert%28id%2C%20user.clone%28%29%29%3B%0A%0A%20%20%20%20%28StatusCode%3A%3ACREATED%2C%20Json%28user%29%29%0A%7D%0A%0A//%20GET%20/users/%3Aid%3A%20%E3%83%A6%E3%83%BC%E3%82%B6%E3%83%BC%E3%82%92%E5%8F%96%E5%BE%97%E3%81%99%E3%82%8B%E3%83%8F%E3%83%B3%E3%83%89%E3%83%A9%0Aasync%20fn%20get_user%28%0A%20%20%20%20State%28db%29%3A%20State%3CDb%3E%2C%0A%20%20%20%20//%20Path%E3%82%A8%E3%82%AF%E3%82%B9%E3%83%88%E3%83%A9%E3%82%AF%E3%82%BF%E3%81%A7%E3%83%91%E3%82%B9%E3%83%91%E3%83%A9%E3%83%A1%E3%83%BC%E3%82%BF%E3%82%92%E5%8F%97%E3%81%91%E5%8F%96%E3%82%8B%0A%20%20%20%20Path%28id%29%3A%20Path%3Cu64%3E%2C%0A%29%20-%3E%20Result%3CJson%3CUser%3E%2C%20StatusCode%3E%20%7B%0A%20%20%20%20let%20db_lock%20%3D%20db.lock%28%29.unwrap%28%29%3B%0A%20%20%20%20if%20let%20Some%28user%29%20%3D%20db_lock.get%28%26id%29%20%7B%0A%20%20%20%20%20%20%20%20Ok%28Json%28user.clone%28%29%29%29%0A%20%20%20%20%7D%20else%20%7B%0A%20%20%20%20%20%20%20%20Err%28StatusCode%3A%3ANOT_FOUND%29%0A%20%20%20%20%7D%0A%7D)

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
