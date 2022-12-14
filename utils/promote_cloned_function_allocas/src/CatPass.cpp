#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/BreakCriticalEdges.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Attributes.h"

#include "./include/PromoteAllocaToReg.hpp"

#define MAIN "main"

using namespace llvm;

namespace {

struct CAT : public ModulePass {

  static char ID;

  CAT() : ModulePass(ID) {}

  bool runOnModule(Module &M) override {
    errs() << "PROMOTE_CLONED_FUNCTION_ALLOCAS\n";

#ifdef MEMORYTOOL_DISABLE_PROMOTE
    return false;
#endif

    bool modified = false;

    // Fetch Noelle
    auto &noelle = getAnalysis<Noelle>();

    // Get function to loops map
    std::unordered_map<Function*, std::unordered_map<Instruction*, StayConnectedNestedLoopForestNode*>> functionToLoopsMap = getFunctionToLoopsMap(M, noelle);
    errs() << "DEBUG: functionToLoopsMap.size() " << functionToLoopsMap.size() << "\n";

    // Fetch analyses we need and use them to promote allocas
    for (auto elem : functionToLoopsMap){
      Function *currFunc = elem.first;
      DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>(*currFunc).getDomTree();
      AssumptionCache &AC = getAnalysis<AssumptionCacheTracker>().getAssumptionCache(*currFunc);
      modified |= promoteAllocasROIs(elem.second, DT, AC);
    }

    return modified;
  }

  // We don't modify the program, so we preserve all analyses.
  // The LLVM IR of functions isn't ready at this point
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    //AU.setPreservesAll();
    
    // Required to promote alloca
    AU.addRequired<Noelle>();
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.setPreservesCFG();

    return;
  }

}; // end of struct CAT

} // end of anonymous namespace

char CAT::ID = 0;
static RegisterPass<CAT> X("CAT", "CAT pass");

// Next there is code to register your pass to "clang"
static CAT *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
                                        [](const PassManagerBuilder &,
                                           legacy::PassManagerBase &PM) {
                                          if (!_PassMaker) {
                                            PM.add(_PassMaker = new CAT());
                                          }
                                        }); // ** for -Ox
static RegisterStandardPasses
    _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
              [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
                if (!_PassMaker) {
                  PM.add(_PassMaker = new CAT());
                }
              }); // ** for -O0
