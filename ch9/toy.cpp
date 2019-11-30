#include "llvm-8.1/ADT/STLExtras.h"
#include "llvm-8.1/Analysis/BasicAliasAnalysis.h"
#include "llvm-8.1/Analysis/Passes.h"
#include "llvm-8.1/IR/DIBuilder.h"
#include "llvm-8.1/IR/IRBuilder.h"
#include "llvm-8.1/IR/LLVMContext.h"
#include "llvm-8.1/IR/LegacyPassManager.h"
#include "llvm-8.1/IR/Module.h"
#include "llvm-8.1/IR/Verifier.h"
#include "llvm-8.1/Support/TargetSelect.h"
#include "llvm-8.1/Transforms/Scalar.h"

#include <cctype>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "../KaleidoscopeJIT.h"

using namespace llvm;
using namespace llvm::orc;


#include "lexer.cpp"

#include "ast.cpp"

#include "parser.cpp"

#include "debug_info.cpp"

#include "codegen.cpp"

#include "top_level_jit_driver.cpp"

//#include "library_funcs.cpp"

#include "main.cpp"
