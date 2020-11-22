#include "SandboxException.h"

SandboxException::SandboxException(const char* message, DWORD errorCode)
	: std::runtime_error(message), ErrorCode(errorCode)
{
}
