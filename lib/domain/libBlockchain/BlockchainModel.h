//
// Created by Jon Stephens on 3/23/22.
//

#ifndef LIBBLOCKCHAIN_BLOCKCHAINTOLLVM_H
#define LIBBLOCKCHAIN_BLOCKCHAINTOLLVM_H

#include "program/Top.h"
#include "Blockchain.h"
#include "CallResolver.h"


namespace vanguard {
    class BlockchainModel {
        public:
            virtual ~BlockchainModel() = default;
            virtual bool isImplementation(std::string contract, const Top<Blockchain<Universe>>::Function &blockchainFn, const llvm::Function &llvmFn) const = 0;
            virtual bool isAnyLowLevelCall(CallExpr<Top<Blockchain<Universe>>> &call)  = 0;
            virtual bool isLowLevelCall(CallExpr<Top<Blockchain<Universe>>>&call)  = 0;
            virtual bool isLowLevelStaticCall(CallExpr<Top<Blockchain<Universe>>> &call)  = 0;
            virtual bool isLowLevelDelegateCall(CallExpr<Top<Blockchain<Universe>>> &call)  = 0;
            virtual bool writesStorage(Top<Blockchain<Universe>>::Instruction &ins) = 0;
            virtual bool readsStorage(Top<Blockchain<Universe>>::Instruction &ins) = 0;
            virtual CallResolver<Top<Blockchain<Universe>>> *callResolver() = 0;
            //virtual bool isExternalCall(const llvm::Function &llvmFn) = 0;
            //virtual bool isDelegateCall(const llvm::Function &llvmFn) = 0;
            //virtual bool isAnyExternalCall(vanguard::Function &llvmFn) const = 0;
            //virtual bool isCall(vanguard::Function &llvmFn) const = 0;
            //virtual bool isStaticCall(vanguard::Function &llvmFn) const = 0;
            //virtual bool isDelegateCall(vanguard::Function &llvmFn) const = 0;

            //virtual bool writesVariable(const BlkVariable &var, vanguard::Instruction &ins) const = 0;
            //virtual bool readsVariable(const BlkVariable &var, vanguard::Instruction &ins) const = 0;
        private:
    };
}



#endif //LIBBLOCKCHAIN_BLOCKCHAINTOLLVM_H
