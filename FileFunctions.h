#pragma once
// 
// function prototypes
//
BOOL CCFileSave(HWND hWnd, LPWSTR pszCurrentFilename, LPWSTR* pszFilename,
BOOL bSelectFolder, int NumTypes, COMDLG_FILTERSPEC* FileTypes, LPCWSTR szDefExt);
BOOL CCFileOpen(HWND hWnd, LPWSTR pszCurrentFilename, LPWSTR* pszFilename,
BOOL bSelectFolder, int NumTypes, COMDLG_FILTERSPEC* FileTypes, LPCWSTR szDefExt);
void SaveWindowPlacement(HWND hWnd, CString& Section);
BOOL RestoreWindowPlacement(HWND hWnd, CString& Section);
void LastErrorMsg(LPCTSTR lpszFunction);
void ExportFile(HWND hWnd, int wmId);
int SaveBMP(WCHAR* Filename, WCHAR* InputFile, int RGBframes, int AutoScale);
int SaveTXT(WCHAR* Filename, WCHAR* InputFile);
int DisplayImage(WCHAR* Filename);
int ImportBMP(HWND hWnd);
int HEX2Binary(HWND hWnd);
int CamIRaImport(HWND hWnd);

BOOL GetProductAndVersion(CString* strProductName,
    CString* strProductVersion,
    CString* strName,
    CString* strCopyright,
    CString* strAppNameEXE
);
