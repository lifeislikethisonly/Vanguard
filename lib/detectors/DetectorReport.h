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
        std::string detectorName;
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
            std::string output = std::string(40,'=') + "\n";
            output += "Detector : " + report.detectorName + "\n";
            output += std::string(40,'=') + "\n";
            for(auto detectorResult : report.detectorResults){
                output += "Location : " + detectorResult->vulLocation->string() + "\n";
                output += "Description : " + detectorResult->vulDescription + "\n";
                output += std::string(40, '-') + "\n";
            }
            return output;
        }
    };
}

#endif //VANGUARD_DETECTORREPORT_H
