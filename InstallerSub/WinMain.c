#include "stdafx.h"
#include "Globals.h"
#include "MainWindow.h"
#include "Resource.h"
#include "objbase.h"

/* Global instance handle */
HINSTANCE g_hInstance = NULL;

/* Our application entry point */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  INITCOMMONCONTROLSEX icc;
  HWND hWnd;

  MSG msg;

  CoInitialize(NULL);

  /* Assign global HINSTANCE */
  g_hInstance = hInstance;

  /* Initialise common controls */
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&icc);

  /* Register our main window class, or error */
  if (!RegisterMainWindowClass())
  {
    MessageBox(NULL, TEXT("Error registering main window class."), TEXT("Error"), MB_ICONERROR | MB_OK);
    return 0;
  }

  /* Create our main window, or error */
  if (!(hWnd = CreateMainWindow()))
  {
    MessageBox(NULL, TEXT("Error creating main window."), TEXT("Error"), MB_ICONERROR | MB_OK);
    return 0;
  }

  /* Show main window and force a paint */
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  /* Main message loop */
  while (GetMessage(&msg, NULL, 0, 0) > 0)
  {
    if (!IsDialogMessage(hWnd, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return (int)msg.wParam;
}
