//
// Created by XIaokang00010 on 2022/11/15.
//

#include "signalReturn.hpp"

#include <utility>

namespace rex {
    signalReturn::signalReturn(const value &v) : msg(v) {

    }

    const value &signalReturn::get() {
        return msg;
    }
} // rex