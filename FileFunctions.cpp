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
// V1.1.0.1 2023-08-22, Added file type specifications to open/save dialogs
//
#include "framework.h"
#include "resource.h"
#include <shobjidl.h>
#include <winver.h>
#include <vector>
#include <atlstr.h>
#include "globals.h"
#include <strsafe.h>
#include "FileFunctions.h"
#include "imaging.h"

//****************************************************************
//
// Generic File Open function
// This uses the current style file dialog for the Windows OS version
//
// BOOL CCFileOpen(HWND hWnd, LPWSTR pszCurrentFilename, LPWSTR* pszFilename,
//                 int NumTypes, COMDLG_FILTERSPEC* FilterSpec)
// 
//      LPWSTR  pszCurrentFilename - if not "" then use as a default file selection
//      LPWSTR* pszFilename - return the selected filename (inluding path) if 'Open' selected
//      BOOL    bSelectFolder - TRUE, select only the folder not a file
//      int     NumTypes - 0 none specifed other wise number in list
//      COMDLG_FILTERSPEC* pointer to file types list
//
// example of file type specifier:
// COMDLG_FILTERSPEC rgSpec[] =
//{
//    { L"Image files", L"*.raw" },
//    { L"All Files", L"*.*" },
//};
// 
// return
//      TRUE - 'Open' selected, pszFilename is valid
//      FALSE-  user cancelled file open
// 
//****************************************************************
BOOL CCFileOpen(HWND hWnd, LPWSTR pszCurrentFilename, LPWSTR* pszFilename,
                BOOL bSelectFolder, int NumTypes, COMDLG_FILTERSPEC* FileTypes,
                LPCWSTR szDefExt)
{
BOOL bReturnValue = FALSE;
// Initialize COM
HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
if (SUCCEEDED(hr))
{
    IFileOpenDialog *pFileOpen = NULL;

    // Create the COM FileOpenDialog object.
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

    if (SUCCEEDED(hr))
    {
        if (!bSelectFolder) {
            hr = pFileOpen->SetOptions(FOS_FORCEFILESYSTEM);
            if (NumTypes != 0 && FileTypes!=NULL) {
                hr = pFileOpen->SetFileTypes(NumTypes, FileTypes);
            }
            if (szDefExt != NULL) {
                hr = pFileOpen->SetDefaultExtension(szDefExt);
            }
        } else {
            hr = pFileOpen->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM); // This selects only a folder, not a filename
        }
        
        if (wcscmp(pszCurrentFilename, L"") != 0)
        {
            hr = pFileOpen->SetFileName(pszCurrentFilename);
        }

        // Show the Open dialog box.
        hr = pFileOpen->Show(NULL);

        // Get the file name from the dialog box.
        if (SUCCEEDED(hr))
        {
            IShellItem* pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                // The allocation for pszFIlename by GetDisplayName must be freed when no longer used
                // using CoTaskMemFree(pszFilename);
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, pszFilename);
                pItem->Release();
                bReturnValue = TRUE;
            }
        }
        pFileOpen->Release();
    }
    // release COM
    CoUninitialize();
} else {
    MessageBox(hWnd, (LPCWSTR) L"Failed to create open file resource\nFatal Error", (LPCWSTR)L"Open file dialog", IDOK);
    return FALSE;
}

return bReturnValue;
}

//****************************************************************
//
// Generic File Save function
// This uses the current style file dialog for the Windows OS version
//
// BOOL CCFileSave(HWND hWnd, LPWSTR pszCurrentFilename, LPWSTR* pszFilename)
// 
//      LPWSTR pszCurrentFilename - if not "" then use as a default file selection
//      LPWSTR* pszFilename - return the selected filename (inluding path) if 'Open' selected
//      BOOL    bSelectFolder - TRUE, select only the folder not a file
//      int     NumTypes - 0 none specifed other wise number in list
//      COMDLG_FILTERSPEC* pointer to file types list
//
// example of file type specifier:
// COMDLG_FILTERSPEC rgSpec[] =
//{
//    { szBMP, L"*.raw" },
//    { szAll, L"*.*" },
//};
// 
//  return
//      TRUE - 'Open' selected, pszFilename is valid
//      FALSE-  user cancelled file open
// 
//  This function also asks for verification for overwriting the selected
//  file if it exists (only if bSelectFolder is FALSE)
// 
//****************************************************************
BOOL CCFileSave(HWND hWnd, LPWSTR pszCurrentFilename, LPWSTR* pszFilename,
                BOOL bSelectFolder, int NumTypes, COMDLG_FILTERSPEC* FileTypes,
                LPCWSTR szDefExt)
{
    BOOL bReturnValue = FALSE;

    // Initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        IFileOpenDialog *pFileSave=NULL;

        // Create the FileOpenDialog object.
        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
            IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

        if (SUCCEEDED(hr))
        {
            if (!bSelectFolder) {
                hr = pFileSave->SetOptions(FOS_FORCEFILESYSTEM | FOS_OVERWRITEPROMPT);
                if (NumTypes != 0 && FileTypes != NULL) {
                    hr = pFileSave->SetFileTypes(NumTypes, FileTypes);
                }
                if (szDefExt != NULL) {
                    hr = pFileSave->SetDefaultExtension(szDefExt);
                }
            } else {
                hr = pFileSave->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM); // This selects only a folder, not a filename
            }
            
            if (wcscmp(pszCurrentFilename, L"") != 0)
            {
                hr = pFileSave->SetFileName(pszCurrentFilename);
            }
            
            // Show the Open dialog box.
            hr = pFileSave->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem;
                hr = pFileSave->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    // The allocation for pszFIlename by GetDisplayName must be freed when no longer used
                    // using CoTaskMemFree(pszFilename);
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, pszFilename);
                    pItem->Release();
                }
                bReturnValue = TRUE;
            }
            pFileSave->Release();
        }
        // release COM
        CoUninitialize();
    } else {
        MessageBox(hWnd, (LPCWSTR) L"Failed to create save file resource\nFatal Error", (LPCWSTR)L"Save file dialog", IDOK);
        return FALSE;
    }

    return bReturnValue;
}

//****************************************************************
//
// Saves the position of a window
//
//****************************************************************
void SaveWindowPlacement(HWND hWnd, CString &Section)
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

    dRes = GetPrivateProfileString(Section, L"showCmd",L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
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

//****************************************************************
//
//  This function displays a BMP or image file in 
//  the image display window.
//
//****************************************************************
int DisplayImage(WCHAR* Filename)
{
    // determine if .BMP or .RAW file type by examing the header
    // record in the file.
    // if the file is an image file ten convert it to a BMP
    // file in order to dusplay it.  Use the filename in global
    // szBMPFilename then update the image.  If iot is a BMP file
    // then copy it to the filename in global szBMPFilename.
    IMAGINGHEADER ImageHeader;
    size_t iRes;

    iRes = ReadImageHeader(Filename, &ImageHeader);
    if (iRes== 1) {
        // this is a image file
        // convert to BMP file
        iRes = SaveBMP(szBMPFilename, Filename, DefaultRBG, AutoScaleResults);
        if (iRes !=1) {
            return (int) iRes;
        }
        // trigger redraw
        if (!IsWindow(hwndImage)) {
            hwndImage = CreateDialog(hInst, MAKEINTRESOURCE(IDD_IMAGE), hwndMain, ImageDlg);
        }
        if (hwndImage != NULL) {
            InvalidateRect(hwndImage, NULL, TRUE);
            ShowWindow(hwndImage, SW_SHOW);
        }
        return 1;
    }

    // This should be a BMP file
    // read BMPheader to verify
    BITMAPFILEHEADER BMPheader;
    
    errno_t ErrNum;
    FILE* In;

    ErrNum = _wfopen_s(&In, Filename, L"rb");
    if (In == NULL) {
        return -2;
    }
    iRes = fread(&BMPheader, sizeof(BMPheader), 1, In);
    if (iRes != 1) {
        fclose(In);
        return -4;
    }
    if (BMPheader.bfType != 0x4d42 || BMPheader.bfReserved1 != 0 || BMPheader.bfReserved2 != 0) {
        fclose(In);
        return -4;
    }
    fclose(In);

    // copy filename to szBMPFilename
    if (!CopyFile(Filename, szBMPFilename, FALSE)) {
        return -3;
    }
    // trigger redraw
    if (!IsWindow(hwndImage)) {
        hwndImage = CreateDialog(hInst, MAKEINTRESOURCE(IDD_IMAGE), hwndMain, ImageDlg);
    }
    if (hwndImage != NULL) {
        InvalidateRect(hwndImage, NULL, TRUE);
        ShowWindow(hwndImage, SW_SHOW);
    }
    return 1;
}

//****************************************************************
//
//  This function is called from the main app message handler in response
//  to an export file menu selection.  It is not intened to be used as
//  a generic file function.
// 
//  This function is used to export a 'image file' to a particular file type
//  file types currently supported for export:
//      .txt    text file version of 'image' file, white space delimited
//      .bmp    Bitmap file of image
//              2 types of BMP files are supported
//              8 bit per pixel, first frame from file, can be auto scaled
//              24 bit per pixel, RGB interpetation, requires a 3 frame image file
//                  a frame is used for each color, 1st frame Red, 2nd frame Green
//                  3rd frame Blue, autoscaling stretchs each color independently
//                  from the others.
//              if input image is odd columns in size it is 0 padded to even size
//
//****************************************************************
void ExportFile(HWND hWnd, int wmId)
{
    PWSTR pszFilename;
    IMAGINGHEADER ImageHeader;
    WCHAR szString[MAX_PATH];
    WCHAR InputFile[MAX_PATH];
    int RGBframes = 0;
    int AutoScale = 0;

    // select name
    if (wmId != IDM_FILE_TOBMP && wmId != IDM_FILE_TOTXT) {
        MessageBox(hWnd, L"Export type is not supported", L"File incompatible", MB_OK);
        return;
    }

    COMDLG_FILTERSPEC RawType[] =
    {
         { L"Image files", L"*.raw" },
         { L"All Files", L"*.*" },
    };

    if (!CCFileOpen(hWnd, (LPWSTR)L"", &pszFilename, FALSE, 2, RawType, L".raw")) {
        return;
    }
    wcscpy_s(InputFile, pszFilename);
    CoTaskMemFree(pszFilename);


    if (ReadImageHeader(InputFile, &ImageHeader) != 1) {
        MessageBox(hWnd, L"Input file is not an image file", L"File incompatible", MB_OK);
        return;
    }

    if (ImageHeader.PixelSize != 1) {
        MessageBox(hWnd, L"Input file must be 1 byte per pixel", L"File incompatible", MB_OK);
        return;
    }
    if (ImageHeader.NumFrames == 3) {
        if (MessageBox(hWnd, L"Input file is 3 frames\nSave this as RGB bitmap file?",
            L"Color BMP", MB_YESNO) == IDYES) {
            RGBframes = 1;;
        }
    }

    if (!RGBframes && ImageHeader.NumFrames != 1) {
        if (MessageBox(hWnd, L"Input file is multi-frame\nonly 1st frame will be used\nContinue?",
            L"Multiple frames", MB_YESNO) != IDYES) {
            return;
        }
    }
    if (!RGBframes) {
        if (MessageBox(hWnd, L"Autoscale image, black to white?",
            L"Greyscale image Scaling", MB_YESNO) == IDYES) {
            AutoScale = 1;        }
    }
    else {
        if (MessageBox(hWnd, L"Autoscale image, each color?",
            L"Color image Scaling", MB_YESNO) == IDYES) {
            AutoScale = 1;
        }
    }

    if (wmId == IDM_FILE_TOBMP) {
        COMDLG_FILTERSPEC BMPType[] =
        {
             { L"BMP files", L"*.bmp" },
             { L"All Files", L"*.*" },
        };
        if (!CCFileSave(hWnd, (LPWSTR) L"", &pszFilename, FALSE, 2, BMPType, L".bmp")) {
            return;
        }
    }
    else if (wmId == IDM_FILE_TOTXT) {
        COMDLG_FILTERSPEC TXTType[] =
        {
             { L"Image files", L"*.txt" },
             { L"All Files", L"*.*" },
        };
        if (!CCFileSave(hWnd, (LPWSTR)L"", &pszFilename, FALSE, 2, TXTType, L".txt")) {
            return;
        }
    }

    wcscpy_s(szString, pszFilename);
    CoTaskMemFree(pszFilename);

    // convert to file
    if (wmId == IDM_FILE_TOBMP) {
        SaveBMP(szString, InputFile,RGBframes, AutoScale);
    }
    else if (wmId == IDM_FILE_TOTXT) {
        SaveTXT(szString, InputFile);
    }

    return;
}

//****************************************************************
//
//  SaveBMP
// 
//  This a generic function is used to export a 'image file' to a BMP file type.
//  It does not have any uer interaction.
// 
//  Parmeters:
//      Filename - BMP output file
//      InputFile - Image file for to be exported
//      RGBframes - RBG/Greyscale interpetation flag (see below)
//                  This flag is ignored if the # of frames in the
//                  image is not a multiple of 3.
//      AutoScale - auto scale the output image
// 
//  Xsize of image must be <= 8192
// 
//  2 types of BMP files are supported:
//
//  Input parameter RBGframes FALSE 
//  8 bit per pixel, first frame from file
//      AutoScale parameter TRUE, auto scale as greyscale image, 0 to 255
//
//  Input parameter RBGframes TRUE
//  24 bit per pixel, RGB interpetation, requires a 3 frame image file
//          a frame is used for each color, 1st frame Red, 2nd frame Green
//          3rd frame Blue, autoscaling stretchs each color independently
//          from the others. Only the first 3 frames are used.
//
//  If input image is odd columns in size it is 0 padded to even size.
//  
//  return value:
//  1 - Success
//  see standardized app error list at top of this source file
//
//****************************************************************
int SaveBMP(WCHAR* Filename, WCHAR* InputFile,int RGBframes, int AutoScale)
{
    int iRes;
    int* InputImage;
    IMAGINGHEADER ImageHeader;
    BITMAPFILEHEADER BMPheader;
    BITMAPINFOHEADER BMPinfoheader;
    RGBQUAD ColorTable[256];
    BYTE* BMPimage;

    iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
    if (iRes != 1) {
        return iRes;
    }

    if (ImageHeader.PixelSize != 1) {
        return 0;
    }

    if (ImageHeader.Xsize > 8192) {
        return 0;
    }

    if (RGBframes && ImageHeader.NumFrames%3!=0) {
        RGBframes = 0;
    }

    DWORD BMPimageBytes;
    int biWidth;

    if (RGBframes) {
        //
        //  24 bpp BMP output type
        // 
        // Frame 1 RED
        // Frame 2 Green
        // Frame 3 BLUE
        //
        // convert image data to 24 bit DIB/BMP format.
        // If xsize is odd add pad to even size.
        // Pad as required in BMP 'stride' size.
        //  
        // only three frames file saved.
        //
        // scale display using RGBQUAD color map
        //
        int RedMin, RedMax;
        int GreenMin, GreenMax;
        int BlueMin, BlueMax;
        int RedOffset;
        int GreenOffset;
        int BlueOffset;
        int ImagePixelRed;
        int ImagePixelGreen;
        int ImagePixelBlue;
        int InputFrameSize;
        int Stride;
        float ScaleRed, OffsetRed;
        float ScaleGreen, OffsetGreen;
        float ScaleBlue, OffsetBlue;

        InputFrameSize = ImageHeader.Xsize * ImageHeader.Ysize;

        if (AutoScale) {
            // scan image for red,green,blue stats for scaling
            RedOffset=0;
            GreenOffset = InputFrameSize;
            BlueOffset = 2 * InputFrameSize;

            if (InputImage[RedOffset] < 0) {
                RedMin = RedMax = 0;
            }
            else if (InputImage[RedOffset] > 255) {
                RedMin = RedMax = 255;
            }
            else {
                RedMin = RedMax = InputImage[RedOffset];
            }

            if (InputImage[GreenOffset] < 0) {
                GreenMin = GreenMax = 0;
            }
            else if (InputImage[GreenOffset] > 255) {
                GreenMin = GreenMax = 255;
            }
            else {
                GreenMin = GreenMax = InputImage[GreenOffset];
            }

            if (InputImage[BlueOffset] < 0) {
                BlueMin = BlueMax = 0;
            }
            else if (InputImage[BlueOffset] > 255) {
                BlueMin = BlueMax = 255;
            }
            else {
                BlueMin = BlueMax = InputImage[BlueOffset];
            }

            for (int i = 0; i < InputFrameSize; i++) {
                ImagePixelRed = InputImage[i + RedOffset];
                ImagePixelGreen = InputImage[i + GreenOffset];
                ImagePixelBlue = InputImage[i + BlueOffset];

                if (ImagePixelRed < RedMin) 
                    RedMin = ImagePixelRed;
                if (ImagePixelGreen < GreenMin) 
                    GreenMin = ImagePixelGreen;
                if (ImagePixelBlue < BlueMin) 
                    BlueMin = ImagePixelBlue;

                if (ImagePixelRed > RedMax) 
                    RedMax = ImagePixelRed;
                if (ImagePixelGreen > GreenMax) 
                    GreenMax = ImagePixelGreen;
                if (ImagePixelBlue > BlueMax) 
                    BlueMax = ImagePixelBlue;
            }
            // calculate scaling
            ScaleRed = (float)255.0 / ((float)RedMax - (float)RedMin);
            OffsetRed = (float)0.0 - (ScaleRed * (float)RedMin);

            ScaleGreen = (float)255.0 / ((float)GreenMax - (float)GreenMin);
            OffsetGreen = (float)0.0 - (ScaleRed * (float)GreenMin);

            ScaleBlue = (float)255.0 / ((float)BlueMax - (float)BlueMin);
            OffsetBlue = (float)0.0 - (ScaleBlue * (float)BlueMin);
        }
        else {
            // No Scaling
            ScaleRed = 1.0;
            ScaleBlue = 1.0;
            ScaleGreen = 1.0;
            OffsetRed = 0.0;
            OffsetBlue = 0.0;
            OffsetGreen = 0.0;
        }

        // correct for odd column size
        biWidth = ImageHeader.Xsize;
        if (biWidth % 2 != 0) {
            // make sure bitmap width is even
            biWidth++;
        }

        // BMP files have a specific requirement for # of bytes per line
        // This is called stride.  The formula used is from the specification.
        Stride = ((((biWidth * 24) + 31) & ~31) >> 3); // 24 bpp
        BMPimageBytes = Stride * ImageHeader.Ysize; // size of image in bytes

        // allocate zero paddded image array
        BMPimage = (BYTE*)calloc(BMPimageBytes, 1);
        if (BMPimage == NULL) {
            delete[] InputImage;
            return -1;
        }

        int BMPOffset;
        BYTE PixelRed, PixelGreen, PixelBlue;

        // copy input image to BMPimage DIB format
        for (int y = 0; y < ImageHeader.Ysize; y++) {
            BlueOffset = y * ImageHeader.Xsize;
            GreenOffset = y * ImageHeader.Xsize + InputFrameSize;
            RedOffset = y * ImageHeader.Xsize + (2* InputFrameSize);
            BMPOffset = y * Stride;
            for (int x = 0; x < ImageHeader.Xsize; x++) {
                ImagePixelRed = InputImage[RedOffset + x];
                ImagePixelGreen = InputImage[GreenOffset + x];
                ImagePixelBlue = InputImage[BlueOffset + x];
                // apply scaling
                ImagePixelRed = (int)(ScaleRed * (float)ImagePixelRed + OffsetRed + 0.5);
                if (ImagePixelRed < 0) {
                    PixelRed = 0;
                }
                else if (ImagePixelRed > 255) {
                    PixelRed = 255;
                }
                else {
                    PixelRed = ImagePixelRed;
                }

                ImagePixelGreen = (int)(ScaleGreen * (float)ImagePixelGreen + OffsetGreen + 0.5);
                if (ImagePixelGreen < 0) {
                    PixelGreen = 0;
                }
                else if (ImagePixelGreen > 255) {
                    PixelGreen = 255;
                }
                else {
                    PixelGreen = ImagePixelGreen;
                }

                ImagePixelBlue = (int)(ScaleBlue * (float)ImagePixelBlue + OffsetBlue + 0.5);
                if (ImagePixelBlue < 0) {
                    PixelBlue = 0;
                }
                else if (ImagePixelBlue > 255) {
                    PixelBlue = 255;
                }
                else {
                    PixelBlue = ImagePixelBlue;
                }
                BMPimage[BMPOffset + (x * 3)] = PixelRed;
                BMPimage[BMPOffset + (x * 3) + 1] = PixelGreen;
                BMPimage[BMPOffset + (x * 3) + 2] = PixelBlue;
            }
        }
    }
    else {
        //
        // convert image data to 8 bit DIB/BMP format.
        // If xsize is odd add pad to even size.
        // Pad as required in BMP 'stride' size.
        //  
        // only first frame file saved.
        //
        // scale display using RGBQUAD color map
        //
        int PixelMin, PixelMax;
        int InputFrameSize;
        int Stride;
        float Scale, Offset;

        if (InputImage[0] < 0) {
            PixelMin = PixelMax = 0;
        }
        else if(InputImage[0] > 255) {
            PixelMin = PixelMax = 255;
        }
        else {
            PixelMin = PixelMax = InputImage[0];
        }

        InputFrameSize = ImageHeader.Xsize * ImageHeader.Ysize;

        // correct for odd column size
        biWidth = ImageHeader.Xsize;
        if (biWidth % 2 != 0) {
            // make sure bitmap width is even
            biWidth++;
        }

        // BMP files have a specific requirement for # of bytes per line
        // This is called stride.  The formula used is from the specification.
        Stride = ((((biWidth * 8) + 31) & ~31) >> 3); // 8 bpp
        BMPimageBytes = Stride * ImageHeader.Ysize; // size of image in bytes

        // allocate zero paddded image array
        BMPimage = (BYTE*) calloc(BMPimageBytes, 1);
        if (BMPimage == NULL) {
            delete[] InputImage;
            return -1;
        }

        int InputOffset;
        int BMPOffset;
        BYTE Pixel;
        int ImagePixel;

        // copy image to BMPimage
        for (int y = 0; y < ImageHeader.Ysize; y++) {
            InputOffset = y * ImageHeader.Xsize;
            BMPOffset = y * Stride;
            for (int x = 0; x < ImageHeader.Xsize; x++) {
                ImagePixel = InputImage[InputOffset + x];
                if (ImagePixel < 0) {
                    Pixel = 0;
                }
                else if (ImagePixel > 255) {
                    Pixel = 255;
                }
                else {
                    Pixel = ImagePixel;
                }
                if (Pixel < PixelMin) PixelMin = Pixel;
                if (Pixel > PixelMax) PixelMax = Pixel;
                BMPimage[BMPOffset + x] = Pixel;
            }
        }

        // scaling applies to colormap only
        if (AutoScale) {
            // compute scaling: Scale, Offset
            if (PixelMax == PixelMin) {
                // array is all the same value
                // Make Image all white
                Scale = 0;
                Offset = 255;
            }
            else {
                Scale = (float)255.0 / ((float)PixelMax - (float)PixelMin);
                Offset = (float)0.0 - (Scale * (float)PixelMin);
            }
        }
        else {
            Offset = 0.0;
            Scale = 1.0;
        }

        // generate RGBDQUAD colormap
        int k;
        for (int i = 0; i <= 255; i++) {
            k = (int)(Scale * (float)i + Offset + 0.5);
            if (k < 0) k = 0;
            if (k > 255) k = 255;
            ColorTable[i].rgbBlue = k;
            ColorTable[i].rgbGreen = k;
            ColorTable[i].rgbRed = k;
            ColorTable[i].rgbReserved = 0;
        }
    }
    delete[] InputImage;

    // fill in BMPheader

    BMPheader.bfType = 0x4d42;  // required ID
    if (!RGBframes) {
        // 8 bpp requires a colormap
        BMPheader.bfSize = (DWORD)(sizeof(BMPheader) + sizeof(BMPinfoheader) + sizeof(ColorTable)) + BMPimageBytes;
    }
    else {
        // 24 bit bpp does not have a colormap
        BMPheader.bfSize = (DWORD)(sizeof(BMPheader) + sizeof(BMPinfoheader)) + BMPimageBytes;
    }
    BMPheader.bfReserved1 = 0;
    BMPheader.bfReserved2 = 0;
    if (!RGBframes) {
        // 8 bpp requires a colormap
        BMPheader.bfOffBits = (DWORD)(sizeof(BMPheader) + sizeof(BMPinfoheader) + sizeof(ColorTable));
    }
    else {
        // 24 bit bpp does not have a colormap
        BMPheader.bfOffBits = (DWORD)(sizeof(BMPheader) + sizeof(BMPinfoheader));
    }

    // fill in BMPinfoheader
    BMPinfoheader.biSize = (DWORD) sizeof(BMPinfoheader);
    BMPinfoheader.biWidth = (LONG) biWidth; // calculated and then padded in needed
    BMPinfoheader.biHeight = (LONG)-ImageHeader.Ysize;
    BMPinfoheader.biPlanes = 1;
    if (RGBframes) {
        BMPinfoheader.biBitCount = 24;
    }
    else {
        BMPinfoheader.biBitCount = 8;
    }
    BMPinfoheader.biCompression = BI_RGB;
    BMPinfoheader.biSizeImage = BMPimageBytes;
    BMPinfoheader.biXPelsPerMeter = 2834;
    BMPinfoheader.biYPelsPerMeter = 2834;
    BMPinfoheader.biClrUsed = 0;
    BMPinfoheader.biClrImportant = 0;

    // write BMP file

    FILE* Out;
    errno_t ErrNum;
    ErrNum = _wfopen_s(&Out, Filename, L"wb");
    if (Out == NULL) {
        free(BMPimage);
        return -2;
    }

    // write the BMPheader
    fwrite(&BMPheader, sizeof(BMPheader), 1, Out);

    // write the BMPinfoheader
    fwrite(&BMPinfoheader, sizeof(BMPinfoheader), 1, Out);

    // write color map only if greyscale image
    if (!RGBframes) {
        fwrite(ColorTable, sizeof(RGBQUAD), 256, Out);
    }

    // write the image data
    int Address = 0;
    for (int i = 0; i < (int)BMPimageBytes; i++) {
        fwrite(&BMPimage[i], 1, 1, Out);
        Address++;
    }

    free(BMPimage);
    fclose(Out);

    return 1;
}

//****************************************************************
//
//  SaveTXT
// 
//  This a generic function is used to export a 'image file' to a text file.
//  It does not have any uer interaction.
// 
//  Parmeters:
//      Filename - text output file
//      InputFile - Image file for to be exported
//
// The output file is in a text format with whitespace as delimiters
// Each row is image is output as a single line in the text file.
// Each pixel in a row is output as a decmial number. 
// The number range of a pixel is determined by the pixel size of the
// input image:
//      1 byte - 0 to 255
//      2 byte - 0 to 65535
//      4 byte - 0 to 2147483648
// If there are multiple frames there is a blank line between frames. 
//  
//  return value:
//  1 - Success
//  see standardized app error list at top of this source file
//
//****************************************************************
int SaveTXT(WCHAR* Filename, WCHAR* InputFile)
{
    int iRes;
    int* InputImage;
    IMAGINGHEADER ImageHeader;

    iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
    if (iRes != 1) {
        return iRes;
    }

    FILE* Out;
    errno_t ErrNum;
    int Address;

    ErrNum = _wfopen_s(&Out, Filename, L"w");
    if (Out == NULL) {
        delete[] InputImage;
        return -2;
    }

    // save file in text format, blank line between frames
    Address = 0;
    int Pixel;

    for (int Frame = 0; Frame < ImageHeader.NumFrames; Frame++) {
        for (int y = 0; y < ImageHeader.Ysize; y++) {
            for (int x = 0; x < ImageHeader.Xsize; x++) {
                Pixel = InputImage[Address];
                // make sure pixel is not less than 0
                if (Pixel < 0) Pixel = 0;
                if (ImageHeader.PixelSize == 1) {
                    // clip value to match pixel size
                    if (Pixel > 255) Pixel = 255;
                    fprintf(Out, "%3d ", Pixel);
                }
                else if (ImageHeader.PixelSize == 1) {
                    // clip value to match pixel size
                    if (Pixel > 65535) Pixel = 65535;
                    fprintf(Out, "%5d ", Pixel);
                }
                else { 
                    fprintf(Out, "%7d ", Pixel);
                }
                Address++;
            }
            fprintf(Out, "\n");
        }
        fprintf(Out, "\n");
    }
    fclose(Out);
    delete[] InputImage;

    return 1;
}
