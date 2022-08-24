#include "IRValidator.h"
#include <unordered_set>
#include <vector>
#include <cassert>
#include <iostream>
#include "../../program/InstructionClasses.h"


namespace vanguard {
    void IRValidator::registerAnalyses() { }
    void IRValidator::startDetection() {
        totBlks = 0;
        totFns = 0;
        totIns = 0;
    }

    std::string IRValidator::name() {
        return "irValidator";
    }

    void IRValidator::countBody(Block *blk) {
        assert(blk != nullptr && "Function does not have a body");
        assert(blk->isEntryBlock() && "Fn body not marked as entry block");

        std::unordered_set<Block *> seen = { blk };
        std::vector<Block *> worklist = { blk };

        while(!worklist.empty()) {
            Block *curBlk = worklist.back();
            worklist.pop_back();

            totBlks++;

            for(auto ins : curBlk->getInstructionsList()) {
                assert(curBlk == ins->getBlock() && "Block does not match instruction block");
                totIns++;
            }

            for(auto next : curBlk->getAllSuccessors()) {
                if(next != nullptr && seen.find(next) == seen.end()) {
                    seen.insert(next);
                    worklist.push_back(next);
                }
            }
        }
    }

    void IRValidator::detect(CompilationUnit &unit) {
        moduleName = unit.getModuleName();
        sourceFileName = unit.getSourceFileName();

        for(auto fn : unit.getAllFunctions()) {
            totFns++;
            if(fn->hasBody()) {
                auto body = fn->getBody();
                assert(body->getFunction() == fn && "Function does not match block function");
                assert(body->isEntryBlock() == true && "Not an entry block.");
                countBody(body);
            }
        }

        std::cout << "\n ---- Listing global variables with their types ---- \n";
        for (auto globalVariableValue: unit.getAllGlobalVariables()){
            auto globalVariable = llvm::dyn_cast<GlobalVariable>(globalVariableValue);
            if (globalVariable->hasName()) {
                auto globalVariableFromName = (GlobalVariable *)unit.getGlobalVariable(globalVariable->getName());
                assert(globalVariableFromName == globalVariable && "globalVariable and globalVariableFromName ");
                assert(globalVariableFromName != nullptr && "Global variable from Name is null.");
                std::cout << globalVariableFromName->getName() << " : " << (globalVariableFromName->getType())->getName() << "\n";
            }
        }

        std::cout << "\n ---- Listing functions with their signature ---- \n";
        std::vector< std::string > functionsNames = {};
        std::vector< std::list< Argument* > > functionsArguments = {};
        std::vector< std::list< Type* > > functionsArgumentTypes = {};
        std::vector< Type* > functionReturnTypes = {};
        int numFunctions = 0;
        for (auto function: unit.getAllFunctions()){
            auto functionFromName = unit.getFunction(function->getName());
            assert(functionFromName != nullptr && "Function from name is null.");
            assert(function == functionFromName && "Function and function from name are not the same." );
            functionsNames.push_back(function->getName());
            functionsArguments.push_back(function->getParams());
            functionsArgumentTypes.push_back(function->getParamTypes());
            functionReturnTypes.push_back(function->getReturnType());
            numFunctions++;
        }
        for(int i = 0; i < numFunctions; i++){
            std::cout << functionsNames[i] << " : (";
            auto itr1 = functionsArguments[i].begin();
            auto itr2 = functionsArgumentTypes[i].begin();
            for( ; itr1 != functionsArguments[i].end() && itr2 != functionsArgumentTypes[i].end(); ++itr1, ++itr2){
                if ((*itr1)->hasName()){
                    std::cout << (*itr1)->getName() << " : ";
                }
                std::cout << (*itr2)->getName() << ", ";
            }
            std::cout << ") -> " << functionReturnTypes[i]->getName() << "\n";
        }


        std::cout << "---- Listing all Instructions with their types ---- \n";
        for(auto fn : unit.getAllFunctions()) {
            std::cout << fn->getName() << ":\n";
            std::list<Instruction*> instructionsList = fn->getInstructionsList();
            if(!instructionsList.empty()) {
                for (auto instruction: instructionsList) {
                    std::cout << instruction->getName() << " : " << instruction->getInstructionType() << "\n";
                }
            }
        }
    }

    void IRValidator::report() {
        std::cout << "Module Name: " << moduleName << "\n";
        std::cout << "Source File Name: " << sourceFileName << "\n";
        std::cout << "Statistics:\n";
        std::cout << "# Functions: " << totFns << "\n";
        std::cout << "# Basic Blocks: " << totBlks << "\n";
        std::cout << "# Instructions: " << totIns << "\n";
    }
}