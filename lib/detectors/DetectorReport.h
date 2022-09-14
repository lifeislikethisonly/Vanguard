//
// Created by Himanshu on 09/09/22.
//

#ifndef VANGUARD_DETECTORREPORT_H
#define VANGUARD_DETECTORREPORT_H

#include "detectors/DetectorResult.h"
#include "../domain/libBlockchain/rapidjson/document.h"
#include "../domain/libBlockchain/rapidjson/stringbuffer.h"
#include "../domain/libBlockchain/rapidjson/prettywriter.h"

namespace vanguard {
    class DetectorReport {
    public:
        std::vector<vanguard::DetectorResult *> detectorResults;
        std::string detectorName;
    };

    class ReportFormatter{
    public:
        virtual std::string format(DetectorReport &report) = 0;
        virtual std::string preamble() = 0; //Sets up the json object.
        /*
         * {
    "issues": [
         */
        virtual std::string postamble() = 0;// closes the json string.

    };

    class JsonFormat : public ReportFormatter{
    public:
        bool isFirstEntry = true;

        std::string preamble() override{
            std::string output = "{\n \"issues\": [";
            return output;
        }

        std::string format(DetectorReport &report ) override{
            std::string output = "";
            for(auto detectorResult : report.detectorResults){
                if(isFirstEntry){
                    isFirstEntry = false;
                }
                else{
                    output += ",";
                }
                output+= "\n  {\n   \"Detector\": \"" + report.detectorName + "\",\n   ";
                output+= "\"Location\": \"" + detectorResult->vulLocation->string() + "\",\n   ";
                output+= "\"Description\": \"" + detectorResult->vulDescription + "\"\n  }";
            }
            return output;

//            rapidjson::Document jsonDoc;
//            jsonDoc.SetObject();
//            rapidjson::Value issues(rapidjson::kArrayType);
//            rapidjson::Document::AllocatorType& allocator = jsonDoc.GetAllocator();
//
//            for(auto detectorResult : report.detectorResults) {
//                rapidjson::Value obj(rapidjson::kObjectType);
//                rapidjson::Value val(rapidjson::kObjectType);
//                val.SetString(report.detectorName.c_str(), static_cast<rapidjson::SizeType>(report.detectorName.length()), allocator);
//                obj.AddMember("Detector", val, allocator);
//                val.SetString(detectorResult->vulLocation->string().c_str(), static_cast<rapidjson::SizeType>(detectorResult->vulLocation->string().length()), allocator);
//                obj.AddMember("Location", val, allocator);
//                val.SetString(detectorResult->vulDescription.c_str(), static_cast<rapidjson::SizeType>(detectorResult->vulDescription.length()), allocator);
//                obj.AddMember("Description",val, allocator);
//                issues.PushBack(obj,allocator);
//            }
//            jsonDoc.AddMember("issues", issues, allocator);
//
//            rapidjson::StringBuffer strbuf;
//            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
//            jsonDoc.Accept(writer);
//            std::string jsonString = strbuf.GetString();
//            return jsonString;
        }

        std::string postamble() override{
            return "\n ]\n}";
        }
    };

    class HumanReadableFormat : public ReportFormatter{
    public:
        std::string preamble() override{
            return "";
        }

        std::string postamble() override{
            return "";
        }

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
