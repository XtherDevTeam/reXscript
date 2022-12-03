//
// Created by XIaokang00010 on 2022/11/13.
//

#ifndef REXSCRIPT_SHARE_HPP
#define REXSCRIPT_SHARE_HPP

#include <cstdint>
#include <string>
#include <share/localr.hpp>
#include <map>
#include <sstream>

namespace rex {
    using vsize = unsigned long long;
    using vint = int64_t;
    using vbool = bool;
    using vdeci = float;
    using vstr = std::wstring;
    using vchar = wchar_t;
    template<typename T>
    using vec = std::vector<T>;
    template<typename A, typename B>
    using map = std::map<A, B>;

    std::string wstring2string(const std::wstring &str);

    std::wstring string2wstring(const std::string &str);

    std::wstring buildErrorMessage(vsize line, vsize col, const vstr &what);

    template<typename T>
    using unsafePtr = T *;
    using unknownPtr = unsafePtr<void *>;
//    using rexNativeFunc = std::function<void(st)

    template<typename T>
    struct managedPtr {
        struct base {
            rex::vsize refCount;
            T v;

            base() : refCount(0), v() {}

            base(T &v) : refCount(0), v(v) {}

            base(const T &v) : refCount(0), v(v) {}
        } *ptr;


        managedPtr<T>(T &v) {
            ptr = new base(v);
            ptr->refCount++;
        }

        managedPtr<T>(const T &v) {
            ptr = new base(v);
            ptr->refCount++;
        }

        managedPtr<T>(managedPtr<T> &v) {
            ptr = v.ptr;
            ptr->refCount++;
        }

        managedPtr<T>(const managedPtr<T> &v) {
            ptr = v.ptr;
            ptr->refCount++;
        }

        template<typename T1>
        managedPtr<T>(const managedPtr<T1> &v) {
            ptr = (managedPtr<T>::base *) v.ptr;
            ptr->refCount++;
        }

        ~managedPtr<T>() {
            ptr->refCount--;
            if (!ptr->refCount) {
                delete ptr;
            }
        }

        T &operator()() {
            return ptr->v;
        }
    };
}

#endif //REXSCRIPT_SHARE_HPP
