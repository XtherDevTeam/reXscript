//
// Created by XIaokang00010 on 2022/11/13.
//

#ifndef REXSCRIPT_STRINGTOLEXERPASS_HPP
#define REXSCRIPT_STRINGTOLEXERPASS_HPP

#include <pass/basicPass.hpp>
#include <share/share.hpp>
#include <frontend/lexer.hpp>

namespace rex {

    class stringToLexerPass : public basicPass<vstr, lexer> {
        std::wistringstream ss;
    public:
        explicit stringToLexerPass(vstr &before);

        lexer run() override;
    };

} // rex

#endif //REXSCRIPT_STRINGTOLEXERPASS_HPP
