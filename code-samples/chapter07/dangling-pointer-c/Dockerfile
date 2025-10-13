# gcc コンパイラを含む軽量なイメージを使用
FROM gcc:latest

# C のソースファイルをコンテナにコピー
WORKDIR /usr/src/app
COPY dangle.c .

# C コードをコンパイル (-Wall で全ての警告を有効に)
RUN gcc -o dangle dangle.c -Wall

# 実行可能ファイルを実行するコマンドを設定
CMD ["./dangle"]
