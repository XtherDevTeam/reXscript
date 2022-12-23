//
// Created by XIaokang00010 on 2022/12/22.
//

#ifndef REXSCRIPT_FFI_HPP
#define REXSCRIPT_FFI_HPP

#include "share/share.hpp"

namespace rex::ffi {
    struct ffiDescriptor {
        struct ffiType {
            vstr cStyle;
            vstr typeToRex;
            vstr rexToType;
        };
        struct ffiExport {
            vstr name;
            vec<vstr> params;
            vstr resultType;
        };
        struct ffiConsts {
            vstr name;
            vstr type;
            vstr value;
        };

        vstr lib;

        map<vstr, ffiType> ffiTypes;

        map<vstr, ffiExport> ffiExports;

        map<vstr, ffiConsts> ffiConsts;

        vstr helpers;

        void fromFile(const vstr &path);

        vstr generateHeader();
    };
}

#endif //REXSCRIPT_FFI_HPP
