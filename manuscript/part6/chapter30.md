# 第30章 実践：TUI アプリケーション (ratatui)

## この章のゴール
- TUI (Text User Interface) が、インタラクティブな CUI アプリケーションを構築するための技術であることを理解する。
- `ratatui` と `crossterm` を使い、ターミナルの描画とキーボード入力を制御できる。
- TUI アプリケーションの基本的な構造（初期化、メインループ、終了処理）を実装できる。
- `ratatui` のウィジェットとレイアウトシステムを使い、簡単な UI を構築できる。

---

## 30.1 なぜこれが必要か？ CUI の表現力を高める
`mini-grep` や `csv-processor` のような CLI ツールは、一度実行すると結果を出力して終了します。しかし、`htop` のようなリソースモニターや `lazygit` のような Git クライアントなど、実行中にユーザーと対話しながら画面を更新し続けるアプリケーションも存在します。

このようなアプリケーションを TUI (Text User Interface) と呼びます。Rust には `ratatui` (旧 `tui-rs`) という非常に人気のある TUI ライブラリがあり、ターミナル上でリッチな UI を構築することができます。

この章では、`ratatui` を使って、キー入力に反応する最もシンプルな TUI アプリケーションを作成します。

## 30.2 プロジェクトの準備
`cargo new tui-app` でプロジェクトを作成し、`Cargo.toml` に必要なクレートを追加します。

```toml
# Cargo.toml
[dependencies]
ratatui = "0.26"
crossterm = "0.27"
```
- `ratatui`: TUI の UI 部品（ウィジェット）やレイアウトを管理します。
- `crossterm`: ターミナルの制御（カーソル移動、raw モードへの切り替え、イベント入力など）を行う低レベルなライブラリです。`ratatui` のバックエンドとして使われます。

## 30.3 TUI アプリケーションの基本構造
TUI アプリケーションは、通常以下の構造を持ちます。

1.  **ターミナルの初期化**: ターミナルを raw モードに設定し、描画用のバッファを用意します。
2.  **メインループ**:
    - UI を描画する (`terminal.draw(...)`)。
    - ユーザーからのイベント（キー入力など）を待つ (`crossterm::event::read()`)。
    - イベントに応じてアプリケーションの状態を更新する。
    - ループを抜ける条件（例：`q` キーが押された）をチェックする。
3.  **ターミナルの復元**: プログラムが終了する際に、ターミナルを元の状態に戻します。これを怠ると、ターミナルの表示が崩れたままになります。

この構造をコードに落とし込んでみましょう。

```rust
// src/main.rs
use crossterm::{
    event::{self, DisableMouseCapture, EnableMouseCapture, Event, KeyCode},
    execute,
    terminal::{disable_raw_mode, enable_raw_mode, EnterAlternateScreen, LeaveAlternateScreen},
};
use ratatui::{
    backend::{Backend, CrosstermBackend},
    layout::{Constraint, Direction, Layout},
    widgets::{Block, Borders, Paragraph},
    Terminal,
};
use std::{error::Error, io};

fn main() -> Result<(), Box<dyn Error>> {
    // 1. ターミナルの初期化
    enable_raw_mode()?;
    let mut stdout = io::stdout();
    execute!(stdout, EnterAlternateScreen, EnableMouseCapture)?;
    let backend = CrosstermBackend::new(stdout);
    let mut terminal = Terminal::new(backend)?;

    // 2. メインループ
    run_app(&mut terminal)?;

    // 3. ターミナルの復元
    disable_raw_mode()?;
    execute!(
        terminal.backend_mut(),
        LeaveAlternateScreen,
        DisableMouseCapture
    )?;
    terminal.show_cursor()?;

    Ok(())
}

fn run_app<B: Backend>(terminal: &mut Terminal<B>) -> io::Result<()> {
    loop {
        // UI の描画
        terminal.draw(|f| {
            let size = f.size();
            let block = Block::default().title("TUI App").borders(Borders::ALL);
            let paragraph = Paragraph::new("Hello, ratatui!\nPress 'q' to quit.");
            f.render_widget(block, size);
            // f.render_widget(paragraph, chunks[0]); // レイアウトを使う場合
        })?;

        // イベントの処理
        if let Event::Key(key) = event::read()? {
            if key.code == KeyCode::Char('q') {
                return Ok(()); // 'q' が押されたらループを抜ける
            }
        }
    }
}
```

## 30.4 `ratatui` による UI 描画
`terminal.draw` に渡すクロージャの中で、UI を構築します。

- **`Block`**: 境界線やタイトルを持つコンテナです。
- **`Paragraph`**: テキストを表示するウィジェットです。
- **`Layout`**: 画面を複数の領域 (`Rect`) に分割します。

`run_app` 関数を少し変更して、レイアウトを使ってみましょう。

```rust
fn run_app<B: Backend>(terminal: &mut Terminal<B>) -> io::Result<()> {
    loop {
        terminal.draw(|f| {
            let chunks = Layout::default()
                .direction(Direction::Vertical)
                .margin(1)
                .constraints([Constraint::Percentage(100)].as_ref())
                .split(f.size());

            let block = Block::default().title("TUI App").borders(Borders::ALL);
            let paragraph_text = "Hello, ratatui!\nPress 'q' to quit.";
            let paragraph = Paragraph::new(paragraph_text).block(block);
            
            f.render_widget(paragraph, chunks[0]);
        })?;

        if let Event::Key(key) = event::read()? {
            if key.code == KeyCode::Char('q') {
                return Ok(());
            }
        }
    }
}
```

`cargo run` を実行すると、タイトルと境界線付きのウィンドウが表示され、`q` を押すと終了することを確認できます。

## 30.5 まとめ
- `ratatui` と `crossterm` を組み合わせることで、Rust でクロスプラットフォームな TUI アプリケーションを構築できます。
- TUI アプリのライフサイクルは「初期化 → メインループ（描画とイベント処理）→ 復元」が基本です。
- `ratatui` は、`Block` や `Paragraph` などのウィジェットと、`Layout` システムを使って宣言的に UI を構築します。

---
次の章では、Rust コードを WebAssembly にコンパイルし、Web ブラウザ上で実行する方法について学びます。
