#include "Sandbox.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <memory>

#include <Windows.h>
#include <detours.h>
#include <UserEnv.h>
#include <sddl.h>

#include "SandboxException.h"
#include "StartupInfo.h"
#include "SecurityCapabilities.h"
#include "AclHelper.h"

#pragma comment(lib, "Userenv.lib")  // AppContainer

LPCSTR rlpDlls[] = {
	//"ntdll",
	"sandbox-hooks32.dll",
};

Sandbox::Sandbox(const std::wstring &name, const GUID &guid)
{
	const int GUID_LENGTH = 37;  // length of a guid in "000000-0000-0000-0000-000000000000" format

	WCHAR buff[GUID_LENGTH + 1];
	_snwprintf_s(buff, sizeof(buff), L"%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	this->appContainerName = name.substr(0, 64 - (GUID_LENGTH + 1)) + L"_" + buff;

	HRESULT result = CreateAppContainerProfile(
		this->appContainerName.c_str(),
		name.c_str(),
		name.c_str(),
		NULL,
		0,
		&this->sid);

	if (!SUCCEEDED(result))
	{
		if (HRESULT_CODE(result) == ERROR_ALREADY_EXISTS)
		{
			result = DeriveAppContainerSidFromAppContainerName(this->appContainerName.c_str(), &this->sid);

			if (!SUCCEEDED(result))
			{
				throw SandboxException("Failed to get existing AppContainer", HRESULT_CODE(result));
			}
		}
		else
		{
			throw SandboxException("Failed to create AppContainer", HRESULT_CODE(result));
		}
	}

	PSID sidAllAppPackages;
	if (!ConvertStringSidToSidW(L"S-1-15-2-1", &sidAllAppPackages))
	{
		throw SandboxException("Unable to retrieve SID for 'ALL APPLICATION PACKAGES'", HRESULT_CODE(result));
	}

	AclHelper::GrantPermissions(sidAllAppPackages, L"sandbox-hooks32.dll", SE_FILE_OBJECT, FILE_GENERIC_READ | FILE_GENERIC_EXECUTE);
	AclHelper::GrantPermissions(sidAllAppPackages, L"sandbox-hooks64.dll", SE_FILE_OBJECT, FILE_GENERIC_READ | FILE_GENERIC_EXECUTE);

	LocalFree(sidAllAppPackages);
}

Sandbox::~Sandbox()
{
	FreeSid(this->sid);

	DeleteAppContainerProfile(this->appContainerName.c_str());
}

void Sandbox::StartProcess(const std::wstring &commandLine)
{
	StartupInfo startupInfo(1);

	SecurityCapabilities securityCapabilities(
		this->sid,
		{
			WinCapabilityPrivateNetworkClientServerSid
		});

	startupInfo.UpdateAttribute(
		PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES,
		securityCapabilities.get(),
		SecurityCapabilities::size);

	// copy because it might be modified
	std::wstring commandLineLocal = commandLine;

	PROCESS_INFORMATION pProcessInfo;

	BOOL success = DetourCreateProcessWithDllsW(
		NULL,
		(LPWSTR)commandLineLocal.data(),
		NULL,
		NULL,
		FALSE,
		EXTENDED_STARTUPINFO_PRESENT,//| CREATE_SUSPENDED,
		NULL,
		NULL,
		(LPSTARTUPINFOW)startupInfo.get(),
		&pProcessInfo,
		sizeof rlpDlls / sizeof rlpDlls[0],
		rlpDlls,
		NULL);

	if (!success)
	{
		throw SandboxException("Failed to create process", GetLastError());
	}

	CloseHandle(pProcessInfo.hThread);

	WaitForSingleObject(pProcessInfo.hProcess, INFINITE);
	CloseHandle(pProcessInfo.hProcess);
}

bool Sandbox::NextMessage(MessageType *type, void **data)
{
	Sleep(INFINITE);

	return false;
}
