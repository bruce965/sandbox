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
#include "Shlwapi.h"

#pragma comment(lib, "userenv.lib")  // AppContainer
#pragma comment(lib, "shlwapi.lib")  // PathCombine

// Skip hooking, disabling communication with the supervisor
//#define DEBUG_SKIP_HOOKING 1

// Skip sandboxing/containerization
//#define DEBUG_SKIP_APPCONTAINER 1

Sandbox::Sandbox(const std::wstring &name, const GUID &guid)
{
	const int GUID_LENGTH = 37;  // length of a guid in "000000-0000-0000-0000-000000000000" format

	WCHAR buff[GUID_LENGTH + 1];
	_snwprintf_s(buff, sizeof(buff), L"%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	WCHAR szModuleFileName[MAX_PATH];
	if (!GetModuleFileNameW(NULL, szModuleFileName, MAX_PATH))
	{
		throw SandboxException("Failed to get sandbox supervisor's module file name while constructing", GetLastError());
	}

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

	WCHAR szDllFileName[MAX_PATH];
	if (!PathCombineW(szDllFileName, szModuleFileName, L"..\\sandbox-hooks32.dll"))
	{
		throw SandboxException("Failed to combine sandbox supervisor's module file name with 32-bit DLL path", GetLastError());
	}
	AclHelper::GrantPermissions(sidAllAppPackages, szDllFileName, SE_FILE_OBJECT, FILE_GENERIC_READ | FILE_GENERIC_EXECUTE);

	if (!PathCombineW(szDllFileName, szModuleFileName, L"..\\sandbox-hooks64.dll"))
	{
		throw SandboxException("Failed to combine sandbox supervisor's module file name with 64-bit DLL path", GetLastError());
	}
	AclHelper::GrantPermissions(sidAllAppPackages, szDllFileName, SE_FILE_OBJECT, FILE_GENERIC_READ | FILE_GENERIC_EXECUTE);

	LocalFree(sidAllAppPackages);
}

Sandbox::~Sandbox()
{
	FreeSid(this->sid);

	DeleteAppContainerProfile(this->appContainerName.c_str());
}

void Sandbox::StartProcess(const std::wstring &commandLine)
{
	StartupInfo startupInfo(2);

#if !DEBUG_SKIP_APPCONTAINER
	// run process in app container
	SecurityCapabilities securityCapabilities(
		this->sid,
		{
			WinCapabilityPrivateNetworkClientServerSid,
		});
	startupInfo.UpdateAttribute(
		PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES,
		securityCapabilities.get(),
		SecurityCapabilities::size);
#endif

	// do not allow to spawn child processes
	DWORD childProcessRestricted = PROCESS_CREATION_CHILD_PROCESS_RESTRICTED;
	startupInfo.UpdateAttribute(
		PROC_THREAD_ATTRIBUTE_CHILD_PROCESS_POLICY,
		&childProcessRestricted,
		sizeof childProcessRestricted);

	HANDLE hReadPipe;
	HANDLE hWritePipe;
	if (!CreatePipe(&hReadPipe, &hWritePipe, NULL, 4096))
	{
		throw SandboxException("Failed to create anonymous pipe", GetLastError());
	}

	CHAR szModuleFileName[MAX_PATH];
	if (!GetModuleFileNameA(NULL, szModuleFileName, MAX_PATH))
	{
		throw SandboxException("Failed to get sandbox supervisor's module file name while starting", GetLastError());
	}

	CHAR szDllFileName[MAX_PATH];
	if (!PathCombineA(szDllFileName, szModuleFileName, "..\\sandbox-hooks32.dll"))
	{
		throw SandboxException("Failed to combine sandbox supervisor's module file name with DLL path", GetLastError());
	}

	std::wstring commandLineLocal = commandLine;  // copy because it might be modified
	PROCESS_INFORMATION processInfo;
#if DEBUG_SKIP_HOOKING
	BOOL success = CreateProcessW(
		NULL,
		(LPWSTR)commandLineLocal.data(),
		NULL,
		NULL,
		FALSE,
		EXTENDED_STARTUPINFO_PRESENT,//| CREATE_SUSPENDED,
		NULL,
		NULL,
		(LPSTARTUPINFOW)startupInfo.get(),
		&processInfo);
#else
	LPCSTR rlpDlls[] = { szDllFileName };
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
		&processInfo,
		sizeof rlpDlls / sizeof rlpDlls[0],
		rlpDlls,
		NULL);
#endif

	if (!success)
	{
		throw SandboxException("Failed to create process", GetLastError());
	}

	CloseHandle(processInfo.hThread);

	//WaitForSingleObject(processInfo.hProcess, INFINITE);
	CloseHandle(processInfo.hProcess);
}

bool Sandbox::NextMessage(MessageType *type, void **data)
{
	Sleep(INFINITE);

	return false;
}
