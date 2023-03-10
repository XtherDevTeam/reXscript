//
// Created by XIaokang00010 on 2022/12/22.
//

#include "ffi.hpp"

namespace rex::ffi {
    void ffiDescriptor::fromFile(const vstr &path) {
        // read file
        vbytes u8Path = wstring2string(path);
        FILE *fp = fopen(u8Path.c_str(), "r+");
        if (!fp)
            throw std::runtime_error("ffiDescriptor::fromFile: fp == nullptr");
        fseek(fp, 0, SEEK_END);
        long pos = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        vbytes buf(pos, vchar{});
        fread(buf.data(), pos, 1, fp);
        fclose(fp);
        std::wstringstream ffiFile(string2wstring(buf));
        buf.clear();

        // parse file
        // 0 is global, 2 is types, 3 is types-parsing, 5 is exports, 7 is helpers, 8 is consts
        int mode{};
        vstr parsingTemp{};

        vstr line{};
        for (std::getline(ffiFile, line); ffiFile; std::getline(ffiFile, line)) {
            if (line.empty())
                continue;
            line.erase(0, line.find_first_not_of(L' '));
            line.erase(line.find_last_not_of(L' ') + 1);

            switch (mode) {
                case 0: {
                    // global
                    if (line.starts_with(L"ffi-lib: ")) {
                        lib = line.substr(9);
                    } else if (line.starts_with(L"ffi-types:")) {
                        mode = 2;
                    } else if (line.starts_with(L"ffi-exports:")) {
                        mode = 5;
                    } else if (line.starts_with(L"ffi-consts:")) {
                        mode = 8;
                    } else if (line.starts_with(L"ffi-helpers")) {
                        mode = 7;
                    } else {
                        throw std::runtime_error("ffiDescriptor::fromFile: invalid ffi config");
                    }
                    break;
                }
                case 2: {
                    if (line.starts_with(L":end-ffi-types")) {
                        mode = 0;
                        break;
                    }
                    // types
                    parsingTemp = line.substr(0, line.length() - 1);
                    mode = 3;
                    break;
                }
                case 3: {
                    // type-parsing
                    if (line.starts_with(L"c-style: ")) {
                        ffiTypes[parsingTemp].cStyle = line.substr(9);
                    } else if (line.starts_with(L"type-to-rex: ")) {
                        ffiTypes[parsingTemp].typeToRex = line.substr(13);
                    } else if (line.starts_with(L"rex-to-type: ")) {
                        ffiTypes[parsingTemp].rexToType = line.substr(13);
                    } else if (line.starts_with(L':' + parsingTemp)) {
                        mode = 2;
                    }
                    break;
                }
                case 5: {
                    // exports-parsing
                    if (line.starts_with(L"name: ")) {
                        parsingTemp = line.substr(6);
                    } else if (line.starts_with(L"symbol: ")) {
                        ffiExports[parsingTemp].name = line.substr(8);
                    } else if (line.starts_with(L"params: ")) {
                        split(line.substr(8), vstr{L","}, ffiExports[parsingTemp].params);
                        for (auto &i: ffiExports[parsingTemp].params) {
                            i.erase(0, i.find_first_not_of(L' '));
                            i.erase(i.find_last_not_of(L' ') + 1);
                        }
                    } else if (line.starts_with(L"result: ")) {
                        ffiExports[parsingTemp].resultType = line.substr(8);
                    } else if (line.starts_with(L":end-ffi-exports")) {
                        mode = 0;
                    }
                    break;
                }
                case 7: {
                    if (line.starts_with(L":end-ffi-helpers")) {
                        mode = 0;
                    } else {
                        helpers += line + L"\n";
                    }
                    break;
                }
                case 8: {
                    // exports-parsing
                    if (line.starts_with(L"name: ")) {
                        parsingTemp = line.substr(6);
                    } else if (line.starts_with(L"symbol: ")) {
                        ffiConsts[parsingTemp].name = line.substr(8);
                    } else if (line.starts_with(L"type: ")) {
                        ffiConsts[parsingTemp].type = line.substr(6);
                    } else if (line.starts_with(L"value: ")) {
                        ffiConsts[parsingTemp].value = line.substr(7);
                    } else if (line.starts_with(L":end-ffi-consts")) {
                        mode = 0;
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }

    vstr ffiDescriptor::generateHeader() {
        std::wstringstream ss;
        ss << L"// This file is auto generated by FFI generator for reXscript." << std::endl;
        ss << L"#include \"" << lib << "\"" << std::endl;
        ss << L"#include <rex.hpp>" << std::endl;
        ss << L"using namespace rex;" << std::endl;

        // export helpers
        ss << helpers << "\n";

        // export definitions
        for (auto &i: ffiExports) {
            ss << "nativeFn(_rexExport_" << i.first << ",interpreter,args,passThisPtr) {" << std::endl;
            // export arguments
            long argCnt{0};
            for (auto &item: i.second.params) {
                const auto &vi = ffiTypes[item];
                vstr replacement = L"args[" + std::to_wstring(argCnt) + L"]";
                replacement = L"(" + replacement + L".isRef() ? " + replacement + L".getRef() : " + replacement + L")";
                ss << vi.cStyle << L" arg" << argCnt << L" = "
                   << replaceAll(vi.rexToType, vstr{L"$param"}, replacement) << L";\n";
                argCnt++;
            }
            // export caller progress
            if (i.second.resultType != L"none")
                ss << L"auto rexCallResult = ";
            ss << i.second.name << L"(";
            if (argCnt) {
                ss << L"arg0";
                for (long c = 1; c < argCnt; c++) {
                    ss << L",arg" << c;
                }
            }
            ss << L");\n";
            // export return statement
            if (i.second.resultType != L"none") {
                ss << L"return "
                   << replaceAll(ffiTypes[i.second.resultType].typeToRex, vstr{L"$param"}, vstr{L"rexCallResult"})
                   << L";\n";
            } else {
                ss << L"return {};\n";
            }
            ss << L"}" << std::endl;
        }

        // export rexModInit
        ss << "extern \"C\" void rexModInit(const managedPtr<environment> &env, const managedPtr<value> &mod) {"
           << std::endl;
        for (auto &i: ffiExports) {
            ss << L"mod->members[L" << std::quoted(i.first) << L"] = "
               << L"managePtr(value{(value::nativeFuncPtr) _rexExport_" << i.first << L"});\n";
        }
        for (auto &i: ffiConsts) {
            vstr replacement =
                    replaceAll(ffiTypes[i.second.type].typeToRex, vstr{L"$param"}, vstr{i.second.value});
            ss << L"mod->members[L" << std::quoted(i.first) << L"] = " << L"managePtr("
               << replacement << L");" << std::endl;
        }
        ss << L"}" << std::endl;

        return ss.str();
    }
}
