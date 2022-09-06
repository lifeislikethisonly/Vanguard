//
// Created by Jon Stephens on 9/1/22.
//

#include "BlockchainFactory.h"
#include <program/Universe.h>
#include <program/WrappedValue.h>

#include <utility>
#include "Blockchain.h"
#include "SummaryReader.h"
#include <program/WrappedType.h>
#include <program/Universe.h>
#include <program/Top.h>

namespace vanguard {
    Top<Blockchain<Universe>>::Function *BlockchainFactory::createBlkFunction(const llvm::Module &module, BlockchainModel &model, std::string contract, std::string name, std::string selector, bool isCnstr, Visibility vis, Mutability mut, std::vector<Variable *> params, std::vector<Variable *> rets, std::vector<std::string> mods) {
        auto fn = new Top<Blockchain<Universe>>::Function(name, selector, isCnstr, vis, mut, std::move(params), std::move(rets), std::move(mods), *this, nullptr);

        for(auto &llvmFn : module) {
            if(model.isImplementation(contract, *fn, llvmFn)) {
                fn->wrap(&llvmFn);
                functionMap[&llvmFn] = fn;
                break;
            }
        }

        return fn;
    }

    Blockchain<Universe>::Contract *BlockchainFactory::createContract(const llvm::Module &module, BlockchainModel &model, std::string& name, std::vector<Top<Blockchain<Universe>>::Function *>& fns, std::vector<Variable *>& vars) {
        auto *contract = new Top<Blockchain<Universe>>::Contract(fns, model, *this, name, vars);
        return contract;
    }

    BlkType<UnknownType> *BlockchainFactory::createBasicType(const llvm::Module &module, BlockchainModel &model, std::string name) {
        //fix this later
        return new BlkType<UnknownType>(name, *this);
    }

    BlkStructType<StructType> *BlockchainFactory::createStruct(const llvm::Module &mod, BlockchainModel &model, const std::string& name, std::vector<Variable *> fields) {
        auto *structType = new BlkStructType<StructType>(std::move(fields), name, *this);
        return structType;
    }

    BlkVariable<Variable> *BlockchainFactory::createVariable(const llvm::Module &module, BlockchainModel &model, std::string name, Type *type) {
        auto *var = new BlkVariable<Variable>(std::move(name), type, *this);
        return var;
    }

    BlkArrayType<ArrayType> *BlockchainFactory::createArrayType(const llvm::Module &module, BlockchainModel &model, std::string name, Type *base) {
        auto *arr = new BlkArrayType<ArrayType>(base, name, *this);
        return arr;
    }

    BlkMapType<MapType> *BlockchainFactory::createMapType(const llvm::Module &module, BlockchainModel &model, std::string name, Type *keyType, Type *valType) {
        auto *map = new BlkMapType<MapType>(keyType, valType, name, *this);
        return map;
    }

    Blockchain<Universe>::CompilationUnit *BlockchainFactory::createUnit(const llvm::Module *module, const std::string& summary) {
        if(module == nullptr) {
            return nullptr;
        }

        if(moduleMap.find(module) == moduleMap.end()){
            SummaryReader reader(*module, summary, this);
            moduleMap[module] = new Top<Blockchain<Universe>>::CompilationUnit(reader.contracts(), *this, module);
        }

        return moduleMap[module];
    }

    Universe::CompilationUnit *BlockchainFactory::createUnit(const llvm::Module *module) {
        return moduleMap[module];
    }

    Universe::Function *BlockchainFactory::createFn(const llvm::Function *fn) {
        if(fn == nullptr) {
            return nullptr;
        }

        if(functionMap.find(fn) == functionMap.end()) {
            functionMap[fn] = new Top<Blockchain<Universe>>::Function(*this, fn);
        }
        return functionMap[fn];
    }

    Universe::Instruction *BlockchainFactory::createIns(const llvm::Instruction *ins)  {
        if(ins == nullptr) {
            return nullptr;
        }

        if (instructionMap.find(ins) == instructionMap.end()) {
            unsigned opcode = ins->getOpcode();
            if (opcode == 1) instructionMap[ins] = new Universe::Return<Top<Blockchain<Universe>>, llvm::ReturnInst>(llvm::dyn_cast<llvm::ReturnInst>(ins), *this);
            else if (opcode == 2) instructionMap[ins] = new Universe::Branch<Top<Blockchain<Universe>>, llvm::BranchInst>(llvm::dyn_cast<llvm::BranchInst>(ins), *this);
            else if (opcode == 3)  instructionMap[ins] = new Universe::Branch<Top<Blockchain<Universe>>, llvm::SwitchInst>(llvm::dyn_cast<llvm::SwitchInst>(ins), *this);
            else if (opcode == 4) instructionMap[ins] = new Universe::Branch<Top<Blockchain<Universe>>, llvm::IndirectBrInst>(llvm::dyn_cast<llvm::IndirectBrInst>(ins), *this);
            else if (opcode == 7) instructionMap[ins] = new Universe::Error<Top<Blockchain<Universe>>, llvm::UnreachableInst>(llvm::dyn_cast<llvm::UnreachableInst>(ins), *this);
            else if (opcode == 12) instructionMap[ins] = new Universe::UnaryOpExpr<Top<Blockchain<Universe>>, llvm::UnaryOperator>(llvm::dyn_cast<llvm::UnaryOperator>(ins), *this);
            else if (opcode >= 13 && opcode <= 30) instructionMap[ins] = new Universe::BinaryOpExpr<Top<Blockchain<Universe>>, llvm::BinaryOperator>(llvm::dyn_cast<llvm::BinaryOperator>(ins), *this);
            else if (opcode == 31) instructionMap[ins] = new Universe::UnknownExpr<Top<Blockchain<Universe>>, llvm::AllocaInst>(llvm::dyn_cast<llvm::AllocaInst>(ins), *this);
            else if (opcode == 32) instructionMap[ins] = new Universe::UnknownExpr<Top<Blockchain<Universe>>, llvm::LoadInst>(llvm::dyn_cast<llvm::LoadInst>(ins), *this);
            else if (opcode == 33)  instructionMap[ins] = new Universe::UnknownExpr<Top<Blockchain<Universe>>, llvm::StoreInst>(llvm::dyn_cast<llvm::StoreInst>(ins), *this);
            else if (opcode == 34) instructionMap[ins] = new Universe::UnknownExpr<Top<Blockchain<Universe>>, llvm::GetElementPtrInst>(llvm::dyn_cast<llvm::GetElementPtrInst>(ins), *this);
            else if (opcode >= 38 && opcode <= 50) instructionMap[ins] = new Universe::CastExpr<Top<Blockchain<Universe>>, llvm::CastInst>(llvm::dyn_cast<llvm::CastInst>(ins), *this);
            else if (opcode == 53 || opcode == 54) instructionMap[ins] = new Universe::BinaryOpExpr<Top<Blockchain<Universe>>, llvm::CmpInst>(llvm::dyn_cast<llvm::CmpInst>(ins), *this);
            else if (opcode == 55) instructionMap[ins] = new Universe::UnknownExpr<Top<Blockchain<Universe>>, llvm::PHINode>(llvm::dyn_cast<llvm::PHINode>(ins), *this);
            else if (opcode == 56) instructionMap[ins] = new Blockchain<Universe>::CallExpr<Top<Blockchain<Universe>>, llvm::CallBase>(llvm::dyn_cast<llvm::CallBase>(ins), *this);
            else if (opcode == 57) instructionMap[ins] = new Universe::TernaryExpr<Top<Blockchain<Universe>>, llvm::SelectInst>(llvm::dyn_cast<llvm::SelectInst>(ins), *this);
            else if (opcode == 61) instructionMap[ins] = new Universe::UnknownExpr<Top<Blockchain<Universe>>, llvm::ExtractElementInst>(llvm::dyn_cast<llvm::ExtractElementInst>(ins), *this);
            else if (opcode == 62) instructionMap[ins] = new Universe::UnknownExpr<Top<Blockchain<Universe>>, llvm::InsertElementInst>(llvm::dyn_cast<llvm::InsertElementInst>(ins), *this);
            else if (opcode == 63) instructionMap[ins] = new Universe::UnknownExpr<Top<Blockchain<Universe>>, llvm::ShuffleVectorInst>(llvm::dyn_cast<llvm::ShuffleVectorInst>(ins), *this);
            else if (opcode == 64) instructionMap[ins] = new Universe::UnknownExpr<Top<Blockchain<Universe>>, llvm::ExtractValueInst>(llvm::dyn_cast<llvm::ExtractValueInst>(ins), *this);
            else if (opcode == 65) instructionMap[ins] = new Universe::UnknownExpr<Top<Blockchain<Universe>>, llvm::InsertValueInst>(llvm::dyn_cast<llvm::InsertValueInst>(ins), *this);
            else if (opcode == 67) instructionMap[ins] = new Universe::UnknownExpr<Top<Blockchain<Universe>>, llvm::FreezeInst>(llvm::dyn_cast<llvm::FreezeInst>(ins), *this);
            else throw std::runtime_error("The instruction class " + (std::string)ins->getOpcodeName() + " with opcode " + std::to_string(opcode) + " does not exist in Vanguard yet.");
        }
        return instructionMap[ins];
    }

    Universe::Block *BlockchainFactory::createBlk(const llvm::BasicBlock *block) {
        if(block == nullptr) {
            return nullptr;
        }

        if (blockMap.find(block) == blockMap.end()){
            blockMap[block] = new Top<Blockchain<Universe>>::Block(*this,block);
        }
        return blockMap[block];
    }


}