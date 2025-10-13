# 第 28 章：Python/Go コードからの移行戦略

## この章のゴール
- Rust を既存の Python/Go プロジェクトに組み込むFFI (Foreign Function Interface) の基本戦略を理解する。
- `PyO3` を使い、PythonのCPUバウンドな処理をRustで高速化する具体的な方法を体験する。
- GoからRustの関数をC ABI経由で呼び出す方法の概要を理解する。
- マイクロサービスアーキテクチャにおいて、サービス単位でRustに置き換える「ストラングラー・フィグ・パターン」を理解する。

---

## 28.1 なぜこれが必要か？ 現実世界のソフトウェア開発

本書をここまで読み進めてきたあなたは、Rustのパフォーマンス、安全性、表現力に魅了されていることでしょう。しかし、現実の仕事の多くは、ゼロから新しいものを作る「グリーンフィールド」開発ではありません。多くの場合、PythonやGoで書かれ、長年ビジネスを支えてきた巨大な「レガシー」コードベースが存在します。

この章の目的は、「すべてをRustで書き直そう！」という夢を語ることではありません。既存のPython/Go資産を尊重しつつ、Rustの強みをどのように戦略的に統合し、システム全体をより良くしていくか、という現実的なアプローチを学ぶことです。

## 28.2 戦略1：FFIによる機能拡張（ピンポイント高速化）

最も一般的で、リスクが低く、効果を実感しやすいのがこのアプローチです。アプリケーション全体の中で、パフォーマンスが特に問題となる、CPUバウンドな（計算量が非常に多い）関数を特定し、その関数だけをRustで書き直し、元の言語から呼び出す方法です。これをFFI (Foreign Function Interface) と呼びます。

### 試してみよう：Pythonの処理を`PyO3`で高速化する

Pythonは素晴らしい言語ですが、純粋なPythonコードは計算処理が遅いという弱点があります。これをRustで解決する体験をしてみましょう。

1. プロジェクトの準備
`PyO3`を使ったRustライブラリを簡単に開発できる `maturin` というツールを使います。

```bash
# Pythonの仮想環境を作成・有効化
python -m venv .venv
source .venv/bin/activate

# maturin と PyO3 をインストール
pip install maturin pyo3
```

次に、`maturin new -b pyo3 py-rust-ext` コマンドでプロジェクトの雛形を作成します。

2. Rustでロジックを実装
`py-rust-ext/src/lib.rs` を編集し、計算量の多い処理（ここでは再帰的なフィボナッチ数計算）を実装します。

```rust
// py-rust-ext/src/lib.rs
use pyo3::prelude::*;

// Pythonから呼び出せるように #[pyfunction] をつける
#[pyfunction]
fn fib(n: u64) -> u64 {
    if n <= 1 {
        n
    } else {
        fib(n - 1) + fib(n - 2)
    }
}

// モジュール定義
#[pymodule]
fn py_rust_ext(_py: Python, m: &PyModule) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(fib, m)?)?;
    Ok(())
}
```
[Rust Playgroundで試す](https://play.rust-lang.org/?version=stable&mode=debug&edition=2021&code=//%20py-rust-ext/src/lib.rs%0Ause%20pyo3%3A%3Aprelude%3A%3A%2A%3B%0A%0A//%20Python%E3%81%8B%E3%82%89%E5%91%BC%E3%81%B3%E5%87%BA%E3%81%9B%E3%82%8B%E3%82%88%E3%81%86%E3%81%AB%20%23%5Bpyfunction%5D%20%E3%82%92%E3%81%A4%E3%81%91%E3%82%8B%0A%23%5Bpyfunction%5D%0Afn%20fib%28n%3A%20u64%29%20-%3E%20u64%20%7B%0A%20%20%20%20if%20n%20%3C%3D%201%20%7B%0A%20%20%20%20%20%20%20%20n%0A%20%20%20%20%7D%20else%20%7B%0A%20%20%20%20%20%20%20%20fib%28n%20-%201%29%20%2B%20fib%28n%20-%202%29%0A%20%20%20%20%7D%0A%7D%0A%0A//%20%E3%83%A2%E3%82%B8%E3%83%A5%E3%83%BC%E3%83%AB%E5%AE%9A%E7%BE%A9%0A%23%5Bpymodule%5D%0Afn%20py_rust_ext%28_py%3A%20Python%2C%20m%3A%20%26PyModule%29%20-%3E%20PyResult%3C%28%29%3E%20%7B%0A%20%20%20%20m.add_function%28wrap_pyfunction%21%28fib%2C%20m%29%3F%29%3F%3B%0A%20%20%20%20Ok%28%28%29%29%0A%7D)

3. ビルドとインストール
プロジェクトのルート（`py-rust-ext/`）で `maturin develop` を実行します。これにより、Rustコードがコンパイルされ、現在のPython環境にモジュールとしてインストールされます。

4. Pythonから呼び出して速度を比較
同じディレクトリに `main.py` を作成し、純粋なPython版とRust版の実行速度を比較します。

```python
# main.py
import time
# maturinでビルドしたRustモジュールをインポート
from py_rust_ext import fib as fib_rust

def fib_py(n):
    if n <= 1:
        return n
    else:
        return fib_py(n - 1) + fib_py(n - 2)

# --- Python版 ---
start = time.time()
result_py = fib_py(35)
end = time.time()
print(f"Python version: result={result_py}, time={end - start:.4f}s")

# --- Rust版 ---
start = time.time()
result_rust = fib_rust(35)
end = time.time()
print(f"Rust version:   result={result_rust}, time={end - start:.4f}s")
```

`python main.py` で実行すると、Rust版が劇的に高速であることが確認できます。これがFFIの威力です。データ分析、画像処理、機械学習の前処理など、計算ボトルネックがある場所で絶大な効果を発揮します。

### Goとの連携

GoとRustを連携させるには、C言語のABI（Application Binary Interface）を介するのが一般的です。

1.  Rust側でC互換の関数 (`extern "C" fn`) を持つ共有ライブラリ (`.so`, `.dylib`など) を作成します。
2.  Go側で `cgo` を使い、その共有ライブラリをインポートして関数を呼び出します。

これは `PyO3` を使うより手動の作業が多くなりますが、同様にGoのアプリケーションの特定の部分をRustで高速化することが可能です。

## 28.3 戦略2：サービス単位での置き換え

もう一つの強力な戦略は、マイクロサービスアーキテクチャにおけるサービス単位での置き換えです。

例えば、多くのサービスが連携して動作しているシステムで、特定のサービス（例：アクセスが集中する認証サービス、大量のデータを処理するETLサービスなど）のパフォーマンスや信頼性が問題になっているとします。

この場合、そのサービスだけをRustで再実装し、同じAPI仕様（REST, gRPCなど）を公開します。そして、ロードバランサーやAPIゲートウェイの設定を切り替えて、リクエストを古いサービスから新しいRustのサービスに向けるのです。

このアプローチは「ストラングラー・フィグ・パターン（絞め殺しのイチジクの木パターン）」とも呼ばれ、既存のシステム全体を止めることなく、一部分ずつ安全にリプレースしていくことができます。

## 28.4 どの戦略を選ぶべきか？

- FFIによる機能拡張:
  - 適している場合: 特定の関数がCPUボトルネックになっている。Pythonのデータサイエンス系ライブラリの内部実装を高速化するようなケース。
  - 利点: 影響範囲が限定的で、導入が比較的容易。すぐに効果を実感できる。
- サービス単位での置き換え:
  - 適している場合: 特定のサービス全体のパフォーマンス、メモリ使用量、信頼性が問題。ステートフルな処理や複雑なI/Oを扱う場合。
  - 利点: レガシーコードを安全に刷新できる。Rustの非同期や型システムの恩恵を最大限に受けられる。

## 28.5 まとめ

- 既存のPython/GoコードベースにRustを導入する場合、すべてを書き換える必要はない。
- FFIは、CPUバウンドな処理をRustで高速化するための強力な武器であり、`PyO3`を使えばPythonとシームレスに連携できる。
- マイクロサービスアーキテクチャでは、問題となっているサービスを丸ごとRustで再実装し、ネットワーク経由で連携させるのが有効な戦略。
- 成功の鍵は、プロファイリングによるボトルネックの正確な特定と、小さく始めて徐々に範囲を広げていくアプローチ。

---

次の第7部では、さらにRustの能力を深く引き出すための、より発展的なトピックを探求します。まずは、Rustのメタプログラミング機能の核である「マクロ」の基礎から始めましょう。

