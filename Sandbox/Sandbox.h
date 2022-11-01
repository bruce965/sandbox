#pragma once

#include <string>

#include <Windows.h>
#include <guiddef.h>

#include "MessageType.h"

class Sandbox
{
private:
	std::wstring appContainerName;
	PSID sid;

public:
	Sandbox(const std::wstring &name, const GUID &guid);
	virtual ~Sandbox();

	void StartProcess(const std::wstring &commandLine);
	bool NextMessage(MessageType *type, void** data);
};
