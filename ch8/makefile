EXE=toy
LLVM_CONFIG=--cxxflags --ldflags --system-libs --libs all
all: toy.cpp
	llvm-config --version
	clang++ -g -O3 toy.cpp `llvm-config ${LLVM_CONFIG}` -o ${EXE}

clean:
	@rm -rf *.o ${EXE}

