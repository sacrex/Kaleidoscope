#include "llvm-3.8/llvm/ADT/STLExtras.h"
#include "llvm-3.8/llvm/IR/IRBuilder.h"
#include "llvm-3.8/llvm/IR/LLVMContext.h"
#include "llvm-3.8/llvm/IR/Module.h"
#include "llvm-3.8/llvm/IR/Verifier.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "lexer.cpp"

#include "ast.cpp"

#include "parser.cpp"

#include "codegen.cpp"

#include "top_level_jit_driver.cpp"

#include "main.cpp"
