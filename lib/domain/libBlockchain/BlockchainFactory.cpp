//
// Created by Jon Stephens on 9/1/22.
//

#include "BlockchainFactory.h"
#include <program/Universe.h>
#include <program/WrappedInstructions.h>
#include <program/WrappedValue.h>

#include <utility>
#include "HelperFunction.h"
#include "Blockchain.h"
#include "SummaryReader.h"
#include <program/WrappedType.h>
#include <program/Universe.h>

namespace vanguard {
    BlkFunction<Universe> *BlockchainFactory::createBlkFunction(const llvm::Module &module, BlockchainModel &model, std::string contract, std::string name, std::string selector, bool isCnstr, Visibility vis, Mutability mut, std::vector<Variable *> params, std::vector<Variable *> rets, std::vector<std::string> mods) {
        auto *fn = new BlkFunction<Universe>(name, selector, isCnstr, vis, mut, std::move(params), std::move(rets), std::move(mods), *this, nullptr);

        for(auto &llvmFn : module) {
            if(model.isImplementation(contract, *fn, llvmFn)) {
                fn->wrap(&llvmFn);
            }
        }

        return fn;
    }

    Blockchain<Universe>::Contract *BlockchainFactory::createContract(const llvm::Module &module, BlockchainModel &model, std::string& name, std::vector<BlkFunction<Universe> *>& fns, std::vector<Variable *>& vars) {
        auto *contract = new Blockchain<Universe>::Contract(*this, name, fns, vars);
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
            moduleMap[module] = new Blockchain<Universe>::CompilationUnit(reader.contracts(), *this, module);
        }

        return moduleMap[module];
    }

    Universe::Function *BlockchainFactory::createFn(const llvm::Function *fn) {
        if(fn == nullptr) {
            return nullptr;
        }

        if(functionMap.find(fn) == functionMap.end()) {
            functionMap[fn] = new HelperFunction<Universe>(*this, fn);
        }
        return functionMap[fn];
    }

    Universe::Instruction *BlockchainFactory::createIns(const llvm::Instruction *ins)  {
        if(ins == nullptr) {
            return nullptr;
        }

        if (instructionMap.find(ins) == instructionMap.end()) {
            unsigned opcode = ins->getOpcode();
            if (opcode == 1) instructionMap[ins] = new ReturnIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::ReturnInst>(ins));
            else if (opcode == 2) instructionMap[ins] = new BranchIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::BranchInst>(ins));
            else if (opcode == 3)  instructionMap[ins] = new SwitchIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::SwitchInst>(ins));
            else if (opcode == 4) instructionMap[ins] = new IndirectBrIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::IndirectBrInst>(ins));
            else if (opcode == 7) instructionMap[ins] = new UnreachableIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::UnreachableInst>(ins));
            else if (opcode == 12) instructionMap[ins] = new UnaryIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::UnaryOperator>(ins));
            else if (opcode >= 13 && opcode <= 30) instructionMap[ins] = new BinaryIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::BinaryOperator>(ins));
            else if (opcode == 31) instructionMap[ins] = new AllocaIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::AllocaInst>(ins));
            else if (opcode == 32) instructionMap[ins] = new LoadIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::LoadInst>(ins));
            else if (opcode == 33)  instructionMap[ins] = new StoreIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::StoreInst>(ins));
            else if (opcode == 34) instructionMap[ins] = new GetElementPtrIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::GetElementPtrInst>(ins));
            else if (opcode >= 38 && opcode <= 50) instructionMap[ins] = new CastIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::CastInst>(ins));
            else if (opcode == 53 || opcode == 54) instructionMap[ins] = new CmpIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::CmpInst>(ins));
            else if (opcode == 55) instructionMap[ins] = new PHIIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::PHINode>(ins));
            else if (opcode == 56) instructionMap[ins] = new CallIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::CallBase>(ins));
            else if (opcode == 57) instructionMap[ins] = new SelectIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::SelectInst>(ins));
            else if (opcode == 61) instructionMap[ins] = new ExtractElementIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::ExtractElementInst>(ins));
            else if (opcode == 62) instructionMap[ins] = new InsertElementIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::InsertElementInst>(ins));
            else if (opcode == 63) instructionMap[ins] = new ShuffleVectorIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::ShuffleVectorInst>(ins));
            else if (opcode == 64) instructionMap[ins] = new ExtractValueIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::ExtractValueInst>(ins));
            else if (opcode == 65) instructionMap[ins] = new InsertValueIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::InsertValueInst>(ins));
            else if (opcode == 67) instructionMap[ins] = new FreezeIns<Blockchain<Universe>>(*this,*llvm::dyn_cast<llvm::FreezeInst>(ins));
            else throw std::runtime_error("The instruction class " + (std::string)ins->getOpcodeName() + " with opcode " + std::to_string(opcode) + " does not exist in Vanguard yet.");
        }
        return instructionMap[ins];
    }

    Universe::Block *BlockchainFactory::createBlk(const llvm::BasicBlock *block) {
        if(block == nullptr) {
            return nullptr;
        }

        if (blockMap.find(block) == blockMap.end()){
            blockMap[block] = new Blockchain<Universe>::Block(*this,block);
        }
        return blockMap[block];
    }


}