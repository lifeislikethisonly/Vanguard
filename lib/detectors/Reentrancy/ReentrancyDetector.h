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

    class ReentrancyDetectorResult : public DetectorResult{
    public:
        ReentrancyDetectorResult(VulLocation *vulLocation, std::string vulDescription): DetectorResult(){
            this->vulLocation = vulLocation;
            this->vulDescription = vulDescription;
        }

        VulLocation *getVulLocation() override{
            return vulLocation;
        }

        std::string getVulDescription() override{
            return vulDescription;
        }
    };

    class ReentrancyDetectorReport : public DetectorReport{
    public:
        explicit ReentrancyDetectorReport(std::vector<DetectorResult *> detectorResults, std::string detectorName): DetectorReport(){
            this->detectorResults = detectorResults;
            this->detectorName = detectorName;
        }
    };

    template <typename Domain>
    class ReentrancyDetector : public UniverseDetector<Domain> {
    public:
        std::vector<DetectorResult *> detectorResults = {};
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
            VulFunctionLocation *vulFunctionLocation = new VulFunctionLocation();
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
                                vulFunctionLocation->function = fn;
                                DetectorResult *detectorResult = new ReentrancyDetectorResult(vulFunctionLocation, "Function might have reentrancy in instruction "+ ins->name());
                                detectorResults.push_back(detectorResult);
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

        virtual DetectorReport *report() override {
            std::cout << "Done!" << std::endl;
            auto *detectorReport = new ReentrancyDetectorReport(detectorResults, this->name());
            return detectorReport;
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
