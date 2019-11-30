### How To Generate Execute File From .Ka

see generate.sh

1. ./toy fib.ka 2> fib.ll
2. clang -x ir fib.ll -c -o fib.o
3. clang printd.o fib.o -o fib
4. ./fib
