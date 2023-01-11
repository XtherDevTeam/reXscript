//
// Created by XIaokang00010 on 2023/1/11.
//

#include "importError.hpp"

namespace rex {
    importError::importError(const vstr& reason) : rexException(reason) {

    }

    const char *importError::what() const noexcept {
        return rexException::what();
    }
} // rex