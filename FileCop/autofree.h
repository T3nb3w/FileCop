#pragma once
#include <Windows.h>
#include <winternl.h>

//RAII wrappers
class AutoFreeStringU
{
    PUNICODE_STRING m_String;
public:
    AutoFreeStringU(const AutoFreeStringU&) = delete;
    void operator=(const AutoFreeStringU&) = delete;
    AutoFreeStringU() { Clear(); }
    AutoFreeStringU(PUNICODE_STRING String) { Set(String); }
    ~AutoFreeStringU() {
        if (m_String && m_String->Buffer) {
            HeapFree(GetProcessHeap(), 0, m_String->Buffer);
            Clear();
        }
    }
    VOID Set(PUNICODE_STRING String) { m_String = String; }
    VOID Clear() { m_String->Buffer = nullptr; m_String = nullptr; }
};

class AutoHeapFree
{
    PVOID m_Block;
public:
    AutoHeapFree(const AutoHeapFree&) = delete;
    void operator=(const AutoHeapFree&) = delete;
    AutoHeapFree() { Clear(); }
    AutoHeapFree(PVOID Block) { Set(Block); }
    ~AutoHeapFree() { if (m_Block) { HeapFree(GetProcessHeap(), 0, m_Block); Clear(); } }
    VOID Set(PVOID Block) { m_Block = Block; }
    VOID Clear() { m_Block = nullptr; }
}; 

class ComInitializer {
    HRESULT m_hr;
public:
    ComInitializer(DWORD flags) : m_hr(CoInitializeEx(nullptr, flags)) {}
    ~ComInitializer() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
    HRESULT result() const { return m_hr; }
};