//
// Created by XIaokang00010 on 2022/12/3.
//

#ifndef REXSCRIPT_INTERPRETER_HPP
#define REXSCRIPT_INTERPRETER_HPP

#include "value.hpp"

namespace rex {

    struct environment {
        struct stackFrame {
            managedPtr<value> moduleCxt;
            vec<value::cxtObject> localCxt;

            void pushLocalCxt(const value::cxtObject &cxt);

            void popLocalCxt();

            stackFrame();

            stackFrame(managedPtr<value> &moduleCxt, const vec<value::cxtObject> &localCxt);
        };

        managedPtr<value> globalCxt;
    };

    class interpreter {
        vec<environment::stackFrame> stack;
        managedPtr<value> moduleCxt;
        managedPtr<environment> env;
    public:
        interpreter(const managedPtr<environment> &env, const managedPtr<value> &moduleCxt);

        static value makeErr(const vstr &errName, const vstr &errMsg);

        value invokeFunc(managedPtr<value> func, const vec<value> &args, const managedPtr<value>& passThisPtr);

        value interpretLvalueExpressions(const AST &target);

        // 整合解释器和符号查找器
        // 对于Rvalue -> 返回ref形式
        // 对于Lvalue -> 返回value形式
        value interpret(const AST &target);
    };

} // rex

#endif //REXSCRIPT_INTERPRETER_HPP
