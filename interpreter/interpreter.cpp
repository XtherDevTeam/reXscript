//
// Created by XIaokang00010 on 2022/12/3.
//

#include <iostream>
#include <utility>
#include "interpreter.hpp"
#include "exceptions/signalReturn.hpp"
#include "builtInMethods.hpp"
#include "exceptions/signalException.hpp"
#include "exceptions/signalContinue.hpp"
#include "exceptions/signalBreak.hpp"
#include "interpreter/value.hpp"
#include "share/share.hpp"
#include "exceptions/rexException.hpp"
#include "exceptions/importError.hpp"

namespace rex {
    managedPtr<environment> rexEnvironmentInstance;
    managedPtr<interpreter> rexInterpreterInstance;

    void environment::stackFrame::pushLocalCxt(const value::cxtObject &cxt) {
        localCxt.push_back(cxt);
    }

    void environment::stackFrame::popLocalCxt() {
        localCxt.pop_back();
    }

    environment::stackFrame::stackFrame(runtimeSourceFileMsg msg) : sourceMsg(std::move(msg)), moduleCxt(nullptr),
                                                                    localCxt() {

    }

    environment::stackFrame::stackFrame(runtimeSourceFileMsg msg, managedPtr<value> &moduleCxt,
                                        const vec<value::cxtObject> &localCxt) :
            sourceMsg(std::move(msg)), moduleCxt(moduleCxt), localCxt(localCxt) {

    }

    vsize environment::stackFrame::getCurLocalCxtIdx() {
        return localCxt.size();
    }

    void environment::stackFrame::backToLocalCxt(vsize idx) {
        while (localCxt.size() > idx)
            localCxt.pop_back();
    }

    environment::stackFrame::operator vstr() {
        return L"stack frame at " + (vstr) sourceMsg;
    }

    environment::thread::thread() : th(), result() {
    }

    void environment::thread::setTh(const managedPtr<std::thread> &v) {
        th = v;
    }

    void environment::thread::setResult(const managedPtr<value> &v) {
        result = v;
    }

    const managedPtr<std::thread> &environment::thread::getTh() {
        return th;
    }

    const managedPtr<value> &environment::thread::getResult() {
        return result;
    }

    interpreter::interpreter(const managedPtr<environment> &env, const managedPtr<value> &moduleCxt) :
            env(env), moduleCxt(moduleCxt), stack(), interpreterCxt() {

    }

    value
    interpreter::invokeFunc(managedPtr<value> func, const vec<value> &args, const managedPtr<value> &passThisPtr) {
        if (func->isRef())
            func = func->refObj;
        try {
            switch (func->kind) {
                case value::vKind::vFunc: {
                    stack.push_back({env->dumpRuntimeSourceFileMsg(func->getFunc()), func->getFunc().moduleCxt, {}});
                    stack.back().pushLocalCxt({});

                    if (passThisPtr)
                        stack.back().localCxt.back()[L"this"] = passThisPtr;

                    for (vsize i = 0; i < args.size(); i++) {
                        if (i < args.size()) {
                            stack.back().localCxt.back()[func->getFunc().argsName[i]] = managePtr(args[i]);
                        } else {
                            stack.back().localCxt.back()[func->getFunc().argsName[i]] = managePtr(value{});
                        }
                    }
                    interpret(func->funcObj->code);
                    stack.pop_back();
                    return {};
                }
                case value::vKind::vLambda: {
                    stack.push_back(
                            {env->dumpRuntimeSourceFileMsg(func->getLambda()), func->getLambda().func.moduleCxt, {}});
                    stack.back().pushLocalCxt({});

                    stack.back().localCxt.back()[L"outer"] = func->getLambda().outerCxt;

                    if (passThisPtr)
                        stack.back().localCxt.back()[L"this"] = passThisPtr;

                    for (vsize i = 0; i < func->getLambda().func.argsName.size(); i++) {
                        if (i < args.size()) {
                            stack.back().localCxt.back()[func->getLambda().func.argsName[i]] = managePtr(args[i]);
                        } else {
                            stack.back().localCxt.back()[func->getLambda().func.argsName[i]] = managePtr(value{});
                        }
                    }
                    interpret(func->getLambda().func.code);
                    stack.pop_back();
                    return {};
                }
                case value::vKind::vNativeFuncPtr: {
                    return (*func->nativeFuncObj)((void *) {this}, args, passThisPtr);
                }
                default: {
                    if (auto it = func->members.find(L"rexInvoke"); it != func->members.end()) {
                        return invokeFunc(it->second, args, func);
                    } else {
                        throw signalException(makeErr(L"internalError", L"not a invoke-able object"));
                    }
                }
            }
        } catch (signalReturn &e) {
            stack.pop_back();
            return e.get();
        }
    }

    value interpreter::makeErr(const vstr &errName, const vstr &errMsg) {
        value res{(value::cxtObject) {}};
        res.members[L"errName"] = managePtr(value{errName, stringMethods::getMethodsCxt()});
        res.members[L"errMsg"] = managePtr(value{errMsg, stringMethods::getMethodsCxt()}); // add string initialization
        return res;
    }

    value interpreter::interpret(const AST &target) {
        switch (target.kind) {
            case AST::treeKind::basicLiterals: {
                switch (target.leaf.kind) {
                    case lexer::token::tokenKind::integer:
                        return {target.leaf.basicVal.vInt};
                    case lexer::token::tokenKind::decimal:
                        return {target.leaf.basicVal.vDeci};
                    case lexer::token::tokenKind::boolean:
                        return {target.leaf.basicVal.vBool};
                    case lexer::token::tokenKind::string:
                        return {target.leaf.strVal, stringMethods::getMethodsCxt()};
                    case lexer::token::tokenKind::kNull:
                    default:
                        return {};
                }
            }
            case AST::treeKind::listLiteral: {
                value::vecObject obj{};
                for (auto &i: target.child) {
                    obj.push_back(managePtr(interpret(i)));
                }
                return {obj, rex::vecMethods::getMethodsCxt()};
            }
            case AST::treeKind::objectLiteral: {
                value::cxtObject cxt;
                value r;
                for (auto &i: target.child) {
                    r = interpret(i.child[1]);
                    cxt[i.child[0].leaf.strVal] = managePtr(r.isRef() ? r.getRef() : r);
                }
                return {cxt};
            }
            case AST::treeKind::identifier: {
                auto &curModCxt = stack.empty() ? moduleCxt : stack.back().moduleCxt;
                // 优先从栈中查找
                if (!stack.empty()) {
                    for (auto it = stack.back().localCxt.rbegin();
                         it != stack.back().localCxt.rend(); it++) {
                        if (auto vit = it->find(target.leaf.strVal); vit != it->end()) {
                            return {vit->second->isRef() ? vit->second->refObj : vit->second};
                        }
                    }
                }
                // 全局范围解释
                if (auto vit = interpreterCxt.find(target.leaf.strVal);
                        vit != interpreterCxt.end()) {
                    // 在当前Module Context中查找
                    return {vit->second->isRef() ? vit->second->refObj : vit->second};
                }
                if (auto vit = curModCxt->members.find(target.leaf.strVal);
                        vit != curModCxt->members.end()) {
                    // 在当前Module Context中查找
                    return {vit->second->isRef() ? vit->second->refObj : vit->second};
                }
                if (auto vit = env->globalCxt->members.find(target.leaf.strVal);
                        vit != env->globalCxt->members.end()) {
                    // 在Global Context中查找
                    return {vit->second->isRef() ? vit->second->refObj : vit->second};
                }
                // 符号不存在
                throw signalException(makeErr(L"internalError", (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": undefined symbol: `" + target.leaf.strVal + L"`"));
            }
            case AST::treeKind::subscriptExpression: {
                // 处理最左下的subscript，在memberExpression右端的交给memberExpression处理分支
                value val = interpret(target.child[0]);
                value &l = val.isRef() ? val.getRef() : val;
                value r = interpret(target.child[1]);
                if (r.isRef())
                    getSelfRef(r);
                switch (l.kind) {
                    case value::vKind::vStr:
                        return {(vint) {(*l.strObj)[r.getInt()]}};
                    case value::vKind::vVec: {
                        return {(*l.vecObj)[r.getInt()]->isRef() ? (*l.vecObj)[r.getInt()]->refObj
                                                                 : (*l.vecObj)[r.getInt()]};
                    }
                    default: {
                        if (auto it = l.members.find(L"rexIndex"); it != l.members.end())
                            return invokeFunc(it->second, {r}, val.isRef() ? val.refObj : managePtr(l));
                        if (r.kind == value::vKind::vStr)
                            return l[r.getStr()];
                        else
                            throw signalException(makeErr(L"internalError", (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": not a subscript-able object"));
                    }
                }
            }
            case AST::treeKind::invokingExpression: {
                // 同理 在memberExpression树右端的同类节点在memberExpression分支处理
                value val = interpret(target.child[0]);
                value &l = val.isRef() ? val.getRef() : val;
                vec<value> args;
                for (auto &i: target.child[1].child) {
                    args.push_back(interpret(i));
                }
                return invokeFunc(managePtr(l), args, nullptr);
            }
            case AST::treeKind::lambdaDefinition: {
                value::lambdaObject lambda;
                lambda.outerCxt = managePtr(value{value::cxtObject{}});
                for (auto &i: target.child[0].child) {
                    value it = interpret(i);
                    if (it.isRef())
                        getSelfRef(it);
                    lambda.outerCxt->members[i.leaf.strVal] = managePtr(it);
                }
                for (auto &i: target.child[1].child) {
                    lambda.func.argsName.push_back(i.leaf.strVal);
                }
                lambda.func.code = target.child[2];
                lambda.func.moduleCxt = stack.empty() ? moduleCxt : stack.back().moduleCxt;
                return {lambda};
            }
            case AST::treeKind::functionDefinition: {
                value::funcObject func;
                for (auto &i: target.child[0].child) {
                    func.argsName.push_back(i.leaf.strVal);
                }
                func.code = target.child[1];
                func.moduleCxt = stack.empty() ? moduleCxt : stack.back().moduleCxt;
                return {func};
            }
            case AST::treeKind::functionDefStmt: {
                value::funcObject func;
                for (auto &i: target.child[1].child) {
                    func.argsName.push_back(i.leaf.strVal);
                }
                func.code = target.child[2];
                func.moduleCxt = stack.empty() ? moduleCxt : stack.back().moduleCxt;
                if (!stack.empty()) {
                    stack.back().localCxt.back()[target.child[0].leaf.strVal] = managePtr(
                            value{func});
                } else if (moduleCxt) {
                    moduleCxt->members[target.child[0].leaf.strVal] = managePtr(value{func});
                } else {
                    env->globalCxt->members[target.child[0].leaf.strVal] = managePtr(
                            value{func});
                }
                return {};
            }
            case AST::treeKind::memberExpression: {
                value val = interpret(target.child[0]);
                value &l = val.isRef() ? val.getRef() : val;
                switch (target.child[1].kind) {
                    case AST::treeKind::identifier: {
                        if (auto it = l.members.find(target.child[1].leaf.strVal); it != l.members.end()) {
                            return {it->second};
                        } else {
                            throw signalException(
                                    makeErr(
                                            L"internalError",
                                            (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": undefined identifier: `" + target.child[1].leaf.strVal + L"`"));
                        }
                    }
                    case AST::treeKind::subscriptExpression: {
                        if (auto it = l.members.find(target.child[1].child[0].leaf.strVal);
                                it != l.members.end()) {
                            value &lhs = *it->second;
                            value r = interpret(target.child[1].child[1]);
                            if (r.isRef())
                                getSelfRef(r);

                            switch (lhs.kind) {
                                case value::vKind::vStr:
                                    return {(vint) {(*lhs.strObj)[r.getInt()]}};
                                case value::vKind::vVec:
                                    return {{(*lhs.vecObj)[r.getInt()]}};
                                default: {
                                    if (auto vit = lhs.members.find(L"rexIndex"); vit != lhs.members.end())
                                        return vit->second;
                                    if (r.kind == value::vKind::vStr)
                                        return lhs[r.getStr()];
                                    else
                                        throw signalException(
                                                makeErr(L"internalError", L"not a subscript-able object"));
                                }
                            }
                        } else {
                            throw signalException(
                                    makeErr(
                                            L"internalError",
                                            (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": undefined identifier: `" + target.child[1].child[0].leaf.strVal + L"`"));
                        }
                    }
                    case AST::treeKind::invokingExpression: {
                        if (auto it = l.members.find(target.child[1].child[0].leaf.strVal); it != l.members.end()) {
                            vec<value> args;
                            for (auto &i: target.child[1].child[1].child) {
                                args.push_back(interpret(i));
                            }
                            return invokeFunc(it->second, args,
                                              val.isRef() ? val.refObj : managePtr(val));
                        } else {
                            throw signalException(
                                    makeErr(
                                            L"internalError",
                                            (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": undefined identifier: `" + target.child[1].child[0].leaf.strVal + L"`"));
                        }
                    }
                    default: {
                        return {};
                    }
                }
            }
            case AST::treeKind::uniqueExpression: {
                value rhs = interpret(target.child[1]);
                value &r = eleGetRef(rhs);
                switch (target.child[0].leaf.kind) {
                    case lexer::token::tokenKind::minus: {
                        switch (r.kind) {
                            case value::vKind::vInt:
                                return {-r.getInt()};
                            case value::vKind::vDeci:
                                return {-r.getDeci()};
                            case value::vKind::vBool:
                                return {!r.getBool()};
                            default: {
                                if (auto it = r.members.find(L"rexNegate"); it != r.members.end())
                                    return invokeFunc(it->second, {}, rhs.isRef() ? rhs.refObj : managePtr(r));
                                else
                                    throw signalException(makeErr(L"internalError",
                                                                  (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": no overloaded callable objects for `rexNegate` operation"));
                            }
                        }
                        break;
                    }
                    case lexer::token::tokenKind::incrementSign: {
                        if (!rhs.isRef())
                            throw signalException(makeErr(L"internalError", (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": expected a referenced object"));

                        switch (r.kind) {
                            case value::vKind::vInt: {
                                rhs.refObj->getInt()++;
                                return rhs;
                            }
                            case value::vKind::vDeci: {
                                rhs.refObj->getDeci()++;
                                return rhs;
                            }
                            default: {
                                if (auto it = r.members.find(L"rexIncrement"); it != r.members.end())
                                    return invokeFunc(it->second, {}, rhs.isRef() ? r.refObj : managePtr(r));
                                else
                                    throw signalException(makeErr(L"internalError",
                                                                  (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": no overloaded callable objects for `rexIncrement` operation"));
                            }
                        }
                        break;
                    }
                    case lexer::token::tokenKind::decrementSign: {
                        if (!rhs.isRef())
                            throw signalException(makeErr(L"internalError", (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": expected a referenced object"));

                        switch (r.kind) {
                            case value::vKind::vInt: {
                                rhs.refObj->getInt()--;
                                return rhs;
                            }
                            case value::vKind::vDeci: {
                                rhs.refObj->getDeci()--;
                                return rhs;
                            }
                            default: {
                                if (auto it = r.members.find(L"rexDecrement"); it != r.members.end())
                                    return invokeFunc(it->second, {}, rhs.isRef() ? r.refObj : managePtr(r));
                                else
                                    throw signalException(makeErr(L"internalError",
                                                                  (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": no overloaded callable objects for `rexDecrement` operation"));
                            }
                        }
                    }
                    case lexer::token::tokenKind::asterisk: {
                        value dest;
                        if (auto it = r.members.find(L"rexClone"); it != r.members.end()) {
                            if (auto dst = invokeFunc(it->second, {},
                                                      rhs.isRef() ? rhs.refObj : managePtr(rhs)); dst.isRef())
                                dest = *dst.refObj;
                            else
                                dest = dst;
                        } else {
                            r.deepCopy(dest);
                        }
                        return dest;
                    }
                    case lexer::token::tokenKind::sharp: {
                        return eleRefObj(rhs);
                    }
                    default: {
                        return {};
                    }
                }
            }
            case AST::treeKind::multiplicationExpression:
            case AST::treeKind::additionExpression:
            case AST::treeKind::binaryShiftExpression:
            case AST::treeKind::logicEqualExpression:
            case AST::treeKind::binaryExpression:
            case AST::treeKind::logicAndExpression: {
                return interpretLvalueExpressions(target);
            }
            case AST::treeKind::assignmentExpression: {
                return interpretAssignments(target);
            }
            case AST::treeKind::blockStmt: {
                stack.back().pushLocalCxt({});
                for (auto &i: target.child) {
                    interpret(i);
                }
                stack.back().popLocalCxt();
                return {};
            }
            case AST::treeKind::letStmt: {
                for (auto &item: target.child) {
                    value rhs = interpret(item.child[1]);

                    if (!stack.empty()) {
                        stack.back().localCxt.back()[item.child[0].leaf.strVal] = managePtr(
                                rhs.isRef() ? rhs.getRef() : rhs);
                    } else if (moduleCxt) {
                        moduleCxt->members[item.child[0].leaf.strVal] = managePtr(rhs.isRef() ? rhs.getRef() : rhs);
                    } else {
                        env->globalCxt->members[item.child[0].leaf.strVal] = managePtr(
                                rhs.isRef() ? rhs.getRef() : rhs);
                    }
                }
                return {};
            }
            case AST::treeKind::whileStmt: {
                value expr = interpret(target.child[0]);
                if (expr.isRef())
                    getSelfRef(expr);

                auto cxtIdx = stack.back().getCurLocalCxtIdx();
                while (expr.getBool()) {
                    try {
                        interpret(target.child[1]);
                    } catch (const signalContinue &e) {
                        stack.back().backToLocalCxt(cxtIdx);
                    } catch (const signalBreak &e) {
                        stack.back().backToLocalCxt(cxtIdx);
                        break;
                    }

                    expr = interpret(target.child[0]);
                    if (expr.isRef())
                        getSelfRef(expr);
                }

                return {};
            }
            case AST::treeKind::forStmt: {
                stack.back().pushLocalCxt({});
                auto cxtIdx = stack.back().getCurLocalCxtIdx();

                interpret(target.child[0]);

                value expr = interpret(target.child[1]);
                if (expr.isRef())
                    getSelfRef(expr);

                while (expr.getBool()) {
                    try {
                        interpret(target.child[3]);
                    } catch (const signalContinue &e) {
                        stack.back().backToLocalCxt(cxtIdx);
                    } catch (const signalBreak &e) {
                        stack.back().backToLocalCxt(cxtIdx);
                        break;
                    }
                    interpret(target.child[2]);

                    expr = interpret(target.child[1]);
                    if (expr.isRef())
                        getSelfRef(expr);
                }

                stack.back().popLocalCxt();
                return {};
            }
            case AST::treeKind::rangeBasedForStmt: {
                // obj.rexIter(), it.next(), it.isEnd(), it.get()
                value object = interpret(target.child[1]);
                std::shared_ptr<value> obj = object.isRef() ? object.refObj : managePtr(object);
                std::shared_ptr<value> rexIter;
                if (auto it = obj->members.find(L"rexIter"); it != obj->members.end()) {
                    rexIter = it->second;
                } else {
                    throw signalException(makeErr(L"internalError", (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": undefined identifier: `rexIter`"));
                }
                std::shared_ptr<value> rIter = managePtr(invokeFunc(rexIter, {}, obj));
                if (rIter->isRef())
                    rIter = rIter->refObj;

                stack.back().pushLocalCxt(
                        (value::cxtObject) {{target.child[0].leaf.strVal, {}}});

                auto &itVal = stack.back().localCxt.back()[target.child[0].leaf.strVal];

                auto cxtIdx = stack.back().getCurLocalCxtIdx();

                std::shared_ptr<value> itNext;

                if (auto it = rIter->members.find(L"next"); it != rIter->members.end())
                    itNext = it->second;
                else
                    throw signalException(makeErr(L"internalError", (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": undefined identifier: `next`"));

                while (true) {
                    if (auto val = invokeFunc(itNext, {}, rIter); val.getVec()[1]->getBool()) {
                        stack.back().backToLocalCxt(cxtIdx);
                        break;
                    } else
                        itVal = val.getVec()[0]->isRef() ? val.getVec()[0]->refObj : val.getVec()[0];

                    try {
                        interpret(target.child[2]);
                    } catch (const signalContinue &e) {
                        stack.back().backToLocalCxt(cxtIdx);
                    } catch (const signalBreak &e) {
                        stack.back().backToLocalCxt(cxtIdx);
                        break;
                    }
                }

                stack.back().popLocalCxt();
                return {};
            }
            case AST::treeKind::ifStmt: {
                value cond = interpret(target.child[0]);
                if (cond.isRef())
                    getSelfRef(cond);

                if (cond.getBool())
                    interpret(target.child[1]);
                return {};
            }
            case AST::treeKind::ifElseStmt: {
                value cond = interpret(target.child[0]);
                if (cond.isRef())
                    getSelfRef(cond);
                if (cond.getBool())
                    interpret(target.child[1]);
                else
                    interpret(target.child[2]);
                return {};
            }
            case AST::treeKind::continueStmt: {
                throw signalContinue{};
            }
            case AST::treeKind::breakStmt: {
                throw signalBreak{};
            }
            case AST::treeKind::tryCatchStmt: {
                auto cxtIdx = stack.back().getCurLocalCxtIdx();
                auto stkIdx = getCurStackIdx();
                try {
                    interpret(target.child[0]);
                } catch (signalException &e) {
                    backToStackIdx(stkIdx);
                    stack.back().backToLocalCxt(cxtIdx);
                    stack.back().pushLocalCxt({{target.child[1].leaf.strVal, managePtr(e.get())}});
                    interpret(target.child[2]);
                    stack.back().popLocalCxt();
                }
                return {};
            }
            case AST::treeKind::throwStmt: {
                throw signalException(interpret(target.child[0]));
            }
            case AST::treeKind::returnStmt: {
                throw signalReturn(interpret(target.child[0]));
            }
            case AST::treeKind::withStmt: {
                auto cxtIdx = stack.back().getCurLocalCxtIdx();
                auto stkIdx = getCurStackIdx();
                stack.back().pushLocalCxt({});
                auto &ex = stack.back().localCxt.back()[target.child[0].leaf.strVal] = eleRefObj(
                        interpret(target.child[1]));

                if (auto it = ex->members.find(L"rexInit"); it != ex->members.end()) {
                    invokeFunc(it->second, {}, ex);
                }

                try {
                    interpret(target.child[2]);
                } catch (rex::signalBreak &e) {
                    if (auto it = ex->members.find(L"rexFree"); it != ex->members.end()) {
                        invokeFunc(it->second, {}, ex);
                    }
                    throw;
                } catch (rex::signalContinue &e) {
                    if (auto it = ex->members.find(L"rexFree"); it != ex->members.end()) {
                        invokeFunc(it->second, {}, ex);
                    }
                    throw;
                } catch (rex::signalReturn &e) {
                    if (auto it = ex->members.find(L"rexFree"); it != ex->members.end()) {
                        invokeFunc(it->second, {}, ex);
                    }
                    throw;
                } catch (rex::signalException &e) {
                    if (auto it = ex->members.find(L"rexFree"); it != ex->members.end()) {
                        invokeFunc(it->second, {}, ex);
                    }
                    throw;
                } catch (rex::importError &e) {
                    if (auto it = ex->members.find(L"rexFree"); it != ex->members.end()) {
                        invokeFunc(it->second, {}, ex);
                    }
                    throw;
                } catch (rex::rexException &e) {
                    if (auto it = ex->members.find(L"rexFree"); it != ex->members.end()) {
                        invokeFunc(it->second, {}, ex);
                    }
                    throw;
                }
                if (auto it = ex->members.find(L"rexFree"); it != ex->members.end()) {
                    invokeFunc(it->second, {}, ex);
                }
                backToStackIdx(stkIdx);
                stack.back().backToLocalCxt(cxtIdx);
                return {};
            }
            default: {
                throw signalException(makeErr(L"internalError", (vstr)(env->dumpRuntimeSourceFileMsg(stack.empty() ? moduleCxt : stack.back().moduleCxt, target)) + L": unexpected AST type"));
            }
        }
    }

    value interpreter::interpretLvalueExpressions(const AST &target) {
        value lhs = interpret(target.child[0]);
        value rhs = interpret(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::asterisk:
                return opMul(lhs, rhs);
            case lexer::token::tokenKind::slash:
                return opDiv(lhs, rhs);
            case lexer::token::tokenKind::percentSign:
                return opMod(lhs, rhs);
            case lexer::token::tokenKind::plus:
                return opAdd(lhs, rhs);
            case lexer::token::tokenKind::minus:
                return opSub(lhs, rhs);
            case lexer::token::tokenKind::binaryShiftLeft:
                return opBinaryShiftLeft(lhs, rhs);
            case lexer::token::tokenKind::binaryShiftRight:
                return opBinaryShiftRight(lhs, rhs);
            case lexer::token::tokenKind::equal:
                return opEqual(lhs, rhs);
            case lexer::token::tokenKind::notEqual:
                return opNotEqual(lhs, rhs);
            case lexer::token::tokenKind::greaterEqual:
                return opGreaterEqual(lhs, rhs);
            case lexer::token::tokenKind::lessEqual:
                return opLessEqual(lhs, rhs);
            case lexer::token::tokenKind::greaterThan:
                return opGreaterThan(lhs, rhs);
            case lexer::token::tokenKind::lessThan:
                return opLessThan(lhs, rhs);
            case lexer::token::tokenKind::binaryOr:
                return opBinaryOr(lhs, rhs);
            case lexer::token::tokenKind::binaryAnd:
                return opBinaryAnd(lhs, rhs);
            case lexer::token::tokenKind::binaryXor:
                return opBinaryXor(lhs, rhs);
            case lexer::token::tokenKind::logicAnd:
                return opLogicAnd(lhs, rhs);
            case lexer::token::tokenKind::logicOr:
                return opLogicOr(lhs, rhs);
            default:
                return {};
        }
    }

    value interpreter::interpretAssignments(const AST &target) {
        value lhs = interpret(target.child[0]);
        auto &l = lhs.refObj;
        value rhs = interpret(target.child[2]);
        if (rhs.isRef())
            getSelfRef(rhs);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::assignSign: {
                (*l) = rhs;
                return lhs;
            }
            case lexer::token::tokenKind::additionAssignment: {
                switch (valueKindComparator(l->kind, rhs.kind)) {
                    case valueKindComparator(value::vKind::vInt, value::vKind::vInt): {
                        l->getInt() += rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vInt, value::vKind::vDeci): {
                        l->getDeci() = (vdeci) l->getInt() + rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vInt, value::vKind::vBool): {
                        l->getInt() = l->getInt() + (vint) rhs.getBool();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vInt): {
                        l->getInt() = (vint) l->getBool() + rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vDeci): {
                        l->getDeci() = (vdeci) l->getBool() + rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vBool): {
                        l->getInt() = (vint) l->getBool() + (vint) rhs.getBool();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vInt): {
                        l->getDeci() = l->getDeci() + (vdeci) rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci): {
                        l->getDeci() = l->getDeci() + rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vBool): {
                        l->getDeci() = l->getDeci() + (vdeci) rhs.getBool();
                        return lhs;
                    }
                    default: {
                        if (auto it = l->members.find(L"rexAddAssign"); it != l->members.end())
                            return invokeFunc(it->second, vec<value>{rhs}, lhs.refObj);
                        else
                            throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                    }
                }
            }
            case lexer::token::tokenKind::multiplicationAssignment: {
                switch (valueKindComparator(l->kind, rhs.kind)) {
                    case valueKindComparator(value::vKind::vInt, value::vKind::vInt): {
                        l->getInt() *= rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vInt, value::vKind::vDeci): {
                        l->getDeci() = (vdeci) l->getInt() * rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vInt, value::vKind::vBool): {
                        l->getInt() = l->getInt() * (vint) rhs.getBool();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vInt): {
                        l->getInt() = (vint) l->getBool() * rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vDeci): {
                        l->getDeci() = (vdeci) l->getBool() * rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vBool): {
                        l->getInt() = (vint) l->getBool() * (vint) rhs.getBool();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vInt): {
                        l->getDeci() = l->getDeci() * (vdeci) rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci): {
                        l->getDeci() = l->getDeci() * rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vBool): {
                        l->getDeci() = l->getDeci() * (vdeci) rhs.getBool();
                        return lhs;
                    }
                    default: {
                        if (auto it = l->members.find(L"rexMulAssign"); it != l->members.end())
                            return invokeFunc(it->second, vec<value>{rhs}, lhs.refObj);
                        else
                            throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                    }
                }
            }
            case lexer::token::tokenKind::reminderAssignment: {
                switch (valueKindComparator(l->kind, rhs.kind)) {
                    case valueKindComparator(value::vKind::vInt, value::vKind::vInt): {
                        l->getInt() %= rhs.getInt();
                        return lhs;
                    }
                    default: {
                        if (auto it = l->members.find(L"rexModAssign"); it != l->members.end())
                            return invokeFunc(it->second, vec<value>{rhs}, lhs.refObj);
                        else
                            throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                    }
                }
            }
            case lexer::token::tokenKind::divisionAssignment: {
                switch (valueKindComparator(l->kind, rhs.kind)) {
                    case valueKindComparator(value::vKind::vInt, value::vKind::vInt): {
                        l->getInt() /= rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vInt, value::vKind::vDeci): {
                        l->getDeci() = (vdeci) l->getInt() / rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vInt, value::vKind::vBool): {
                        l->getInt() = l->getInt() / (vint) rhs.getBool();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vInt): {
                        l->getInt() = (vint) l->getBool() / rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vDeci): {
                        l->getDeci() = (vdeci) l->getBool() / rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vBool): {
                        l->getInt() = (vint) l->getBool() / (vint) rhs.getBool();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vInt): {
                        l->getDeci() = l->getDeci() / (vdeci) rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci): {
                        l->getDeci() = l->getDeci() / rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vBool): {
                        l->getDeci() = l->getDeci() / (vdeci) rhs.getBool();
                        return lhs;
                    }
                    default: {
                        if (auto it = l->members.find(L"rexDivAssign"); it != l->members.end())
                            return invokeFunc(it->second, vec<value>{rhs}, lhs.refObj);
                        else
                            throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                    }
                }
            }
            case lexer::token::tokenKind::subtractionAssignment: {
                switch (valueKindComparator(l->kind, rhs.kind)) {
                    case valueKindComparator(value::vKind::vInt, value::vKind::vInt): {
                        l->getInt() -= rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vInt, value::vKind::vDeci): {
                        l->getDeci() = (vdeci) l->getInt() - rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vInt, value::vKind::vBool): {
                        l->getInt() = l->getInt() - (vint) rhs.getBool();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vInt): {
                        l->getInt() = (vint) l->getBool() - rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vDeci): {
                        l->getDeci() = (vdeci) l->getBool() - rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vBool, value::vKind::vBool): {
                        l->getInt() = (vint) l->getBool() - (vint) rhs.getBool();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vInt): {
                        l->getDeci() = l->getDeci() - (vdeci) rhs.getInt();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci): {
                        l->getDeci() = l->getDeci() - rhs.getDeci();
                        return lhs;
                    }
                    case valueKindComparator(value::vKind::vDeci, value::vKind::vBool): {
                        l->getDeci() = l->getDeci() - (vdeci) rhs.getBool();
                        return lhs;
                    }
                    default: {
                        if (auto it = l->members.find(L"rexSubAssign"); it != l->members.end())
                            return invokeFunc(it->second, vec<value>{rhs}, lhs.refObj);
                        else
                            throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                    }
                }
            }
            default: {
                return {};
            }
        }
    }

    vsize interpreter::getCurStackIdx() {
        return stack.size();
    }

    void interpreter::backToStackIdx(vsize stkIdx) {
        while (stack.size() > stkIdx) stack.pop_back();
    }

    value interpreter::opAdd(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() + b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() + (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() + b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() + b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() + (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() + b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() + (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() + (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() + b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexAdd"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opSub(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() - b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() - (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() - b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() - b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() - (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() - b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() - (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() - (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() - b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexSub"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opMul(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() * b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() * (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() * b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() * b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() * (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() * b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() * (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() * (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() * b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexMul"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opDiv(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() / b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() / (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() / b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() / b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() / (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() / b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() / (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() / (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() / b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexDiv"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opMod(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() % b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() % (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() % b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() % (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexMod"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opBinaryShiftLeft(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() << b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() << (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() << b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() << (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexBinaryShiftLeft"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opBinaryShiftRight(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() >> b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() >> (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() >> b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() >> (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexBinaryShiftLeft"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opEqual(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() == b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() == (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() == b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() == b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() == (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() == b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() == (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() == (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() == b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexEqual"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opNotEqual(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() != b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() != (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() != b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() != b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() != (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() != b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() != (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() != (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() != b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexNotEqual"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opGreaterEqual(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() >= b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() >= (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() >= b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() >= b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() >= (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() >= b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() >= (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() >= (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() >= b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexGreaterEqual"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opLessEqual(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() <= b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() <= (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() <= b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() <= b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() <= (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() <= b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() <= (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() <= (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() <= b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexLessEqual"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opGreaterThan(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() > b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() > (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() > b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() > b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() > (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() > b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() > (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() > (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() > b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexGreaterThan"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opLessThan(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() < b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() < (vint) b.getBool();
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci):
                return (vdeci) a.getInt() < b.getDeci();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() < b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() < (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci):
                return (vdeci) a.getBool() < b.getDeci();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt):
                return a.getDeci() < (vdeci) b.getInt();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool):
                return a.getDeci() < (vdeci) b.getBool();
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci):
                return a.getDeci() < b.getDeci();
            default: {
                if (auto it = a.members.find(L"rexLessThan"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opBinaryOr(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() | b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() | (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() | b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() | (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexBinaryOr"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opBinaryAnd(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() & b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() & (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() & b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() & (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexBinaryAnd"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opBinaryXor(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() ^ b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() ^ (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() ^ b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() ^ (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexBinaryXor"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opLogicAnd(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() && b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() && (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() && b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() && (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexLogicAnd"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    value interpreter::opLogicOr(value &a, value &b) {
        if (a.isRef())
            getSelfRef(a);
        if (b.isRef())
            getSelfRef(b);

        switch (valueKindComparator(a.kind, b.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt):
                return a.getInt() || b.getInt();
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool):
                return a.getInt() || (vint) b.getBool();
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt):
                return (vint) a.getBool() || b.getInt();
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool):
                return (vint) a.getBool() || (vint) b.getBool();
            default: {
                if (auto it = a.members.find(L"rexLogicOr"); it != a.members.end())
                    return invokeFunc(it->second, {b}, managePtr(a));
                else
                    throw signalException(makeErr(L"typeError", L"unsupported operation between " + a.getKind() + L" and " + b.getKind()));
            }
        }
    }

    vstr interpreter::getBacktrace() {
        vstr result;
        vsize count{0};
        for (auto i = stack.rbegin(); i != stack.rend(); i++) {
            result += L"#" + std::to_wstring(count++) + L" " + (vstr) *i + L"\n";
        }
        return result;
    }

    value interpreter::makeIt(const managedPtr<value> &v, bool isEnd) {
        return {value::vecObject{v, managePtr(value{isEnd})}, vecMethods::getMethodsCxt()};
    }

    vint spawnThread(const managedPtr<environment> &env, const managedPtr<value> &cxt, const managedPtr<value> &func,
                     const vec<value> &args,
                     const managedPtr<value> &passThisPtr) {
        vint id{env->threadIdCounter++};
        env->threadPool[id].setTh(
                std::make_shared<std::thread>(rexThreadWrapper, env, id, cxt, func, args, passThisPtr));

        return id;
    }

    void
    rexThreadWrapper(managedPtr<environment> env, vint tid, managedPtr<value> cxt, managedPtr<value> func,
                     vec<value> args,
                     managedPtr<value> passThisPtr) {
        auto it = managePtr(interpreter{env, cxt});
        it->interpreterCxt[L"thread_id"] = managePtr(value{tid});
        try {
            auto res = it->invokeFunc(func, args, passThisPtr);
            env->threadPool[tid].setResult(managePtr(res.isRef() ? res.getRef() : res));
        } catch (rex::signalException &e) {
            std::cerr << "Uncaught exception in thread " << tid << "> " << rex::wstring2string((rex::value) e.get())
                      << std::endl;
            std::cerr << wstring2string(it->getBacktrace()) << std::endl;
            throw;
        } catch (rex::rexException &e) {
            std::cerr << "Uncaught exception in thread " << tid << "> " << e.what() << std::endl;
            std::cerr << wstring2string(it->getBacktrace()) << std::endl;
            throw;
        } catch (std::exception &e) {
            std::cerr << "Uncaught exception in thread " << tid << "> " << e.what() << std::endl;
            std::cerr << wstring2string(it->getBacktrace()) << std::endl;
            throw;
        }
        env->threadPool[tid].setResult(managePtr(value{}));
    }

    value waitForThread(const managedPtr<environment> &env, vint id) {
        env->threadPool[id].getTh()->join();
        value res = env->threadPool[id].getResult();
        env->threadPool.erase(id);
        return res;
    }

    environment::runtimeSourceFileMsg environment::dumpRuntimeSourceFileMsg(const value::funcObject &func) {
        return {func.moduleCxt->members[L"__path__"]->getStr(), func.code.leaf.line, func.code.leaf.col};
    }

    environment::runtimeSourceFileMsg environment::dumpRuntimeSourceFileMsg(const value::lambdaObject &lambda) {
        return {lambda.func.moduleCxt->members[L"__path__"]->getStr(), lambda.func.code.leaf.line,
                lambda.func.code.leaf.col};
    }

    environment::runtimeSourceFileMsg
    environment::dumpRuntimeSourceFileMsg(const managedPtr<value> &moduleCxt, AST target) {
        return {moduleCxt->members[L"__path__"]->getStr(), target.leaf.line, target.leaf.col};
    }

    environment::runtimeSourceFileMsg::operator vstr() {
        return L"near " + file + L" line " + std::to_wstring(line) + L" column " + std::to_wstring(col);
    }
} // rex