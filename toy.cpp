#include "KaleidoscopeJIT.h"

#include "llvm-8.1/ADT/APFloat.h"
#include "llvm-8.1/ADT/STLExtras.h"
#include "llvm-8.1/IR/BasicBlock.h"
#include "llvm-8.1/IR/Constants.h"
#include "llvm-8.1/IR/DerivedTypes.h"
#include "llvm-8.1/IR/Function.h"
#include "llvm-8.1/IR/IRBuilder.h"
#include "llvm-8.1/IR/LLVMContext.h"
#include "llvm-8.1/IR/LegacyPassManager.h"
#include "llvm-8.1/IR/Module.h"
#include "llvm-8.1/IR/Type.h"
#include "llvm-8.1/IR/Verifier.h"
#include "llvm-8.1/Support/TargetSelect.h"
#include "llvm-8.1/Target/TargetMachine.h"
#include "llvm-8.1/Transforms/InstCombine/InstCombine.h"
#include "llvm-8.1/Transforms/Scalar.h"
#include "llvm-8.1/Transforms/Scalar/GVN.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace llvm;
using namespace llvm::orc;

#include "lexer.cpp"

#include "ast.cpp"

#include "parser.cpp"

#include "codegen.cpp"

#include "top_level_jit_driver.cpp"

#include "main.cpp"
