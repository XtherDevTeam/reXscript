//
// Created by XIaokang00010 on 2022/12/2.
//

#include "signalException.hpp"

#include <utility>

namespace rex {
    signalException::signalException(const value &v) : msg(v) {

    }

    value &signalException::get() {
        return msg;
    }
} // rex