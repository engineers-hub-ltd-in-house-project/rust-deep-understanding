# Rust Deep Understanding EPUB変換ガイド

本ガイドは、本プロジェクトのMarkdownファイルをEPUB形式に変換するための手順書です。

## 前提条件

以下のツールをインストールしてください：

### 必須ツール
- Git
- Pandoc (https://pandoc.org/installing.html)

### インストール方法

#### macOS
```bash
brew install pandoc
```

#### Ubuntu/Debian
```bash
sudo apt-get install pandoc
```

#### Windows
Pandocの公式サイトからインストーラーをダウンロード
https://pandoc.org/installing.html

---

## 方法1: Pandoc を使用（推奨・シンプル）

### ステップ1: リポジトリのクローン（外部ユーザー向け）
```bash
git clone https://github.com/engineers-hub-ltd-in-house-project/rust-deep-understanding.git
cd rust-deep-understanding
```

既にリポジトリをクローン済みの場合は、このステップを省略してください。

### ステップ2: manuscript ディレクトリに移動
```bash
cd manuscript
```

### ステップ3: ファイル構造を確認
```bash
ls -la
```

現在の構造:
```
manuscript/
├── part0/      # 第0部：導入編
├── part1/      # 第1部：Rustの基礎
├── part2/      # 第2部：データ構造
├── part3/      # 第3部：抽象化とコード再利用
├── part4/      # 第4部：プロジェクト管理とテスト
├── part5/      # 第5部：並行性とAsync
├── part6/      # 第6部：実践プロジェクト
├── part7/      # 第7部：発展的トピック
└── appendix/   # 付録
```

### ステップ4: 全Markdownファイルを結合してEPUB変換
```bash
pandoc $(find . -name "*.md" | sort) \
  --from markdown \
  --to epub3 \
  --output ../rust-deep-understanding.epub \
  --metadata title="Python/Goエンジニアのための実践Rust入門" \
  --metadata author="Engineers Hub" \
  --metadata language="ja" \
  --toc \
  --toc-depth=3 \
  --epub-chapter-level=2
```

### ステップ5: 出力を確認
```bash
cd ..
ls -lh rust-deep-understanding.epub
```

---

## 方法2: SUMMARY.md の順序に従って変換

### ステップ1-2: 方法1と同様

### ステップ3: SUMMARY.md を確認
```bash
cat ../SUMMARY.md
```

### ステップ4: SUMMARY.md に記載された順序でファイルを指定

SUMMARY.mdの内容に基づいて、手動でファイルリストを作成：

```bash
pandoc \
  part0/chapter00.md \
  part1/chapter01.md \
  part1/chapter02.md \
  part1/chapter03.md \
  part1/chapter04.md \
  part1/chapter05.md \
  part1/chapter06.md \
  part1/chapter07.md \
  part1/chapter08.md \
  part1/chapter09.md \
  part2/chapter10.md \
  part2/chapter11.md \
  part2/chapter12.md \
  part3/chapter13.md \
  part3/chapter14.md \
  part3/chapter15.md \
  part3/chapter16.md \
  part4/chapter17.md \
  part4/chapter18.md \
  part4/chapter19.md \
  part4/chapter20.md \
  part4/chapter21.md \
  part5/chapter22.md \
  part5/chapter23.md \
  part5/chapter24.md \
  part6/chapter25.md \
  part6/chapter26.md \
  part6/chapter27.md \
  part6/chapter28.md \
  part7/chapter29.md \
  part7/chapter30.md \
  part7/chapter31.md \
  part7/chapter32.md \
  appendix/appendixA.md \
  appendix/appendixB.md \
  appendix/appendixC.md \
  --from markdown \
  --to epub3 \
  --output ../rust-deep-understanding.epub \
  --metadata title="Python/Goエンジニアのための実践Rust入門" \
  --metadata author="Engineers Hub" \
  --metadata language="ja" \
  --toc \
  --toc-depth=3 \
  --epub-chapter-level=2
```

---

## 方法3: mdBook を使用（将来的な対応）

現在、本プロジェクトはmdBook形式に対応していませんが、将来的に対応する場合は以下の手順を使用できます。

### 前提: mdBookのインストール
```bash
cargo install mdbook
cargo install mdbook-epub
```

### book.toml の作成
プロジェクトルートに以下の内容で作成：

```toml
[book]
title = "Python/Goエンジニアのための実践Rust入門"
authors = ["Engineers Hub"]
language = "ja"
src = "manuscript"

[output.html]

[output.epub]
```

### EPUBをビルド
```bash
mdbook build
```

### 出力確認
```bash
ls -lh book/epub/
```

---

## カスタマイズオプション

### メタデータの追加
```bash
--metadata title="タイトル" \
--metadata author="著者名" \
--metadata language="ja" \
--metadata date="2025-10-18" \
--metadata publisher="Engineers Hub"
```

### 表紙画像の追加
```bash
--epub-cover-image=cover.png
```

### CSSスタイルの適用
```bash
--css=style.css
```

### チャプターレベルの調整
```bash
--epub-chapter-level=1  # H1がチャプター
--epub-chapter-level=2  # H2がチャプター（デフォルト）
```

---

## トラブルシューティング

### エラー: pandoc: command not found
Pandocをインストールしてください。

### エラー: ファイルが見つからない
manuscript ディレクトリ内のファイル構造を確認してください。
```bash
find manuscript -name "*.md"
```

### 画像が表示されない
相対パスで画像を参照している場合、画像ファイルも含める必要があります。
```bash
--resource-path=.:manuscript:manuscript/images
```

### 日本語が文字化けする
UTF-8エンコーディングを明示的に指定します。
```bash
--metadata lang="ja"
```

---

## 推奨ワークフロー

1. まず方法1を試す（最もシンプル）
2. 順序が重要な場合は方法2
3. プロジェクトがmdBook対応済みなら方法3

---

## 変換後の確認

### EPUBリーダーで開く
- macOS: Apple Books
- Windows: Calibre
- Linux: Calibre または FBReader
- ブラウザ拡張: EPUBReader

### EPUBの内容を確認（任意）
```bash
# EPUBは実際にはZIPファイル
unzip -l rust-deep-understanding.epub
```

---

## 補足: スクリプト化

頻繁に変換する場合、以下のシェルスクリプトを作成できます：

```bash
#!/bin/bash
# convert-to-epub.sh

cd manuscript

pandoc $(find . -name "*.md" | sort) \
  --from markdown \
  --to epub3 \
  --output ../rust-deep-understanding.epub \
  --metadata title="Python/Goエンジニアのための実践Rust入門" \
  --metadata author="Engineers Hub" \
  --metadata language="ja" \
  --toc \
  --toc-depth=3 \
  --epub-chapter-level=2

echo "変換完了: rust-deep-understanding.epub"
```

実行権限を付与：
```bash
chmod +x convert-to-epub.sh
./convert-to-epub.sh
```

---

## プロジェクト構造との対応

本プロジェクトの構造:
```
rust-deep-understanding/
├── docs/                    # 本ガイドを含むドキュメント
├── manuscript/              # 本書の原稿（Markdown形式）
│   ├── part0/              # 第0部：導入編
│   ├── part1/              # 第1部：Rustの基礎
│   ├── part2/              # 第2部：データ構造
│   ├── part3/              # 第3部：抽象化とコード再利用
│   ├── part4/              # 第4部：プロジェクト管理とテスト
│   ├── part5/              # 第5部：並行性とAsync
│   ├── part6/              # 第6部：実践プロジェクト
│   ├── part7/              # 第7部：発展的トピック
│   └── appendix/           # 付録
├── code-samples/           # サンプルコード
├── proposal/               # 企画資料
├── README.md               # プロジェクト概要
├── SUMMARY.md              # 目次
└── WRITING_GUIDELINES.md   # 執筆ガイドライン
```
