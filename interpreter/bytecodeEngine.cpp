//
// Created by XIaokang00010 on 2023/1/14.
//

#include "bytecodeEngine.hpp"

#include <utility>

namespace rex::bytecodeEngine {
    managedPtr<environment> rexEnvironmentInstance = managePtr(environment{});

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


    environment::runtimeSourceFileMsg environment::dumpRuntimeSourceFileMsg(const value::funcObject &func) {
        return {func.moduleCxt->members[L"__path__"]->getStr(), func.code.leaf.line, func.code.leaf.col};
    }

    environment::runtimeSourceFileMsg environment::dumpRuntimeSourceFileMsg(const value::lambdaObject &lambda) {
        return {lambda.func.moduleCxt->members[L"__path__"]->getStr(), lambda.func.code.leaf.line,
                lambda.func.code.leaf.col};
    }

    uint64_t environment::putCodeStruct(const managedPtr<codeStruct> &v) {
        codeStructs.push_back(v);
        return codeStructs.size() - 1;
    }

    environment::runtimeSourceFileMsg::operator vstr() {
        return L"near " + file + L" line " + std::to_wstring(line) + L" column " + std::to_wstring(col);
    }

    codeBuilder::codeBuilder(bytecodeModule mod, codeStruct &currentBlock) : mod(std::move(mod)), currentBlock(
            currentBlock) {

    }

    uint64_t codeBuilder::getNextCur() {
        return (uint64_t) (currentBlock.code.end() - currentBlock.code.begin());
    }

    void codeBuilder::buildBasicLiterals(const AST &target) {
        switch (target.leaf.kind) {
            case lexer::token::tokenKind::string:
                currentBlock.code.push_back(
                        {bytecodeStruct::opCode::stringNew, {currentBlock.putStringConst(target.leaf.strVal)}});
                break;
            case lexer::token::tokenKind::integer:
                currentBlock.code.push_back({bytecodeStruct::opCode::intConst, {target.leaf.basicVal.vInt}});
                break;
            case lexer::token::tokenKind::decimal:
                currentBlock.code.push_back({bytecodeStruct::opCode::deciConst, {target.leaf.basicVal.vDeci}});
                break;
            case lexer::token::tokenKind::boolean:
                currentBlock.code.push_back({bytecodeStruct::opCode::boolConst, {target.leaf.basicVal.vBool}});
                break;
            case lexer::token::tokenKind::kNull:
                currentBlock.code.push_back({bytecodeStruct::opCode::nullConst, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildIdentifier(const AST &target) {
        currentBlock.code.push_back({bytecodeStruct::opCode::find, {currentBlock.putNames(target.leaf.strVal)}});
    }

    void codeBuilder::buildSubscriptExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[1]);
        currentBlock.code.push_back({bytecodeStruct::opCode::index, {}});
    }

    void codeBuilder::buildInvokeExpr(const AST &target) {
        buildExpr(target.child[0]);
        std::for_each(target.child[1].child.rbegin(), target.child[1].child.rend(), [&](const AST &ast) {
            buildExpr(ast);
        });
        currentBlock.code.push_back({bytecodeStruct::opCode::invoke, {(uint64_t) target.child[1].child.size()}});
    }

    void codeBuilder::buildListLiteral(const AST &target) {
        std::for_each(target.child.rbegin(), target.child.rend(), [&](const AST &ast) {
            buildExpr(ast);
        });
        currentBlock.code.push_back({bytecodeStruct::opCode::arrayNew, {(uint64_t) target.child.size()}});
    }

    void codeBuilder::buildObjectLiteral(const AST &target) {
        std::for_each(target.child.rbegin(), target.child.rend(), [&](const AST &ast) {
            currentBlock.code.push_back(
                    {bytecodeStruct::opCode::putIndex, {currentBlock.putStringConst(ast.child[0].leaf.strVal)}});
            buildExpr(ast.child[1]);
        });
        currentBlock.code.push_back({bytecodeStruct::opCode::objectNew, {(uint64_t) target.child.size()}});
    }

    void codeBuilder::buildLambdaDef(const AST &target) {
        std::for_each(target.child[0].child.rbegin(), target.child[0].child.rend(), [&](const AST &ast) {
            currentBlock.code.push_back({bytecodeStruct::opCode::putIndex, {currentBlock.putNames(ast.leaf.strVal)}});
        });
        std::for_each(target.child[1].child.rbegin(), target.child[1].child.rend(), [&](const AST &ast) {
            currentBlock.code.push_back({bytecodeStruct::opCode::putIndex, {currentBlock.putNames(ast.leaf.strVal)}});
        });
        // build function body
        codeBuilder cb(mod, mod.codeStructs[mod.putCodeStruct({})]);
        cb.buildStmt(target.child[2]);
        currentBlock.code.push_back({bytecodeStruct::opCode::funcNew, {(uint64_t) target.child[1].child.size()}});
        currentBlock.code.push_back({bytecodeStruct::opCode::lambdaNew, {(uint64_t) target.child[0].child.size()}});
    }

    void codeBuilder::buildFuncDef(const AST &target) {
        std::for_each(target.child[0].child.rbegin(), target.child[0].child.rend(), [&](const AST &ast) {
            currentBlock.code.push_back({bytecodeStruct::opCode::putIndex, {currentBlock.putNames(ast.leaf.strVal)}});
        });
        // build function body
        codeBuilder cb(mod, mod.codeStructs[mod.putCodeStruct({})]);
        cb.buildStmt(target.child[1]);
        currentBlock.code.push_back({bytecodeStruct::opCode::funcNew, {(uint64_t) target.child[0].child.size()}});
    }

    void codeBuilder::buildMemberExpr(const AST &target) {
        buildExpr(target.child[0]);
        switch (target.child[1].kind) {
            case AST::treeKind::identifier:
                currentBlock.code.push_back(
                        {bytecodeStruct::opCode::findAttr, {currentBlock.putNames(target.child[1].leaf.strVal)}});
                break;
            case AST::treeKind::subscriptExpression:
                currentBlock.code.push_back({bytecodeStruct::opCode::findAttr,
                                             {currentBlock.putNames(target.child[1].child[0].leaf.strVal)}});
                buildExpr(target.child[1].child[1]);
                currentBlock.code.push_back({bytecodeStruct::opCode::index, {}});
                break;
            case AST::treeKind::invokingExpression:
                currentBlock.code.push_back({bytecodeStruct::opCode::duplicate, {}});
                currentBlock.code.push_back({bytecodeStruct::opCode::findAttr,
                                             {currentBlock.putNames(target.child[1].child[0].leaf.strVal)}});

                std::for_each(
                        target.child[1].child[1].child.rbegin(), target.child[1].child[1].child.rend(),
                        [&](const AST &ast) {
                            buildExpr(ast);
                        });
                currentBlock.code.push_back(
                        {bytecodeStruct::opCode::invokeMethod, {(uint64_t) target.child[1].child[1].child.size()}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildUniqueExpr(const AST &target) {
        buildExpr(target.child[1]);
        switch (target.child[0].leaf.kind) {
            case lexer::token::tokenKind::minus:
                currentBlock.code.push_back({bytecodeStruct::opCode::opNegate, {}});
                break;
            case lexer::token::tokenKind::incrementSign:
                currentBlock.code.push_back({bytecodeStruct::opCode::opIncrement, {}});
                break;
            case lexer::token::tokenKind::decrementSign:
                currentBlock.code.push_back({bytecodeStruct::opCode::opDecrement, {}});
                break;
            case lexer::token::tokenKind::asterisk:
                currentBlock.code.push_back({bytecodeStruct::opCode::deepCopy, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildMulExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::asterisk:
                currentBlock.code.push_back({bytecodeStruct::opCode::opMul, {}});
                break;
            case lexer::token::tokenKind::slash:
                currentBlock.code.push_back({bytecodeStruct::opCode::opDiv, {}});
                break;
            case lexer::token::tokenKind::percentSign:
                currentBlock.code.push_back({bytecodeStruct::opCode::opMod, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildAddExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::plus:
                currentBlock.code.push_back({bytecodeStruct::opCode::opAdd, {}});
                break;
            case lexer::token::tokenKind::minus:
                currentBlock.code.push_back({bytecodeStruct::opCode::opSub, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildBinShiftExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::binaryShiftLeft:
                currentBlock.code.push_back({bytecodeStruct::opCode::opBinaryShiftLeft, {}});
                break;
            case lexer::token::tokenKind::binaryShiftRight:
                currentBlock.code.push_back({bytecodeStruct::opCode::opBinaryShiftRight, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildLogicEqExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::equal:
                currentBlock.code.push_back({bytecodeStruct::opCode::opEqual, {}});
                break;
            case lexer::token::tokenKind::notEqual:
                currentBlock.code.push_back({bytecodeStruct::opCode::opNotEqual, {}});
                break;
            case lexer::token::tokenKind::greaterEqual:
                currentBlock.code.push_back({bytecodeStruct::opCode::opGreaterEqual, {}});
                break;
            case lexer::token::tokenKind::lessEqual:
                currentBlock.code.push_back({bytecodeStruct::opCode::opLessEqual, {}});
                break;
            case lexer::token::tokenKind::greaterThan:
                currentBlock.code.push_back({bytecodeStruct::opCode::opGreaterThan, {}});
                break;
            case lexer::token::tokenKind::lessThan:
                currentBlock.code.push_back({bytecodeStruct::opCode::opLessThan, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildBinExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::binaryOr:
                currentBlock.code.push_back({bytecodeStruct::opCode::opBinaryOr, {}});
                break;
            case lexer::token::tokenKind::binaryAnd:
                currentBlock.code.push_back({bytecodeStruct::opCode::opBinaryAnd, {}});
                break;
            case lexer::token::tokenKind::binaryXor:
                currentBlock.code.push_back({bytecodeStruct::opCode::opBinaryXor, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildLogicAndExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);

        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::logicAnd:
                currentBlock.code.push_back({bytecodeStruct::opCode::opLogicAnd, {}});
                break;
            case lexer::token::tokenKind::logicOr:
                currentBlock.code.push_back({bytecodeStruct::opCode::opLogicOr, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildAssignmentExpr(const AST &target) {
        buildExpr(target.child[0]);
        buildExpr(target.child[2]);
        switch (target.child[1].leaf.kind) {
            case lexer::token::tokenKind::assignSign:
                currentBlock.code.push_back({bytecodeStruct::opCode::assign, {}});
                break;
            case lexer::token::tokenKind::additionAssignment:
                currentBlock.code.push_back({bytecodeStruct::opCode::addAssign, {}});
                break;
            case lexer::token::tokenKind::subtractionAssignment:
                currentBlock.code.push_back({bytecodeStruct::opCode::subAssign, {}});
                break;
            case lexer::token::tokenKind::multiplicationAssignment:
                currentBlock.code.push_back({bytecodeStruct::opCode::mulAssign, {}});
                break;
            case lexer::token::tokenKind::divisionAssignment:
                currentBlock.code.push_back({bytecodeStruct::opCode::divAssign, {}});
                break;
            case lexer::token::tokenKind::reminderAssignment:
                currentBlock.code.push_back({bytecodeStruct::opCode::modAssign, {}});
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildBlockStmt(const AST &target) {
        currentBlock.code.push_back({bytecodeStruct::opCode::pushLocalCxt, {}});
        std::for_each(target.child.begin(), target.child.end(), [&](const AST &ast) {
            buildStmt(ast);
        });
        currentBlock.code.push_back({bytecodeStruct::opCode::popLocalCxt, {}});
    }

    void codeBuilder::buildLetStmt(const AST &target) {
        std::for_each(target.child.begin(), target.child.end(), [&](const AST &ast) {
            buildExpr(target.child[1]);
            currentBlock.code.push_back(
                    {bytecodeStruct::opCode::createOrAssign, {currentBlock.putNames(ast.child[0].leaf.strVal)}});
        });
    }

    void codeBuilder::buildWhileStmt(const AST &target) {
        uint64_t begin = getNextCur();
        buildExpr(target.child[0]);
        uint64_t jumpOutOfLoopWithCond = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jumpIfFalse, {(vint) 114514}});
        buildStmt(target.child[1]);
        uint64_t jumpBack = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jump, {(vint) (begin - jumpBack)}});
        uint64_t end = getNextCur();
        currentBlock.code[jumpOutOfLoopWithCond].opargs = (vint) (end - jumpOutOfLoopWithCond);
        uint64_t curIdx = 0;
        std::for_each(currentBlock.code.begin(), currentBlock.code.end(), [&](auto &i) {
            if (i.opcode == bytecodeStruct::opCode::fakeOpBreak)
                i = {bytecodeStruct::opCode::jump, (vint) (end - curIdx)};
            if (i.opcode == bytecodeStruct::opCode::fakeOpContinue)
                i = {bytecodeStruct::opCode::jump, (vint) (jumpBack - curIdx)};
            curIdx++;
        });
    }

    void codeBuilder::buildForStmt(const AST &target) {
        currentBlock.code.push_back({bytecodeStruct::opCode::pushLocalCxt, {}});
        // init stmt
        buildStmt(target.child[0]);
        // cond stmt
        uint64_t begin = getNextCur();
        buildExpr(target.child[1]);
        uint64_t jumpOutOfLoopWithCond = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jumpIfFalse, {(vint) 114514}});
        // block stmt
        buildStmt(target.child[3]);
        uint64_t endBlock = getNextCur();
        buildStmt(target.child[2]);
        uint64_t jumpBack = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jump, {(vint) 114514}});
        uint64_t end = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::popLocalCxt, {}});
        // fill the jump positions
        currentBlock.code[jumpOutOfLoopWithCond].opargs = (vint) (end - jumpOutOfLoopWithCond);
        currentBlock.code[jumpBack].opargs = (vint) (begin - jumpBack);
        // replace fake op
        uint64_t curIdx = 0;
        std::for_each(currentBlock.code.begin(), currentBlock.code.end(), [&](auto &i) {
            if (i.opcode == bytecodeStruct::opCode::fakeOpBreak)
                i = {bytecodeStruct::opCode::jump, (vint) (end - curIdx)};
            if (i.opcode == bytecodeStruct::opCode::fakeOpContinue)
                i = {bytecodeStruct::opCode::jump, (vint) (jumpBack - curIdx)};
            curIdx++;
        });
    }

    void codeBuilder::buildForEachStmt(const AST &target) {
        // leave it empty
    }

    void codeBuilder::buildIfStmt(const AST &target) {
        buildExpr(target.child[0]);
        uint64_t jumpStmt = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jumpIfFalse, {(vint) 114514}});
        buildStmt(target.child[1]);
        uint64_t end = getNextCur();
        currentBlock.code[jumpStmt].opargs = (vint) (end - jumpStmt);
    }

    void codeBuilder::buildIfElseStmt(const AST &target) {
        buildExpr(target.child[0]);
        uint64_t jumpStmt = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jumpIfFalse, {(vint) 114514}});
        buildStmt(target.child[1]);
        uint64_t jumpOutOfElse = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jump, {(vint) 114514}});
        uint64_t elseBlock = getNextCur();
        buildStmt(target.child[2]);
        uint64_t end = getNextCur();

        currentBlock.code[jumpStmt].opargs = (vint) (elseBlock - jumpStmt);
        currentBlock.code[jumpOutOfElse].opargs = (vint) (end - jumpOutOfElse);
    }

    void codeBuilder::buildReturnStmt(const AST &target) {
        buildExpr(target.child[0]);
        currentBlock.code.push_back({bytecodeStruct::opCode::ret, {}});
    }

    void codeBuilder::buildContinueStmt(const AST &target) {
        currentBlock.code.push_back({bytecodeStruct::opCode::fakeOpContinue, {}});
    }

    void codeBuilder::buildBreakStmt(const AST &target) {
        currentBlock.code.push_back({bytecodeStruct::opCode::fakeOpBreak, {}});
    }

    void codeBuilder::buildTryCatchStmt(const AST &target) {
        uint64_t pushEventHandler = getNextCur();
        // try
        currentBlock.code.push_back({bytecodeStruct::opCode::pushExceptionHandler, {(vint) 114514}});
        buildStmt(target.child[0]);
        currentBlock.code.push_back({bytecodeStruct::opCode::popExceptionHandler, {}});
        uint64_t jumpOut = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::jump, {(vint) 114514}});
        // catch
        uint64_t catchBlock = getNextCur();
        currentBlock.code.push_back({bytecodeStruct::opCode::pushLocalCxt, {}});
        currentBlock.code.push_back({bytecodeStruct::opCode::createOrAssign, {currentBlock.putNames(target.child[1].leaf.strVal)}});
        buildStmt(target.child[2]);
        currentBlock.code.push_back({bytecodeStruct::opCode::popLocalCxt, {}});
        // fill the addresses
        uint64_t end = getNextCur();
        currentBlock.code[pushEventHandler].opargs = (vint) (catchBlock - pushEventHandler);
        currentBlock.code[jumpOut].opargs = (vint) (end - jumpOut);
    }

    void codeBuilder::buildThrowStmt(const AST &target) {
        buildExpr(target.child[0]);
        currentBlock.code.push_back({bytecodeStruct::opCode::opThrow, {}});
    }

    void codeBuilder::buildFuncDefStmt(const AST &target) {
        const vstr &funcName = target.child[0].leaf.strVal;
        std::for_each(target.child[1].child.rbegin(), target.child[1].child.rend(), [&](const AST &ast) {
            currentBlock.code.push_back({bytecodeStruct::opCode::putIndex, {currentBlock.putNames(ast.leaf.strVal)}});
        });
        codeBuilder cb(mod, mod.codeStructs[mod.putCodeStruct({})]);
        cb.buildStmt(target.child[2]);
        currentBlock.code.push_back({bytecodeStruct::opCode::funcNew, {(uint64_t) target.child[1].child.size()}});
        currentBlock.code.push_back({bytecodeStruct::opCode::createOrAssign, {currentBlock.putNames(funcName)}});
    }

    void codeBuilder::buildStmt(const AST &target) {
        switch (target.kind) {
            case AST::treeKind::identifier:
            case AST::treeKind::subscriptExpression:
            case AST::treeKind::invokingExpression:
            case AST::treeKind::listLiteral:
            case AST::treeKind::objectLiteral:
            case AST::treeKind::lambdaDefinition:
            case AST::treeKind::functionDefinition:
            case AST::treeKind::memberExpression:
            case AST::treeKind::uniqueExpression:
            case AST::treeKind::multiplicationExpression:
            case AST::treeKind::additionExpression:
            case AST::treeKind::binaryShiftExpression:
            case AST::treeKind::logicEqualExpression:
            case AST::treeKind::binaryExpression:
            case AST::treeKind::logicAndExpression:
                buildExpr(target);
                currentBlock.code.push_back({bytecodeStruct::opCode::popTop, {}});
                break;
            case AST::treeKind::assignmentExpression:
                buildAssignmentExpr(target);
                break;
            case AST::treeKind::blockStmt:
                buildBlockStmt(target);
                break;
            case AST::treeKind::letStmt:
                buildLetStmt(target);
                break;
            case AST::treeKind::whileStmt:
                buildWhileStmt(target);
                break;
            case AST::treeKind::forStmt:
                buildForStmt(target);
                break;
            case AST::treeKind::rangeBasedForStmt:
                buildForEachStmt(target);
                break;
            case AST::treeKind::ifStmt:
                buildIfStmt(target);
                break;
            case AST::treeKind::ifElseStmt:
                buildIfElseStmt(target);
                break;
            case AST::treeKind::returnStmt:
                buildReturnStmt(target);
                break;
            case AST::treeKind::continueStmt:
                buildContinueStmt(target);
                break;
            case AST::treeKind::breakStmt:
                buildBreakStmt(target);
                break;
            case AST::treeKind::tryCatchStmt:
                buildTryCatchStmt(target);
                break;
            case AST::treeKind::throwStmt:
                buildThrowStmt(target);
                break;
            case AST::treeKind::functionDefStmt:
                buildFuncDefStmt(target);
                break;
            default:
                break;
        }
    }

    void codeBuilder::buildExpr(const AST &target) {
        switch (target.kind) {
            case AST::treeKind::identifier:
                buildIdentifier(target);
                break;
            case AST::treeKind::subscriptExpression:
                buildSubscriptExpr(target);
                break;
            case AST::treeKind::invokingExpression:
                buildInvokeExpr(target);
                break;
            case AST::treeKind::basicLiterals:
                buildBasicLiterals(target);
                break;
            case AST::treeKind::listLiteral:
                buildListLiteral(target);
                break;
            case AST::treeKind::objectLiteral:
                buildObjectLiteral(target);
                break;
            case AST::treeKind::lambdaDefinition:
                buildLambdaDef(target);
                break;
            case AST::treeKind::functionDefinition:
                buildFuncDef(target);
                break;
            case AST::treeKind::memberExpression:
                buildMemberExpr(target);
                break;
            case AST::treeKind::uniqueExpression:
                buildUniqueExpr(target);
                break;
            case AST::treeKind::multiplicationExpression:
                buildMulExpr(target);
                break;
            case AST::treeKind::additionExpression:
                buildAddExpr(target);
                break;
            case AST::treeKind::binaryShiftExpression:
                buildBinShiftExpr(target);
                break;
            case AST::treeKind::logicEqualExpression:
                buildLogicEqExpr(target);
                break;
            case AST::treeKind::binaryExpression:
                buildBinExpr(target);
                break;
            case AST::treeKind::logicAndExpression:
                buildLogicAndExpr(target);
                break;
            default:
                break;
        }
    }
}
