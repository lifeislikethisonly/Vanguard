//
// Created by Jon Stephens on 3/23/22.
//

#include "SolangModel.h"
#include "llvm/IR/Instructions.h"
#include <regex>
#include <sstream>
#include <stdexcept>
#include <program/InstructionClasses.h>
using namespace std;

namespace vanguard {
    class TypeTrans : public TypeVisitor {
    public:
        string result;
        void visit(const ArrayType &t) override {
            t.baseType()->accept(*this);
            result += ":";
        }

        void visit(const MapType &t) override {
            std::string oldResult = result;
            result = "";
            t.keyType()->accept(*this);
            string keyType = result;
            result = "";
            t.valueType()->accept(*this);
            string valType = result;

            stringstream namestream;
            namestream << oldResult
                       << "mapping:"
                       << keyType << ":"
                       << valType;
            result = namestream.str();
        }

        void visit(const ObjectType &t) override {
            result = t.name().substr(t.name().find_last_of(' ') + 1, t.name().length());
        }

        void commonVisit(const Type &t) override {
            result = t.name();
        }
    };

    string getTypeString(const Type *type) {
        TypeTrans trans;
        type->accept(trans);
        return trans.result;
    }

    bool SolangModel::isImplementation(std::string contract, const Top<Blockchain<Universe>>::Function &blockchainFn, const llvm::Function &llvmFn) const {
        if(!llvmFn.hasName()) {
            return false;
        }

        stringstream ss;
        ss << ".*" << contract << "::";

        if(blockchainFn.isConstructor()) {
            ss << blockchainFn.name() << ".*";
        }
        else {
            //ss << "function::" << blockchainFn.name() ;
            ss << ".*" << blockchainFn.name() ;
            if(!blockchainFn.params().empty() || !blockchainFn.modifiers().empty()) {
                ss << "__";
            }

            for(auto var : blockchainFn.params()) {
                ss << getTypeString(var->type()) << ".*";
            }

            for(auto &modifier : blockchainFn.modifiers()) {
                ss << modifier << ".*";
            }

            ss << "$";
        }

        std::regex reg(ss.str());
        return regex_match(llvmFn.getName().str(), reg);
    }

    bool SolangModel::isAnyLowLevelCall(CallExpr<Top<Blockchain<Universe>>> &call)  {
        return isLowLevelCall(call) || isLowLevelStaticCall(call) || isLowLevelDelegateCall(call);
    }

    bool SolangModel::isLowLevelCall(CallExpr<Top<Blockchain<Universe>>> &call)  {
        for(auto tgt : call.targets()) {
            if(tgt->name() == "call") {
                return true;
            }
        }

        return false;
    }

    bool SolangModel::isLowLevelStaticCall(CallExpr<Top<Blockchain<Universe>>> &call)  {
        for(auto tgt : call.targets()) {
            if(tgt->name() == "callStatic") {
                return true;
            }
        }

        return false;
    }

    bool SolangModel::isLowLevelDelegateCall(CallExpr<Top<Blockchain<Universe>>> &call)  {
        for(auto tgt : call.targets()) {
            if(tgt->name() == "callDelegate") {
                return true;
            }
        }

        return false;
    }

    class CallTargetResolver : public InstructionClassVisitor<Top<Blockchain<Universe>>> {
    public:
        bool isCall = false;
        std::vector<Top<Blockchain<Universe>>::Function *> tgts;
        void visit(const CallExpr<Top<Blockchain<Universe>>> &v) override{
            isCall = true;
            tgts = v.targets();
        }
    };

    bool SolangModel::writesStorage(Top<Blockchain<Universe>>::Instruction &ins) {
        CallTargetResolver tgtResolver;
        ins.accept(tgtResolver);

        if(!tgtResolver.isCall) {
            return false;
        }

        stringstream ss;
        ss << ".*::function::" << "v?__set_.*";

        /*for(auto tgt : tgtResolver.tgts) {
            string fnName = tgt->name();
            std::regex reg(ss.str());
            if(regex_match(fnName, reg)) {
                return true;
            }
        }*/

        return std::any_of(tgtResolver.tgts.begin(), tgtResolver.tgts.end(), [&ss](auto fn){std::regex reg(ss.str()); return regex_match(fn->name(), reg);});
    }

    bool SolangModel::readsStorage(Top<Blockchain<Universe>>::Instruction &ins) {
        CallTargetResolver tgtResolver;
        ins.accept(tgtResolver);

        if(!tgtResolver.isCall) {
            return false;
        }

        stringstream ss;
        ss << ".*::function::" << "v?__get_.*";

        return std::any_of(tgtResolver.tgts.begin(), tgtResolver.tgts.end(), [&ss](auto fn){std::regex reg(ss.str()); return regex_match(fn->name(), reg);});
    }

    CallResolver<Top<Blockchain<Universe>>> *SolangModel::callResolver() {
        return nullptr;
    }

    /*class TypeTrans : public BlkTypeVisitor {
    public:
        string result;
        void visit(const BlkArrayType &t) override {
            t.base().accept(*this);
            result += ":";
        }

        void visit(const BlkMapType &t) override {
            t.key().accept(*this);
            string keyType = result;
            result = "";
            t.value().accept(*this);
            string valType = result;

            stringstream namestream;
            namestream << "mapping:"
                       << keyType << ":"
                       << valType;
            result = namestream.str();
        }

        void visit(const BlkUserType &t) override {
            result = t.blkName().substr(t.blkName().find_last_of(' ') + 1, t.blkName().length());
        }

        void visit(const BlkElementaryType &t) override {
            result = t.blkName();
        }
    };


    string getTypeString(const BlkType &type) {
        TypeTrans trans;
        type.accept(trans);
        return trans.result;
    }

    bool SolangModel::isTranslation(const BlkFunction &blockchainFn, vanguard::Function &fn) const {
        const llvm::Function& llvmFn = fn.unwrap();
        if(!llvmFn.hasName()) {
            return false;
        }

        stringstream ss;
        ss << ".*" << blockchainFn.parent()->blkName() << "::";

        if(blockchainFn.isConstructor()) {
            ss << blockchainFn.blkName() << ".*";
        }
        else {
            //ss << "function::" << blockchainFn.name() ;
            ss << ".*" << blockchainFn.blkName() ;
            if(!blockchainFn.parameters().empty() || !blockchainFn.modifiers().empty()) {
                ss << "__";
            }

            for(auto var : blockchainFn.parameters()) {
                ss << getTypeString(var->varType()) << ".*";
            }

            for(auto &modifier : blockchainFn.modifiers()) {
                ss << modifier << ".*";
            }

            ss << "$";
        }

        std::regex reg(ss.str());
        return regex_match(llvmFn.getName().str(), reg);
    }

    bool SolangModel::isAnyExternalCall(vanguard::Function &fn) const {
        return isCall(fn) || isStaticCall(fn) || isDelegateCall(fn);
    }

    bool SolangModel::isCall(vanguard::Function &fn) const {
        const llvm::Function& llvmFn = fn.unwrap();
        if(!llvmFn.hasName()) {
            return false;
        }

        string name = llvmFn.getName().str();
        return name == "call";
    }

    bool SolangModel::isStaticCall(vanguard::Function &fn) const {
        const llvm::Function& llvmFn = fn.unwrap();
        if(!llvmFn.hasName()) {
            return false;
        }

        string name = llvmFn.getName().str();
        return name == "callStatic";
    }

    bool SolangModel::isDelegateCall(vanguard::Function &fn) const {
        const llvm::Function& llvmFn = fn.unwrap();
        if(!llvmFn.hasName()) {
            return false;
        }

        string name = llvmFn.getName().str();
        return name == "callDelegate";
    }

    bool SolangModel::writesVariable(const BlkVariable &var, vanguard::Instruction &ins) const {
        const llvm::Instruction& llvmIns = ins.unwrap();
        if(auto call = llvm::dyn_cast<CallInst>(&llvmIns)) {
            auto fn = call->getCalledFunction();
            stringstream ss;
            ss << ".*::function::v?__set_" << var.blkName() << ".*";
            std::regex reg(ss.str());

            if(fn->hasName() && regex_match(fn->getName().str(), reg)) {
                return true;
            }
        }

        return false;
    }

    bool SolangModel::readsVariable(const BlkVariable &var, vanguard::Instruction &ins) const {
        const llvm::Instruction& llvmIns = ins.unwrap();
        if(auto call = llvm::dyn_cast<CallInst>(&llvmIns)) {
            auto fn = call->getCalledFunction();
            stringstream ss;
            ss << ".*::function::v?__get_" << var.blkName() << ".*";
            std::regex reg(ss.str());

            if(fn->hasName() && regex_match(fn->getName().str(), reg)) {
                return true;
            }
        }

        return false;
    }

    bool SolangModel::writesStorage(vanguard::Function &fn) {
        const llvm::Function& llvmFn = fn.unwrap();
        if(!llvmFn.hasName()) {
            return false;
        }

        string fnName = llvmFn.getName().str();

        stringstream ss;
        ss << ".*::function::" << "v?__set_.*";

        std::regex reg(ss.str());
        return regex_match(fnName, reg);
    }

    bool SolangModel::readsStorage(vanguard::Function &fn) {
        const llvm::Function& llvmFn = fn.unwrap();
        if(!llvmFn.hasName()) {
            return false;
        }

        string fnName = llvmFn.getName().str();

        stringstream ss;
        ss << ".*::function::" << "v?__get_.*";

        std::regex reg(ss.str());
        return regex_match(fnName, reg);
    }*/
}