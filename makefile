EXE=toy

all: toy.cpp
	clang++ -g -O3 toy.cpp `llvm-config-3.8 --cxxflags` -std=c++14 -o ${EXE}

clean:
	@rm -rf *.o ${EXE}


