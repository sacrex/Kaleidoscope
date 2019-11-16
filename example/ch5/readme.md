### 输出CFG(svg/png/...)

1. llvm-as < xxx.ll | opt -view-cfg -o xxx.bc
2. 在当前的目录下会生成一个 .xxx.dot
3. dot -Tpng -o xxx.png
4. google-chrome xxx.png


