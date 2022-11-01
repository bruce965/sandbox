//#define _CRT_SECURE_NO_WARNINGS_

// https://guidedhacking.com/threads/windows-api-hooking-how-to-hide-process-from-task-manager.12061/
// https://www.youtube.com/watch?v=uS22dBJpr7U

#include <cstdlib>
#include <cstdio>
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <winternl.h>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

typedef NTSTATUS (WINAPI *ptr_NtOpenFile)(
	PHANDLE            FileHandle,
	ACCESS_MASK        DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK   IoStatusBlock,
	ULONG              ShareAccess,
	ULONG              OpenOptions);

ptr_NtOpenFile OriginalNtOpenFile;

NTSTATUS WINAPI HookNtOpenFile(
	PHANDLE            FileHandle,
	ACCESS_MASK        DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK   IoStatusBlock,
	ULONG              ShareAccess,
	ULONG              OpenOptions)
{
	printf("INFO: detected call to NtOpenFile.\n");

	return OriginalNtOpenFile(
		FileHandle,
		DesiredAccess,
		ObjectAttributes,
		IoStatusBlock,
		ShareAccess,
		OpenOptions);
}

BOOL DoHook()
{
	HMODULE hOriginalNtdll = GetModuleHandle(TEXT("ntdll"));
	if (hOriginalNtdll == NULL)
	{
		printf("ERROR: failed to get module information, last error: %d\n", GetLastError());
		return false;
	}

	OriginalNtOpenFile = (ptr_NtOpenFile)GetProcAddress(hOriginalNtdll, "NtOpenFile");

	MODULEINFO modInfo;
	HANDLE hProcess = GetCurrentProcess();
	HMODULE hModule = GetModuleHandle(NULL);
	if (!GetModuleInformation(hProcess, hModule, &modInfo, sizeof(MODULEINFO)))
	{
		printf("ERROR: failed to get module information, last error: %d\n", GetLastError());
		return FALSE;
	}

	LPBYTE pAddress = (LPBYTE)modInfo.lpBaseOfDll;
	PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER)pAddress;
	PIMAGE_NT_HEADERS pINH = (PIMAGE_NT_HEADERS)(pAddress + pIDH->e_lfanew);
	PIMAGE_OPTIONAL_HEADER pIOH = (PIMAGE_OPTIONAL_HEADER)&(pINH->OptionalHeader);
	PIMAGE_IMPORT_DESCRIPTOR pIID = (PIMAGE_IMPORT_DESCRIPTOR)(pAddress + pIOH->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	BOOL descriptorFound = FALSE;
	for (; pIID->Characteristics; pIID++)
	{
		if (!strcmp("ntdll.dll", (char*)(pAddress + pIID->Name)))
		{
			descriptorFound = TRUE;
			break;
		}
	}

	if (!descriptorFound)
	{
		printf("ERROR: target descriptor not found.\n");
		return FALSE;
	}

	PIMAGE_THUNK_DATA pITD = (PIMAGE_THUNK_DATA)(pAddress + pIID->OriginalFirstThunk);
	PIMAGE_THUNK_DATA pFirstThunkTest = (PIMAGE_THUNK_DATA)(pAddress + pIID->FirstThunk);
	PIMAGE_IMPORT_BY_NAME pIIBN;

	BOOL functionFound = FALSE;
	for (; !(pITD->u1.Ordinal & IMAGE_ORDINAL_FLAG) && pITD->u1.AddressOfData; pITD++)
	{
		pIIBN = (PIMAGE_IMPORT_BY_NAME)(pAddress + pITD->u1.AddressOfData);
		if (!strcmp("NtOpenFile", (char*)pIIBN->Name))
		{
			functionFound = TRUE;
			break;
		}

		pFirstThunkTest++;
	}

	if (!functionFound)
	{
		printf("ERROR: target function not found.\n");
		return FALSE;
	}
	
	DWORD dwOldProtect;
	if (!VirtualProtect((LPVOID)&(pFirstThunkTest->u1.Function), sizeof(DWORD), PAGE_READWRITE, &dwOldProtect))
	{
		printf("ERROR: failed to unlock virtual page, last error: %d\n", GetLastError());
		return FALSE;
	}

	pFirstThunkTest->u1.Function = (DWORD)HookNtOpenFile;

	if (!VirtualProtect((LPVOID)&(pFirstThunkTest->u1.Function), sizeof(DWORD), dwOldProtect, &dwOldProtect))
	{
		printf("WARN: failed to restore virtual page, last error: %d\n", GetLastError());
	}

	if (!CloseHandle(hModule))
	{
		printf("WARN: failed close module handle, last error: %d\n", GetLastError());
	}

	// TODO

	return TRUE;
}

int main(int argc, char *argv[])
{
	STARTUPINFO pStartupInfo = { 0 };
	PROCESS_INFORMATION pProcessInfo;

	LPTSTR szCmdline = StrDup(TEXT("..\\Debug\\Test.exe"));
	if (szCmdline == NULL)
	{
		printf("ERROR: failed to allocate process string.\n");
		return -1;
	}

	BOOL success = CreateProcess(
		NULL,
		szCmdline,
		NULL,
		NULL,
		FALSE,
		CREATE_SUSPENDED,
		NULL,
		NULL,
		&pStartupInfo,
		&pProcessInfo);

	LocalFree(szCmdline);

	if (success == FALSE)
	{
		printf("ERROR: failed to create process: %d\n", GetLastError());
		return -1;
	}

	if (!DoHook())
	{
		printf("ERROR: failed to hook.\n");
		return -1;
	}

	CloseHandle(pProcessInfo.hThread);
	CloseHandle(pProcessInfo.hProcess);

	return 0;
}
