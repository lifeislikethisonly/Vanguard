//
// Created by Jon Stephens on 8/31/22.
//

#ifndef VANGUARD_BLOCKCHAIN_H
#define VANGUARD_BLOCKCHAIN_H

#include <utility>
#include "BlkValue.h"
#include <iostream>

enum Mutability {
    PURE,
    VIEW,
    PAYABLE,
    NONPAYABLE
};

enum Visibility {
    EXTERNAL,
    PUBLIC,
    INTERNAL,
    PRIVATE,
    DEFAULT
};

namespace vanguard {
    class BlockchainModel;

    template<class Domain>
    class Blockchain : public Domain {
    public:
        class Function;
        class Block;
        class Instruction;

        class Contract : public ObjectType {
        public:
            Contract(BlockchainModel &model, UnitFactory &factory, std::string name, std::vector<Variable *> vars/*, std::vector<BlkType *> inherits,
            std::vector<BlkEnum *> enums, std::vector<BlkStruct *> structs, std::vector<BlkEvent *> events*/) : model(model), contractName(std::move(name)), vars(std::move(vars)), ObjectType(factory) {
            };

            std::string name() const override {
                return contractName;
            }

            //const Function *findFunction(vanguard::Function &fn) const;

            const std::vector<Variable *> &variables() const {
                return vars;
            }

            BlockchainModel &blockchainModel() {
                return model;
            }
            //std::vector<BlkContract *> inherits() const;
        private:
            const std::string contractName;
            BlockchainModel &model;
            /*std::vector<BlkUserType *> contractInherits;
            std::vector<BlkEvent *> contractEvents;
            std::vector<BlkEnum *> contractEnums;
            std::vector<BlkStruct *> contractStructs;*/
            std::vector<Variable *> vars;
        };

        class CompilationUnit : public Domain::CompilationUnit {
        public:
            template<typename ...Args>
            explicit CompilationUnit(std::vector<Contract *> contracts, Args&&... args) : unitContracts(contracts), Domain::CompilationUnit(std::forward<Args>(args)...) {};
            std::vector<Contract *> contracts() const {
                return unitContracts;
            }
        private:
            std::vector<Contract *> unitContracts;
        };

        class Function : public Domain::Function {
        public:
            template<typename ...Args>
            explicit Function(std::string name, std::string selector, bool isCnstr, Visibility vis, Mutability mut, std::vector<Variable *> params, std::vector<Variable *> rets, std::vector<std::string> mods, Args&&... args) : helper(false), parentContract(nullptr), fnParams(std::move(params)), rets(std::move(rets)), mods(std::move(mods)), fnName(std::move(name)), isCnstr(isCnstr), vis(vis), mut(mut), sel(std::move(selector)), Domain::Function(std::forward<Args>(args)...) {};

            template<typename ...Args>
            explicit Function(Args&&... args) : helper(true), parentContract(nullptr), fnParams({}), rets({}), mods({}), fnName(), isCnstr(false), vis(PUBLIC), mut(NONPAYABLE), sel(""), Domain::Function(std::forward<Args>(args)...) {};

            bool isHelper() const {
                return helper;
            }

            std::string name() const override {
                if(helper) {
                    return Domain::Function::name();
                }
                return fnName;
            }

            std::string selector() const {
                return sel;
            }

            std::vector<Variable*> params() const override {
                return fnParams;
            }

            std::vector<std::string> modifiers() const {
                return mods;
            }

            //virtual Type* returnType();

            bool isConstructor() const {
                return isCnstr;
            }

            Visibility visibility() const {
                return vis;
            }

            Mutability mutability() const {
                return mut;
            }

            void wrap(const llvm::Function *fn) {
                this->function = fn;
            }

            template<typename CDomain>
            typename CDomain::Contract *contract() {
                return dynamic_cast<typename CDomain::Contract *>(parentContract);
            }

            void setContract(Contract *contract) {
                if(parentContract == nullptr) {
                    parentContract = contract;
                }
            }

            //virtual std::vector<Instruction*> instructions() const = 0;
            //virtual std::vector<Block *> blocks() const = 0;

        private:
            bool helper;
            std::string sel;
            std::vector<Variable*> fnParams;
            std::vector<Variable *> rets;
            std::vector<std::string> mods;
            std::string fnName;
            bool isCnstr;
            Visibility vis;
            Mutability mut;
            Contract *parentContract;
        };

        class Block : public Domain::Block {
        public:
            template<typename ...Args>
            explicit Block(Args&&... args) : Domain::Block(std::forward<Args>(args)...) {};

            //virtual std::unordered_set<Block*> successors() const = 0;
            //virtual std::vector<Instruction *> instructions() const = 0;
            virtual Function* function() const = 0;
        };

        class Instruction : public Domain::Instruction {
        public:
            template<typename ...Args>
            explicit Instruction(Args&&... args) : Domain::Instruction(std::forward<Args>(args)...) {};

            virtual bool isAnyLowLevelCall()  {
                return false;
            }

            virtual bool isLowLevelCall()  {
                return false;
            }

            virtual bool isLowLevelStaticCall()  {
                return false;
            }

            virtual bool isLowLevelDelegateCall()  {
                return false;
            }

            template<typename IDomain>
            bool writesStorage()  {
                auto ins = dynamic_cast<typename IDomain::Instruction *>(this);
                auto contract = ins->block()->function()->template contract<Blockchain<Domain>>();
                if(contract != nullptr) {
                    auto &model = contract->blockchainModel();
                    return model.writesStorage(*ins);
                }
                return false;
            }

            template<typename IDomain>
            bool readsStorage()  {
                auto ins = dynamic_cast<typename IDomain::Instruction *>(this);
                auto contract = ins->block()->function()->template contract<Blockchain<Domain>>();
                if(contract != nullptr) {
                    auto &model = contract->blockchainModel();
                    return model.readsStorage(*ins);
                }
                return false;
            }

            virtual Block* block() const = 0;
        };

        template<typename InsDomain, typename Wrap>
        class CallExpr : public Domain::template CallExpr<InsDomain, Wrap> {
        public:
            template<typename ...Args>
            explicit CallExpr(Args&&... args) : Domain::template CallExpr<InsDomain, Wrap>(std::forward<Args>(args)...) {};

            bool isAnyLowLevelCall()  override {
                auto contract = this->block()->function()->template contract<Blockchain<Domain>>();
                if(contract != nullptr) {
                    auto &model = contract->blockchainModel();
                    return model.isAnyLowLevelCall(*this);
                }
                return false;
            }

            bool isLowLevelCall()  override {
                auto contract = this->block()->function()->template contract<Blockchain<Domain>>();
                if(contract != nullptr) {
                    auto &model = contract->blockchainModel();
                    return model.isLowLevelCall(*this);
                }
                return false;
            }

            bool isLowLevelStaticCall()  override {
                auto contract = this->block()->function()->template contract<Blockchain<Domain>>();
                if(contract != nullptr) {
                    auto &model = contract->blockchainModel();
                    return model.isLowLevelStaticCall(*this);
                }
                return false;
            }

            bool isLowLevelDelegateCall()  override {
                auto contract = this->block()->function()->template contract<Blockchain<Domain>>();
                if(contract != nullptr) {
                    auto &model = contract->blockchainModel();
                    return model.isLowLevelDelegateCall(*this);
                }
                return false;
            }

            std::vector<typename InsDomain::Function*> targets() const override {
                auto contract = this->block()->function()->template contract<Blockchain<Domain>>();
                InsDomain *universe = this->block()->function()->unit()->universe();
                if(contract != nullptr) {
                    auto &model = contract->blockchainModel();
                    auto callResolver = model.callResolver();
                    if(callResolver != nullptr) {
                        auto tgts = callResolver->resolve(*static_cast<const vanguard::CallExpr<InsDomain> *>(this), *universe);
                        if(!tgts.empty()) {
                            return tgts;
                        }
                    }
                }
                return Domain::template CallExpr<InsDomain, Wrap>::targets();
            }
        };

        template<typename ...Args>
        explicit Blockchain(Args&&... args) : Domain(std::forward<Args>(args)...) {};

        template<typename CDomain>
        std::vector<typename CDomain::Contract *> contracts() const {
            std::vector<typename CDomain::Contract *> contracts;
            for(auto unit : this->units()) {
                auto u = dynamic_cast<Blockchain<Domain>::CompilationUnit *>(unit);
                for(auto contract : u->contracts()) {
                    contracts.push_back(dynamic_cast<typename CDomain::Contract *>(contract));
                }
            }

            return contracts;
        }
    };
}



#endif //VANGUARD_BLOCKCHAIN_H
