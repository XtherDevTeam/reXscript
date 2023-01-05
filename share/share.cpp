//
// Created by XIaokang00010 on 2022/11/13.
//

#include <share/share.hpp>
#include "share/whereami/whereami.h"

std::string rex::wstring2string(const std::wstring &str) {
    vbytes ret{};
    utf8Unicode::unicodeToUtf8(str, ret);
    return ret;
}

std::wstring rex::string2wstring(const std::string &str) {
    vstr ret{};
    utf8Unicode::utf8ToUnicode(str, ret);
    return ret;
}

std::wstring rex::string2wstring(const std::wstring &str) {
    return str;
}

std::wstring rex::buildErrorMessage(rex::vsize line, rex::vsize col, const rex::vstr &what) {
    std::wstringstream ss;
    ss << "At line " << line << " column " << col << ": " << what;
    return ss.str();
}

void rex::parseString(std::wistream &input, rex::vstr &value) {
    vchar ch = '\0';
    while (input) {
        if (!input.get(ch)) break;
        if (ch == '\\') {
            if (!input.get(ch)) break;
            switch (ch) {
                case '\\':
                case '"':
                case '\'':
                case '/':
                    value.push_back(ch);
                    break;
                case 'b':
                    value.push_back('\b');
                    break;
                case 'f':
                    value.push_back('\f');
                    break;
                case 'n':
                    value.push_back('\n');
                    break;
                case 'r':
                    value.push_back('\r');
                    break;
                case 't':
                    value.push_back('\t');
                    break;
                case 'u': {
                    vchar fuckutf{};
                    for (vint i = 3; input && i >= 0; i--) {
                        if (!input.get(ch)) break;
                        if ('a' <= ch and ch <= 'z')
                            fuckutf += ((ch - 'a' + 10) * (1 << 4 * i));
                        else if ('A' <= ch and ch <= 'Z')
                            fuckutf += ((ch - 'A' + 10) * (1 << 4 * i));
                        else
                            fuckutf += ((ch - '0') * (1 << 4 * i));
                    }
                    value.push_back(fuckutf);
                    break;
                }
                default:
                    value.push_back(ch);
                    break;
            }
        } else {
            value.push_back(ch);
        }
    }
}

rex::vstr rex::getOSName() {
    return operatingSystem;
}

rex::vstr rex::getCPUArch() {
    return architecture;
}

rex::vstr rex::getDylibSuffix() {
    return dylibSuffix;
}

rex::vbytes rex::getPrintableBytes(const rex::vbytes &src) {
    std::stringstream ss;
    for (auto &c: src) {
        if (isprint(c)) {
            ss << c;
        } else {
            ss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((unsigned char) c);
        }
    }
    return ss.str();
}

rex::vbytes rex::getRexExecPath() {
    std::string path;
    int length, dirnameLength;

    length = wai_getExecutablePath(nullptr, 0, &dirnameLength);
    path.resize(length + 1);
    wai_getExecutablePath(path.data(), length, &dirnameLength);
    path[length] = '\0';
    return path.substr(0, path.rfind(std::filesystem::path::preferred_separator));
}

void rex::path::join(rex::vstr &a, const rex::vstr &b) {
    if (a.empty()) {
        a = b;
        return;
    }
    if (b.empty()) {
        a = a;
        return;
    }
    if (a.back() == sep && b.front() == sep) {
        a = a + b.substr(1);
        return;
    }
    if (a.back() == sep || b.front() == sep) {
        a = a + b;
        return;
    }
    a = a + sep + b;
}

rex::vstr rex::path::getRealpath(const rex::vstr &path) {
    return string2wstring(std::filesystem::absolute(std::filesystem::path(wstring2string(path))).string());
}
