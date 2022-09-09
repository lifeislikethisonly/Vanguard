//
// Created by Himanshu on 09/09/22.
//

#ifndef VANGUARD_DETECTORREPORT_H
#define VANGUARD_DETECTORREPORT_H

#include "detectors/DetectorResult.h"

namespace vanguard {
    class DetectorReport {
    public:
        std::vector<vanguard::DetectorResult> detectorResults;
        virtual std::string getReportString() = 0;
    };
}

#endif //VANGUARD_DETECTORREPORT_H
