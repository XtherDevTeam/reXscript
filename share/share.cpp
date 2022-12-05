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

void rex::parseString(std::wistream &input, rex::vstr &value) {
    vchar ch = '\0';
    while (input) {
        if(!input.get(ch)) break;
        if (ch == '\\') {
            if(!input.get(ch)) break;
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
                        if(!input.get(ch)) break;
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
