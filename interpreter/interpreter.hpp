//
// Created by XIaokang00010 on 2022/12/3.
//

#ifndef REXSCRIPT_INTERPRETER_HPP
#define REXSCRIPT_INTERPRETER_HPP

#include "share/share.hpp"
#include "value.hpp"
#include <thread>

namespace rex {

    struct environment {
        struct stackFrame {
            managedPtr<value> moduleCxt;
            vec<value::cxtObject> localCxt;

            void pushLocalCxt(const value::cxtObject &cxt);

            void popLocalCxt();

            vsize getCurLocalCxtIdx();

            void backToLocalCxt(vsize idx);

            stackFrame();

            stackFrame(managedPtr<value> &moduleCxt, const vec<value::cxtObject> &localCxt);
        };

        class thread {
            managedPtr<std::thread> th;
            managedPtr<value> result;
        public:
            thread();

            void setTh(const managedPtr<std::thread> &v);

            const managedPtr<std::thread> &getTh();

            void setResult(const managedPtr<value> &v);

            const managedPtr<value> & getResult();
        };

        managedPtr<value> globalCxt;

        map<vint, thread> threadPool;

        vint threadIdCounter;
    };

    class interpreter {
    public:
        vec<environment::stackFrame> stack;
        managedPtr<value> moduleCxt;
        managedPtr<environment> env;

        vsize getCurStackIdx();

        void backToStackIdx(vsize stkIdx);

        interpreter(const managedPtr<environment> &env, const managedPtr<value> &moduleCxt);

        static value makeErr(const vstr &errName, const vstr &errMsg);

        value invokeFunc(managedPtr<value> func, const vec<value> &args, const managedPtr<value> &passThisPtr);

        value opAdd(value &a, value &b);

        value opSub(value &a, value &b);

        value opMul(value &a, value &b);

        value opDiv(value &a, value &b);

        value opMod(value &a, value &b);

        value opBinaryShiftLeft(value &a, value &b);

        value opBinaryShiftRight(value &a, value &b);

        value opEqual(value &a, value &b);

        value opNotEqual(value &a, value &b);

        value opGreaterEqual(value &a, value &b);

        value opLessEqual(value &a, value &b);

        value opGreaterThan(value &a, value &b);

        value opLessThan(value &a, value &b);

        value opBinaryOr(value &a, value &b);

        value opBinaryAnd(value &a, value &b);

        value opBinaryXor(value &a, value &b);

        value opLogicAnd(value &a, value &b);

        value opLogicOr(value &a, value &b);

        value interpretLvalueExpressions(const AST &target);

        value interpretAssignments(const AST &target);

        // 整合解释器和符号查找器
        // 对于Rvalue -> 返回ref形式
        // 对于Lvalue -> 返回value形式
        value interpret(const AST &target);
    };

    void
    rexThreadWrapper(const managedPtr<environment> &env, vint tid, const managedPtr<value> &cxt, const managedPtr<value> &func,
                     const vec<value> &args,
                     const managedPtr<value>& passThisPtr = nullptr);

    vint spawnThread(const managedPtr<environment> &env, const managedPtr<value> &cxt, const managedPtr<value> &func,
                     const vec<value> &args, const managedPtr<value>& passThisPtr = nullptr);

    value waitForThread(const managedPtr<environment> &env, vint id);
} // rex

#endif //REXSCRIPT_INTERPRETER_HPP
