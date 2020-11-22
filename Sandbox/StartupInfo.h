#pragma once

#include <memory>

#include <Windows.h>

class StartupInfo
{
private:
	STARTUPINFOEXW startupInfo;
	std::unique_ptr<std::int8_t[]> attributeList;

public:
	StartupInfo(int attributesCount);
	virtual ~StartupInfo();

	void UpdateAttribute(DWORD_PTR attribute, PVOID value, SIZE_T size);
	LPSTARTUPINFOEXW get();
};
