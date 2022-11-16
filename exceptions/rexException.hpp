//
// Created by XIaokang00010 on 2022/11/13.
//

#ifndef REXSCRIPT_REXEXCEPTION_HPP
#define REXSCRIPT_REXEXCEPTION_HPP

#include <exception>
#include <string>
#include <share/share.hpp>

namespace rex {

    class rexException : std::exception {
        std::string why;
    public:
        rexException(const vstr& str);

        const char * what() const noexcept override;
    };

} // rex

#endif //REXSCRIPT_REXEXCEPTION_HPP
