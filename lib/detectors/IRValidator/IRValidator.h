//
// Created by Jon Stephens on 8/14/22.
//

#ifndef VANGUARD_IRVALIDATOR_H
#define VANGUARD_IRVALIDATOR_H

#include "../UnitDetector.h"
#include <iostream>
#include "InstructionTrans.h"
#include "ValueTrans.h"

namespace vanguard {
    template<typename Domain>
    class IRValidator : public UnitDetector<Domain> {
    public:
        std::vector<Requirement *> registerAnalyses() override {
            return {};
        }

        void startDetection() override {
            totBlks = 0;
            totFns = 0;
            totIns = 0;
        }

        std::string getInstructionString(const typename Domain::Instruction &v){
            InstructionTrans<Domain> trans;
            v.accept(trans);
            return trans.result;
        }

        void detect(typename Domain::CompilationUnit &unit) override {
            moduleName = unit.name();
            sourceFileName = unit.sourceFile();

            for(auto fn : unit.functions()) {
                totFns++;
                if(fn->hasBody()) {
                    auto body = fn->body();
                    assert(body->function() == fn && "Function does not match block function");
                    assert(body->isEntry() == true && "Not an entry block.");
                    countBody(body);
                }
            }

            if(debug_ir_validator){
                std::cout << "\n ---- Listing global variables with their types ---- \n";
            }
            for (auto globalVariableValue: unit.globalVariables()){
                auto globalVariable = llvm::dyn_cast<Variable>(globalVariableValue);
                if (globalVariable->hasName()) {
                    auto globalVariableFromName = (Variable *) unit.findGlobalVariable(globalVariable->name());
                    assert(globalVariableFromName == globalVariable && "globalVariable and globalVariableFromName ");
                    assert(globalVariableFromName != nullptr && "Global variable from Name is null.");
                    auto globalVariableName = globalVariableFromName->name();
                    auto globalVariableType = (globalVariableFromName->type())->name();
                    if (debug_ir_validator) {
                        std::cout << globalVariableName << " : " << globalVariableType << "\n";
                    }
                }
            }

            if (debug_ir_validator) {
                std::cout << "\n ---- Listing functions with their signature ---- \n";
            }
            std::vector< std::string > functionsNames = {};
            std::vector< std::vector< Variable* > > functionsArguments = {};
            std::vector< Type* > functionReturnTypes = {};
            int numFunctions = 0;
            for (auto function: unit.functions()){
                auto functionFromName = unit.findFunction(function->name());
                assert(functionFromName != nullptr && "Function from name is null.");
                assert(function == functionFromName && "Function and function from name are not the same." );
                functionsNames.push_back(function->name());
                functionsArguments.push_back(function->params());
                functionReturnTypes.push_back(function->returnType());
                numFunctions++;
            }
            for(int i = 0; i < numFunctions; i++){
                auto funcName = functionsNames[i];
                if (debug_ir_validator) {
                    std::cout << functionsNames[i] << " : (";
                }
                auto itr1 = functionsArguments[i].begin();
                for( ; itr1 != functionsArguments[i].end(); ++itr1){
                    if ((*itr1)->hasName()){
                        auto argName = (*itr1)->name();
                        if (debug_ir_validator){
                            std::cout << argName << " : ";
                        }
                    }
                    auto arg_type_name = (*itr1)->type()->name();
                    if (debug_ir_validator) {
                        std::cout << arg_type_name << ", ";
                    }
                }
                auto ret_type = functionReturnTypes[i]->name();
                if (debug_ir_validator) {
                    std::cout << ") -> " << ret_type << "\n";
                }
            }

            if(debug_ir_validator) {
                std::cout << "---- Listing all Instructions with their types ---- \n";
            }
            for(auto fn : unit.functions()) {
                if (debug_ir_validator) {
                    std::cout << fn->name() << ":\n";
                }
                auto instructionsList = fn->instructions();
                if(!instructionsList.empty()) {
                    for (auto instruction: instructionsList) {
                        std::string instString = getInstructionString(*instruction);
                        if (debug_ir_validator) {
                            std::cout << instString << "\n";
                        }
                    }
                }
            }
        }

        DetectorReport *report() override {
            if (debug_ir_validator) {
                std::cout << "Module Name: " << moduleName << "\n";
                std::cout << "Source File Name: " << sourceFileName << "\n";
                std::cout << "Statistics:\n";
                std::cout << "# Functions: " << totFns << "\n";
                std::cout << "# Basic Blocks: " << totBlks << "\n";
                std::cout << "# Instructions: " << totIns << "\n";
            }
            std::cout << "IR validated successfully! \n";
        }

        static std::string name() {
            return "irValidator";
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

        bool debug_ir_validator = false;
        int totFns = 0;
        int totIns = 0;
        int totBlks = 0;
        std::string moduleName;
        std::string sourceFileName;
    };
}



#endif //VANGUARD_IRVALIDATOR_H
