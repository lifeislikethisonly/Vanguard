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
        FPDetectorResult(VulLocation *vulLocation, std::string vulDescription, std::string detectorName): DetectorResult(){
            this->detectorName = detectorName;
            this->vulnerability = std::make_tuple(vulLocation, vulDescription);
        }

        VulLocation *getVulLocation() override{
            return std::get<0>(vulnerability);
        }

        std::string getVulDescription() override{
            return std::get<1>(vulnerability);
        }

        std::string getDetectorName() override{
            return detectorName;
        }
    };

    class FPDetectorReport: public DetectorReport{
    public:
        explicit FPDetectorReport(std::vector<DetectorResult *> detectorResults): DetectorReport(){
            this->detectorResults = detectorResults;
        }
    };

    template<typename Domain>
    class FunctionPrinter : public FunctionDetector<Domain> {
    public:
        VulFunctionLocation *vulFunctionLocation = new VulFunctionLocation();
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
            vulFunctionLocation->function = &fn;
            vulDescription = "This function might be vulnerable";
            fpDetectorResult = new FPDetectorResult(vulFunctionLocation, vulDescription, this->name());
            detectorResults.push_back(fpDetectorResult);
            return false;
        }
        DetectorReport report() override {
            std::cout << "Done!" << std::endl;
            FPDetectorReport *detectorReport = new FPDetectorReport(detectorResults);
            return *detectorReport;
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
