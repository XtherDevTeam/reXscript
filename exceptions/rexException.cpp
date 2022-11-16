//
// Created by XIaokang00010 on 2022/11/13.
//

#include "rexException.hpp"

namespace rex {
    rexException::rexException(const vstr &str) : why(wstring2string(str)) {}

    const char *rexException::what() const noexcept {
        return why.c_str();
    }
} // rex