//
// Created by XIaokang00010 on 2022/11/13.
//

#include "unexpectedTokenException.hpp"

#include <utility>

namespace rex {
    unexpectedTokenException::unexpectedTokenException(vsize line, vsize col, vchar ch) : rexException(
            buildErrorMessage(line, col, vstr(L"Unexpected token: ") + ch)) {}
} // rex