//
// Created by Jon Stephens on 9/1/22.
//

#ifndef VANGUARD_BLOCKCHAINFACTORY_H
#define VANGUARD_BLOCKCHAINFACTORY_H

#include <program/LLVMFactory.h>
#include <unordered_map>
#include "Blockchain.h"
#include "BlkType.h"
#include "BlockchainModel.h"

namespace vanguard {
    class BlockchainFactory : public LLVMFactory {
    public:
        BlockchainFactory() = default;

        BlkType<UnknownType> *createBasicType(const llvm::Module &module, BlockchainModel &model, std::string name);

        Top<Blockchain<Universe>>::Function *createBlkFunction(const llvm::Module &module, BlockchainModel &model, std::string contract, std::string name, std::string selector, bool isCnstr, Visibility vis, Mutability mut, std::vector<Variable *> params, std::vector<Variable *> rets, std::vector<std::string> mods);

        Blockchain<Universe>::Contract *createContract(const llvm::Module &module, BlockchainModel &model, std::string& name, std::vector<Top<Blockchain<Universe>>::Function *>& fns, std::vector<Variable *>& vars);

        BlkStructType<StructType> *createStruct(const llvm::Module &mod, BlockchainModel &model, const std::string& name, std::vector<Variable *> fields);

        BlkVariable<Variable> *createVariable(const llvm::Module &module, BlockchainModel &model, std::string name, Type *type);

        BlkArrayType<ArrayType> *createArrayType(const llvm::Module &module, BlockchainModel &model, std::string name, Type *base);

        BlkMapType<MapType> *createMapType(const llvm::Module &module, BlockchainModel &model, std::string name, Type *keyType, Type *valType);

        Blockchain<Universe>::CompilationUnit *createUnit(const llvm::Module *module, const std::string& summary);

        Universe::CompilationUnit *createUnit(const llvm::Module *module) override;

        Universe::Function *createFn(const llvm::Function *function) override;

        Universe::Instruction *createIns(const llvm::Instruction *instruction) override;

        Universe::Block *createBlk(const llvm::BasicBlock *block) override;

        /*Type *createType(const llvm::Type *t) override;

        Value *createVal(const llvm::Value *val) override;*/

    private:

        std::unordered_map<const llvm::Module*, Blockchain<Universe>::CompilationUnit*> moduleMap;
        std::unordered_map<const llvm::Function*, Universe::Function*> functionMap;
        std::unordered_map<const llvm::BasicBlock*, Universe::Block*> blockMap;
        std::unordered_map<const llvm::Instruction*, Universe::Instruction*> instructionMap;
        //std::unordered_map<const llvm::Type*, Type*> typeMap;
        //std::unordered_map<const llvm::Value*, Value*> valueMap;
    };
}



#endif //VANGUARD_BLOCKCHAINFACTORY_H
