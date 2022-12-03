//
// Created by XIaokang00010 on 2022/11/13.
//

#include "parserException.hpp"

namespace rex {
    parserException::parserException(vsize line, vsize col, const vstr &reason) : rexException(
            buildErrorMessage(line, col, reason)) {

    }
} // rex