#include <stdio.h>

int* dangle() {
    int x = 123;
    printf("Inside dangle(): Address of x = %p\n", (void*)&x);
    return &x; // x のメモリアドレスを返す
} // 関数が終わると x は破棄され、このメモリ領域は無効になる

int main() {
    int* ptr = dangle();
    printf("Inside main(): Pointer ptr = %p\n", (void*)ptr);

    // ptr が指すメモリは既に無効！
    // ここで *ptr を読み書きすると、何が起こるかわからない（未定義動作）
    // 環境によってはクラッシュ(Segmentation fault)するか、予測不能なゴミデータが表示される
    printf("Dereferencing dangling pointer: *ptr = %d\n", *ptr);

    printf("\nAttempting to WRITE to the dangling pointer...\n");
    *ptr = 789; // 無効なメモリへの書き込み！非常に危険。
    printf("Value after write attempt: *ptr = %d\n", *ptr);

    return 0;
}
