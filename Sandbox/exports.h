#pragma once

#include <Windows.h>

extern "C" __declspec(dllexport) BOOL __cdecl CreateSandbox(
	PHANDLE phSandbox,
	LPCWSTR lpName,
	GUID guid);

extern "C" __declspec(dllexport) BOOL __cdecl StartSandboxProcess(
	HANDLE hSandbox,
	LPCWSTR lpCommandLine);

extern "C" __declspec(dllexport) BOOL __cdecl GetSandboxMessage(
	HANDLE hSandbox,
	PDWORD pdwType,
	PVOID *pData);

extern "C" __declspec(dllexport) BOOL __cdecl ConfirmSandboxMessage(
	HANDLE hSandbox,
	PVOID pData);

extern "C" __declspec(dllexport) BOOL __cdecl DestroySandbox(
	HANDLE hSandbox);
