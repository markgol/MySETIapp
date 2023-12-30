//
// MySETIapp, a set tools for decoding bitstreams into various formats and manipulating those files
// FileFunctions.cpp
// (C) 2023, Mark Stegall
// Author: Mark Stegall
//
// This file is part of MySETIapp.
//
// MySETIapp is free software : you can redistribute it and /or modify it under
// the terms of the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// MySETIapp is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with MySETIapp.
// If not, see < https://www.gnu.org/licenses/>.//
//  FileFunctions.cpp
//
// Filename functions for gettting the filename for opening and saving
// Saving and restoring the main window size and position from one execution to the next
// Windows Error reporting message dialog
// Getting version info from the Windows 'resource' definitions
//
// These are file open, save and error reporting arebased on the examples given by Microsoft Docs
// https://docs.microsoft.com/en-us/windows/win32/learnwin32/example--the-open-dialog-box
// 
// The Get Verion info function was based on the original post by Mark Ransom on stack overflow:
// https://stackoverflow.com/questions/316626/how-do-i-read-from-a-version-resource-in-visual-c
// It was changed to include more complete information from from the Version resource and resolve
// syntax errors.
//
// Application standardized error numbers for functions that perform transform processes:
//      1 - success
//      0 - parameter or image header problem
//     -1 memory allocation failure
//     -2 open file failure
//     -3 file read failure
//     -4 incorect file type
//     -5 file sizes mismatch
//     -6 not yet implemented
//
// Some function return TRUE/FALSE results
// 
// V1.0.0.1 2023-08-20, Initial Release
// V1.1.0.1 2023-08-22  Added file type specifications to open/save dialogs
// V1.2.1.1 2023-09-06  Added ImportBMP
//                      Added Hex2Binary
//                      Corrected ImageDlg to display results file only once.
// V1.2.2.1 2023-09-06  Path must exist check is done on selected files in dialogs.
//                      Corrected bug introduced in V1.2.1 where incorrect frame size
//                      was used in the ImportBMP function.
// V1.2.4.1 2023-09-09  Added import CamIRa IMG file
//                      Clean up of file open/save filename handling
// V1.2.5.1 2023-09-09  Updated default directory handling
// V1.2.6.1 2023-09-24  Correction, Display Image/BMP file was not displaying BMP file
//                      Changed, display of 16, 32 bit image data, using scaled BMP file.
//                      The BMP file is still only a 8bpp file.
//                      (not applicable to A Sign in Space project)
// V1.2.8.1 2023-10-18  Add filesize function
// V1.2.10.1 2023-11-5  Changed, Export file, output file default is same as input file with .bmp extension
//                      Changed, ExportBMP, added automatically saving a matching .png using a global flag  
// V1.3.1.1 2023-12-6   Moved app functions from FileFunction.cpp to AppFunctions.cpp
//
#include "framework.h"
#include "resource.h"
#include <shobjidl.h>
#include <winver.h>
#include <vector>
#include <atlstr.h>
#include <gdiplus.h>
#include <strsafe.h>
#include "AppErrors.h"
#include "AppFunctions.h"
#include "ImageDialog.h"
#include "globals.h"

//****************************************************************
//
// Saves the position of a window
//
//****************************************************************
void SaveWindowPlacement(HWND hWnd, CString& Section)
{
    CString csString;
    // save window size and location for next time
    WINDOWPLACEMENT MainWindowPlacement;
    GetWindowPlacement(hWnd, &MainWindowPlacement);
    csString.Format(L"%u", MainWindowPlacement.showCmd);
    WritePrivateProfileString(Section, L"showCmd", csString, (LPCTSTR)strAppNameINI);
    csString.Format(L"%u", MainWindowPlacement.flags);
    WritePrivateProfileString(Section, L"flags", csString, (LPCTSTR)strAppNameINI);
    csString.Format(L"%ld", MainWindowPlacement.ptMaxPosition.x);
    WritePrivateProfileString(Section, L"ptMaxPosition.x", csString, (LPCTSTR)strAppNameINI);
    csString.Format(L"%ld", MainWindowPlacement.ptMaxPosition.y);
    WritePrivateProfileString(Section, L"ptMaxPosition.y", csString, (LPCTSTR)strAppNameINI);
    csString.Format(L"%ld", MainWindowPlacement.ptMinPosition.x);
    WritePrivateProfileString(Section, L"ptMinPosition.x", csString, (LPCTSTR)strAppNameINI);
    csString.Format(L"%ld", MainWindowPlacement.ptMinPosition.y);
    WritePrivateProfileString(Section, L"ptMinPosition.y", csString, (LPCTSTR)strAppNameINI);
    csString.Format(L"%ld", MainWindowPlacement.rcNormalPosition.bottom);
    WritePrivateProfileString(Section, L"bottom", csString, (LPCTSTR)strAppNameINI);
    csString.Format(L"%ld", MainWindowPlacement.rcNormalPosition.left);
    WritePrivateProfileString(Section, L"left", csString, (LPCTSTR)strAppNameINI);
    csString.Format(L"%ld", MainWindowPlacement.rcNormalPosition.right);
    WritePrivateProfileString(Section, L"right", csString, (LPCTSTR)strAppNameINI);
    csString.Format(L"%ld", MainWindowPlacement.rcNormalPosition.top);
    WritePrivateProfileString(Section, L"top", csString, (LPCTSTR)strAppNameINI);
}

//****************************************************************
//
// Restores the position of a window
//
//****************************************************************
BOOL RestoreWindowPlacement(HWND hWnd, CString& Section)
{
    WCHAR szString[MAX_PATH + 1];
    DWORD dRes;
    int iRes;

    // save window size and location for next time
    WINDOWPLACEMENT MainWindowPlacement;

    dRes = GetPrivateProfileString(Section, L"showCmd", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
    if (dRes == 0)
    {
        return FALSE;
    }
    else
    {
        iRes = swscanf_s(szString, L"%u", &MainWindowPlacement.showCmd);
        if (iRes == 0) {
            return FALSE;
        }
    }

    dRes = GetPrivateProfileString(Section, L"flags", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
    if (dRes == 0)
    {
        return FALSE;
    }
    else
    {
        iRes = swscanf_s(szString, L"%u", &MainWindowPlacement.flags);
        if (iRes == 0) {
            return FALSE;
        }
    }

    dRes = GetPrivateProfileString(Section, L"ptMaxPosition.x", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
    if (dRes == 0)
    {
        return FALSE;
    }
    else
    {
        iRes = swscanf_s(szString, L"%ld", &MainWindowPlacement.ptMaxPosition.x);
        if (iRes == 0) {
            return FALSE;
        }
    }

    dRes = GetPrivateProfileString(Section, L"ptMaxPosition.y", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
    if (dRes == 0)
    {
        return FALSE;
    }
    else
    {
        iRes = swscanf_s(szString, L"%ld", &MainWindowPlacement.ptMaxPosition.y);
        if (iRes == 0) {
            return FALSE;
        }
    }

    dRes = GetPrivateProfileString(Section, L"ptMinPosition.x", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
    if (dRes == 0)
    {
        return FALSE;
    }
    else
    {
        iRes = swscanf_s(szString, L"%ld", &MainWindowPlacement.ptMinPosition.x);
        if (iRes == 0) {
            return FALSE;
        }
    }

    dRes = GetPrivateProfileString(Section, L"ptMinPosition.y", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
    if (dRes == 0)
    {
        return FALSE;
    }
    else
    {
        iRes = swscanf_s(szString, L"%ld", &MainWindowPlacement.ptMinPosition.y);
        if (iRes == 0) {
            return FALSE;
        }
    }

    dRes = GetPrivateProfileString(Section, L"bottom", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
    if (dRes == 0)
    {
        return FALSE;
    }
    else
    {
        iRes = swscanf_s(szString, L"%ld", &MainWindowPlacement.rcNormalPosition.bottom);
        if (iRes == 0) {
            return FALSE;
        }
    }

    dRes = GetPrivateProfileString(Section, L"left", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
    if (dRes == 0)
    {
        return FALSE;
    }
    else
    {
        iRes = swscanf_s(szString, L"%ld", &MainWindowPlacement.rcNormalPosition.left);
        if (iRes == 0) {
            return FALSE;
        }
    }

    dRes = GetPrivateProfileString(Section, L"right", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
    if (dRes == 0)
    {
        return FALSE;
    }
    else
    {
        iRes = swscanf_s(szString, L"%ld", &MainWindowPlacement.rcNormalPosition.right);
        if (iRes == 0) {
            return FALSE;
        }
    }

    dRes = GetPrivateProfileString(Section, L"top", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
    if (dRes == 0)
    {
        return FALSE;
    }
    else
    {
        iRes = swscanf_s(szString, L"%ld", &MainWindowPlacement.rcNormalPosition.top);
        if (iRes == 0) {
            return FALSE;
        }
    }
    MainWindowPlacement.length = sizeof(WINDOWPLACEMENT);
    if (!SetWindowPlacement(hWnd, &MainWindowPlacement))
    {
        LastErrorMsg(L"SetWindowPlacement");
        return FALSE;
    }

    return TRUE;
}

//****************************************************************
//
// This displays the last system error message
// 
//  This is from Microsoft:
//  https://docs.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
//
//****************************************************************
void LastErrorMsg(LPCTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));

    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);

    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw);
}

//****************************************************************
//
// This function was originally posted on stack overflow:
// https://stackoverflow.com/questions/316626/how-do-i-read-from-a-version-resource-in-visual-c, Mark Ransom
//  It has beedn changed to also include readback of the copyright, name and exectuable filename
//  Corrected SetString parameter cast from (LPCSTR) to (LPCTSTR) to allow generic use of CString type rather than only CStringA
//
//  This function retrieves the Product Name, Program Version, Name (typically license information), and CopyRight from the version
//  structure in this application.  It also reports the full application EXE filename plus full path to the EXE.
//  Application exe name from the version structure in this application.  It is typically reported in the About Dlg
// 
//****************************************************************
BOOL GetProductAndVersion(CString* strProductName, CString* strProductVersion,
    CString* strName, CString* strCopyright, CString* strAppNameEXE)
{

    // get the filename of the executable containing the version resource
    TCHAR szFilename[MAX_PATH + 1] = { 0 };
    if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0)
    {
        //MessageBox("GetModuleFileName failed with error %d\n", GetLastError());
        return false;
    }

    // allocate a block of memory for the version info
    DWORD dummy;
    DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
    if (dwSize == 0)
    {
        //TRACE("GetFileVersionInfoSize failed with error %d\n", GetLastError());
        return false;
    }
    std::vector<BYTE> data(dwSize);

    // load the version info
    if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0]))
    {
        //TRACE("GetFileVersionInfo failed with error %d\n", GetLastError());
        return false;
    }

    // get the name and version strings
    LPVOID pvProductName = NULL;
    unsigned int iProductNameLen = 0;
    LPVOID pvProductVersion = NULL;
    unsigned int iProductVersionLen = 0;
    LPVOID pvCopyright = NULL;
    unsigned int iCopyrightLen = 0;
    LPVOID pvName = NULL;
    unsigned int iNameLen = 0;

    // replace "040904b0" with the language ID of your resources, Block Header ID English US is 040904b0
    if (!VerQueryValue(&data[0], _T("\\StringFileInfo\\040904b0\\ProductName"), &pvProductName, &iProductNameLen) ||
        !VerQueryValue(&data[0], _T("\\StringFileInfo\\040904b0\\ProductVersion"), &pvProductVersion, &iProductVersionLen) ||
        !VerQueryValue(&data[0], _T("\\StringFileInfo\\040904b0\\LegalCopyright"), &pvCopyright, &iCopyrightLen) ||
        !VerQueryValue(&data[0], _T("\\StringFileInfo\\040904b0\\CompanyName"), &pvName, &iNameLen))
    {
        //TRACE("Can't obtain ProductName, ProductVersion, Copyright or Name from resources\n");
        return false;
    }

    // Copy results to function parameters
    strProductName->SetString((LPCTSTR)pvProductName, iProductNameLen);
    strProductVersion->SetString((LPCTSTR)pvProductVersion, iProductVersionLen);
    strName->SetString((LPCTSTR)pvName, iNameLen);
    strCopyright->SetString((LPCTSTR)pvCopyright, iCopyrightLen);
    strAppNameEXE->SetString(szFilename);

    return true;
}

//******************************************************************************
//
// MessageMySETIappError
//
//******************************************************************************
void MessageMySETIappError(HWND hWnd, int ErrNo, const wchar_t* Title) {
    switch (ErrNo) {
    case 1:
        MessageBox(hWnd, L"Success", Title, MB_OK);
        break;

    case 0:
        MessageBox(hWnd, L"Parameter or format invalid", Title, MB_OK);
        break;

    case -1:
        MessageBox(hWnd, L"Memory allocation failure\nexit application", Title, MB_OK);
        break;

    case -2:
        MessageBox(hWnd, L"File was not found or could not be opened", Title, MB_OK);
        break;

    case -3:
        MessageBox(hWnd, L"File read error", Title, MB_OK);
        break;

    case -4:
        MessageBox(hWnd, L"Invalid file type", Title, MB_OK);
        break;

    case -5:
        MessageBox(hWnd, L"Sizes mismatched", Title, MB_OK);
        break;

    case -6:
        MessageBox(hWnd, L"Not yet implemented", Title, MB_OK);
        break;

    default:
        break;
    }
    return;
}

