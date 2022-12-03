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

    value interpreter::interpret(const AST &) {
        return value();
    }

    value interpreter::invokeFunc(value *val, const vec<value> &args, value *passThisPtr) {
        return value();
    }

    value *interpreter::getRvalue(const AST &target) {
        switch (target.kind) {
            case AST::treeKind::identifier: {

            }
            case AST::treeKind::invokingExpression: {

            }
            case AST::treeKind::subscriptExpression: {

            }
            case AST::treeKind::memberExpression: {

            }
            default: {

            }
        }
    }

    value interpreter::makeErr(const vstr &errName, const vstr &errMsg) {
        value res{value::vKind::vObject, (unsafePtr<unknownPtr>)nullptr};
        res[L"errName"] = (value){value::vKind::vStr, env->envHeap.createStr(errName)};
        res[L"errMsg"] = (value){value::vKind::vStr, env->envHeap.createStr(errMsg)};
        return res;
    }
}