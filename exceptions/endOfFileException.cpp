//
// Created by XIaokang00010 on 2022/11/13.
//

#include "endOfFileException.hpp"

namespace rex {
    endOfFileException::endOfFileException() : rexException(L"Stream returns an EoF.") {}
} // rex