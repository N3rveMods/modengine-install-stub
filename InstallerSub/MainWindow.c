#include "stdafx.h"
#include "AboutDialog.h"
#include "Globals.h"
#include "MainWindow.h"
#include "Resource.h"

#include "stdio.h"

#include <assert.h>
#include <stdbool.h>

#include "UtilityModule.h"
#include "Settings.h"

extern bool gCurrentDownloadingQuittedWithError;
extern int gCurrentDownloadingPercentage;
extern bool gCurrentDownloadingIsComplete;

static LPCTSTR MainWndClass = TEXT("Win32 Dialog Application");

/* Window procedure for our main window */
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static WCHAR* installFolderPath = NULL;

	static int progressCounter = 0;
	static BOOL isUnzippingDone = false;

	switch (msg)
	{
	case WM_COMMAND:
	{
		WORD id = LOWORD(wParam);

		switch (id)
		{
	

		case IDOK: // Will trigger downloaing and installing.
		{
			PWSTR theFolder2Select = NULL;
			theFolder2Select = PickDirectory(hWnd);

			if (theFolder2Select != NULL) {
				// Disable the Install button.
				EnableWindow(GetDlgItem(hWnd, IDOK), false);

				installFolderPath = theFolder2Select;

				HWND hwndStatusText = GetDlgItem(hWnd, IDC_STATUS_TEXT1);
				assert(hwndStatusText != NULL);
				SetWindowText(hwndStatusText, L"Downloading...");

				assert(installFolderPath != NULL);
				
				HANDLE hThread;
				DWORD threadID;
				hThread = CreateThread(NULL, // security attributes ( default if NULL )
					0, // stack SIZE default if 0
					threadFunctionForDownload, // Start Address
					installFolderPath, // input data
					0, // creational flag ( start if  0 )
					&threadID); // thread ID
			}

			break;
		}
		case IDCANCEL:
		{
			DestroyWindow(hWnd);
			return 0;
		}
		}
		break;
	}

	case WM_TIMER:
	{
		if (wParam == IDT_TIMER1) {
			HWND hwndPB = GetDlgItem(hWnd, IDC_PROGRESS1);
			assert(hwndPB != NULL);

			if (gCurrentDownloadingQuittedWithError == true) {
				static bool runOnceAlready = false;
				if (!runOnceAlready) {
					runOnceAlready = true;

					MessageBox(hWnd, L"Failed to download.", L"ERROR", MB_OK);
					PostQuitMessage(0);
					return 0;
				}
			}

			if (isUnzippingDone == false) {
				if (gCurrentDownloadingIsComplete == false) {
					SendMessage(hwndPB, PBM_SETPOS, (int)(gCurrentDownloadingPercentage * PROGRESSBAR_STEPS_FOR_DOWNLOADING / 100.0), 0);
					progressCounter = (int)(gCurrentDownloadingPercentage * PROGRESSBAR_STEPS_FOR_DOWNLOADING / 100.0);
				}
				else {
					SendMessage(hwndPB, PBM_SETPOS, PROGRESSBAR_STEPS_FOR_DOWNLOADING, 0);
					progressCounter = PROGRESSBAR_STEPS_FOR_DOWNLOADING;

					// Do unzipping in sync mode. Unzipping is typically not time-consuming.
					HWND hwndStatusText = GetDlgItem(hWnd, IDC_STATUS_TEXT1);
					assert(hwndStatusText != NULL);
					SetWindowText(hwndStatusText, L"Unzipping...");
					unzip2LocalFolder(installFolderPath);
					isUnzippingDone = true;
				}
			}
			else {
				if (progressCounter < PROGRESSBAR_STEPS_FOR_DOWNLOADING + PROGRESSBAR_STEPS_FOR_UNZIPPING) {
					progressCounter++;
					SendMessage(hwndPB, PBM_SETPOS, progressCounter, 0);
				}
				else if (progressCounter == PROGRESSBAR_STEPS_FOR_DOWNLOADING + PROGRESSBAR_STEPS_FOR_UNZIPPING) {
					// Don't quit. User can view 100% progress for a timer period.
					progressCounter++; // Increase the counter to enter the branch below on next timer.
				}
				else {
					// Progress bar goes to 100% and waits 1 time period for user to view 100% progress bar. Just quit now.
					PostQuitMessage(0);
					return 0;
				}
			}
		}
		break;
	}


	case WM_CREATE:
	{
		HWND dummyWnd;
		RECT rect;

		/* We want the OS to position the window, but this can't be done with windows created using CreateDialog.
		 * Therefore we create a temporary dummy window to see where it would have been positioned, and then move the
		 * dialog to that position. */
		dummyWnd = CreateWindowEx(0, TEXT("STATIC"), TEXT("STATIC"), 0, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, NULL, NULL,
			g_hInstance, NULL);
		GetWindowRect(dummyWnd, &rect);
		DestroyWindow(dummyWnd);

		/* Move the dialog to where the dummy window was positioned */
		SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

		SetTimer(hWnd,             // handle to main window 
			IDT_TIMER1,            // timer identifier 
			50,					// 0.05 second interval 
			(TIMERPROC)NULL);
		
		return 0;
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	}

	// Using DefWindowProc instead of DefDlgProc??, making it, in fact, a Window instead of a Dialog.
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

/* Register a class for our main window */
BOOL RegisterMainWindowClass()
{
	WNDCLASSEX wc;

	/* Class for our main window */
	wc.cbSize = sizeof(wc);
	wc.style = 0;
	wc.lpfnWndProc = &MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = DLGWINDOWEXTRA;
	wc.hInstance = g_hInstance;
	wc.hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE |
		LR_DEFAULTCOLOR | LR_SHARED);
	wc.hCursor = (HCURSOR)LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_SHARED);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = MainWndClass;
	wc.hIconSm = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	return (RegisterClassEx(&wc)) ? TRUE : FALSE;
}

/* Create an instance of our main window */
HWND CreateMainWindow()
{
	/* Create instance of main window */
	HWND hWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, NULL);

	HWND hwndPB = GetDlgItem(hWnd, IDC_PROGRESS1);
	assert(hwndPB != NULL);
	SendMessage(hwndPB, PBM_SETPOS, 0, 0);
	//SendMessage(hwndPB, PBM_SETRANGE, 0, PROGRESSBAR_STEPS_FOR_DOWNLOADING + PROGRESSBAR_STEPS_FOR_UNZIPPING);
	SendMessage(hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, PROGRESSBAR_STEPS_FOR_DOWNLOADING + PROGRESSBAR_STEPS_FOR_UNZIPPING));



	return hWnd;
}
