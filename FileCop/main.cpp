#include <iostream>
#include <Windows.h>
#include <ShlObj_core.h>
#include <format>
#include "filecop.h"

VOID Usage(VOID)
{
	std::cout << "usage: FileCop.exe <FullPathOfSourceFile> <FullPathOfDestination>\n";
}

INT __cdecl main(int argc, PCHAR argv[])
{
	if (argc < 3)
	{
		Usage();
		return EXIT_FAILURE;
	}

	if (IsUserAnAdmin())
	{
		std::cout << "[!] Running as Admin: WARNING\n";
	}

	PCCH SourceFile = argv[1];
	PCCH DestinationFile = argv[2];
	std::cout << "[+] SourceFile=" << SourceFile << '\n';
	std::cout << "[+] FullPathOfDestination=" << DestinationFile<<  '\n';

	if (!FileCop(SourceFile, DestinationFile))
	{
		std::cout << std::format("[-] FileCop({},{})\n", SourceFile, DestinationFile);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}