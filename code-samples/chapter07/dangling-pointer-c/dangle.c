#include <stdio.h>

int* dangle() {
    int x = 123;
    return &x; // x のメモリアドレスを返す
} // 関数が終わると x は破棄され、このメモリ領域は無効になる

int main() {
    int* ptr = dangle();
    // ptr が指すメモリは既に無効！
    // ここで *ptr を読み書きすると、何が起こるかわからない（未定義動作）
    printf("Value: %d\n", *ptr); // クラッシュするか、ゴミデータが表示される
    return 0;
}
