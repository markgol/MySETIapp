#pragma once
//
// GLobal variables
// 

extern CString strProductName;
extern CString strProductVersion;
extern CString strName;
extern CString strCopyright;
extern CString strAppNameEXE;
extern CString strAppNameINI;
extern WCHAR szBMPFilename[MAX_PATH];
extern WCHAR szCurrentFilename[MAX_PATH];
extern WCHAR szBMPFilename[MAX_PATH];
extern WCHAR szTempImageFilename[MAX_PATH];
extern int DisplayResults;
extern int AutoScaleResults;
extern int DefaultRBG;
extern int AutoSize;
extern HWND hwndMain;
extern HWND hwndImage;
extern HINSTANCE hInst;
extern INT_PTR CALLBACK ImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern void MessageMySETIappError(HWND hWnd, int ErrNo, const wchar_t* Title);
