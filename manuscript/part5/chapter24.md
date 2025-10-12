# 第 24 章：実践的な非同期プログラミング：TCPサーバーを作ってみよう

## この章のゴール
- `tokio::net::TcpListener` を使い、ネットワークからの接続を非同期に待ち受けられる。
- `tokio::spawn` を使い、クライアントの接続ごとに独立した非同期タスクを生成できる。
- `tokio::io` の非同期トレイト (`AsyncReadExt`, `AsyncWriteExt`) を使って、ソケットの読み書きができる。
- `while let` ループと `read.await` を組み合わせ、クライアントからのデータを継続的に受信する方法を理解する。

---

## 24.1 なぜこれが必要か？ サーバーアプリケーションの基本構造

前の章で `async`/`await` の基礎を学びました。しかし、実際のアプリケーション、特にネットワークサーバーでは、不特定多数のクライアントから、いつ来るかわからないリクエストを同時に処理する必要があります。

この章では、これまでに学んだ非同期の知識を総動員して、すべてのネットワークサーバーの基本となる「TCPエコーサーバー」を構築します。このサーバーは、接続してきたクライアントが送信したメッセージを、そのままクライアントに送り返すだけのシンプルなものですが、実践的な非同期プログラミングの重要なパターンが詰まっています。

## 24.2 TCPエコーサーバーの構築

`cargo new async-echo-server` でプロジェクトを作り、`Cargo.toml` に `tokio` を追加しましょう。

```toml
# Cargo.toml
[dependencies]
tokio = { version = "1", features = ["full"] }
```

### 試してみよう：サーバーを実装し、動かしてみる

サーバーの実装は、大きく分けて2つの部分から構成されます。

1.  **main関数**: `TcpListener` を作成し、指定したアドレス（`127.0.0.1:8080`）でバインドします。その後、ループでクライアントからの接続を待ち受け (`.accept().await`) ます。
2.  **接続ごとの処理**: 新しい接続が確立されるたびに、`tokio::spawn` で新しい非同期タスクを生成します。このタスクは、そのクライアントとの通信を専任で担当します。

```rust
// src/main.rs
use tokio::io::{self, AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpListener;

#[tokio::main]
async fn main() -> io::Result<()> {
    // 1. TcpListener を作成し、アドレスにバインド
    let listener = TcpListener::bind("127.0.0.1:8080").await?;
    println!("Server listening on 127.0.0.1:8080");

    // 2. ループで接続を待ち受ける
    loop {
        // .accept() は非同期に接続を待ち、(Socket, SocketAddr) のタプルを返す
        let (mut socket, _) = listener.accept().await?;
        println!("New client connected!");

        // 3. 接続ごとに新しいタスクを生成
        tokio::spawn(async move {
            // 4. ソケットの読み書きを行う
            let mut buf = [0; 1024];

            // ループでソケットからのデータを読み込む
            loop {
                match socket.read(&mut buf).await {
                    // 0が返されたら、クライアントが接続を閉じたことを意味する
                    Ok(0) => {
                        println!("Client disconnected.");
                        return;
                    }
                    Ok(n) => {
                        // 読み込んだデータをそのまま書き戻す（エコー）
                        if socket.write_all(&buf[0..n]).await.is_err() {
                            // 書き込みエラーが発生した場合、おそらく接続が切れている
                            eprintln!("Failed to write to socket.");
                            return;
                        }
                    }
                    Err(e) => {
                        eprintln!("Failed to read from socket; err = {:?}", e);
                        return;
                    }
                }
            }
        });
    }
}
```

### サーバーの実行とテスト

このコードを `cargo run` で実行すると、サーバーが起動し、接続待ち状態になります。
別のターミナルを開き、`netcat` (または `telnet`) コマンドでサーバーに接続してみましょう。

```bash
# 別のターミナル
$ nc 127.0.0.1 8080
hello       <-- 入力してEnter
hello       <-- サーバーから返ってくる
world       <-- 入力してEnter
world       <-- サーバーから返ってくる
^C          <-- Ctrl+Cで切断
```

複数のターミナルから同時に接続しても、それぞれが独立してエコーバックされることを確認できます。これは、`tokio::spawn` によって、各クライアントの処理が独立した軽量タスクとして並行に実行されているためです。

## 24.3 コードの解説

- **`TcpListener::bind(...).await`**: ネットワークソケットを特定のアドレスにバインドする処理はI/O操作なので、非同期に行われます。
- **`listener.accept().await`**: 新しいクライアントからの接続が来るまで、現在のタスクを中断し、CPUを他のタスクに明け渡します。
- **`tokio::spawn(async move { ... })`**: `accept` が完了するたびに、新しい非同期タスクを生成します。`socket` の所有権は `move` キーワードによってこの新しいタスクに移ります。これにより、メインの `accept` ループはブロックされることなく、すぐに次の接続を待つことができます。
- **`socket.read(&mut buf).await`**: ソケットからデータが到着するまで非同期に待機します。データが到着すると、バッファに読み込み、読み込んだバイト数を返します。
- **`socket.write_all(...).await`**: バッファの内容をすべてソケットに書き込むまで非同期に待機します。

## 24.4 まとめ

- `tokio::net` モジュールは、TCPやUDPといったネットワークプログラミングのための非同期APIを提供する。
- サーバーの基本的な形は「`accept` ループ + 接続ごとの `tokio::spawn`」。これにより、高い並行性を実現できる。
- `tokio::io` の `AsyncReadExt` と `AsyncWriteExt` トレイトは、非同期なデータの読み書きのための便利なメソッドを提供する。
- 非同期コードにおけるI/O操作は、`.await` を付けることでCPUをブロックせず、効率的なリソース利用が可能になる。

---

次の第6部では、これまでに学んだ知識を総動員して、より実践的なプロジェクト、まずは定番のCLIツールの開発に取り組みます。
