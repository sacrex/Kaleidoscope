EXE=toy
LLVM_CONFIG=--cxxflags --ldflags --system-libs --libs core
all: toy.cpp
	clang++ -g -O3 toy.cpp `llvm-config-3.8 ${LLVM_CONFIG}` -std=c++14 -o ${EXE}

clean:
	@rm -rf *.o ${EXE}


