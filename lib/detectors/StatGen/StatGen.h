//
// Created by Jon Stephens on 8/14/22.
//

#ifndef VANGUARD_STATGEN_H
#define VANGUARD_STATGEN_H

#include "../UnitDetector.h"
#include <unordered_set>
#include <iostream>

namespace vanguard {
    template<typename Domain>
    class StatGen : public UnitDetector<Domain> {
    public:
        std::vector<Requirement *> registerAnalyses() override {
            return {};
        }

        void startDetection() override {
            totBlks = 0;
            totFns = 0;
            totIns = 0;
        }

        void detect(typename Domain::CompilationUnit &unit) override {
            for(auto fn : unit.functions()) {
                totFns++;
                if(fn->hasBody()) {
                    auto body = fn->body();
                    assert(body->function() == fn && "Function does not match block function");

                    countBody(body);
                }
            }
        }

        DetectorReport report() override {
            std::cout << "Statistics:" << std::endl;
            std::cout << "# Functions: " << totFns << std::endl;
            std::cout << "# Basic Blocks: " << totBlks << std::endl;
            std::cout << "# Instructions: " << totIns << std::endl;
        }

        static std::string name() {
            return "statGen";
        }

        static Detector::DetectorDomain domain() {
            return Detector::BASIC;
        }
    private:
        void countBody(typename Domain::Block *blk) {
            assert(blk != nullptr && "Function does not have a body");
            assert(blk->isEntry() && "Fn body not marked as entry block");

            std::unordered_set<typename Domain::Block *> seen = { blk };
            std::vector<typename Domain::Block *> worklist = { blk };

            while(!worklist.empty()) {
                typename Domain::Block *curBlk = worklist.back();
                worklist.pop_back();

                totBlks++;

                for(auto ins : curBlk->instructions()) {
                    assert(curBlk == ins->block() && "Block does not match instruction block");
                    totIns++;
                }

                for(auto next : curBlk->successors()) {
                    if(next != nullptr && seen.find(next) == seen.end()) {
                        seen.insert(next);
                        worklist.push_back(next);
                    }
                }
            }
        }

        int totFns = 0;
        int totIns = 0;
        int totBlks = 0;
    };
}



#endif //VANGUARD_STATGEN_H
