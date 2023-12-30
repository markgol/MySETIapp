#pragma once

BOOL GetProductAndVersion(CString* strProductName,
    CString* strProductVersion,
    CString* strName,
    CString* strCopyright,
    CString* strAppNameEXE
);

void SaveWindowPlacement(HWND hWnd, CString& Section);
BOOL RestoreWindowPlacement(HWND hWnd, CString& Section);
void LastErrorMsg(LPCTSTR lpszFunction);
void MessageMySETIappError(HWND hWnd, int ErrNo, const wchar_t* Title);
