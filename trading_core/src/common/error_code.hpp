/**============================================================================
Name        : error_code.hpp
Created on  : 25.10.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Defines common error codes shared across Trading Core modules.
============================================================================**/

#ifndef FINANCETECHNOLOGYPROJECTS_ERROR_CODE_HPP
#define FINANCETECHNOLOGYPROJECTS_ERROR_CODE_HPP

namespace trading::common
{
    enum class ErrorCode
    {
        InvalidArgument,
        InvalidState,
        NotFound,
        Timeout,
        NetworkError,
        ParseError,
        ProtocolError,
        ResourceUnavailable,
        InternalError
    };
}

#endif // !FINANCETECHNOLOGYPROJECTS_RROR_CODE_HPP