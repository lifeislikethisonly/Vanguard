#include "Universe.h"
#include "LLVMFactory.h"
#include "llvm/IR/InstIterator.h"
#include <iostream>

namespace vanguard{
    
    Universe::Function::Function(UnitFactory &factory, const llvm::Function* func): factory(factory), function(func){
    }

    std::string Universe::Function::name() const {
        return function->getName().str();
    }

    std::vector<Variable*> Universe::Function::params() const {
        std::vector<Variable*> params = {};
        for (auto itr = function->arg_begin(); itr != function->arg_end(); itr++){
            params.push_back((Variable *) factory.createVal(itr));
        }
        return params;
    }

    Type* Universe::Function::returnType() const {
        return factory.createType(function->getReturnType());
    }

    bool Universe::Function::hasBody() const {
        return !function->isDeclaration();
    }

    Universe::Block* Universe::Function::head() const {
        return factory.createBlk(&function->getEntryBlock());
    }

    std::vector<Universe::Instruction*> Universe::Function::insts() const {
        std::vector<Instruction*> instructionsList = {};
        for (auto &blk : *function){
            for(auto &ins : blk) {
                instructionsList.push_back(factory.createIns(&ins));
            }
        }
        return instructionsList;
    }

    std::vector<Universe::Block *> Universe::Function::blks() const {
        std::vector<Block *> blocks = {};
        for (auto &blk : *function){
            blocks.push_back(factory.createBlk(&blk));
        }
        return blocks;
    }

    const llvm::Function *Universe::Function::unwrap(){
        return function;
    }

    Universe::CompilationUnit *Universe::Function::compilationUnit() const {
        return factory.createUnit(function->getParent());
    }
}