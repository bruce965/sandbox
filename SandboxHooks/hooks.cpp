#include "hooks.h"

#include <Windows.h>
#include <detours.h>
#include <winternl.h>

#if _DEBUG
#include <stdio.h>

static FILE* debugLog = NULL;
static BOOL debugWritingLog = FALSE;

#define DEBUG_OPEN_LOG(path) {\
	if (debugLog == NULL)\
	{\
		fopen_s(&debugLog, path, "w");\
	}\
}

#define DEBUG_LOG(x, ...) {\
	if (!debugWritingLog && debugLog != NULL)\
	{\
		debugWritingLog = TRUE;\
		fprintf(debugLog, (x), __VA_ARGS__);\
		fflush(debugLog);\
		debugWritingLog = FALSE;\
	}\
}

#define DEBUG_CLOSE_LOG() {\
	if (debugLog != NULL)\
	{\
		fclose(debugLog);\
		debugLog = NULL;\
	}\
}
#else
#define DEBUG_OPEN_LOG()
#define DEBUG_LOG(x, ...)
#define DEBUG_CLOSE_LOG()
#endif

typedef NTSTATUS(NTAPI* LP_NtOpenFile)(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG ShareAccess,
	ULONG OpenOptions);

typedef NTSTATUS(NTAPI* LP_NtCreateFile)(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	PLARGE_INTEGER AllocationSize,
	ULONG FileAttributes,
	ULONG ShareAccess,
	ULONG CreateDisposition,
	ULONG CreateOptions,
	PVOID EaBuffer,
	ULONG EaLength);

typedef NTSTATUS(NTAPI* LP_NtWriteFile)(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID Buffer,
	ULONG Length,
	PLARGE_INTEGER ByteOffset,
	PULONG Key);

typedef NTSTATUS(NTAPI* LP_NtWaitForSingleObject)(
	HANDLE Handle,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout);

static LP_NtOpenFile Real_NtOpenFile = NULL;
static LP_NtCreateFile Real_NtCreateFile = NULL;
static LP_NtWriteFile Real_NtWriteFile = NULL;
static LP_NtWaitForSingleObject Real_NtWaitForSingleObject = NULL;

static NTSTATUS NTAPI Fake_NtOpenFile(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG ShareAccess,
	ULONG OpenOptions)
{
	DEBUG_LOG("INFO: NtOpenFile: %wZ\n", ObjectAttributes->ObjectName);

	return Real_NtOpenFile(
		FileHandle,
		DesiredAccess,
		ObjectAttributes,
		IoStatusBlock,
		ShareAccess,
		OpenOptions);
}

static NTSTATUS NTAPI Fake_NtCreateFile(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	PLARGE_INTEGER AllocationSize,
	ULONG FileAttributes,
	ULONG ShareAccess,
	ULONG CreateDisposition,
	ULONG CreateOptions,
	PVOID EaBuffer,
	ULONG EaLength)
{
	DEBUG_LOG("INFO: NtCreateFile: %wZ (Handle: ", ObjectAttributes->ObjectName);

	NTSTATUS retval = Real_NtCreateFile(
		FileHandle,
		DesiredAccess,
		ObjectAttributes,
		IoStatusBlock,
		AllocationSize,
		FileAttributes,
		ShareAccess,
		CreateDisposition,
		CreateOptions,
		EaBuffer,
		EaLength);

	DEBUG_LOG("%p)\n", &FileHandle);

	return retval;
}

static NTSTATUS NTAPI Fake_NtWriteFile(
	HANDLE FileHandle,
	HANDLE Event,
	PIO_APC_ROUTINE ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	PVOID Buffer,
	ULONG Length,
	PLARGE_INTEGER ByteOffset,
	PULONG Key)
{
	DEBUG_LOG("INFO: NtWriteFile: (Handle: %p)\n", FileHandle);

	return Real_NtWriteFile(
		FileHandle,
		Event,
		ApcRoutine,
		ApcContext,
		IoStatusBlock,
		Buffer,
		Length,
		ByteOffset,
		Key);
}

static NTSTATUS NTAPI Fake_NtWaitForSingleObject(
	HANDLE Handle,
	BOOLEAN Alertable,
	PLARGE_INTEGER Timeout)
{
	//DEBUG_LOG("INFO: NtWaitForSingleObject: %p\n", Handle);

	return Real_NtWaitForSingleObject(
		Handle,
		Alertable,
		Timeout);
}

void HooksAttachAll()
{
	DEBUG_OPEN_LOG("C:\\Users\\User\\Documents\\Projects\\Sandbox\\SandboxLauncher\\bin\\Debug\\net472\\log.txt");

	DEBUG_LOG("INFO: HooksAttachAll...");

	Real_NtCreateFile = (LP_NtCreateFile)DetourFindFunction("ntdll.dll", "NtCreateFile");
	Real_NtOpenFile = (LP_NtOpenFile)DetourFindFunction("ntdll.dll", "NtOpenFile");
	Real_NtWriteFile = (LP_NtWriteFile)DetourFindFunction("ntdll.dll", "NtWriteFile");
	Real_NtWaitForSingleObject = (LP_NtWaitForSingleObject)DetourFindFunction("ntdll.dll", "NtWaitForSingleObject");
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach(&(PVOID&)Real_NtOpenFile, Fake_NtOpenFile);
	DetourAttach(&(PVOID&)Real_NtCreateFile, Fake_NtCreateFile);
	DetourAttach(&(PVOID&)Real_NtWriteFile, Fake_NtWriteFile);
	DetourAttach(&(PVOID&)Real_NtWaitForSingleObject, Fake_NtWaitForSingleObject);

	DetourTransactionCommit();

	DEBUG_LOG(" (done)\n");
}

void HooksDetachAll()
{
	DEBUG_LOG("INFO: HooksDetachAll...");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourDetach(&(PVOID&)Real_NtOpenFile, Fake_NtOpenFile);
	DetourDetach(&(PVOID&)Real_NtCreateFile, Fake_NtCreateFile);
	DetourDetach(&(PVOID&)Real_NtWriteFile, Fake_NtWriteFile);
	DetourDetach(&(PVOID&)Real_NtWaitForSingleObject, Fake_NtWaitForSingleObject);

	DetourTransactionCommit();

	DEBUG_LOG(" (done)\n");

	DEBUG_CLOSE_LOG();
}
