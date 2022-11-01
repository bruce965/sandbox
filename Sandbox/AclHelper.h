#pragma once

#include <string>

#include <Windows.h>
#include <AccCtrl.h>

class AclHelper
{
public:
	static bool GrantPermissions(PSID sid, std::wstring objectName, SE_OBJECT_TYPE objectType, DWORD permissions);
};
