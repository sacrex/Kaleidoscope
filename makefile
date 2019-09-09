EXE=toy
LLVM_CONFIG=--cxxflags --ldflags --system-libs --libs core mcjit orcjit native
all: toy.cpp
	llvm-config --version
	clang++ -O1 -rdynamic toy.cpp `llvm-config ${LLVM_CONFIG}` -o ${EXE}

clean:
	@rm -rf *.o ${EXE}

