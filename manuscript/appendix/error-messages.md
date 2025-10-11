# 付録 B：エラーメッセージ対応表

Rust のコンパイラは非常に親切で、詳細なエラーメッセージと修正案を提示してくれます。この付録では、初心者が遭遇しがちな代表的なエラーメッセージとその原因、対処法をまとめます。

## `error[E0382]: borrow of moved value`
- **メッセージ例:** `borrow of moved value: `v``
- **原因:** 所有権が既に他の場所にムーブされた（移動した）値を、参照しようとしました。
- **よくある状況:**
  ```rust
  let v = vec![1, 2, 3];
  let v2 = v; // vの所有権がv2にムーブ
  println!("v[0] is: {}", v[0]); // error: vはもう使えない
  ```
- **解決法:**
  - 所有権をムーブする代わりに、`.clone()` で値を複製する。
  - 値をムーブするのではなく、参照 (`&v`) を渡す。

## `error[E0502]: cannot borrow `...` as mutable because it is also borrowed as immutable`
- **メッセージ例:** `cannot borrow `s` as mutable because it is also borrowed as immutable`
- **原因:** ある値に対して不変の借用 (`&`) が存在している間に、可変の借用 (`&mut`) を行おうとしました。借用規則（「複数の不変参照」or「一つの可変参照」）に違反しています。
- **よくある状況:**
  ```rust
  let mut s = String::from("hello");
  let r1 = &s; // 不変の借用
  let r2 = &mut s; // error: 可変の借用
  println!("{}", r1);
  ```
- **解決法:**
  - 不変の借用と可変の借用のスコープが重ならないように、コードの順序を調整する。

## `error[E0597]: `...` does not live long enough`
- **メッセージ例:** `` `s` does not live long enough``
- **原因:** ある値への参照が、その値自体よりも長く生存しようとしています。これはダングリングポインタ（無効なメモリを指すポインタ）を防ぐためのライフタイム規則によるものです。
- **よくある状況:**
  ```rust
  let reference_to_nothing = {
      let s = String::from("hello");
      &s // error: sはこのブロックの終わりでドロップされる
  };
  ```
- **解決法:**
  - 参照を返す代わりに、値そのものの所有権を返す。
  - 参照先の値が、参照よりも長く生存するようにスコープを調整する。

## `error[E0277]: the trait bound `...` is not satisfied`
- **メッセージ例:** ``the trait bound `MyStruct: std::fmt::Debug` is not satisfied``
- **原因:** ある関数やマクロが、特定のトレイトを実装した型を期待しているのに、実装していない型を渡そうとしました。
- **よくある状況:**
  ```rust
  struct MyStruct;
  // println!("{:?}", MyStruct); // error: MyStructはDebugトレイトを実装していない
  ```
- **解決法:**
  - 必要なトレイトを `#[derive(...)]` で自動実装するか、手動で `impl` ブロックを記述する。
  - 例: `#[derive(Debug, Clone, PartialEq)]`

## `error[E0499]: cannot borrow `...` as mutable more than once at a time`
- **メッセージ例:** `cannot borrow `x` as mutable more than once at a time`
- **原因:** 同じ値に対して、同時に複数の可変の借用 (`&mut`) を作ろうとしました。
- **よくある状況:**
  ```rust
  let mut list = vec![1, 2, 3];
  let first = &mut list[0];
  let second = &mut list[1]; // ここではまだエラーにならないが...
  *first = 7;
  *second = 8;
  // let first_again = &mut list[0]; // error!
  ```
- **解決法:**
  - 複数の可変参照が必要な場合は、`split_at_mut` のような関数を使って、重複しない範囲への可変参照を取得する。
  - 内部可変性パターン (`RefCell` など) の利用を検討する。

## `error: expected a `&str`, found a `String`` (関数の引数)
- **原因:** `&str` を期待する関数に `String` 型の値を渡そうとした。
- **解決法:**
  - Rust の Deref 変換により、`&String` は自動的に `&str` に変換されます。したがって、`my_function(&my_string)` のように、`String` への参照を渡せば解決します。

コンパイラのエラーメッセージは、あなたの最高の味方です。焦らずにメッセージをよく読み、提案された修正 (`help: ...`) を試してみてください。

