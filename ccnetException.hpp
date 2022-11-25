#ifndef CCNETEXCEPTION_H
#define CCNETEXCEPTION_H

#include "ccnet_types.hpp"

const int EXCEPTION_BAD_CRC_IN_RESPONSE = 1;
// Exception
class ccnetException : public std::exception
{
    std::string m_error;
    std::string str;
    int code;

public:
    ccnetException() : code(0), m_error("")
    {
        str = m_error + std::string(": ") + std::to_string(code);
    };

    ccnetException(std::string error) : m_error(error), code(0)
    {
        str = m_error + std::string(": ") + std::to_string(code);
    };

    ccnetException(std::string error, int _code) : code(_code), m_error(error)
    {
        str = m_error + std::string(": ") + std::to_string(code);
    };

    int getCode()
    {
        return code;
    }

    const char *what() const noexcept override
    {
        return str.c_str();
    }
};

#endif // CCNETEXCEPTION_H