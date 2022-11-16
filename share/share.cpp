//
// Created by XIaokang00010 on 2022/11/13.
//

#include <share/share.hpp>

std::string rex::wstring2string(const std::wstring &str) {
    return std::move(localr::string_convert<localr::codecvt<wchar_t, char>>::in(str));
}

std::wstring rex::string2wstring(const std::string &str) {
    return std::move(localr::string_convert<localr::codecvt<char, wchar_t>>::in(str));
}

std::wstring rex::buildErrorMessage(rex::vsize line, rex::vsize col, const rex::vstr &what) {
    std::wstringstream ss;
    ss << "At line " << line << " column " << col << ": " << what;
    return ss.str();
}