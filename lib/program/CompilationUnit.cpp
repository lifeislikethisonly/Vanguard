#include "Universe.h"
#include "UnitFactory.h"

namespace vanguard{

    Universe::CompilationUnit::CompilationUnit(UnitFactory &factory, const llvm::Module* mod): factory(factory), module(mod){}

    std::string Universe::CompilationUnit::name(){
        return module->getModuleIdentifier();
    }

    std::string Universe::CompilationUnit::sourceFile(){
        return module->getSourceFileName();
    }

    Universe::Function* Universe::CompilationUnit::findFn(std::string name) const {
        for (auto fn : this->fns()){
            if (fn->name() == name) return fn;
        }
        return nullptr;
    }

    Value* Universe::CompilationUnit::findGlobalVariable(std::string name) {
        //auto &llvmToVanguard = LLVMtoVanguard::getInstance();
        auto globalVariable = module->getGlobalVariable(llvm::StringRef(name), true);
        if (globalVariable == nullptr){
            return nullptr;
        }
        return factory.createVal(globalVariable);
    }

    std::list<Universe::Function*> Universe::CompilationUnit::fns() const {
        //auto &llvmToVanguard = LLVMtoVanguard::getInstance();
        std::list<Function*> allFunctions = {};
        for(auto &F: *module){
            allFunctions.push_back(factory.createFn(&F));
        }
        return allFunctions;
    }

    std::list<Value*> Universe::CompilationUnit::globalVariables(){
        //auto &llvmToVanguard = LLVMtoVanguard::getInstance();
        std::list<Value*> allGlobalVariables = {};
        for (auto gv= module->global_begin(); gv != module->global_end(); gv++){
            allGlobalVariables.push_back(factory.createVal(&*gv));
        }
        return allGlobalVariables;
    }

    const llvm::Module* Universe::CompilationUnit::unwrap() {
        return module;
    }

}