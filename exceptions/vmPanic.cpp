//
// Created by XIaokang00010 on 2023/1/20.
//

#include "vmPanic.hpp"

namespace rex {
    vmPanic::vmPanic(const vstr &reason) : rexException(reason) {

    }

    const char *vmPanic::what() const noexcept {
        return rexException::what();
    }
} // rex