# Rust Deep Understanding EPUB 出版ガイド

本ガイドは、本プロジェクトの Markdown ファイルを EPUB 形式に変換し、Amazon Kindle Direct Publishing (KDP) などのプラットフォームで出版するための手順書です。

---

## Part 1: EPUB ファイルの生成

高品質な EPUB ファイルを生成することが、出版プロセスの第一歩です。
メタデータや表紙画像を適切に設定することで、ストアでの見栄えが良くなります。

### 1. 前提条件

EPUB の生成には `Pandoc` が必要です。インストールされていない場合は、公式サイトの手順に従ってください。

- **Pandoc:** [https://pandoc.org/installing.html](https://pandoc.org/installing.html)

macOS の場合:
```bash
brew install pandoc
```

### 2. EPUB 変換コマンド

プロジェクトのルートディレクトリで以下のコマンドを実行します。

**重要**: `--from markdown-yaml_metadata_block` オプションは必須です。これを省略するとYAMLパースエラーが発生します。

```bash
pandoc manuscript/part*/*.md \
  --from markdown-yaml_metadata_block \
  --to epub3 \
  --output rust-deep-understanding.epub \
  --resource-path=./manuscript \
  --toc \
  --toc-depth=3 \
  --split-level=2 \
  --metadata-file=metadata.yaml \
  --epub-cover-image=images/cover.png \
  --css=style.css
```

**オプション説明**:
- `--from markdown-yaml_metadata_block`: manuscript内の `---` をYAMLブロックとして解釈しない（必須）
- `--split-level=2`: 章レベルでファイルを分割（`--epub-chapter-level`は非推奨）
- `--toc-depth=3`: 目次の階層を3レベルまで生成

### 3. メタデータと表紙の準備 (重要)

出版用の EPUB には、正確なメタデータと魅力的な表紙が不可欠です。

#### メタデータファイル (`metadata.yaml`)

プロジェクトのルートに `metadata.yaml` というファイルを作成し、以下の内容を記述します。

```yaml
---
title: 'Python/Go エンジニアのための実践 Rust 入門'
author: Engineers Hub
publisher: Engineers Hub Publishing
language: ja-JP
rights: © 2025 Engineers Hub. All rights reserved.
...
```

**重要**:
- ファイルの先頭は `---`、末尾は `...` で終わること
- titleにスラッシュやコロンが含まれる場合はシングルクォートで囲む
- languageはBCP 47形式（例: ja-JP, en-US）

#### 表紙画像 (`images/cover.png`)

`images` ディレクトリに `cover.png` という名前で表紙画像を配置します。
KDP が推奨するカバー画像の仕様は以下の通りです。

- **形式:** JPEG または TIFF
- **寸法:** 理想的な高さ/幅の比率は **1.6:1**
- **サイズ:** 理想的な寸法は **高さ 2,560 ピクセル x 幅 1,600 ピクセル**

#### スタイルシート (`style.css`)

コードブロックや引用のスタイルを整えるため、カスタム CSS を適用できます。
シンプルな `style.css` の例：

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

### 4. 生成された EPUB の確認

変換後、Calibre などの EPUB リーダーでファイルを開き、以下の点を確認します。

- 目次が正しく生成されているか？
- 画像は表示されているか？
- コードブロックのスタイルは崩れていないか？
- メタデータ（タイトル、著者名）は正しいか？

### 5. トラブルシューティング

#### YAML parse exception エラー

**エラーメッセージ**:
```
YAML parse exception at line 1, column 0,
while scanning for the next token:
found character that cannot start any token
```

**原因**: manuscript内の `---`（水平線）がYAMLメタデータブロックとして誤認識されている。

**解決方法**: `--from markdown-yaml_metadata_block` オプションを必ず指定する。

#### メタデータが反映されない

**確認事項**:
- metadata.yamlの末尾が `...` で終わっているか
- metadata.yamlの先頭行にコメントが含まれていないか
- 文字コードがUTF-8であるか

#### 表紙画像が表示されない

**確認事項**:
- `images/cover.png` が存在するか
- 画像サイズが最低1000px以上あるか（推奨: 2560x1600px）
- ファイル形式がPNGまたはJPEGであるか

---

## Part 2: Amazon KDP での出版

EPUB ファイルが準備できたら、いよいよ出版手続きです。

### 1. KDP アカウントの準備

1.  **アカウント作成:** [KDP のサイト](https://kdp.amazon.co.jp/) にアクセスし、Amazon アカウントでサインインまたは新規作成します。
2.  **情報入力:** 右上の「アカウント」から、以下の情報を完了させます。
    - **著者/出版社情報:** 氏名、住所など。
    - **支払い受取情報:** 売上の振込先となる銀行口座を登録します。
    - **税に関する情報:** オンラインのインタビューに回答し、必要な税務情報を提出します。

### 2. 新しい電子書籍の登録

KDP の「本棚」ページで、「新しいタイトルを作成」から「電子書籍」を選択します。

### 3. ステップ 1: 電子書籍の詳細

フォームに従って、書籍の基本情報を入力します。

- **言語:** `日本語`
- **本のタイトル:** `Python/Go エンジニアのための実践 Rust 入門`
- **サブタイトル:** (任意)
- **著者:** `Engineers Hub`
- **内容紹介:** ストアに表示される書籍の説明文です。読者の興味を引くように、本書のターゲットや学べることを具体的に記述します。
- **出版に関して必要な権利:** 「これはパブリックドメインの作品ではなく、私は出版に関して必要な権利を所有しています」を選択します。
- **キーワード:** 読者が検索しそうなキーワードを 7 つまで入力します。（例: `Rust`, `プログラミング`, `入門`, `Go`, `Python`, `エンジニア`, `非同期処理`）
- **カテゴリー:** 本書に最も適したカテゴリーを 2 つ選択します。（例: `コンピュータ・IT > プログラミング > その他`）

すべて入力したら、「保存して続行」をクリックします。

### 4. ステップ 2: 電子書籍のコンテンツ

- **原稿:**
    - **デジタル著作権管理 (DRM):** 「はい」を推奨します。これにより、不正コピーを抑制できます。一度設定すると変更できません。
    - **アップロード:** 「電子書籍の原稿をアップロード」ボタンをクリックし、Part 1 で生成した `rust-deep-understanding.epub` ファイルを選択します。
- **表紙:**
    - 「カバークリエーターを起動」または「表紙ファイルをアップロード」を選択します。
    - Part 1 で準備した表紙画像がある場合は、「自分で作成した表紙をアップロード」を選択し、JPEG ファイルをアップロードします。
- **電子書籍のプレビュー:**
    - アップロードが完了すると、オンラインプレビューアーが表示されます。様々なデバイス（Kindle, タブレット, スマートフォン）での表示を確認し、レイアウト崩れがないかチェックします。
    - 詳細な確認には「Kindle Previewer」デスクトップアプリの利用を推奨します。
- **電子書籍の ISBN:**
    - 個人で出版する場合、通常は不要です。KDP が無料で ASIN (Amazon Standard Identification Number) を割り当てます。

問題がなければ、「保存して続行」をクリックします。

### 5. ステップ 3: 電子書籍の価格設定

- **KDP セレクトへの登録:** (任意) Kindle Unlimited の対象に含めたい場合は、登録します。
- **出版地域:** 「すべての地域 (全世界での権利)」が一般的です。
- **ロイヤリティと価格設定:**
    - **ロイヤリティプラン:** 70% または 35% を選択します。70% プランを選択するには、価格が特定の範囲内 (例: ¥250〜¥1,250) である必要があります。
    - **価格:** 主要マーケットプレイス（`amazon.co.jp`）で希望小売価格を入力します。他の地域の価格は自動的に計算されます。

すべての設定が完了したら、「Kindle 電子書籍を出版」ボタンをクリックします。

### 6. 出版後の流れ

- **審査:** Amazon による審査が行われます。通常は 24〜72 時間で完了します。
- **公開:** 審査が承認されると、Kindle ストアで書籍が公開されます。
- **修正:** 出版後も、KDP の本棚から詳細情報の修正や原稿の改訂版をアップロードすることが可能です。

---

## Part 3: その他のプラットフォーム

Amazon KDP 以外にも、以下のプラットフォームで個人出版が可能です。

- **楽天 Kobo ライティングライフ:**
  - [https://www.kobo.com/jp/ja/p/writinglife](https://www.kobo.com/jp/ja/p/writinglife)
  - 楽天 Kobo ストアで販売できます。手順は KDP と類似しています。
- **Apple Books:**
  - [https://authors.apple.com/](https://authors.apple.com/)
  - Apple のブックストアで販売できます。macOS が必要になる場合があります。
- **Google Play ブックス:**
  - [https://play.google.com/books/publish/](https://play.google.com/books/publish/)
  - Google Play ストアで販売できます。

複数のプラットフォームで販売することで、より多くの読者にアプローチできます。
