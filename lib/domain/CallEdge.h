#ifndef VANGUARD_DOMAIN_CALLEDGE_H
#define VANGUARD_DOMAIN_CALLEDGE_H

#include "../program/Function.cpp"

namespace vanguard {
    class CallEdge {
        Universe::Function* caller();
        Universe::Function* callee();
    };
}

#endif
