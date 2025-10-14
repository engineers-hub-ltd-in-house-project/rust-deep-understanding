# 第31章 実践：WebAssembly モジュール (wasm-bindgen)

## この章のゴール
- WebAssembly (WASM) が、ブラウザ上でネイティブに近いパフォーマンスでコードを実行するための技術であることを理解する。
- `wasm-pack` と `wasm-bindgen` を使い、Rust のコードを JavaScript から呼び出せる WASM モジュールにコンパイルできる。
- Rust で実装した関数を JavaScript から呼び出し、その結果を Web ページに表示できる。
- `Leptos` のようなフレームワークを使わない、より低レベルな Rust と JavaScript の連携方法を体験する。

---

## 31.1 なぜこれが必要か？ Web で Rust のパフォーマンスを
`Leptos` の章では、Rust をフルスタックで使う方法を見ました。その中核技術が WebAssembly (WASM) です。WASM は、ブラウザ上で JavaScript 以外の言語（C++, Go, Rust など）を実行可能にするためのバイナリフォーマットです。

これにより、JavaScript が苦手とする CPU バウンドな重い計算処理（画像処理、暗号化、物理シミュレーションなど）を Rust で実装し、Web アプリケーション全体のパフォーマンスを向上させることができます。

この章では、フレームワークを使わずに、素の Rust 関数を WASM にコンパイルし、JavaScript から直接呼び出す方法を学びます。

## 31.2 プロジェクトの準備
Rust の WASM 開発には `wasm-pack` というツールが不可欠です。

```bash
# wasm-pack のインストール
cargo install wasm-pack
# プロジェクトの作成
wasm-pack new wasm-lib
cd wasm-lib
```
`wasm-pack new` は、WASM ライブラリとして必要な設定（`Cargo.toml` の `crate-type` など）が済んだプロジェクトを生成します。

## 31.3 Rust に関数を実装する
`src/lib.rs` を編集し、JavaScript から呼び出したい関数を実装します。`#[wasm_bindgen]` 属性を付けることで、`wasm-pack` が必要な JavaScript との連携コード（グルーコード）を自動生成してくれます。

```rust
// src/lib.rs
use wasm_bindgen::prelude::*;

// JavaScript の `alert` 関数をインポート
#[wasm_bindgen]
extern "C" {
    fn alert(s: &str);
}

// JavaScript に公開する関数
#[wasm_bindgen]
pub fn greet(name: &str) {
    alert(&format!("Hello, {}!", name));
}

// 足し算を行う簡単な関数
#[wasm_bindgen]
pub fn add(a: u32, b: u32) -> u32 {
    a + b
}
```

## 31.4 WASM モジュールのビルド
プロジェクトのルートで `wasm-pack build` を実行します。

```bash
# ブラウザで直接使える ES モジュールとしてビルド
wasm-pack build --target web
```
`pkg` ディレクトリが生成され、その中にコンパイルされた `.wasm` ファイルと、それを簡単に使うための JavaScript ファイル (`.js`)、TypeScript の型定義ファイル (`.d.ts`) が含まれています。

## 31.5 JavaScript から呼び出す
`wasm-lib` ディレクトリのルートに `index.html` と `index.js` を作成します。

```html
<!-- index.html -->
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Hello wasm-pack!</title>
  </head>
  <body>
    <script type="module" src="./index.js"></script>
  </body>
</html>
```

```javascript
// index.js
import init, { greet, add } from './pkg/wasm_lib.js';

async function run() {
  // WASM モジュールを初期化
  await init();

  // Rust の関数を呼び出す
  greet('WebAssembly');
  
  const sum = add(2, 3);
  console.log(`2 + 3 = ${sum}`);
}

run();
```

簡単な Web サーバー（例：`python -m http.server`）を起動して `index.html` にアクセスすると、アラートが表示され、コンソールに計算結果が出力されます。

## 31.6 まとめ
- WebAssembly (WASM) は、Rust などの言語をブラウザで高速に実行するための技術です。
- `wasm-pack` は、Rust コードを WASM モジュールにビルド、テスト、公開するための一連のツールを提供します。
- `#[wasm_bindgen]` 属性は、Rust と JavaScript 間のデータ型や関数の呼び出しを簡単にするための「魔法」です。
- この方法を使うことで、既存の JavaScript プロジェクトのパフォーマンスが重要な部分だけを、段階的に Rust に置き換えていくことができます。

---
次の章では、`Leptos` を使って、サーバーサイドとクライアントサイドを両方 Rust で記述する、より統合されたフルスタック Web アプリケーション開発に挑戦します。
