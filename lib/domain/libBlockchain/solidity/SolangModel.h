//
// Created by Jon Stephens on 3/23/22.
//

#ifndef LIBBLOCKCHAIN_SOLANGTOLLVM_H
#define LIBBLOCKCHAIN_SOLANGTOLLVM_H

#include "../BlockchainModel.h"
#include "llvm/IR/Function.h"

namespace vanguard {
    class SolangModel : public BlockchainModel {
    public:
        bool isImplementation(std::string contract, const Top<Blockchain<Universe>>::Function &blockchainFn, const llvm::Function &llvmFn) const override;
        //bool isExternalCall(const llvm::Function &llvmFn) override;
        //bool isDelegateCall(const llvm::Function &llvmFn) override;
        bool isAnyLowLevelCall(CallExpr<Top<Blockchain<Universe>>> &call)  override;
        bool isLowLevelCall(CallExpr<Top<Blockchain<Universe>>> &call)  override;
        bool isLowLevelStaticCall(CallExpr<Top<Blockchain<Universe>>> &call)  override;
        bool isLowLevelDelegateCall(CallExpr<Top<Blockchain<Universe>>> &call)  override;
        bool writesStorage(Top<Blockchain<Universe>>::Instruction &ins) override;
        bool readsStorage(Top<Blockchain<Universe>>::Instruction &ins) override;
        CallResolver<Top<Blockchain<Universe>>> *callResolver() override;
        /*bool writesVariable(const BlkVariable &var, vanguard::Instruction &ins) const override;
        bool readsVariable(const BlkVariable &var, vanguard::Instruction &ins) const override;*/

        /*static bool writesStorage(vanguard::Function &fn);
        static bool readsStorage(vanguard::Function &fn);*/
    private:
    };
}



#endif //LIBBLOCKCHAIN_SOLANGTOLLVM_H
