#include "StartupInfo.h"

#include "SandboxException.h"

StartupInfo::StartupInfo(int attributesCount)
{
	SIZE_T attributeSize = 0;
	InitializeProcThreadAttributeList(NULL, attributesCount, NULL, &attributeSize);
	this->attributeList = std::unique_ptr<std::int8_t[]>(new std::int8_t[attributeSize]);

	this->startupInfo = { 0 };
	this->startupInfo.StartupInfo.cb = sizeof(STARTUPINFOEXW);
	this->startupInfo.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)attributeList.get();

	if (!InitializeProcThreadAttributeList(startupInfo.lpAttributeList, attributesCount, NULL, &attributeSize))
	{
		throw SandboxException("Failed to initialize process attributes list", GetLastError());
	}
}

StartupInfo::~StartupInfo()
{
	if (this->attributeList)
	{
		DeleteProcThreadAttributeList((LPPROC_THREAD_ATTRIBUTE_LIST)attributeList.get());
	}
}

void StartupInfo::UpdateAttribute(DWORD_PTR attribute, PVOID value, SIZE_T size)
{
	if (!UpdateProcThreadAttribute(this->startupInfo.lpAttributeList, 0, attribute, value, size, NULL, NULL))
	{
		throw SandboxException("Failed to set process/thread attribute", GetLastError());
	}
}

LPSTARTUPINFOEXW StartupInfo::get() {
	return &this->startupInfo;
}
