#pragma once
#include <vector>
#include <string>
#include <stdarg.h>

namespace StringUtil
{
    inline void DivideCommand(std::vector<std::string>& _vec_command, std::string& _str_command, const char _token = ' ')
    {
        int off = 0;
        for (auto i = 0; i < _str_command.size(); ++i)
        {
            if (_str_command.at(i) == _token)
            {
                _vec_command.emplace_back(_str_command.substr(off, i - off));
                off = i + 1;
            }
        }
        _vec_command.emplace_back(_str_command.substr(off, _str_command.size()));

    }

    inline int Replace(std::string& str, const char* pszFrom, size_t unFromLen, const char* pszTo, size_t unToLen)
    {
        int cnt = 0;
        size_t nPos = 0;

        while (true)
        {
            nPos = str.find(pszFrom, nPos);
            if (nPos == std::string::npos) break;

            str.replace(nPos, unFromLen, pszTo);
            nPos += unToLen;

            cnt++;
        }

        return cnt;
    }
    inline int Replace(std::string& str, const char* pszFrom, const char* pszTo)
    {
        return Replace(str, pszFrom, strlen(pszFrom), pszTo, strlen(pszTo));
    }

    template< typename _Elem > struct str_api {};
    template<>
    struct str_api< char >
    {
        static int vsnprintf(char* _Dest, size_t len, const char* fmt, va_list& args) { return _vsnprintf_s(_Dest, len, len - 1, fmt, args); }
        static int vsprintf_p(char* _Dest, size_t len, const char* fmt, va_list& args) { return _vsprintf_p(_Dest, len - 1, fmt, args); }
    };
    template< typename _Elem >
    inline std::basic_string< _Elem > format(const _Elem* fmt, ...)
    {
        _Elem buf[128];
        va_list args;
        va_start(args, fmt);

        _Elem* p = buf;
        size_t buf_len = sizeof(buf) / sizeof(_Elem);

        while (-1 == str_api< _Elem >::vsnprintf(p, buf_len, fmt, args))
        {
            if (p != buf) delete[] p;

            buf_len *= 2;

            p = new _Elem[buf_len];
        }

        std::basic_string< _Elem > strTemp = p;

        if (p != buf) delete[] p;

        return strTemp;
    }
};
