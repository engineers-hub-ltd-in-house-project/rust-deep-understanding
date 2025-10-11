# 第 12 章：コレクション (Vec, HashMap, String)

## この章のゴール
- Rust の主要なコレクションである `Vec`, `HashMap`, `String` を使いこなせるようになる。
- 所有権がコレクションにどのように影響するかを理解する。
- `&str` と `String` の違いと使い分けを説明できるようになる。
- コレクションに対する基本的な操作 (追加, 取得, 更新, 削除) を行えるようになる。

## 前章の復習
前の章では、`Result` 型を使った堅牢なエラーハンドリングについて学びました。エラーが発生する可能性のある処理を安全に扱い、プログラムの信頼性を高める方法を習得しました。

## なぜこれが必要なのか？
ほとんどのプログラムでは、単一の値だけでなく、複数の値のリストや、キーと値のペアといったデータの集まりを扱う必要があります。コレクションは、これらのデータをメモリ上に効率的に格納し、操作するための基本的なツールです。Rust のコレクションは、所有権システムと密接に連携することで、他の言語にはないメモリ安全性を実現しています。

## Python/Go ではこうやっていた
Python や Go では、動的な配列やハッシュマップを直感的に利用してきました。
- **Python:** `list` や `dict` を使い、要素の追加やアクセスを自由に行ってきました。
  ```python
  my_list = [1, 2, 3]
  my_list.append(4)
  
  my_dict = {"apple": 1}
  my_dict["orange"] = 2
  ```
- **Go:** `slice` や `map` を使って同様の操作を行いました。
  ```go
  mySlice := []int{1, 2, 3}
  mySlice = append(mySlice, 4)
  
  myMap := make(map[string]int)
  myMap["apple"] = 1
  ```

## Rust ではこう書く
Rust のコレクションも基本的な考え方は似ていますが、所有権のルールが加わる点が重要です。

### `Vec<T>`: 動的配列
`Vec<T>` (ベクタ) は、同じ型の値を連続したメモリ領域に格納します。
```rust
// 新しい空のベクタを作成
let mut v: Vec<i32> = Vec::new();

// 初期値を持つベクタを vec! マクロで作成
let mut v = vec![1, 2, 3];

// 要素の追加
v.push(4);
v.push(5);

// 要素へのアクセス
let third: &i32 = &v[2]; // 添字でアクセス (範囲外だとパニック)
println!("The third element is {}", third);

match v.get(2) { // .get を使うと Option<&T> が返るため安全
    Some(third) => println!("The third element is {}", third),
    None => println!("There is no third element."),
}

// 全要素のイテレート
for i in &v {
    println!("{}", i);
}

// 可変参照で要素を変更
for i in &mut v {
    *i += 10;
}
```

### `HashMap<K, V>`: ハッシュマップ
キーと値のペアを格納します。
```rust
use std::collections::HashMap;

let mut scores = HashMap::new();

// 要素の追加
scores.insert(String::from("Blue"), 10);
scores.insert(String::from("Yellow"), 50);

// 値の取得
let team_name = String::from("Blue");
let score = scores.get(&team_name); // Option<&V> を返す

// 値の上書き
scores.insert(String::from("Blue"), 25);

// キーが存在しない場合のみ値を挿入
scores.entry(String::from("Red")).or_insert(30);
scores.entry(String::from("Blue")).or_insert(30); // Blueは存在するので変更されない

// 全要素のイテレート
for (key, value) in &scores {
    println!("{}: {}", key, value);
}
```

### `String` と `&str`
Rust の文字列は少し特殊で、2つの主要な型があります。
- `String`: ヒープ上に確保された、伸長可能で可変な UTF-8 エンコードされた文字列。所有権を持ちます。
- `&str`: 文字列スライス。`String` や文字列リテラルの一部を不変で参照します。所有権を持ちません。

```rust
// 文字列リテラルは &str 型
let s1 = "hello";

// String を作成
let mut s2 = String::from("hello");
s2.push_str(", world!"); // 文字列を追加

// &str から String への変換
let s3 = "initial contents".to_string();

// String から &str への参照
let s4: &str = &s2;
```
**なぜ2種類あるのか？**
`&str` を使うことで、`String` のデータをコピーすることなく、効率的に文字列の一部を関数に渡したり、参照したりできます。関数が文字列の所有権を必要としない場合は、`&str` を引数に取るのが一般的です。

## よくあるエラーと対処法
### エラー 1: `index out of bounds`
**原因:** `vec[index]` の形式で、ベクタの範囲外のインデックスにアクセスしようとしました。
```rust
let v = vec![1, 2, 3];
let does_not_exist = &v[99]; // パニック！
```
**解決法:** `.get(index)` を使い、返り値の `Option` を `match` や `if let` で安全に処理します。
```rust
if let Some(value) = v.get(99) {
    println!("The value is {}", value);
} else {
    println!("Index 99 is out of bounds.");
}
```

### エラー 2: 不変なコレクションへの変更
**原因:** `mut` で宣言されていないコレクションを変更しようとしました。
```rust
let v = vec![1, 2, 3];
v.push(4); // コンパイルエラー！
```
**解決法:** コレクションを `mut` として宣言します。 `let mut v = vec![1, 2, 3];`

## 練習問題
### 問題 1: 単語カウンター
文字列を受け取り、各単語の出現回数を `HashMap` に格納して返す関数 `word_count` を実装してください。

**ヒント:** 文字列を単語に分割するには `.split_whitespace()` メソッドが使えます。

## この章のまとめ
- `Vec<T>` は可変長のリスト、`HashMap<K, V>` はキーと値のペアを格納する。
- コレクションを操作する際は、所有権と可変性 (`mut`) を意識する必要がある。
- `String` は所有権を持つ文字列、`&str` はその参照。効率的な処理のために使い分ける。
- 範囲外アクセスには `.get()` を使って安全に処理することが推奨される。

## 次の章へ
コレクションの個々の要素を一つずつ処理する操作は非常に一般的です。次の章では、Rust の強力な「イテレータ」機能を使って、これらのコレクションをより効率的かつ宣言的に操作する方法について学びます。

