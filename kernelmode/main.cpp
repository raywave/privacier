#include <ntdef.h>
#include <ntifs.h>
#include <ntddk.h>
#include <ntdddisk.h>
#include <scsi.h>
#include <intrin.h>
#include <stdlib.h>
#include <Ntstrsafe.h>
#include <windef.h>
#include <ntifs.h>

#pragma region please_move_me_to_header_file

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#define DRIVER_NAME "privacier"

#define IO_INIT_REQUEST             CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0666, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) // Init cheat
#define IO_WRITE_REQUEST            CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0667, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) // Write Proccess Memory
#define IO_READ_REQUEST             CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0668, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) // Read Proccess Memory
#define IO_PEB_REQUEST              CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0669, METHOD_BUFFERED, FILE_SPECIAL_ACCESS) // Get PEB

typedef struct init_t {
	DWORD32 gameId;
	NTSTATUS result;
} init, * p_init;

typedef struct read_t {
	DWORD_PTR address;
	void* response;
	SIZE_T size;
	NTSTATUS result;
} read, * p_read;

typedef struct write_t {
	DWORD_PTR address;
	void* value;
	SIZE_T size;
	NTSTATUS result;
} write, * p_write;

typedef struct peb_t {
	void* data;
	NTSTATUS result;
} peb, * p_peb;

extern "C" {
	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
	NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);
	NTKERNELAPI PPEB NTAPI PsGetProcessPeb(IN PEPROCESS Process);
	NTKERNELAPI
		PVOID
		NTAPI
		PsGetProcessWow64Process(_In_ PEPROCESS Process);
}

extern "C" extern POBJECT_TYPE * IoDriverObjectType;

/*
typedef struct _PEB_LDR_DATA {
	ULONG Length;
	BOOLEAN Initialized;
	PVOID SsHandle;
	LIST_ENTRY ModuleListLoadOrder;
	LIST_ENTRY ModuleListMemoryOrder;
	LIST_ENTRY ModuleListInitOrder;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	BYTE Reserved1[16];
	PVOID Reserved2[10];
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef void(__stdcall* PPS_POST_PROCESS_INIT_ROUTINE)(void);

typedef struct _PEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[1];
	PVOID Reserved3[2];
	PPEB_LDR_DATA Ldr;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	PVOID Reserved4[3];
	PVOID AtlThunkSListPtr;
	PVOID Reserved5;
	ULONG Reserved6;
	PVOID Reserved7;
	ULONG Reserved8;
	ULONG AtlThunkSListPtr32;
	PVOID Reserved9[45];
	BYTE Reserved10[96];
	PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
	BYTE Reserved11[128];
	PVOID Reserved12[1];
	ULONG SessionId;
} PEB, * PPEB;

typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;  // in bytes
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;  // LDR_*
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	PVOID SectionPointer;
	ULONG CheckSum;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;
*/

typedef struct _PEB_LDR_DATA32
{
	ULONG Length;
	UCHAR Initialized;
	ULONG SsHandle;
	LIST_ENTRY32 InLoadOrderModuleList;
	LIST_ENTRY32 InMemoryOrderModuleList;
	LIST_ENTRY32 InInitializationOrderModuleList;
} PEB_LDR_DATA32, * PPEB_LDR_DATA32;

typedef struct _LDR_DATA_TABLE_ENTRY32
{
	LIST_ENTRY32 InLoadOrderLinks;
	LIST_ENTRY32 InMemoryOrderLinks;
	LIST_ENTRY32 InInitializationOrderLinks;
	ULONG DllBase;
	ULONG EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING32 FullDllName;
	UNICODE_STRING32 BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY32 HashLinks;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY32, * PLDR_DATA_TABLE_ENTRY32;

typedef struct _PEB32
{
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	ULONG Mutant;
	ULONG ImageBaseAddress;
	ULONG Ldr;
	ULONG ProcessParameters;
	ULONG SubSystemData;
	ULONG ProcessHeap;
	ULONG FastPebLock;
	ULONG AtlThunkSListPtr;
	ULONG IFEOKey;
	ULONG CrossProcessFlags;
	ULONG UserSharedInfoPtr;
	ULONG SystemReserved;
	ULONG AtlThunkSListPtr32;
	ULONG ApiSetMap;
} PEB32, * PPEB32;

#pragma endregion

namespace data {
	HANDLE gameId;
	PEPROCESS gameProcess;
	HANDLE cheatId;
	PEPROCESS cheatProcess;
}

void init_cheat_data(PIRP irp) {
	p_init pInitData = (p_init)irp->AssociatedIrp.SystemBuffer;
	if (pInitData) {
		data::gameId = (HANDLE)pInitData->gameId;

		pInitData->result = PsLookupProcessByProcessId(data::gameId, &data::gameProcess);
	}
}

PVOID get_module_entry(PEPROCESS pe, LPCWSTR ModuleName) {
	if (!pe) { return 0; }
	__try
	{
		PPEB32 pPeb32 = (PPEB32)PsGetProcessWow64Process(pe);
		if (!pPeb32 || !pPeb32->Ldr) {
			return 0;
		}

		for (PLIST_ENTRY32 pListEntry = (PLIST_ENTRY32)((PPEB_LDR_DATA32)pPeb32->Ldr)
			->InLoadOrderModuleList.Flink;
			pListEntry != &((PPEB_LDR_DATA32)pPeb32->Ldr)->InLoadOrderModuleList;
			pListEntry = (PLIST_ENTRY32)pListEntry->Flink) {
			PLDR_DATA_TABLE_ENTRY32 pEntry =
				CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY32, InLoadOrderLinks);

			if (wcscmp((PWCH)pEntry->BaseDllName.Buffer, ModuleName) == 0) {
				PVOID ModuleAddress = (PVOID)pEntry->DllBase;
				return ModuleAddress;
			}
		}

		return 0;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
	return 0;
}

NTSTATUS KeReadVirtualMemory(PEPROCESS process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size, PSIZE_T ReadedBytes) {
	__try {
		MmCopyVirtualMemory(data::gameProcess, SourceAddress, process, TargetAddress, Size, KernelMode, ReadedBytes);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return STATUS_ACCESS_VIOLATION;
	}
	return STATUS_SUCCESS;
}

NTSTATUS KeWriteVirtualMemory(PEPROCESS process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size, PSIZE_T WrittenBytes) {
	__try {
		MmCopyVirtualMemory(process, TargetAddress, data::gameProcess, SourceAddress, Size, KernelMode, WrittenBytes);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return STATUS_ACCESS_VIOLATION;
	}
	return STATUS_SUCCESS;
}

NTSTATUS ctl_io(PDEVICE_OBJECT device_obj, PIRP irp) {
	ULONG informationSize = 0;
	auto stack = IoGetCurrentIrpStackLocation(irp);

	const ULONG controlCode = stack->Parameters.DeviceIoControl.IoControlCode;
	size_t size = 0;

	p_read pReadRequest;
	p_write pWriteRequest;
	p_peb pPebRequest;

	switch (controlCode) {
	case IO_INIT_REQUEST:
		init_cheat_data(irp);
		informationSize = sizeof(init);
		break;
	case IO_WRITE_REQUEST:
		pWriteRequest = (p_write)irp->AssociatedIrp.SystemBuffer;
		if (pWriteRequest) {
			if (pWriteRequest->address < 0x7FFFFFFFFFFF) {
				SIZE_T bytes;
				pWriteRequest->result = KeWriteVirtualMemory(PsGetCurrentProcess(), (PVOID)pWriteRequest->address, pWriteRequest->value, pWriteRequest->size, &bytes);
			}
			else {
				pWriteRequest->result = STATUS_ACCESS_VIOLATION;
			}
		}
		informationSize = sizeof(write);
		break;
	case IO_READ_REQUEST:
		pReadRequest = (p_read)irp->AssociatedIrp.SystemBuffer;

		if (pReadRequest) {
			if (pReadRequest->address < 0x7FFFFFFFFFFF) {
				SIZE_T bytes;
				pReadRequest->result = KeReadVirtualMemory(PsGetCurrentProcess(), (PVOID)pReadRequest->address, pReadRequest->response, pReadRequest->size, &bytes);
			}
			else {
				pReadRequest->response = nullptr;
				pReadRequest->result = STATUS_ACCESS_VIOLATION;
			}
		}
		informationSize = sizeof(read);
		break;
	case IO_PEB_REQUEST:
		pPebRequest = (p_peb)irp->AssociatedIrp.SystemBuffer;
		if (pPebRequest) {
			KAPC_STATE apc;
			KeStackAttachProcess(data::gameProcess, &apc);
			PVOID base_address = get_module_entry(data::gameProcess, L"client.dll");
			KeUnstackDetachProcess(&apc);
			pPebRequest->data = base_address;
			pPebRequest->result = pPebRequest->data ? 0 : 1;
		}
		informationSize = sizeof(peb);
		break;
	}

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = informationSize;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return irp->IoStatus.Status;
}

NTSTATUS unsupported_io(PDEVICE_OBJECT device_obj, PIRP irp) {
	irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return irp->IoStatus.Status;
}

NTSTATUS create_io(PDEVICE_OBJECT device_obj, PIRP irp) {
	UNREFERENCED_PARAMETER(device_obj);

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return irp->IoStatus.Status;
}

NTSTATUS close_io(PDEVICE_OBJECT device_obj, PIRP irp) {
	UNREFERENCED_PARAMETER(device_obj);
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return irp->IoStatus.Status;
}

NTSTATUS InitializeDriver(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registery_path) {
	auto  status = STATUS_SUCCESS;
	UNICODE_STRING  sym_link, dev_name;
	PDEVICE_OBJECT  dev_obj;

	RtlInitUnicodeString(&dev_name, L"\\Device\\" DRIVER_NAME);
	status = IoCreateDevice(driver_obj, 0, &dev_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &dev_obj);

	if (status != STATUS_SUCCESS) {
		return status;
	}

	RtlInitUnicodeString(&sym_link, L"\\DosDevices\\" DRIVER_NAME);
	status = IoCreateSymbolicLink(&sym_link, &dev_name);

	if (status != STATUS_SUCCESS) {
		return status;
	}

	dev_obj->Flags |= DO_BUFFERED_IO;

	for (int t = 0; t <= IRP_MJ_MAXIMUM_FUNCTION; t++)
		driver_obj->MajorFunction[t] = unsupported_io;

	driver_obj->MajorFunction[IRP_MJ_CREATE] = create_io;
	driver_obj->MajorFunction[IRP_MJ_CLOSE] = close_io;
	driver_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ctl_io;
	driver_obj->DriverUnload = NULL;

	dev_obj->Flags &= ~DO_DEVICE_INITIALIZING;

	return status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registery_path) {
	auto status = STATUS_SUCCESS;
	UNICODE_STRING  drv_name;

	RtlInitUnicodeString(&drv_name, L"\\Driver\\" DRIVER_NAME);
	status = IoCreateDriver(&drv_name, &InitializeDriver);

	return STATUS_SUCCESS;
}