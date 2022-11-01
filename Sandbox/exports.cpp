#include "exports.h"

#include <stdexcept>

#include "Sandbox.h"
#include "MessageType.h"
#include "SandboxException.h"

BOOL __cdecl CreateSandbox(
	PHANDLE phSandbox,
	LPCWSTR lpName,
	GUID guid,
	BSTR* errorMessage,
	DWORD* errorCode)
{
	try
	{
		Sandbox* sandbox = new Sandbox(
			lpName,
			guid);

		*phSandbox = (PHANDLE)sandbox;
	}
	catch (const SandboxException& e)
	{
		*errorMessage = e.AllocErrorMessage();
		*errorCode = e.ErrorCode;
		return FALSE;
	}

	*errorMessage = nullptr;
	*errorCode = 0;
	return TRUE;
}

BOOL __cdecl StartSandboxProcess(
	HANDLE hSandbox,
	LPCWSTR lpCommandLine,
	BSTR* errorMessage,
	DWORD* errorCode)
{
	Sandbox* sandbox = (Sandbox*)hSandbox;

	try
	{
		sandbox->StartProcess(
			std::wstring(lpCommandLine));
	}
	catch (const SandboxException& e)
	{
		*errorMessage = e.AllocErrorMessage();
		*errorCode = e.ErrorCode;
		return FALSE;
	}

	*errorMessage = nullptr;
	*errorCode = 0;
	return TRUE;
}

BOOL __cdecl GetSandboxMessage(
	HANDLE hSandbox,
	PDWORD pdwType,
	PVOID *ppData)
{
	Sandbox *sandbox = (Sandbox *)hSandbox;
	
	MessageType type;
	void *data;
	bool result = sandbox->NextMessage(&type, &data);

	*pdwType = (DWORD)type;
	*ppData = (PVOID)data;
	return (BOOL)result;
}

BOOL __cdecl ConfirmSandboxMessage(
	HANDLE hSandbox,
	PVOID pData)
{
	Sandbox *sandbox = (Sandbox *)hSandbox;

	// TODO

	return TRUE;
}

BOOL __cdecl DestroySandbox(
	HANDLE hSandbox)
{
	Sandbox *sandbox = (Sandbox *)hSandbox;

	delete sandbox;

	return TRUE;
}
