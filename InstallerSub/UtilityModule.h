#pragma once

//#include <wininet.h> // W.S.: Won't compile if we include <wininet.h> BEFORE <urlmon.h>

#include <urlmon.h>
#pragma comment(lib, "Urlmon.lib")

#include <wininet.h> // W.S.: Compiles OK if we include <wininet.h> AFTER <urlmon.h>
#pragma comment(lib, "wininet.lib")

#ifdef __cplusplus
extern "C" {
#endif
	
	DWORD WINAPI threadFunctionForDownload(LPVOID lpParam);

	PWSTR PickDirectory(HWND hwnd);

	int unzip2LocalFolder(WCHAR* installFolderPath);
	HRESULT downloadFromServer(WCHAR* installFolder);

#ifdef __cplusplus
}
#endif