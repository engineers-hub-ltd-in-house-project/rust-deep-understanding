# EPUB 変換ガイド

本ガイドは、このプロジェクトの Markdown 原稿から一つの EPUB ファイルを生成するための手順書です。

---

## 概要

このガイドでは `Pandoc` というツールを利用して、`manuscript` ディレクトリ内の Markdown ファイルを結合し、電子書籍形式 (EPUB) に変換します。

最終的な成果物として、`rust-deep-understanding.epub` というファイルがプロジェクトのルートディレクトリに生成されます。

---

## 1. 準備

まず、EPUB の生成に必要なツールをインストールし、書籍の品質を高めるためのアセット（素材）を準備します。

### 必要なツール: Pandoc

EPUB の変換には `Pandoc` が不可欠です。以下の手順でインストールしてください。

- **公式サイト:** [pandoc.org/installing.html](https://pandoc.org/installing.html)

**macOS (Homebrew)**
```bash
brew install pandoc
```

**Ubuntu/Debian**
```bash
sudo apt-get install pandoc
```

**Windows**
公式サイトからインストーラーをダウンロードして実行します。

### アセットの準備 (推奨)

出版レベルの EPUB を生成するために、以下のファイルをプロジェクトルートに配置することを推奨します。

- **`metadata.yaml`**: 書籍のタイトルや著者名などのメタデータを定義します。
- **`images/cover.png`**: EPUB の表紙となる画像です。
- **`style.css`**: コードブロックなどの見た目を整えるためのスタイルシートです。

#### `metadata.yaml` の例

```yaml
---
title: 'Python/Go エンジニアのための実践 Rust 入門'
author: 'Engineers Hub'
language: 'ja-JP'
date: '2025-10-18'
publisher: 'Engineers Hub Publishing'
rights: '© 2025 Engineers Hub. All rights reserved.'
---
```

#### `images/cover.png` の準備

`images` ディレクトリを作成し、その中に表紙として使用したい画像 (例: `cover.png`) を配置します。
Kindle が推奨する理想的な画像の寸法は、**高さ 2,560 ピクセル x 幅 1,600 ピクセル** です。

#### `style.css` の例

```css
/* style.css */
code {
  font-family: "Courier New", Courier, monospace;
  background-color: #f4f4f4;
  padding: 2px 4px;
  border-radius: 4px;
}

pre {
  background-color: #f4f4f4;
  padding: 1em;
  overflow-x: auto;
  border-radius: 5px;
}
```

---

## 2. 変換手順 (推奨ワークフロー)

`SUMMARY.md` に記載された章の順序通りに EPUB を生成する、最も確実で推奨される方法です。

### ステップ 1: EPUB 生成スクリプトの実行

以下の内容で `build-epub.sh` という名前のシェルスクリプトをプロジェクトのルートに作成します。

```bash
#!/bin/bash
# build-epub.sh
# SUMMARY.md の順序に従って EPUB を生成するスクリプト

# SUMMARY.md から Markdown ファイルのリストを抽出 (- [title](path) の形式を想定)
FILES=$(grep -oP '\[.*\]\(\K[^)]+' SUMMARY.md)

# Pandoc を実行して EPUB を生成
pandoc ${FILES} \
  --from markdown+mark \
  --to epub3 \
  --output rust-deep-understanding.epub \
  --resource-path=./manuscript \
  --toc \
  --toc-depth=3 \
  --epub-chapter-level=2 \
  --metadata-file=metadata.yaml \
  --epub-cover-image=images/cover.png \
  --css=style.css

# 完了メッセージ
if [ $? -eq 0 ]; then
  echo "✅ EPUB 生成完了: rust-deep-understanding.epub"
else
  echo "❌ EPUB 生成に失敗しました。"
fi
```

スクリプトに実行権限を付与し、実行します。

```bash
chmod +x build-epub.sh
./build-epub.sh
```

### ステップ 2: 生成された EPUB の確認

変換が完了したら、`rust-deep-understanding.epub` が生成されていることを確認します。
Calibre などの EPUB リーダーでファイルを開き、以下の点を確認してください。

- 目次が正しい順序で表示されているか
- 表紙、タイトル、著者名が正しく設定されているか
- 画像やコードブロックの表示崩れがないか

**主な EPUB リーダー:**
- **macOS:** `Apple Books`
- **Windows / Linux:** `Calibre`
- **ブラウザ拡張:** `EPUBReader`

---

## 3. その他の変換方法

### ファイル名順での簡易変換

章の順序を厳密に問わないテストビルドなど、手早く変換したい場合に利用できます。

```bash
pandoc manuscript/part*/*.md \
  --from markdown \
  --to epub3 \
  --output rust-deep-understanding-quick.epub \
  --metadata title="Rust Deep Understanding (Quick Build)" \
  --toc
```

*注意: この方法はファイル名のアルファベット順でファイルを結合するため、意図した章の順序にならない可能性があります。*

### mdBook を使用 (将来的な対応)

本プロジェクトが `mdBook` 形式に完全対応した場合は、以下のコマンドで EPUB を生成できます。

```bash
# mdBook と mdbook-epub のインストール
cargo install mdbook mdbook-epub

# ビルド
mdbook build
```

生成された EPUB は `book/epub/` ディレクトリに格納されます。

---

## 4. トラブルシューティング

### `pandoc: command not found`
Pandoc がインストールされていないか、PATH が通っていません。準備のセクションを参考にインストールしてください。

### 画像が表示されない
`pandoc` コマンドに `--resource-path` オプションを追加し、画像が格納されているディレクトリへのパスを指定します。推奨ワークフローのスクリプトには、この設定が既に含まれています。

### 日本語が文字化けする
Markdown ファイルが UTF-8 形式で保存されていることを確認してください。また、`metadata.yaml` の `language` 設定が `ja-JP` になっていることを確認します。
