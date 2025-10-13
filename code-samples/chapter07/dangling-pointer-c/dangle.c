#include <stdio.h>
#include <stdlib.h> // fflush のために追加

int* dangle() {
    int x = 123;
    printf("Inside dangle(): Address of x = %p\n", (void*)&x);
    fflush(stdout); // 出力バッファを強制的にフラッシュ
    return &x; // x のメモリアドレスを返す
} // 関数が終わると x は破棄され、このメモリ領域は無効になる

int main() {
    int* ptr = dangle();
    printf("Inside main(): Pointer ptr = %p\n", (void*)ptr);
    fflush(stdout);

    // ptr が指すメモリは既に無効！
    // ここで *ptr を読み書きすると、何が起こるかわからない（未定義動作）
    printf("Dereferencing dangling pointer: *ptr = %d\n", *ptr);
    fflush(stdout);

    printf("\nAttempting to WRITE to the dangling pointer...\n");
    fflush(stdout);
    *ptr = 789; // 無効なメモリへの書き込み！ここでクラッシュする可能性が高い
    printf("Value after write attempt: *ptr = %d\n", *ptr); // この行は実行されないかもしれない
    fflush(stdout);

    return 0;
}
