//
// Created by Jon Stephens on 8/13/22.
//

#ifndef VANGUARD_FUNCTIONPRINTER_H
#define VANGUARD_FUNCTIONPRINTER_H

#include "../FunctionDetector.h"
#include <iostream>

namespace vanguard {

    template<typename Domain>
    class FunctionPrinter : public FunctionDetector<Domain> {
    public:
        using CompilationUnit = typename Domain::CompilationUnit;
        using Function = typename Domain::Function;
        using Block = typename Domain::Block;
        using Instruction = typename Domain::Instruction;

        std::vector<Requirement *> registerAnalyses() override {
            return {};
        }

        void startDetection() override {}
        bool detect(Function &fn) override {
            std::cout << "Found Function: " << fn.name() << std::endl;
            return false;
        }
        DetectorReport report() override {
            std::cout << "Done!" << std::endl;
        }

        static Detector::DetectorDomain domain() {
            return Detector::BASIC;
        }

        static std::string name() {
            return "fnPrinter";
        }
    private:
    };
}



#endif //VANGUARD_FUNCTIONPRINTER_H
