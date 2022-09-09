//
// Created by Himanshu on 09/09/22.
//

#ifndef VANGUARD_DETECTORREPORT_H
#define VANGUARD_DETECTORREPORT_H

#include "detectors/DetectorResult.h"

namespace vanguard {
    class DetectorReport {
    public:
        std::vector<vanguard::DetectorResult *> detectorResults;
    };

    class ReportFormatter{
    public:
        virtual std::string format(DetectorReport &report) = 0;
    };

    class JsonFormat : public ReportFormatter{
    public:
        std::string format(DetectorReport &report ) override{
            return "";
        }
    };

    class HumanReadableFormat : public ReportFormatter{
    public:
        std::string format(DetectorReport &report ) override{
            return "";
        }
    };
}

#endif //VANGUARD_DETECTORREPORT_H
