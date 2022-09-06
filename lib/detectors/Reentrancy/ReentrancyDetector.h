//
// Created by Jon Stephens on 9/3/22.
//

#ifndef VANGUARD_REENTRANCYDETECTOR_H
#define VANGUARD_REENTRANCYDETECTOR_H

#include <analysis/reachability/Reachable.h>
#include <domain/libBlockchain/BlockchainModel.h>
#include <detectors/UniverseDetector.h>
#include <iostream>
#include <unordered_map>

namespace vanguard {
    template <typename Domain>
    class ReentrancyDetector : public UniverseDetector<Domain> {
    public:
        using CompilationUnit = typename Domain::CompilationUnit;
        using Function = typename Domain::Function;
        using Block = typename Domain::Block;
        using Instruction = typename Domain::Instruction;

        struct CallTargetResolver : public InstructionClassVisitor<Domain> {
            std::vector<typename Domain::Function *> tgts;
            void visit(const CallExpr<Domain> &v) override{
                tgts = v.targets();
            }
        };

        struct StateModifiedReachability : public Reachable<Domain> {
            StateModifiedReachability(std::unordered_set<Function *>& writesStorage) : storageWriteFns(writesStorage) {};

            bool condition(Instruction &ins) {
                CallTargetResolver resolver;
                ins.accept(resolver);

                return ins.template writesStorage<Domain>() || std::any_of(resolver.tgts.begin(), resolver.tgts.end(),
                        [&](auto tgt){return storageWriteFns.find(tgt) != storageWriteFns.end();});
            }

            std::unordered_set<Function *>& storageWriteFns;
        };

        void process(Function *fn) {
            processed.insert(fn);
            StateModifiedReachability reach(writesStorage);

            for(auto blk : fn->blocks()) {
                for(auto ins : blk->instructions()) {
                    CallTargetResolver resolver;
                    ins->accept(resolver);

                    for(auto tgt : resolver.tgts) {
                        if(processed.find(tgt) == processed.end()) {
                            process(tgt);
                        }

                        if(ins->isAnyLowLevelCall() || calls.find(tgt) != calls.end()) {
                            calls.insert(fn);
                            if(reach.reachable(*ins)) {
                                std::cout << "vulnerable" << std::endl;
                            }
                        }

                        if(ins->template writesStorage<Domain>() || writesStorage.find(tgt) != writesStorage.end()) {
                            writesStorage.insert(fn);
                        }
                    }
                }
            }
        }

        void detect(Domain &universe) override {
            auto contracts = universe.template contracts<Domain>();
            for(auto contract : contracts) {
                for(auto fn : contract->functions()) {
                    process(fn);
                }
            }
        }

        virtual void report() override {
            std::cout << "Done!" << std::endl;
        }

        static std::string name() {
            return "Reentrancy";
        }

        static Detector::DetectorDomain domain() {
            return Detector::BLOCKCHAIN;
        }
    private:
        std::unordered_set<Function *> processed;
        std::unordered_set<Function *> writesStorage;
        std::unordered_set<Function *> calls;
    };
}


#endif //VANGUARD_REENTRANCYDETECTOR_H
