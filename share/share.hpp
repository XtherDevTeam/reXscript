//
// Created by XIaokang00010 on 2022/11/13.
//

#ifndef REXSCRIPT_SHARE_HPP
#define REXSCRIPT_SHARE_HPP

#include <cstdint>
#include <string>
#include <share/utfutils.hpp>
#include <map>
#include <sstream>
#include <memory>
#include <functional>
#include <filesystem>

namespace rex {
    using vsize = unsigned long long;
    using vint = int64_t;
    using vbool = bool;
    using vdeci = float;
    using vstr = std::wstring;
    using vchar = wchar_t;
    using vbytes = std::string;
    using vbyte = char;
    template<typename T>
    using vec = std::vector<T>;
    template<typename A, typename B>
    using map = std::map<A, B>;

    std::string wstring2string(const std::wstring &str);

    std::wstring string2wstring(const std::string &str);

    std::wstring buildErrorMessage(vsize line, vsize col, const vstr &what);

    template<typename T>
    using unsafePtr = T *;

    template<typename T>
    using managedPtr = std::shared_ptr<T>;

    using unknownPtr = unsafePtr<void *>;

    template<typename T>
    managedPtr<T> managePtr(const T &v) {
        return std::make_shared<T>(v);
    }

    void parseString(std::wistream &input, vstr &value);

#define nativeFn(name, interpreter, args, passThisPtr) rex::value name(void *interpreter, rex::vec<rex::value> args, const rex::managedPtr<rex::value> &passThisPtr)

    namespace path {
        constexpr vchar sep = '/';

        void join(vstr &a, const vstr &b);
    };
}

#endif //REXSCRIPT_SHARE_HPP
