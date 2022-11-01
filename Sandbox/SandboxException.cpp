#include "SandboxException.h"

SandboxException::SandboxException(const char* message, DWORD errorCode)
	: std::runtime_error(message), ErrorCode(errorCode)
{
}

BSTR SandboxException::AllocErrorMessage() const
{
    // https://stackoverflow.com/a/5298344/1135019
    const char* input = this->what();

    BSTR result = NULL;
    int lenA = lstrlenA(input);
    int lenW = ::MultiByteToWideChar(CP_ACP, 0, input, lenA, NULL, 0);
    if (lenW > 0)
    {
        result = ::SysAllocStringLen(0, lenW);
        ::MultiByteToWideChar(CP_ACP, 0, input, lenA, result, lenW);
    }

    return result;
}
