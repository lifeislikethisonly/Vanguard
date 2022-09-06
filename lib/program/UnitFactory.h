//
// Created by Jon Stephens on 8/31/22.
//

#ifndef VANGUARD_UNITFACTORY_H
#define VANGUARD_UNITFACTORY_H

#include "Universe.h"
#include <list>
#include "Type.h"
#include "Value.h"

namespace vanguard{

    class UnitFactory {
    public:
        virtual Universe::CompilationUnit *createUnit(const llvm::Module *module) = 0;

        virtual Universe::Function *createFn(const llvm::Function *function) = 0;

        virtual Universe::Instruction *createIns(const llvm::Instruction *instruction) = 0;

        virtual Universe::Block *createBlk(const llvm::BasicBlock *block) = 0;

        virtual Type *createType(const llvm::Type *t) = 0;

        virtual Value *createVal(const llvm::Value *val) = 0;

    };
}


#endif //VANGUARD_UNITFACTORY_H
