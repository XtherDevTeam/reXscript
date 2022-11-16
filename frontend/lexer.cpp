//
// Created by XIaokang00010 on 2022/11/13.
//

#include "lexer.hpp"
#include <exceptions/unexpectedTokenException.hpp>
#include <exceptions/endOfFileException.hpp>

namespace rex {
    lexer::lexer(std::wistream &ss) : stream(ss), line(0), col(0), curCh() {
        getCh();
    }

    void lexer::getCh() {
        vchar backup = curCh;
        curCh = stream.get();
        if (curCh == -1) {
            curCh = L'\0';
            if (!backup)
                throw endOfFileException();
        }
        if (curCh == L'\n') {
            line++, col = 0;
        } else {
            col++;
        }
    }

    lexer::token lexer::scan() {
        if (curCh == '\0')
            return {line, col, token::tokenKind::eof, L""};
        while (curCh == ' ' or curCh == '\n' or curCh == '\r' or curCh == '\t') getCh();
        if (std::isalpha(curCh)) {
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
        } else {
            throw unexpectedTokenException(line, col, curCh);
        }
    }

    lexer::token lexer::alphaStart() {
        lexer::token tok{line, col, token::tokenKind::identifier, vstr()};
        tok.value += curCh;
        getCh();
        while (isalpha(curCh) or isdigit(curCh) or curCh == L'_') {
            tok.value += curCh;
            getCh();
        }

        // 关键词处理
        if (tok.value == L"return") {
            tok.kind = token::tokenKind::kReturn;
        } else if (tok.value == L"continue") {
            tok.kind = token::tokenKind::kContinue;
        } else if (tok.value == L"break") {
            tok.kind = token::tokenKind::kBreak;
        } else if (tok.value == L"for") {
            tok.kind = token::tokenKind::kFor;
        } else if (tok.value == L"forEach") {
            tok.kind = token::tokenKind::kForEach;
        } else if (tok.value == L"while") {
            tok.kind = token::tokenKind::kWhile;
        } else if (tok.value == L"func") {
            tok.kind = token::tokenKind::kFunc;
        } else if (tok.value == L"object") {
            tok.kind = token::tokenKind::kObject;
        } else if (tok.value == L"closure") {
            tok.kind = token::tokenKind::kClosure;
        } else if (tok.value == L"var") {
            tok.kind = token::tokenKind::kVar;
        } else if (tok.value == L"import") {
            tok.kind = token::tokenKind::kImport;
        } else if (tok.value == L"as") {
            tok.kind = token::tokenKind::kAs;
        } else if (tok.value == L"in") {
            tok.kind = token::tokenKind::kIn;
        } else if (tok.value == L"if") {
            tok.kind = token::tokenKind::kIf;
        } else if (tok.value == L"else") {
            tok.kind = token::tokenKind::kElse;
        } else if (tok.value == L"true" or tok.value == L"false") {
            tok.kind = token::tokenKind::boolean;
        }

        return tok;
    }

    lexer::token lexer::strStart() {
        lexer::token tok{line, col, token::tokenKind::string, vstr()};
        getCh();
        while (curCh != '"') {
            if (curCh == '\\') {
                getCh();
                tok.value += '\\';
            }
            tok.value += curCh;
            getCh();
        }
        getCh(); // skip "
        return tok;
    }

    lexer::token lexer::digitStart() {
        lexer::token tok{line, col, token::tokenKind::integer, vstr()};
        tok.value += curCh;
        getCh();
        while (isdigit(curCh)) {
            tok.value += curCh;
            getCh();
        }
        if (curCh == '.') {
            tok.kind = token::tokenKind::decimal;
            tok.value += curCh;
            while (isdigit(curCh)) {
                tok.value += curCh;
            }
        }
        return tok;
    }

    lexer::token lexer::minusStart() {
        lexer::token tok{line, col, token::tokenKind::minus, L"-"};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::subtractionAssignment;
            tok.value += '=';
            getCh();
        } else if (curCh == '-') {
            tok.kind = token::tokenKind::decrementSign;
            tok.value += '-';
            getCh();
        } else if (curCh == '>') {
            tok.kind = token::tokenKind::toSign;
            tok.value += '>';
            getCh();
        }
        return tok;
    }

    lexer::token lexer::plusStart() {
        lexer::token tok{line, col, token::tokenKind::plus, L"+"};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::additionAssignment;
            tok.value += '=';
            getCh();
        } else if (curCh == '+') {
            tok.kind = token::tokenKind::decrementSign;
            tok.value += '+';
            getCh();
        }
        return tok;
    }

    lexer::token lexer::asteriskStart() {
        lexer::token tok{line, col, token::tokenKind::asterisk, L"*"};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::multiplicationAssignment;
            tok.value += '=';
            getCh();
        }
        return tok;
    }

    lexer::token lexer::slashStart() {
        lexer::token tok{line, col, token::tokenKind::slash, L"/"};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::divisionAssignment;
            tok.value += '=';
            getCh();
        } else if (curCh == '/') {
            getCh();
            while (curCh and curCh != '\n')
                getCh();
            return scan(); // 单行注释解析
        } else if (curCh == '*') {
            getCh();
            while (curCh) {
                if (curCh  == '*') {
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
        lexer::token tok{line, col, token::tokenKind::percentSign, L"%"};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::reminderAssignment;
            tok.value += '=';
            getCh();
        }
        return tok;
    }

    lexer::token lexer::equalStart() {
        lexer::token tok{line, col, token::tokenKind::assignSign, L"="};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::equal;
            tok.value += '=';
            getCh();
        }
        return tok;
    }

    lexer::token lexer::notStart() {
        lexer::token tok{line, col, token::tokenKind::logicNot, L"!"};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::notEqual;
            tok.value += '=';
            getCh();
        }
        return tok;
    }

    lexer::token lexer::lessStart() {
        lexer::token tok{line, col, token::tokenKind::lessThan, L"<"};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::lessEqual;
            tok.value += '=';
            getCh();
        }
        return tok;
    }

    lexer::token lexer::greaterStart() {
        lexer::token tok{line, col, token::tokenKind::greaterThan, L">"};
        getCh();
        if (curCh == '=') {
            tok.kind = token::tokenKind::greaterEqual;
            tok.value += '=';
            getCh();
        }
        return tok;
    }

    lexer::token lexer::semicolonStart() {
        lexer::token tok{line, col, token::tokenKind::semicolon, L";"};
        getCh();
        return tok;
    }

    lexer::token lexer::colonStart() {
        lexer::token tok{line, col, token::tokenKind::colon, L":"};
        getCh();
        return tok;
    }

    lexer::token lexer::commaStart() {
        lexer::token tok{line, col, token::tokenKind::comma, L","};
        getCh();
        return tok;
    }

    lexer::token lexer::dotStart() {
        lexer::token tok{line, col, token::tokenKind::dot, L"."};
        getCh();
        return tok;
    }

    lexer::token lexer::leftParenthesesStart() {
        lexer::token tok{line, col, token::tokenKind::leftParentheses, L"("};
        getCh();
        return tok;
    }

    lexer::token lexer::rightParenthesesStart() {
        lexer::token tok{line, col, token::tokenKind::rightParentheses, L")"};
        getCh();
        return tok;
    }

    lexer::token lexer::leftBracketStart() {
        lexer::token tok{line, col, token::tokenKind::leftBracket, L"["};
        getCh();
        return tok;
    }

    lexer::token lexer::rightBracketStart() {
        lexer::token tok{line, col, token::tokenKind::rightBracket, L"]"};
        getCh();
        return tok;
    }

    lexer::token lexer::leftBracesStart() {
        lexer::token tok{line, col, token::tokenKind::leftBraces, L"{"};
        getCh();
        return tok;
    }

    lexer::token lexer::rightBracesStart() {
        lexer::token tok{line, col, token::tokenKind::rightBraces, L"}"};
        getCh();
        return tok;
    }

    void lexer::saveState() {
        states.push_back({line, col, (vsize)stream.tellg(), curCh, curToken});
    }

    void lexer::returnState() {
        lexerState &state = states.back();
        line = state.line, col = state.col, curCh = state.curCh, curToken = state.curToken;
        stream.seekg(static_cast<long long>(state.pos));
        dropState();
    }

    void lexer::dropState() {
        states.pop_back();
    }
} // rex