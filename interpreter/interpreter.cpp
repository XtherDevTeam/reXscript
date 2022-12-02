//
// Created by XIaokang00010 on 2022/11/30.
//

#include "interpreter.hpp"

namespace rex {
    environment::stackFrame::stackFrame() : globalContext(nullptr), localContext() {}

    environment::stackFrame::stackFrame(heap::vItem *globCxt) : globalContext(globCxt), localContext() {}

    environment::stackFrame::stackFrame(heap::vItem *globCxt, const map<vstr, value> &initLocalCxt) :
        globalContext(globCxt), localContext((vec<map<vstr, value>>){initLocalCxt}) {}

    void environment::stackFrame::pushLocalContext(const map<vstr, value> &initLocalCxt) {
        localContext.push_back(initLocalCxt);
    }
    
    void environment::stackFrame::popLocalContext() {
        localContext.pop_back();
    }

    interpreter::interpreter() : env(nullptr) {}

    interpreter::interpreter(environment *env) : env(env) {}
}