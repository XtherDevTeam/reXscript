//
// Created by XIaokang00010 on 2022/12/3.
//

#include "interpreter.hpp"
#include "exceptions/signalReturn.hpp"
#include "builtInMethods.hpp"
#include "exceptions/signalException.hpp"

namespace rex {
    void environment::stackFrame::pushLocalCxt(const value::cxtObject &cxt) {
        localCxt.push_back(cxt);
    }

    void environment::stackFrame::popLocalCxt() {
        localCxt.pop_back();
    }

    environment::stackFrame::stackFrame() : moduleCxt(nullptr), localCxt() {

    }

    environment::stackFrame::stackFrame(managedPtr<value> &moduleCxt, const vec<value::cxtObject> &localCxt) :
            moduleCxt(moduleCxt), localCxt(localCxt) {

    }

    interpreter::interpreter(const managedPtr<environment> &env, const managedPtr<value> &moduleCxt) : env(env),
                                                                                                       moduleCxt(
                                                                                                               moduleCxt),
                                                                                                       stack() {

    }

    value
    interpreter::invokeFunc(managedPtr<value> func, const vec<value> &args, const managedPtr<value> &passThisPtr) {
        if (func->isRef())
            func = func->refObj;
        try {
            switch (func->kind) {
                case value::vKind::vFunc: {
                    stack.push_back({stack.empty() ? moduleCxt : stack.back().moduleCxt, {}});
                    stack.back().pushLocalCxt({});

                    if (passThisPtr)
                        stack.back().localCxt.back()[L"this"] = passThisPtr;

                    for (vsize i = 0; i < args.size(); i++) {
                        stack.back().localCxt.back()[func->funcObj->argsName[i]] = managePtr(args[i]);
                    }
                    interpret(func->funcObj->code);
                    stack.pop_back();
                    return {};
                }
                case value::vKind::vLambda: {
                    stack.push_back({stack.empty() ? moduleCxt : stack.back().moduleCxt, {}});
                    stack.back().pushLocalCxt({});

                    if (passThisPtr)
                        stack.back().localCxt.back()[L"this"] = passThisPtr;

                    for (vsize i = 0; i < args.size(); i++) {
                        stack.back().localCxt.back()[func->lambdaObj->func.argsName[i]] = managePtr(args[i]);
                    }
                    interpret(func->lambdaObj->func.code);
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
                for (auto &i: target.child) {
                    cxt[i.child[0].leaf.strVal] = managePtr(interpret(i.child[1]));
                }
                return {cxt};
            }
            case AST::treeKind::identifier: {
                // 优先从栈中查找
                if (!stack.empty()) {
                    for (auto it = stack.back().localCxt.rbegin();
                         it != stack.back().localCxt.rend(); it++) {
                        if (auto vit = it->find(target.leaf.strVal); vit != it->end()) {
                            return {vit->second};
                        }
                    }
                }
                // 全局范围解释
                if (auto it = moduleCxt->members.find(target.leaf.strVal);
                        it != moduleCxt->members.end()) {
                    // 在当前Module Context中查找
                    return {it->second};
                }
                if (auto it = env->globalCxt->members.find(target.leaf.strVal);
                        it != env->globalCxt->members.end()) {
                    // 在Global Context中查找
                    return {it->second};
                }
                // 符号不存在
                throw signalException(makeErr(L"internalError", L"undefined symbol: `" + target.leaf.strVal + L"`"));
            }
            case AST::treeKind::subscriptExpression: {
                // 处理最左下的subscript，在memberExpression右端的交给memberExpression处理分支
                value val = interpret(target.child[0]);
                value &l = val.isRef() ? val.getRef() : val;
                value r = interpret(target.child[1]);
                if (r.isRef())
                    r = r.getRef();
                switch (l.kind) {
                    case value::vKind::vStr:
                        return {(vint) {(*l.strObj)[r.getInt()]}};
                    case value::vKind::vVec:
                        return {{(*l.vecObj)[r.getInt()]}};
                    default: {
                        if (auto it = l.members.find(L"rexSubscript"); it != l.members.end())
                            return it->second;
                        if (r.kind == value::vKind::vStr)
                            return l[r.getStr()];
                        else
                            throw signalException(makeErr(L"internalError", L"not a subscript-able object"));
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
                    lambda.outerCxt->members[i.leaf.strVal] = managePtr(interpret(i));
                }
                for (auto &i: target.child[1].child) {
                    lambda.func.argsName.push_back(i.leaf.strVal);
                }
                lambda.func.code = target.child[2];
                return {lambda};
            }
            case AST::treeKind::functionDefinition: {
                value::funcObject func;
                for (auto &i: target.child[0].child) {
                    func.argsName.push_back(i.leaf.strVal);
                }
                func.code = target.child[1];
                return {func};
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
                                            L"undefined identifier: `" + target.child[1].leaf.strVal + L"`"));
                        }
                    }
                    case AST::treeKind::subscriptExpression: {
                        if (auto it = l.members.find(target.child[1].child[0].leaf.strVal);
                                it != l.members.end()) {
                            value &lhs = *it->second;
                            value r = interpret(target.child[1].child[1]);
                            if (r.isRef())
                                r = r.getRef();

                            switch (lhs.kind) {
                                case value::vKind::vStr:
                                    return {(vint) {(*lhs.strObj)[r.getInt()]}};
                                case value::vKind::vVec:
                                    return {{(*lhs.vecObj)[r.getInt()]}};
                                default: {
                                    if (auto vit = lhs.members.find(L"rexSubscript"); vit != lhs.members.end())
                                        return vit->second;
                                    if (r.kind == value::vKind::vStr)
                                        return lhs[r.getStr()];
                                    else
                                        throw signalException(makeErr(L"internalError", L"not a subscript-able object"));
                                }
                            }
                        } else {
                            throw signalException(
                                    makeErr(
                                            L"internalError",
                                            L"undefined identifier: `" + target.child[1].leaf.strVal + L"`"));
                        }
                    }
                    case AST::treeKind::invokingExpression: {
                        if (auto it = l.members.find(target.child[1].leaf.strVal); it != l.members.end()) {
                            vec<value> args;
                            for (auto &i: target.child[1].child) {
                                args.push_back(interpret(i));
                            }
                            return invokeFunc(it->second, args,
                                              val.isRef() ? val.refObj : managePtr(val));
                        } else {
                            throw signalException(
                                    makeErr(
                                            L"internalError",
                                            L"undefined identifier: `" + target.child[1].leaf.strVal + L"`"));
                        }
                    }
                    default: {
                        return {};
                    }
                }
            }
            case AST::treeKind::uniqueExpression: {
                value rhs = interpret(target.child[1]);
                value &r = rhs.isRef() ? rhs.getRef() : rhs;
                switch (target.child[1].leaf.kind) {
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
                                    throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexNegate` operation"));
                            }
                        }
                    }
                    case lexer::token::tokenKind::incrementSign: {
                        if (!rhs.isRef())
                            throw signalException(makeErr(L"internalError", L"expected a referenced object"));

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
                                    throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexIncrement` operation"));
                            }
                        }
                    }
                    case lexer::token::tokenKind::decrementSign: {
                        if (!rhs.isRef())
                            throw signalException(makeErr(L"internalError", L"expected a referenced object"));

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
                                    throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexDecrement` operation"));
                            }
                        }
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
            default: {
                return {};
            }
        }
    }

    value interpreter::interpretLvalueExpressions(const AST &target) {
        value lhs = interpret(target.child[0]);
        if (lhs.isRef())
            lhs = lhs.getRef();
        value rhs = interpret(target.child[2]);
        if (rhs.isRef())
            rhs = rhs.getRef();

        switch (valueKindComparator(lhs.kind, rhs.kind)) {
            case valueKindComparator(value::vKind::vInt, value::vKind::vInt): {
                switch (target.child[1].leaf.kind) {
                    case lexer::token::tokenKind::asterisk:
                        return {lhs.getInt() * rhs.getInt()};
                    case lexer::token::tokenKind::slash:
                        return {lhs.getInt() / rhs.getInt()};
                    case lexer::token::tokenKind::percentSign:
                        return {lhs.getInt() % rhs.getInt()};
                    case lexer::token::tokenKind::plus:
                        return {lhs.getInt() + rhs.getInt()};
                    case lexer::token::tokenKind::minus:
                        return {lhs.getInt() - rhs.getInt()};
                    case lexer::token::tokenKind::binaryShiftLeft:
                        return {lhs.getInt() << rhs.getInt()};
                    case lexer::token::tokenKind::binaryShiftRight:
                        return {lhs.getInt() >> rhs.getInt()};
                    case lexer::token::tokenKind::equal:
                        return {lhs.getInt() == rhs.getInt()};
                    case lexer::token::tokenKind::notEqual:
                        return {lhs.getInt() != rhs.getInt()};
                    case lexer::token::tokenKind::greaterEqual:
                        return {lhs.getInt() >= rhs.getInt()};
                    case lexer::token::tokenKind::lessEqual:
                        return {lhs.getInt() <= rhs.getInt()};
                    case lexer::token::tokenKind::greaterThan:
                        return {lhs.getInt() > rhs.getInt()};
                    case lexer::token::tokenKind::lessThan:
                        return {lhs.getInt() < rhs.getInt()};
                    case lexer::token::tokenKind::binaryAnd:
                        return {lhs.getInt() & rhs.getInt()};
                    case lexer::token::tokenKind::binaryOr:
                        return {lhs.getInt() | rhs.getInt()};
                    case lexer::token::tokenKind::binaryXor:
                        return {lhs.getInt() ^ rhs.getInt()};
                    case lexer::token::tokenKind::logicAnd:
                        return {lhs.getInt() && rhs.getInt()};
                    case lexer::token::tokenKind::logicOr:
                        return {lhs.getInt() || rhs.getInt()};
                    default:
                        throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                }
            }
            case valueKindComparator(value::vKind::vInt, value::vKind::vDeci): {
                switch (target.child[1].leaf.kind) {
                    case lexer::token::tokenKind::asterisk:
                        return {(vdeci)lhs.getInt() * rhs.getDeci()};
                    case lexer::token::tokenKind::slash:
                        return {(vdeci)lhs.getInt() / rhs.getDeci()};
                    case lexer::token::tokenKind::plus:
                        return {(vdeci)lhs.getInt() * rhs.getDeci()};
                    case lexer::token::tokenKind::minus:
                        return {(vdeci)lhs.getInt() - rhs.getDeci()};
                    case lexer::token::tokenKind::equal:
                        return {(vdeci)lhs.getInt() == rhs.getDeci()};
                    case lexer::token::tokenKind::notEqual:
                        return {(vdeci)lhs.getInt() != rhs.getDeci()};
                    case lexer::token::tokenKind::greaterEqual:
                        return {(vdeci)lhs.getInt() >= rhs.getDeci()};
                    case lexer::token::tokenKind::lessEqual:
                        return {(vdeci)lhs.getInt() <= rhs.getDeci()};
                    case lexer::token::tokenKind::greaterThan:
                        return {(vdeci)lhs.getInt() > rhs.getDeci()};
                    case lexer::token::tokenKind::lessThan:
                        return {(vdeci)lhs.getInt() < rhs.getDeci()};
                    case lexer::token::tokenKind::binaryAnd:
                        return {lhs.getInt() & rhs.getInt()};
                    case lexer::token::tokenKind::binaryOr:
                        return {lhs.getInt() | rhs.getInt()};
                    case lexer::token::tokenKind::binaryXor:
                        return {lhs.getInt() ^ rhs.getInt()};
                    case lexer::token::tokenKind::logicAnd:
                        return {lhs.getInt() && (vint)rhs.getDeci()};
                    case lexer::token::tokenKind::logicOr:
                        return {lhs.getInt() || (vint)rhs.getDeci()};
                    default:
                        throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                }
            }
            case valueKindComparator(value::vKind::vInt, value::vKind::vBool): {
                switch (target.child[1].leaf.kind) {
                    case lexer::token::tokenKind::asterisk:
                        return {lhs.getInt() * rhs.getBool()};
                    case lexer::token::tokenKind::slash:
                        return {lhs.getInt() / rhs.getBool()};
                    case lexer::token::tokenKind::plus:
                        return {lhs.getInt() + rhs.getBool()};
                    case lexer::token::tokenKind::minus:
                        return {lhs.getInt() - rhs.getBool()};
                    case lexer::token::tokenKind::equal:
                        return {lhs.getInt() == rhs.getBool()};
                    case lexer::token::tokenKind::notEqual:
                        return {lhs.getInt() != rhs.getBool()};
                    case lexer::token::tokenKind::greaterEqual:
                        return {lhs.getInt() >= rhs.getBool()};
                    case lexer::token::tokenKind::lessEqual:
                        return {lhs.getInt() <= rhs.getBool()};
                    case lexer::token::tokenKind::greaterThan:
                        return {lhs.getInt() > rhs.getBool()};
                    case lexer::token::tokenKind::lessThan:
                        return {lhs.getInt() < rhs.getBool()};
                    case lexer::token::tokenKind::binaryAnd:
                        return {lhs.getInt() & rhs.getInt()};
                    case lexer::token::tokenKind::binaryOr:
                        return {lhs.getInt() | rhs.getInt()};
                    case lexer::token::tokenKind::binaryXor:
                        return {lhs.getInt() ^ rhs.getInt()};
                    case lexer::token::tokenKind::logicAnd:
                        return {lhs.getInt() && rhs.getBool()};
                    case lexer::token::tokenKind::logicOr:
                        return {lhs.getInt() || rhs.getBool()};
                    default:
                        throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                }
            }
            case valueKindComparator(value::vKind::vDeci, value::vKind::vInt): {
                switch (target.child[1].leaf.kind) {
                    case lexer::token::tokenKind::asterisk:
                        return {lhs.getDeci() * (vdeci)rhs.getInt()};
                    case lexer::token::tokenKind::slash:
                        return {lhs.getDeci() / (vdeci)rhs.getInt()};
                    case lexer::token::tokenKind::plus:
                        return {lhs.getDeci() + (vdeci)rhs.getInt()};
                    case lexer::token::tokenKind::minus:
                        return {lhs.getDeci() - (vdeci)rhs.getInt()};
                    case lexer::token::tokenKind::equal:
                        return {lhs.getDeci() == (vdeci)rhs.getInt()};
                    case lexer::token::tokenKind::notEqual:
                        return {lhs.getDeci() != (vdeci)rhs.getInt()};
                    case lexer::token::tokenKind::greaterEqual:
                        return {lhs.getDeci() >= (vdeci)rhs.getInt()};
                    case lexer::token::tokenKind::lessEqual:
                        return {lhs.getDeci() <= (vdeci)rhs.getInt()};
                    case lexer::token::tokenKind::greaterThan:
                        return {lhs.getDeci() > (vdeci)rhs.getInt()};
                    case lexer::token::tokenKind::lessThan:
                        return {lhs.getDeci() < (vdeci)rhs.getInt()};
                    case lexer::token::tokenKind::binaryAnd:
                        return {lhs.getInt() & rhs.getInt()};
                    case lexer::token::tokenKind::binaryOr:
                        return {lhs.getInt() | rhs.getInt()};
                    case lexer::token::tokenKind::binaryXor:
                        return {lhs.getInt() ^ rhs.getInt()};
                    case lexer::token::tokenKind::logicAnd:
                        return {(vbool)lhs.getDeci() && rhs.getBool()};
                    case lexer::token::tokenKind::logicOr:
                        return {(vbool)lhs.getDeci() || rhs.getBool()};
                    default:
                        throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                }
            }
            case valueKindComparator(value::vKind::vDeci, value::vKind::vDeci): {
                switch (target.child[1].leaf.kind) {
                    case lexer::token::tokenKind::asterisk:
                        return {lhs.getDeci() * rhs.getDeci()};
                    case lexer::token::tokenKind::slash:
                        return {lhs.getDeci() / rhs.getDeci()};
                    case lexer::token::tokenKind::plus:
                        return {lhs.getDeci() + rhs.getDeci()};
                    case lexer::token::tokenKind::minus:
                        return {lhs.getDeci() - rhs.getDeci()};
                    case lexer::token::tokenKind::equal:
                        return {lhs.getDeci() == rhs.getDeci()};
                    case lexer::token::tokenKind::notEqual:
                        return {lhs.getDeci() != rhs.getDeci()};
                    case lexer::token::tokenKind::greaterEqual:
                        return {lhs.getDeci() >= rhs.getDeci()};
                    case lexer::token::tokenKind::lessEqual:
                        return {lhs.getDeci() <= rhs.getDeci()};
                    case lexer::token::tokenKind::greaterThan:
                        return {lhs.getDeci() > rhs.getDeci()};
                    case lexer::token::tokenKind::lessThan:
                        return {lhs.getDeci() < rhs.getDeci()};
                    case lexer::token::tokenKind::binaryAnd:
                        return {lhs.getInt() & rhs.getInt()};
                    case lexer::token::tokenKind::binaryOr:
                        return {lhs.getInt() | rhs.getInt()};
                    case lexer::token::tokenKind::binaryXor:
                        return {lhs.getInt() ^ rhs.getInt()};
                    case lexer::token::tokenKind::logicAnd:
                        return {(vbool)lhs.getDeci() && (vbool)rhs.getDeci()};
                    case lexer::token::tokenKind::logicOr:
                        return {(vbool)lhs.getDeci() || (vbool)rhs.getDeci()};
                    default:
                        throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                }
            }
            case valueKindComparator(value::vKind::vDeci, value::vKind::vBool): {
                switch (target.child[1].leaf.kind) {
                    case lexer::token::tokenKind::asterisk:
                        return {lhs.getDeci() * (vdeci)rhs.getBool()};
                    case lexer::token::tokenKind::slash:
                        return {lhs.getDeci() / (vdeci)rhs.getBool()};
                    case lexer::token::tokenKind::plus:
                        return {lhs.getDeci() + (vdeci)rhs.getBool()};
                    case lexer::token::tokenKind::minus:
                        return {lhs.getDeci() - (vdeci)rhs.getBool()};
                    case lexer::token::tokenKind::equal:
                        return {lhs.getDeci() == (vdeci)rhs.getBool()};
                    case lexer::token::tokenKind::notEqual:
                        return {lhs.getDeci() != (vdeci)rhs.getBool()};
                    case lexer::token::tokenKind::greaterEqual:
                        return {lhs.getDeci() >= (vdeci)rhs.getBool()};
                    case lexer::token::tokenKind::lessEqual:
                        return {lhs.getDeci() <= (vdeci)rhs.getBool()};
                    case lexer::token::tokenKind::greaterThan:
                        return {lhs.getDeci() > (vdeci)rhs.getBool()};
                    case lexer::token::tokenKind::lessThan:
                        return {lhs.getDeci() < (vdeci)rhs.getBool()};
                    case lexer::token::tokenKind::binaryAnd:
                        return {lhs.getInt() & rhs.getInt()};
                    case lexer::token::tokenKind::binaryOr:
                        return {lhs.getInt() | rhs.getInt()};
                    case lexer::token::tokenKind::binaryXor:
                        return {lhs.getInt() ^ rhs.getInt()};
                    case lexer::token::tokenKind::logicAnd:
                        return {(vbool)lhs.getDeci() && rhs.getBool()};
                    case lexer::token::tokenKind::logicOr:
                        return {(vbool)lhs.getDeci() || rhs.getBool()};
                    default:
                        throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                }
            }
            case valueKindComparator(value::vKind::vBool, value::vKind::vInt): {
                switch (target.child[1].leaf.kind) {
                    case lexer::token::tokenKind::asterisk:
                        return {lhs.getBool() * rhs.getInt()};
                    case lexer::token::tokenKind::slash:
                        return {lhs.getBool() / rhs.getInt()};
                    case lexer::token::tokenKind::percentSign:
                        return {lhs.getBool() % rhs.getInt()};
                    case lexer::token::tokenKind::plus:
                        return {lhs.getBool() + rhs.getInt()};
                    case lexer::token::tokenKind::minus:
                        return {lhs.getBool() - rhs.getInt()};
                    case lexer::token::tokenKind::equal:
                        return {lhs.getBool() == rhs.getInt()};
                    case lexer::token::tokenKind::notEqual:
                        return {lhs.getBool() != rhs.getInt()};
                    case lexer::token::tokenKind::greaterEqual:
                        return {lhs.getBool() >= rhs.getInt()};
                    case lexer::token::tokenKind::lessEqual:
                        return {lhs.getBool() <= rhs.getInt()};
                    case lexer::token::tokenKind::greaterThan:
                        return {lhs.getBool() > rhs.getInt()};
                    case lexer::token::tokenKind::lessThan:
                        return {lhs.getBool() < rhs.getInt()};
                    case lexer::token::tokenKind::binaryAnd:
                        return {lhs.getInt() & rhs.getInt()};
                    case lexer::token::tokenKind::binaryOr:
                        return {lhs.getInt() | rhs.getInt()};
                    case lexer::token::tokenKind::binaryXor:
                        return {lhs.getInt() ^ rhs.getInt()};
                    case lexer::token::tokenKind::logicAnd:
                        return {lhs.getBool() && (vbool)rhs.getInt()};
                    case lexer::token::tokenKind::logicOr:
                        return {lhs.getBool() || (vbool)rhs.getInt()};
                    default:
                        throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                }
            }
            case valueKindComparator(value::vKind::vBool, value::vKind::vDeci): {
                switch (target.child[1].leaf.kind) {
                    case lexer::token::tokenKind::asterisk:
                        return {(vdeci)lhs.getBool() * rhs.getDeci()};
                    case lexer::token::tokenKind::slash:
                        return {(vdeci)lhs.getBool() / rhs.getDeci()};
                    case lexer::token::tokenKind::plus:
                        return {(vdeci)lhs.getBool() + rhs.getDeci()};
                    case lexer::token::tokenKind::minus:
                        return {(vdeci)lhs.getBool() - rhs.getDeci()};
                    case lexer::token::tokenKind::equal:
                        return {(vdeci)lhs.getBool() == rhs.getDeci()};
                    case lexer::token::tokenKind::notEqual:
                        return {(vdeci)lhs.getBool() != rhs.getDeci()};
                    case lexer::token::tokenKind::greaterEqual:
                        return {(vdeci)lhs.getBool() >= rhs.getDeci()};
                    case lexer::token::tokenKind::lessEqual:
                        return {(vdeci)lhs.getBool() <= rhs.getDeci()};
                    case lexer::token::tokenKind::greaterThan:
                        return {(vdeci)lhs.getBool() > rhs.getDeci()};
                    case lexer::token::tokenKind::lessThan:
                        return {(vdeci)lhs.getBool() < rhs.getDeci()};
                    case lexer::token::tokenKind::binaryAnd:
                        return {lhs.getInt() & rhs.getInt()};
                    case lexer::token::tokenKind::binaryOr:
                        return {lhs.getInt() | rhs.getInt()};
                    case lexer::token::tokenKind::binaryXor:
                        return {lhs.getInt() ^ rhs.getInt()};
                    case lexer::token::tokenKind::logicAnd:
                        return {lhs.getBool() && (vbool)rhs.getDeci()};
                    case lexer::token::tokenKind::logicOr:
                        return {lhs.getBool() || (vbool)rhs.getDeci()};
                    default:
                        throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                }
            }
            case valueKindComparator(value::vKind::vBool, value::vKind::vBool): {
                switch (target.child[1].leaf.kind) {
                    case lexer::token::tokenKind::asterisk:
                    case lexer::token::tokenKind::slash:
                    case lexer::token::tokenKind::equal:
                        return {lhs.getBool() == rhs.getBool()};
                    case lexer::token::tokenKind::notEqual:
                        return {lhs.getBool() != rhs.getBool()};
                    case lexer::token::tokenKind::greaterThan:
                        return {lhs.getBool() > rhs.getBool()};
                    case lexer::token::tokenKind::lessThan:
                        return {lhs.getBool() < rhs.getBool()};
                    case lexer::token::tokenKind::binaryAnd:
                        return {lhs.getInt() & rhs.getInt()};
                    case lexer::token::tokenKind::binaryOr:
                        return {lhs.getInt() | rhs.getInt()};
                    case lexer::token::tokenKind::binaryXor:
                        return {lhs.getInt() ^ rhs.getInt()};
                    case lexer::token::tokenKind::logicAnd:
                        return {lhs.getBool() && rhs.getBool()};
                    case lexer::token::tokenKind::logicOr:
                        return {lhs.getBool() || rhs.getBool()};
                    default:
                        throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                }
            }
            default: {
                lhs.deepCopy(lhs); // deep copy to make changes
                rhs.deepCopy(rhs); // deep copy to make changes
                switch (target.child[1].leaf.kind) {
                    case lexer::token::tokenKind::asterisk: {
                        if (auto it = lhs.members.find(L"rexMul"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexMul` operation"));
                        }
                    }
                    case lexer::token::tokenKind::slash:{
                        if (auto it = lhs.members.find(L"rexDiv"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexDiv` operation"));
                        }
                    }
                    case lexer::token::tokenKind::percentSign:{
                        if (auto it = lhs.members.find(L"rexMod"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexMod` operation"));
                        }
                    }
                    case lexer::token::tokenKind::plus: {
                        if (auto it = lhs.members.find(L"rexAdd"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexAdd` operation"));
                        }
                    }
                    case lexer::token::tokenKind::minus:{
                        if (auto it = lhs.members.find(L"rexSub"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexSub` operation"));
                        }
                    }
                    case lexer::token::tokenKind::binaryShiftLeft:{
                        if (auto it = lhs.members.find(L"rexBinaryShiftLeft"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexBinaryShiftLeft` operation"));
                        }
                    }
                    case lexer::token::tokenKind::binaryShiftRight:{
                        if (auto it = lhs.members.find(L"rexBinaryShiftRight"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexBinaryShiftRight` operation"));
                        }
                    }
                    case lexer::token::tokenKind::greaterEqual: {
                        if (auto it = lhs.members.find(L"rexGreaterEqual"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexGreaterEqual` operation"));
                        }
                    }
                    case lexer::token::tokenKind::lessEqual:{
                        if (auto it = lhs.members.find(L"rexLessEqual"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexLessEqual` operation"));
                        }
                    }
                    case lexer::token::tokenKind::greaterThan:{
                        if (auto it = lhs.members.find(L"rexGreaterThan"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexGreaterThan` operation"));
                        }
                    }
                    case lexer::token::tokenKind::lessThan:{
                        if (auto it = lhs.members.find(L"rexLessThan"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexLessThan` operation"));
                        }
                    }
                    case lexer::token::tokenKind::binaryAnd: {
                        if (auto it = lhs.members.find(L"rexBinaryAnd"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexBinaryAnd` operation"));
                        }
                    }
                    case lexer::token::tokenKind::binaryOr:{
                        if (auto it = lhs.members.find(L"rexBinaryOr"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexBinaryOr` operation"));
                        }
                    }
                    case lexer::token::tokenKind::binaryXor:{
                        if (auto it = lhs.members.find(L"rexBinaryXor"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexBinaryXor` operation"));
                        }
                    }
                    case lexer::token::tokenKind::logicAnd:{
                        if (auto it = lhs.members.find(L"rexLogicAnd"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexLogicAnd` operation"));
                        }
                    }
                    case lexer::token::tokenKind::logicOr:{
                        if (auto it = lhs.members.find(L"rexLogicOr"); it != lhs.members.end()) {
                            return invokeFunc(it->second, {lhs, rhs}, managePtr(lhs));
                        } else {
                            throw signalException(makeErr(L"internalError", L"no overloaded callable objects for `rexLogicOr` operation"));
                        }
                    }
                    default:
                        throw signalException(makeErr(L"typeError", L"cannot evaluate this expression"));
                }
            }
        }
    }
} // rex