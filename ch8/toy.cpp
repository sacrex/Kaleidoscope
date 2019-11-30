#include "llvm-8.1/ADT/APFloat.h"
#include "llvm-8.1/ADT/Optional.h"
#include "llvm-8.1/ADT/STLExtras.h"
#include "llvm-8.1/IR/BasicBlock.h"
#include "llvm-8.1/IR/Constants.h"
#include "llvm-8.1/IR/DerivedTypes.h"
#include "llvm-8.1/IR/Function.h"
#include "llvm-8.1/IR/Instructions.h"
#include "llvm-8.1/IR/IRBuilder.h"
#include "llvm-8.1/IR/LLVMContext.h"
#include "llvm-8.1/IR/LegacyPassManager.h"
#include "llvm-8.1/IR/Module.h"
#include "llvm-8.1/IR/Type.h"
#include "llvm-8.1/IR/Verifier.h"
#include "llvm-8.1/Support/FileSystem.h"
#include "llvm-8.1/Support/Host.h"
#include "llvm-8.1/Support/raw_ostream.h"
#include "llvm-8.1/Support/TargetRegistry.h"
#include "llvm-8.1/Support/TargetSelect.h"
#include "llvm-8.1/Target/TargetMachine.h"
#include "llvm-8.1/Target/TargetOptions.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace llvm;
using namespace llvm::sys;


#include "lexer.cpp"

#include "ast.cpp"

#include "parser.cpp"

#include "codegen.cpp"

#include "top_level_jit_driver.cpp"

#include "library_funcs.cpp"

#include "main.cpp"
