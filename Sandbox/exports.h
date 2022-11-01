#pragma once

#include <Windows.h>
#include <comutil.h>

extern "C" __declspec(dllexport) BOOL __cdecl CreateSandbox(
	PHANDLE phSandbox,
	LPCWSTR lpName,
	GUID guid,
	BSTR * errorMessage,
	DWORD * errorCode);

extern "C" __declspec(dllexport) BOOL __cdecl StartSandboxProcess(
	HANDLE hSandbox,
	LPCWSTR lpCommandLine,
	BSTR *errorMessage,
	DWORD *errorCode);

extern "C" __declspec(dllexport) BOOL __cdecl GetSandboxMessage(
	HANDLE hSandbox,
	PDWORD pdwType,
	PVOID *pData);

extern "C" __declspec(dllexport) BOOL __cdecl ConfirmSandboxMessage(
	HANDLE hSandbox,
	PVOID pData);

extern "C" __declspec(dllexport) BOOL __cdecl DestroySandbox(
	HANDLE hSandbox);
