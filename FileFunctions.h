#pragma once
// 
// function prototypes
//
int CCFileSave(HWND hWnd, LPWSTR pszCurrentFilename, LPWSTR* pszFilename, BOOL bSelectFolder);
int CCFileOpen(HWND hWnd, LPWSTR pszCurrentFilename, LPWSTR* pszFilename, BOOL bSelectFolder);
void SaveWindowPlacement(HWND hWnd, CString& Section);
BOOL RestoreWindowPlacement(HWND hWnd, CString& Section);
void LastErrorMsg(LPCTSTR lpszFunction);
void ExportFile(HWND hWnd, int wmId);
int SaveBMP(WCHAR* Filename, WCHAR* InputFile, int RGBframes, int AutoScale);
int SaveTXT(WCHAR* Filename, WCHAR* InputFile);
int DisplayImage(WCHAR* Filename);

BOOL GetProductAndVersion(CString* strProductName,
    CString* strProductVersion,
    CString* strName,
    CString* strCopyright,
    CString* strAppNameEXE
);
