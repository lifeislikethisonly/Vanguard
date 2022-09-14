//
// Created by Jon Stephens on 8/13/22.
//

#ifndef VANGUARD_FUNCTIONPRINTER_H
#define VANGUARD_FUNCTIONPRINTER_H

#include "../FunctionDetector.h"
#include <iostream>

namespace vanguard {

    class FPDetectorResult: public DetectorResult{
    public:
        FPDetectorResult(VulLocation *vulLocation, std::string vulDescription): DetectorResult(){
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

    class FPDetectorReport: public DetectorReport{
    public:
        explicit FPDetectorReport(std::vector<DetectorResult *> detectorResults, std::string detectorName): DetectorReport(){
            this->detectorResults = detectorResults;
            this->detectorName = detectorName;
        }
    };

    template<typename Domain>
    class FunctionPrinter : public FunctionDetector<Domain> {
    public:
        std::string vulDescription;
        FPDetectorResult *fpDetectorResult = nullptr;
        std::vector<DetectorResult *> detectorResults;

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
            VulFunctionLocation *vulFunctionLocation = new VulFunctionLocation();
            vulFunctionLocation->function = &fn;
            vulDescription = "This function might be vulnerable";
            fpDetectorResult = new FPDetectorResult(vulFunctionLocation, vulDescription);
            detectorResults.push_back(fpDetectorResult);
            return false;
        }
        DetectorReport *report() override {
            std::cout << "Done!" << std::endl;
            FPDetectorReport *detectorReport = new FPDetectorReport(detectorResults, this->name());
            return detectorReport;
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
