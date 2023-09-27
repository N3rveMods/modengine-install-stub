#include "stdafx.h"

#include "UtilityModule.h"
#include "Settings.h"

#include <Shldisp.h>
#include <atlbase.h>

#include <assert.h>
#include <windows.h>
#include <ShlObj.h>
#include <wrl.h>

#include <string.h>
#include <iostream>

using namespace std;


extern "C" bool gCurrentDownloadingQuittedWithError = false;
extern "C" int gCurrentDownloadingPercentage = 0;
extern "C" bool gCurrentDownloadingIsComplete = false;

/**
	The callback handler class for URLDownloadToFile(..)
*/
class CallbackHandler : public IBindStatusCallback
{
private:
	int m_percentLast;

public:
	CallbackHandler() : m_percentLast(0)
	{
	}

	// IUnknown

	HRESULT STDMETHODCALLTYPE
		QueryInterface(REFIID riid, void** ppvObject)
	{

		if (IsEqualIID(IID_IBindStatusCallback, riid)
			|| IsEqualIID(IID_IUnknown, riid))
		{
			*ppvObject = reinterpret_cast<void*>(this);
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE
		AddRef()
	{
		return 2UL;
	}

	ULONG STDMETHODCALLTYPE
		Release()
	{
		return 1UL;
	}

	// IBindStatusCallback

	HRESULT STDMETHODCALLTYPE
		OnStartBinding(DWORD     /*dwReserved*/,
			IBinding* /*pib*/)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		GetPriority(LONG* /*pnPriority*/)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		OnLowResource(DWORD /*reserved*/)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		OnProgress(ULONG   ulProgress,
			ULONG   ulProgressMax,
			ULONG   ulStatusCode,
			LPCWSTR /*szStatusText*/)
	{
		switch (ulStatusCode)
		{
		case BINDSTATUS_FINDINGRESOURCE:
			cout << _T("Finding resource...") << endl;
			break;
		case BINDSTATUS_CONNECTING:
			cout << _T("Connecting...") << endl;
			break;
		case BINDSTATUS_SENDINGREQUEST:
			cout << _T("Sending request...") << endl;
			break;
		case BINDSTATUS_MIMETYPEAVAILABLE:
			cout << _T("Mime type available") << endl;
			break;
		case BINDSTATUS_CACHEFILENAMEAVAILABLE:
			cout << _T("Cache filename available") << endl;
			break;
		case BINDSTATUS_BEGINDOWNLOADDATA:
			cout << _T("Begin download") << endl;
			break;
		case BINDSTATUS_DOWNLOADINGDATA:
		case BINDSTATUS_ENDDOWNLOADDATA:
		{
			int percent = (int)(100.0 * static_cast<double>(ulProgress)
				/ static_cast<double>(ulProgressMax));
			if (m_percentLast < percent)
			{
				m_percentLast = percent;
				gCurrentDownloadingPercentage = m_percentLast;

				WCHAR msgbuf[300];
				_snwprintf(msgbuf, sizeof(msgbuf) - 1, L"Download percentage is %d\r\n", m_percentLast);
				OutputDebugStringW(msgbuf);
				
			}
			if (ulStatusCode == BINDSTATUS_ENDDOWNLOADDATA)
			{
				gCurrentDownloadingIsComplete = true;

				cout << endl << _T("End download") << endl;
			}
		}
		break;

		default:
		{
			cout << _T("Status code : ") << ulStatusCode << endl;
		}
		}
		// The download can be cancelled by returning E_ABORT here
		// of from any other of the methods.
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE
		OnStopBinding(HRESULT /*hresult*/,
			LPCWSTR /*szError*/)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		GetBindInfo(DWORD*    /*grfBINDF*/,
			BINDINFO* /*pbindinfo*/)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		OnDataAvailable(DWORD      /*grfBSCF*/,
			DWORD      /*dwSize*/,
			FORMATETC* /*pformatetc*/,
			STGMEDIUM* /*pstgmed*/)
	{
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE
		OnObjectAvailable(REFIID    /*riid*/,
			IUnknown* /*punk*/)
	{
		return E_NOTIMPL;
	}
};



/** 
	For urlPath like https://somewhere.org/theData.zip, this function GetFileNameFromURL will return "theData.zip".
*/
WCHAR *GetFileNameFromURL(WCHAR *urlPath)
{
	WCHAR *filename = wcsrchr(urlPath, '/'); // wcsrchr is the wide char version of strrchr
	if (filename == NULL)
		filename = urlPath;
	else
		filename++;
	return filename;
}


/** 
	This function is declared in header file with extern "C" { ... }, such that it can be called from C.
	This funcion utilizes COM modules to do folder selection.
*/
PWSTR PickDirectory(HWND hwnd)
{
	OutputDebugStringW(L"Entering PickDirectory...\n");

	PWSTR wCharPath2Return = nullptr;

	Microsoft::WRL::ComPtr<IFileDialog> folderPicker;
	auto hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&folderPicker));
	assert(SUCCEEDED(hr));

	DWORD pickerOptions;
	hr = folderPicker->GetOptions(&pickerOptions);
	assert(SUCCEEDED(hr));

	hr = folderPicker->SetOptions(pickerOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM); // W.S.: Added the FOS_FORCEFILESYSTEM mask, to rule out such selection as "Computer" which is not a real folder in file system.
	assert(SUCCEEDED(hr));

	PWSTR savedialtitle = (LPWSTR)L"Select where you want to install ModEngine?";
	hr = folderPicker->SetTitle(savedialtitle);
	assert(SUCCEEDED(hr));

	if (SUCCEEDED(folderPicker->Show(hwnd)))
	{
		Microsoft::WRL::ComPtr<IShellItem> folderShellItem;
		hr = folderPicker->GetResult(&folderShellItem);
		assert(SUCCEEDED(hr));

		PWSTR folderPath = nullptr;
		hr = folderShellItem->GetDisplayName(SIGDN_FILESYSPATH, &folderPath);
		assert(SUCCEEDED(hr));

		OutputDebugStringW(L"Picked folder: ");
		OutputDebugStringW(folderPath);
		OutputDebugStringW(L"\r\n");

		wCharPath2Return = new WCHAR[wcslen(folderPath) + 1];
		wcscpy_s(wCharPath2Return, wcslen(folderPath) + 1, folderPath); // Added "+1" to the 2nd parameter, to avoid crash when the folderPath is something like virtual cloud drive ending with non-ascii chars.

		OutputDebugStringW(L"The path to return is: ");
		OutputDebugStringW(wCharPath2Return);
		OutputDebugStringW(L"\r\n");

		CoTaskMemFree(folderPath);
	}

	OutputDebugStringW(L"Exiting PickDirectory...\n");

	return wCharPath2Return;
}

/**
	We use this function to unzip the downloaded file under installation directory chosen by users.
	This function utilize Windows module to do the unzipping job. Had tried unzipping with libzip first, but eventually switched to current approach, for smaller size of the exe file.
*/
static bool Unzip2Folder(BSTR lpZipFile, BSTR lpFolder)
{
	IShellDispatch *pISD;

	Folder  *pZippedFile = 0L;
	Folder  *pDestination = 0L;

	long FilesCount = 0;
	IDispatch* pItem = 0L;
	FolderItems *pFilesInside = 0L;

	VARIANT Options, OutFolder, InZipFile, Item;
	CoInitialize(NULL);
	__try {
		if (CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD) != S_OK)
			return 1;

		InZipFile.vt = VT_BSTR;
		InZipFile.bstrVal = lpZipFile;
		pISD->NameSpace(InZipFile, &pZippedFile);
		if (!pZippedFile)
		{
			pISD->Release();
			return 1;
		}

		OutFolder.vt = VT_BSTR;
		OutFolder.bstrVal = lpFolder;
		pISD->NameSpace(OutFolder, &pDestination);
		if (!pDestination)
		{
			pZippedFile->Release();
			pISD->Release();
			return 1;
		}

		pZippedFile->Items(&pFilesInside);
		if (!pFilesInside)
		{
			pDestination->Release();
			pZippedFile->Release();
			pISD->Release();
			return 1;
		}

		pFilesInside->get_Count(&FilesCount);
		if (FilesCount < 1)
		{
			pFilesInside->Release();
			pDestination->Release();
			pZippedFile->Release();
			pISD->Release();
			return 0;
		}

		pFilesInside->QueryInterface(IID_IDispatch, (void**)&pItem);

		Item.vt = VT_DISPATCH;
		Item.pdispVal = pItem;

		Options.vt = VT_I4;
		Options.lVal = 1024 | 512 | 16 | 4;//http://msdn.microsoft.com/en-us/library/bb787866(VS.85).aspx

		bool retval = pDestination->CopyHere(Item, Options) == S_OK;

		pItem->Release(); pItem = 0L;
		pFilesInside->Release(); pFilesInside = 0L;
		pDestination->Release(); pDestination = 0L;
		pZippedFile->Release(); pZippedFile = 0L;
		pISD->Release(); pISD = 0L;

		return retval;

	}
	__finally
	{
		CoUninitialize();
	}
}

/**
	This function is called in event handling. The main job inside this function is done by invoking Unzip2Folder(..).
*/
int unzip2LocalFolder(WCHAR* installFolderPath) {
	WCHAR* fileNameFromURL = GetFileNameFromURL(URL_FOR_DOWNLOAD_WSTR);
	assert(fileNameFromURL != nullptr);
	
	//WCHAR localPath4DownloadedFile[MAX_PATH] = { 0L };
	WCHAR *localPath4DownloadedFile = new WCHAR[MAX_PATH + 1];
	if (DOWNLOAD_TO_C_ROOT_DIR == true) {
		//wcscpy_s(localPath4DownloadedFile, MAX_PATH, L"C:\\");
		wcscpy_s(localPath4DownloadedFile, MAX_PATH, L"C:"); // W.S.: localPath4DownloadedFile shall not have extra "\\"s, else Unzip2Folder will do nothing..
	}
	else {
		wcscpy_s(localPath4DownloadedFile, MAX_PATH, installFolderPath);
	}
	wcscat_s(localPath4DownloadedFile, MAX_PATH, L"\\");
	wcscat_s(localPath4DownloadedFile, MAX_PATH, fileNameFromURL);

	CComBSTR file(localPath4DownloadedFile); // localPath4DownloadedFile shall not have extra "\\"s, else Unzip2Folder will do nothing..
	CComBSTR folder(installFolderPath);
	Unzip2Folder(file, folder);

	return 0;
}

/**
	The thread function for downloading remote URL to local machine.
*/
DWORD WINAPI threadFunctionForDownload(LPVOID lpParam) {
	return downloadFromServer((WCHAR *)lpParam);
}

/**
	Downloads file (using given URL) from remote server.
	The main job inside this function is done by invoking URLDownloadToFile(..).
	
	This function is running in another thread (not the main thread), such that the UI in main thread will not be blocked.
	Reason: As URLDownloadToFile will not return until download is complete or download quits due to some errors, and as this downloadFromServer(..) calls downloadFromServer, this function will take a relatively long time to finish/return if the file to download is large. According to my test, for a testing remote zip file of around 40MB, it might take minutes to download. That will block the UI if we call this function directly in main thread (message handler). 
*/
HRESULT downloadFromServer(WCHAR* installFolder) {
	assert(installFolder != nullptr);

	WCHAR* fileNameFromURL = GetFileNameFromURL(URL_FOR_DOWNLOAD_WSTR);
	assert(fileNameFromURL != nullptr);
	
	WCHAR localPath4DownloadedFile[MAX_PATH] = { 0L };
	if (DOWNLOAD_TO_C_ROOT_DIR == true) {
		wcscpy_s(localPath4DownloadedFile, MAX_PATH, L"C:\\");
	}
	else {
		wcscpy_s(localPath4DownloadedFile, MAX_PATH, installFolder);
	}
	wcscat_s(localPath4DownloadedFile, MAX_PATH, L"\\");
	wcscat_s(localPath4DownloadedFile, MAX_PATH, fileNameFromURL);

	OutputDebugStringW(L"fileNameFromURL is ");
	OutputDebugStringW(fileNameFromURL);
	OutputDebugStringW(L"\n");
	OutputDebugStringW(L"localPath4DownloadedFile is ");
	OutputDebugStringW(localPath4DownloadedFile);
	OutputDebugStringW(L"\n");

	
	DeleteUrlCacheEntry(URL_FOR_DOWNLOAD_WSTR); // Remove cache for the URL as in parameter.
	
	//W.S.: To show percentage while downloading larger files, we supplied a callback function as the last parameter for URLDownloadToFile(..).
	//
	//HRESULT response = URLDownloadToFile(NULL, URL_FOR_DOWNLOAD_WSTR, localPath4DownloadedFile, 0, NULL);
	CallbackHandler callbackHandler;
	IBindStatusCallback* pBindStatusCallback = NULL;
	callbackHandler.QueryInterface(IID_IBindStatusCallback,
		reinterpret_cast<void**>(&pBindStatusCallback));
	HRESULT response = URLDownloadToFile(NULL, URL_FOR_DOWNLOAD_WSTR, localPath4DownloadedFile, 0, pBindStatusCallback); // The last parameter indicates the callback function.

	if (response != S_OK) {
		// Download quitted with error. Notify the main process (which does UI handling) by setting global variables.
		gCurrentDownloadingQuittedWithError = true;
	}

	//W.S.: In the line below, an invalid URL is used, just for pure testing purpose. Need to commet off this line in release versions.
	//HRESULT response = URLDownloadToFile(NULL, (L"https://WRONGURL.chatlifier.com/myapp.zip"), L"C:\\\\ValidData.zip", 0, NULL);

	return response;
}