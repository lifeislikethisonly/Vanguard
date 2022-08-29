//
// Created by Jon Stephens on 3/20/22.
//

#include "../include/BlkFunction.h"

#include <utility>
#include "../include/BlockchainModel.h"

using namespace std;

namespace blockchain {
    BlkFunction::BlkFunction(BlockchainModel *blk2llvm, std::string &name, bool isCnstr, Visibility visibility, Mutability mutability, vector<BlkVariable *> params,
                             vector<BlkVariable *> rets, vector<string> mods) : fnSelector(""), fnIsConstructor(isCnstr), visible(visibility), mut(mutability), fnParams(std::move(params)), fnReturns(std::move(rets)), fnMods(std::move(mods)), BlkNode(FUNCTION, blk2llvm, name) {
        registerParent(fnParams);
        registerParent(fnReturns);
    }

    BlkFunction::~BlkFunction() {
        deleter(fnParams);
        deleter(fnReturns);
    }

    void BlkFunction::setSelector(string selector) {
        if(fnSelector.empty()) {
            fnSelector = selector;
        }
    }

    string BlkFunction::selector() {
        return fnSelector;
    }

    Visibility BlkFunction::visibility() const {
        return visible;
    }

    Mutability BlkFunction::mutability() const {
        return mut;
    }

    bool BlkFunction::isConstructor() const {
        return fnIsConstructor;
    }

    const vector<BlkVariable *> &BlkFunction::parameters() const {
        return fnParams;
    }

    const vector<std::string> &BlkFunction::modifiers() const {
        return fnMods;
    }

    bool BlkFunction::isTranslation(vanguard::Function &fn) const {
        return blkTollvm->isTranslation(*this, fn);
    }

    Visibility BlkFunction::toVisibility(const string& visStr) {
        if(visStr == "external") { return EXTERNAL; }
        if(visStr == "public") { return PUBLIC; }
        if(visStr == "internal") { return INTERNAL; }
        if(visStr == "private") { return PRIVATE; }
        if(visStr == "default") { return DEFAULT; }

        throw runtime_error("Unknown visibility " + visStr);
    }

    Mutability BlkFunction::toMutability(const string& mutStr) {
        if(mutStr == "pure") { return PURE; }
        if(mutStr == "view") { return VIEW; }
        if(mutStr == "payable") { return PAYABLE; }
        if(mutStr == "nonpayable") { return NONPAYABLE; }

        throw runtime_error("Unknown mutability " + mutStr);
    }
}