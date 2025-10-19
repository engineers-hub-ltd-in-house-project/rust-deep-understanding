# Project-Specific CLAUDE.md

## Pandoc EPUB Generation - Critical Instructions

### YAML Metadata Block Conflict

**PROBLEM**: Manuscript Markdownファイル内に水平線として `---` を使用しているため、PandocがこれをYAMLメタデータブロックと誤認識してパースエラーが発生する。

**ERROR MESSAGE**:
```
YAML parse exception at line 1, column 0,
while scanning for the next token:
found character that cannot start any token
```

**SOLUTION**: 必ず `--from markdown-yaml_metadata_block` オプションを指定してYAMLメタデータブロックのパースを無効化すること。

### Correct Pandoc Command

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

**CRITICAL**: `--from markdown-yaml_metadata_block` の指定を絶対に省略しないこと。

### metadata.yaml Format

正しいYAMLメタデータファイルの書式:

```yaml
---
title: 'Python/Go エンジニアのための実践 Rust 入門'
author: Engineers Hub
publisher: Engineers Hub Publishing
language: ja-JP
rights: © 2025 Engineers Hub. All rights reserved.
...
```

**REQUIRED**:
- ファイル先頭に `---`
- ファイル末尾に `...` または `---`
- titleフィールドはシングルクォートで囲む（スラッシュやコロンを含む場合）
- languageフィールドはBCP 47形式（例: ja-JP, en-US）

### Troubleshooting

1. **YAML parse exception発生時**:
   - `--from markdown-yaml_metadata_block` が指定されているか確認
   - metadata.yamlの末尾が `...` で終わっているか確認
   - metadata.yamlの先頭行にコメントがないか確認

2. **メタデータが反映されない場合**:
   - `--metadata-file=metadata.yaml` のパスが正しいか確認
   - metadata.yamlの文字コードがUTF-8か確認

3. **表紙画像が表示されない場合**:
   - `images/cover.png` が存在するか確認
   - 画像サイズがKDP推奨仕様（2560x1600px推奨、最低1000px以上）を満たしているか確認

### Project Structure

```
.
├── CLAUDE.md                    # このファイル
├── metadata.yaml                # EPUBメタデータ
├── style.css                    # EPUBスタイルシート
├── images/
│   └── cover.png               # 表紙画像（1376x864px以上）
├── manuscript/
│   ├── part0/
│   ├── part1/
│   └── ...                     # 各章のMarkdownファイル
├── docs/
│   ├── epub-publishing-guide.md
│   └── plan/
│       ├── ai-instructions.md
│       └── ...
└── rust-deep-understanding.epub # 生成されたEPUBファイル
```

### AI Instructions

AI（Claude、Gemini等）にEPUB生成を依頼する際は、必ず以下を伝えること:

1. `--from markdown-yaml_metadata_block` オプションの必須指定
2. metadata.yamlの末尾は `...` で終わること
3. manuscript内の `---` はYAMLブロックではなく水平線であること

### References

- [EPUB出版ガイド](docs/epub-publishing-guide.md)
- [AI作成指示書](docs/plan/ai-instructions.md)
- [Pandoc公式マニュアル](https://pandoc.org/MANUAL.html)
