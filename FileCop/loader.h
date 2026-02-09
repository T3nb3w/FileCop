#pragma once
#include <Windows.h>
#include <winnt.h>
#include <winternl.h>
#include <TlHelp32.h>

//============================================================
// Notes
// - Structures keep exact layout (NO std:: types inside)
// - All offsets preserved
// - ABI / calling convention preserved
//============================================================

namespace nt
{
    //============================================================
    // PEB / LDR STRUCTURES
    //============================================================

    struct PEB_LDR_DATAX
    {
        ULONG      Length;                         // 0x000
        UCHAR      Initialized;                    // 0x004
        PVOID      SsHandle;                       // 0x008
        LIST_ENTRY InLoadOrderModuleList;           // 0x010
        LIST_ENTRY InMemoryOrderModuleList;         // 0x020
        LIST_ENTRY InInitializationOrderModuleList; // 0x030
        PVOID      EntryInProgress;                 // 0x040
        UCHAR      ShutdownInProgress;              // 0x048
        PVOID      ShutdownThreadId;                // 0x050
    };

    using PPEB_LDR_DATAX = PEB_LDR_DATAX*;

    //============================================================

    struct LDR_DATA_TABLE_ENTRYX
    {
        LIST_ENTRY      InLoadOrderLinks;            // 0x000
        LIST_ENTRY      InMemoryOrderLinks;          // 0x010
        LIST_ENTRY      InInitializationOrderLinks;  // 0x020
        PVOID           DllBase;                     // 0x030
        PVOID           EntryPoint;                  // 0x038
        ULONG           SizeOfImage;                 // 0x040
        UNICODE_STRING  FullDllName;                 // 0x048
        UNICODE_STRING  BaseDllName;                 // 0x058
        ULONG           Flags;                       // 0x068
        // other fields
    };

    using PLDR_DATA_TABLE_ENTRYX = LDR_DATA_TABLE_ENTRYX*;

    //============================================================

    struct CURDIRX
    {
        UNICODE_STRING DosPath; // 0x000
        PVOID          Handle;  // 0x010
    };

    using PCURDIRX = CURDIRX*;

    //============================================================

    struct RTL_DRIVE_LETTER_CURDIRX
    {
        USHORT Flags;      // 0x000
        USHORT Length;     // 0x002
        ULONG  TimeStamp;  // 0x004
        STRING DosPath;    // 0x008
    };

    using PRTL_DRIVE_LETTER_CURDIRX = RTL_DRIVE_LETTER_CURDIRX*;

    //============================================================

    constexpr ULONG RTL_MAX_DRIVE_LETTERS = 32;

    struct RTL_USER_PROCESS_PARAMETERSX
    {
        ULONG   MaximumLength;            // 0x000
        ULONG   Length;                   // 0x004
        ULONG   Flags;                    // 0x008
        ULONG   DebugFlags;               // 0x00c
        PVOID   ConsoleHandle;            // 0x010
        ULONG   ConsoleFlags;             // 0x018
        PVOID   StandardInput;            // 0x020
        PVOID   StandardOutput;           // 0x028
        PVOID   StandardError;            // 0x030
        CURDIRX CurrentDirectory;         // 0x038
        UNICODE_STRING DllPath;           // 0x050
        UNICODE_STRING ImagePathName;     // 0x060
        UNICODE_STRING CommandLine;       // 0x070
        PVOID   Environment;              // 0x080
        ULONG   StartingX;                // 0x088
        ULONG   StartingY;                // 0x08c
        ULONG   CountX;                   // 0x090
        ULONG   CountY;                   // 0x094
        ULONG   CountCharsX;              // 0x098
        ULONG   CountCharsY;              // 0x09c
        ULONG   FillAttribute;            // 0x0a0
        ULONG   WindowFlags;              // 0x0a4
        ULONG   ShowWindowFlags;           // 0x0a8
        UNICODE_STRING WindowTitle;       // 0x0b0
        UNICODE_STRING DesktopInfo;       // 0x0c0
        UNICODE_STRING ShellInfo;         // 0x0d0
        UNICODE_STRING RuntimeData;       // 0x0e0
        RTL_DRIVE_LETTER_CURDIRX CurrentDirectories[RTL_MAX_DRIVE_LETTERS]; // 0x0f0
        ULONGLONG EnvironmentSize;         // 0x3f0
        ULONGLONG EnvironmentVersion;      // 0x3f8
        PVOID   PackageDependencyData;     // 0x400
        ULONG   ProcessGroupId;            // 0x408
        ULONG   LoaderThreads;             // 0x40c
        UNICODE_STRING RedirectionDllName; // 0x410
        UNICODE_STRING HeapPartitionName;  // 0x420
        PVOID   DefaultThreadpoolCpuSetMasks;     // 0x430
        ULONG   DefaultThreadpoolCpuSetMaskCount; // 0x438
        ULONG   DefaultThreadpoolThreadMaximum;   // 0x43c
    };

    using PRTL_USER_PROCESS_PARAMETERSX = RTL_USER_PROCESS_PARAMETERSX*;

    //============================================================
    // TEB OFFSETS & ACCESSORS (x64)
    //============================================================

    namespace teb
    {
        constexpr ULONG StackBase = 0x08;
        constexpr ULONG StackLimit = 0x10;
        constexpr ULONG Self = 0x30;
        constexpr ULONG ClientIdUniqueProcess = 0x40;
        constexpr ULONG ClientIdUniqueThread = 0x48;
        constexpr ULONG ProcessEnvironmentBlock = 0x60;

        inline PUCHAR StackBasePtr()
        {
            return reinterpret_cast<PUCHAR>(__readgsqword(StackBase));
        }

        inline PUCHAR StackLimitPtr()
        {
            return reinterpret_cast<PUCHAR>(__readgsqword(StackLimit));
        }

        inline PTEB SelfPtr()
        {
            return reinterpret_cast<PTEB>(__readgsqword(Self));
        }

        inline HANDLE ProcessId()
        {
            return reinterpret_cast<HANDLE>(__readgsqword(ClientIdUniqueProcess));
        }

        inline HANDLE ThreadId()
        {
            return reinterpret_cast<HANDLE>(__readgsqword(ClientIdUniqueThread));
        }

        inline PPEB Peb()
        {
            return reinterpret_cast<PPEB>(__readgsqword(ProcessEnvironmentBlock));
        }
    }

    //============================================================
    // PEB OFFSETS & ACCESSORS
    //============================================================

    namespace peb
    {
        constexpr ULONG ImageBaseAddress = 0x10;
        constexpr ULONG Ldr = 0x18;
        constexpr ULONG ProcessParameters = 0x20;
        constexpr ULONG ProcessHeap = 0x30;
        constexpr ULONG NumberOfHeaps = 0xe8;
        constexpr ULONG ProcessHeaps = 0xf0;
        constexpr ULONG GdiSharedHandleTable = 0xf8;

        inline PVOID ImageBase(PPEB peb)
        {
            return *reinterpret_cast<PVOID*>(
                reinterpret_cast<PUCHAR>(peb) + ImageBaseAddress
                );
        }

        inline PVOID LdrData(PPEB peb)
        {
            return *reinterpret_cast<PVOID*>(
                reinterpret_cast<PUCHAR>(peb) + Ldr);
        }
    }

    //============================================================
    // LDR ENUMERATION
    //============================================================

    using PLDR_ENUM_CALLBACK =
        VOID(NTAPI*)(PLDR_DATA_TABLE_ENTRYX Module, PVOID Context, BOOLEAN* Stop);

    extern "C"
    {
        NTSTATUS NTAPI LdrEnumerateLoadedModules(
            BOOLEAN ReservedFlag,
            PLDR_ENUM_CALLBACK EnumProc,
            PVOID Context);

        NTSTATUS NTAPI LdrFindEntryForAddress(
            PVOID Address,
            PVOID* Entry);
        using LdrFindEntryForAddress_t = NTSTATUS(NTAPI*)(PVOID Address, PVOID* Entry);
           

        NTSTATUS NTAPI LdrGetDllHandle(
            PWSTR DllPath,
            PULONG DllCharacteristics,
            PUNICODE_STRING DllName,
            HMODULE* DllHandle);

        NTSTATUS NTAPI LdrLoadDll(
            PWSTR DllPath,
            PULONG DllCharacteristics,
            PUNICODE_STRING DllName,
            HMODULE* DllHandle);

        NTSTATUS NTAPI LdrGetProcedureAddress(
            HMODULE DllHandle,
            PANSI_STRING ExportName,
            WORD Ordinal,
            PVOID* ExportAddress);

        VOID NTAPI RtlInitAnsiString(
            PANSI_STRING DestinationString,
            PCSZ SourceString);

        NTSTATUS NTAPI RtlAnsiStringToUnicodeString(
            PUNICODE_STRING DestinationString,
            PCANSI_STRING SourceString,
            BOOLEAN AllocateDestinationString);

        PPEB NTAPI RtlGetCurrentPeb();
        using RtlGetCurrentPeb_t = PPEB(NTAPI*)(VOID);
        VOID NTAPI RtlAcquirePebLock();
        using RtlAcquirePebLock_t = PPEB(NTAPI*)(VOID);
        VOID NTAPI RtlReleasePebLock();
        using RtlReleasePebLock_t = PPEB(NTAPI*)(VOID);

    }

} // namespace nt