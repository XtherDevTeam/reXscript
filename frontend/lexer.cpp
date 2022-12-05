//
// Created by XIaokang00010 on 2022/11/13.
//

#include <iomanip>
#include "lexer.hpp"
#include <exceptions/unexpectedTokenException.hpp>
#include <exceptions/endOfFileException.hpp>

namespace rex {
    lexer::lexer(std::wistream &ss) : stream(ss), line(0), col(0), curCh() {
        getCh();
    }

    void lexer::getCh() {
        if (!stream) {
            throw endOfFileException();
        }
        if(!stream.get(curCh)) {
            curCh = '\0';
        }
        if (curCh == L'\n') {
            line++, col = 0;
        } else {
            col++;
        }
    }

    lexer::token lexer::scan() {
        if (curCh == '\0') {
            return {line, col, token::tokenKind::eof};
        }
        while (curCh == ' ' or curCh == '\n' or curCh == '\r' or curCh == '\t') getCh();
        if (std::isalpha(curCh) or curCh == '_') {
            return curToken = alphaStart();
        } else if (std::isdigit(curCh)) {
            return curToken = digitStart();
        } else if (curCh == L'+') {
            return curToken = plusStart();
        } else if (curCh == L'-') {
            return curToken = minusStart();
        } else if (curCh == L'*') {
            return curToken = asteriskStart();
        } else if (curCh == L'/') {
            return curToken = slashStart();
        } else if (curCh == L'%') {
            return curToken = percentSignStart();
        } else if (curCh == L'!') {
            return curToken = notStart();
        } else if (curCh == L'=') {
            return curToken = equalStart();
        } else if (curCh == L'>') {
            return curToken = greaterStart();
        } else if (curCh == L'<') {
            return curToken = lessStart();
        } else if (curCh == L'"') {
            return curToken = strStart();
        } else if (curCh == L';') {
            return curToken = semicolonStart();
        } else if (curCh == L':') {
            return curToken = colonStart();
        } else if (curCh == L',') {
            return curToken = commaStart();
        } else if (curCh == L'.') {
            return curToken = dotStart();
        } else if (curCh == L'(') {
            return curToken = leftParenthesesStart();
        } else if (curCh == L')') {
            return curToken = rightParenthesesStart();
        } else if (curCh == L'[') {
            return curToken = leftBracketStart();
        } else if (curCh == L']') {
            return curToken = rightBracketStart();
        } else if (curCh == L'{') {
            return curToken = leftBracesStart();
        } else if (curCh == L'}') {
            return curToken = rightBracesStart();
        } else if (curCh == '&') {
            return curToken = andStart();
        } else if (curCh == '|') {
            return curToken = orStart();
        } else if (curCh == '^') {
            return curToken = xorStart();
        } else {
            throw unexpectedTokenException(line, col, curCh);
        }
    }

    lexer::token lexer::alphaStart() {
        lexer::token tok{line, col, token::tokenKind::identifier, vstr()};
        vstr tempStr;
        tempStr += curCh;
        getCh();
        while (isalpha(curCh) or isdigit(curCh) or curCh == L'_') {
            tempStr += curCh;
            getCh();
        }

        // 关键词处理
        if (tempStr == L"return") {
            tok.kind = token::tokenKind::kReturn;
        } else if (tempStr == L"continue") {
            tok.kind = token::tokenKind::kContinue;
        } else if (tempStr == L"break") {
            tok.kind = token::tokenKind::kBreak;
        } else if (tempStr == L"for") {
            tok.kind = token::tokenKind::kFor;
        } else if (tempStr == L"forEach") {
            tok.kind = token::tokenKind::kForEach;
        } else if (tempStr == L"while") {
            tok.kind = token::tokenKind::kWhile;
        } else if (tempStr == L"func") {
            tok.kind = token::tokenKind::kFunc;
        } else if (tempStr == L"object") {
            tok.kind = token::tokenKind::kObject;
        } else if (tempStr == L"lambda") {
            tok.kind = token::tokenKind::kClosure;
        } else if (tempStr == L"let") {
            tok.kind = token::tokenKind::kLet;
        } else if (tempStr == L"as") {
            tok.kind = token::tokenKind::kAs;
        } else if (tempStr == L"in") {
            tok.kind = token::tokenKind::kIn;
        } else if (tempStr == L"if") {
            tok.kind = token::tokenKind::kIf;
        } else if (tempStr == L"else") {
            tok.kind = token::tokenKind::kElse;
        } else if (tempStr == L"try") {
            tok.kind = token::tokenKind::kTry;
        } else if (tempStr == L"catch") {
            tok.kind = token::tokenKind::kCatch;
        } else if (tempStr == L"throw") {
            tok.kind = token::tokenKind::kThrow;
        } else if (tempStr == L"true" or tempStr == L"false") {
            tok.kind = token::tokenKind::boolean;
            tok.basicVal.vBool = tempStr == L"true";
        } else {
            tok.strVal = tempStr;
        }

        return tok;
    }

    lexer::token lexer::strStart() {
        lexer::token tok{line, col, token::tokenKind::string, vstr()};
        getCh();
        while (curCh != '"') {
            if (curCh == '\\') {
                getCh();
                tok.strVal += '\\';
            }
            tok.strVal += curCh;
            getCh();
        }
        getCh(); // skip "
        std::wistringstream ss{tok.strVal};
        tok.strVal = {};
        parseString(ss, tok.strVal);
        return tok;
    }

    lexer::token lexer::digitStart() {
        lexer::token tok{line, col, token::tokenKind::integer, (vint) 0};
        vstr tempStr;
        tempStr += curCh;
        getCh();
        while (isdigit(curCh)) {
            tempStr += curCh;
            getCh();
        }
        if (curCh == '.') {
            tok.kind = token::tokenKind::decimal;
            tempStr += curCh;
            getCh();
            while (isdigit(curCh)) {
                tempStr += curCh;
                getCh();
            }
        }
        if (tok.kind == token::tokenKind::integer)
            tok.basicVal.vInt = std::stol(tempStr);
        else
            tok.basicVal.vDeci = std::stof(tempStr);
        return tok;
    }

    lexer::token lexer::minusStart() {
        lexer::token tok{line, col, token::tokenKind::minus};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::subtractionAssignment;
            getCh();
        } else if (curCh == '-') {
            tok.kind = token::tokenKind::decrementSign;
            getCh();
        } else if (curCh == '>') {
            tok.kind = token::tokenKind::toSign;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::plusStart() {
        lexer::token tok{line, col, token::tokenKind::plus};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::additionAssignment;
            getCh();
        } else if (curCh == '+') {
            tok.kind = token::tokenKind::decrementSign;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::asteriskStart() {
        lexer::token tok{line, col, token::tokenKind::asterisk};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::multiplicationAssignment;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::slashStart() {
        lexer::token tok{line, col, token::tokenKind::slash};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::divisionAssignment;
            getCh();
        } else if (curCh == '/') {
            getCh();
            while (curCh and curCh != '\n')
                getCh();
            return scan(); // 单行注释解析
        } else if (curCh == '*') {
            getCh();
            while (curCh) {
                if (curCh == '*') {
                    getCh();
                    if (curCh == '/') {
                        getCh();
                        break;
                    }
                }
                getCh();
            }
            return scan(); // 多行注释解析
        }
        return tok;
    }

    lexer::token lexer::percentSignStart() {
        lexer::token tok{line, col, token::tokenKind::percentSign};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::reminderAssignment;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::equalStart() {
        lexer::token tok{line, col, token::tokenKind::assignSign};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::equal;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::notStart() {
        lexer::token tok{line, col, token::tokenKind::logicNot};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::notEqual;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::lessStart() {
        lexer::token tok{line, col, token::tokenKind::lessThan};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::lessEqual;
            getCh();
        } else if (curCh == '<') {
            tok.kind = token::tokenKind::binaryShiftLeft;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::greaterStart() {
        lexer::token tok{line, col, token::tokenKind::greaterThan};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::greaterEqual;
            getCh();
        } else if (curCh == '>') {
            tok.kind = token::tokenKind::binaryShiftRight;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::semicolonStart() {
        lexer::token tok{line, col, token::tokenKind::semicolon};
        getCh();
        return tok;
    }

    lexer::token lexer::colonStart() {
        lexer::token tok{line, col, token::tokenKind::colon};
        getCh();
        return tok;
    }

    lexer::token lexer::commaStart() {
        lexer::token tok{line, col, token::tokenKind::comma};
        getCh();
        return tok;
    }

    lexer::token lexer::dotStart() {
        lexer::token tok{line, col, token::tokenKind::dot};
        getCh();
        return tok;
    }

    lexer::token lexer::leftParenthesesStart() {
        lexer::token tok{line, col, token::tokenKind::leftParentheses};
        getCh();
        return tok;
    }

    lexer::token lexer::rightParenthesesStart() {
        lexer::token tok{line, col, token::tokenKind::rightParentheses};
        getCh();
        return tok;
    }

    lexer::token lexer::leftBracketStart() {
        lexer::token tok{line, col, token::tokenKind::leftBracket};
        getCh();
        return tok;
    }

    lexer::token lexer::rightBracketStart() {
        lexer::token tok{line, col, token::tokenKind::rightBracket};
        getCh();
        return tok;
    }

    lexer::token lexer::leftBracesStart() {
        lexer::token tok{line, col, token::tokenKind::leftBraces};
        getCh();
        return tok;
    }

    lexer::token lexer::rightBracesStart() {
        lexer::token tok{line, col, token::tokenKind::rightBraces};
        getCh();
        return tok;
    }

    void lexer::saveState() {
        states.emplace_back(line, col, (vint) stream.tellg(), curCh, curToken);
    }

    void lexer::returnState() {
        stream.clear();
        lexerState &state = states.back();
        line = state.line, col = state.col, curCh = state.curCh, curToken = state.curToken;
        stream.seekg(static_cast<long long>(state.pos));
        dropState();
    }

    void lexer::dropState() {
        if (!states.empty())
            states.pop_back();
    }

    lexer::token lexer::andStart() {
        lexer::token tok{line, col, token::tokenKind::binaryAnd};
        getCh();
        if (curCh == '&') {
            tok.kind = token::tokenKind::logicAnd;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::orStart() {
        lexer::token tok{line, col, token::tokenKind::binaryOr};
        getCh();
        if (curCh == '|') {
            tok.kind = token::tokenKind::logicOr;
            getCh();
        }
        return tok;
    }

    lexer::token lexer::xorStart() {
        lexer::token tok{line, col, token::tokenKind::binaryXor};
        getCh();
        return tok;
    }

    lexer::token::vBasicValue::vBasicValue(vint v) : vInt(v) {

    }

    lexer::token::vBasicValue::vBasicValue(vdeci v) : vDeci(v) {

    }

    lexer::token::vBasicValue::vBasicValue(vbool v) : vBool(v) {

    }

    lexer::token::vBasicValue::vBasicValue() : vInt(0) {

    }


    lexer::token::token() : line(), col(), kind(), basicVal(), strVal() {

    }

    lexer::token::token(vsize line, vsize col, lexer::token::tokenKind kind) :
            line(line), col(col), kind(kind), basicVal(), strVal() {

    }

    lexer::token::token(vsize line, vsize col, lexer::token::tokenKind kind, lexer::token::vBasicValue basicVal) :
            line(line), col(col), kind(kind), basicVal(basicVal), strVal() {

    }

    lexer::token::token(vsize line, vsize col, lexer::token::tokenKind kind, vstr strVal) :
            line(line), col(col), kind(kind), basicVal(), strVal(std::move(strVal)) {

    }


    lexer::lexerState::lexerState() : line(), col(), pos(), curCh(), curToken() {

    }

    lexer::lexerState::lexerState(vsize line, vsize col, std::istream::pos_type pos, vchar curCh, lexer::token curToken)
            :
            line(line), col(col), pos(pos), curCh(curCh), curToken(std::move(curToken)) {

    }
} // rex