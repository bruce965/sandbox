#pragma once

#include <stdexcept>

#include <wtypes.h>
#include <comutil.h>


class SandboxException :
    public std::runtime_error
{
public:
    const DWORD ErrorCode;

    SandboxException(const char* message, DWORD errorCode);

    BSTR AllocErrorMessage() const;
};
