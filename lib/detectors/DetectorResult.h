//
// Created by Himanshu on 09/09/22.
//

#ifndef VANGUARD_DETECTORRESULT_H
#define VANGUARD_DETECTORRESULT_H

#include "../program/Universe.h"

namespace vanguard{
    class VulLocation{
    public:
        virtual std::string string() = 0;
    };

    class VulFunctionLocation: public VulLocation{
    public:
        Universe::Function *function;
        std::string string() override{
            return function->name();
        }
    };

    class DetectorResult{
    public:
        std::tuple<VulLocation *, std::string> vulnerability;
        std::string detectorName;

        virtual VulLocation *getVulLocation() = 0;
        virtual std::string getVulDescription() = 0;
        virtual std::string getDetectorName() = 0;
    };
}

#endif //VANGUARD_DETECTORRESULT_H
