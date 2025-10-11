# 付録 C：Python/Go→Rust 対応表

この付録は、Python や Go で慣れ親しんだ概念や構文が、Rust ではどのように表現されるかを素早く参照するための対応表です。

## 基本的な型

| 用途 | Python | Go | Rust |
|---|---|---|---|
| 整数 | `int` | `int`, `int64` | `i32`, `i64`, `usize` |
| 浮動小数点数 | `float` | `float64` | `f64` |
| 真偽値 | `bool` | `bool` | `bool` |
| 文字列 (不変) | `str` | `string` | `&str` |
| 文字列 (可変) | (strは不変) | (stringは不変) | `String` |
| 文字 | (`str`) | `rune` | `char` |
| 型なし/値なし | `None` | `nil` | `Option::None` |

## データ構造

| 用途 | Python | Go | Rust |
|---|---|---|---|
| 動的配列 | `list` | `slice` | `Vec<T>` |
| 固定長配列 | (`tuple`) | `array` | `[T; N]` |
| ハッシュマップ | `dict` | `map` | `HashMap<K, V>` |
| タプル | `tuple` | (`struct`) | `(T, U, ...)` |
| 集合 | `set` | (`map[T]struct{}`) | `HashSet<T>` |

## 制御構文

| 用途 | Python | Go | Rust |
|---|---|---|---|
| if-else | `if-elif-else` | `if-else if-else` | `if-else if-else` (式) |
| ループ | `for`, `while` | `for`, `for-range` | `loop`, `while`, `for-in` |
| パターンマッチ | (`match` in 3.10+) | `switch-case` | `match` (式、網羅性チェック) |
| エラー処理 | `try-except` | `if err != nil` | `Result<T, E>`, `?` 演算子 |

## 関数と抽象化

| 用途 | Python | Go | Rust |
|---|---|---|---|
| 関数定義 | `def` | `func` | `fn` |
| ラムダ/無名関数 | `lambda` | `func() {}` | `|| {}` (クロージャ) |
| ジェネリクス | `TypeVar` | 型パラメータ | `<T>` |
| インターフェース | ABC, `Protocol` | `interface` | `trait` |
| メソッド定義 | `class` 内の `def` | `func (recv T) ...` | `impl MyType { fn ... }` |
| 構造体/クラス | `class` | `struct` | `struct` |
| 列挙型 | `Enum` | (`iota`) | `enum` (代数的データ型) |

## 並行性

| 用途 | Python | Go | Rust |
|---|---|---|---|
| 軽量スレッド | (`asyncio.Task`) | `goroutine` | `tokio::spawn` (タスク) |
| OS スレッド | `threading` | (`runtime`) | `std::thread::spawn` |
| 通信 | `queue` | `channel` | `std::sync::mpsc`, `tokio::sync::mpsc` |
| 排他制御 | `Lock` | `Mutex` | `std::sync::Mutex`, `tokio::sync::Mutex` |
| 非同期 | `async/await` | (goroutine) | `async/await` |

## ツール

| 用途 | Python | Go | Rust |
|---|---|---|---|
| パッケージ管理 | `pip`, `poetry` | `go mod` | `cargo` |
| ビルド | (N/A) | `go build` | `cargo build` |
| テスト | `pytest`, `unittest` | `go test` | `cargo test` |
| フォーマッター | `black`, `autopep8` | `gofmt` | `rustfmt` |
| リンター | `pylint`, `flake8` | `go vet` | `clippy` |
| ドキュメント | `Sphinx`, `pydoc` | `godoc` | `rustdoc` |

この表はあくまで大まかな対応であり、各言語のセマンティクスや設計思想の違いにより、1対1で完全に対応するわけではない点に注意してください。

