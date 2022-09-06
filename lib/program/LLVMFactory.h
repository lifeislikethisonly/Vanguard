#ifndef VANGUARD_PROGRAM_LLVMTOVANGUARD_H
#define VANGUARD_PROGRAM_LLVMTOVANGUARD_H


#include "UnitFactory.h"
#include <unordered_map>
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Value.h"
#include "UniverseInstructionClass.h"

namespace vanguard{

    class LLVMFactory : public UnitFactory {
        public:
            LLVMFactory();

            Universe::CompilationUnit *createUnit(const llvm::Module *module) override;

            Universe::Function *createFn(const llvm::Function *function) override;

            Universe::Instruction *createIns(const llvm::Instruction *instruction) override;

            Universe::Block *createBlk(const llvm::BasicBlock *block) override;

            Type *createType(const llvm::Type *t) override;

            Value *createVal(const llvm::Value *val) override;

        private:

            std::unordered_map<const llvm::Module*, Universe::CompilationUnit*> moduleMap;
            std::unordered_map<const llvm::Function*, Universe::Function*> functionMap;
            std::unordered_map<const llvm::BasicBlock*, Universe::Block*> blockMap;
            std::unordered_map<const llvm::Instruction*, Universe::Instruction*> instructionMap;
            std::unordered_map<const llvm::Type*, Type*> typeMap;
            std::unordered_map<const llvm::Value*, Value*> valueMap;

    };
}

#endif //VANGUARD_PROGRAM_LLVMTOVANGUARD_H
