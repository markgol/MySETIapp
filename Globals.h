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
extern BOOL DisplayResults;
extern BOOL AutoScaleResults;
extern BOOL DefaultRBG;
extern BOOL AutoSize;
extern BOOL AutoPNG;
extern BOOL ShowStatusBar;
extern ImageDialog* ImgDlg;
extern HWND hwndMain;
extern HWND hwndImage;
extern HINSTANCE hInst;
extern INT_PTR CALLBACK ImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
