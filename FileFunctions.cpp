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
// V1.3.1.1 2023-12-6   Corrected setting options in FileOpen and FileSave.  This does not have actual impact
//                      on the existing application since it doesn't use the Folders only option.
//                      Replaced application error numbers with #define to improve clarity
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
#include "ImageDialog.h"
#include "globals.h"
#include "imaging.h"
#include "FileFunctions.h"

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
         if (wcscmp(pszCurrentFilename, L"") != 0) {
            //parse for just filename
            int err;
            WCHAR Drive[_MAX_DRIVE];
            WCHAR Dir[_MAX_DIR];
            WCHAR Fname[_MAX_FNAME];
            WCHAR Ext[_MAX_EXT];
            WCHAR Directory[MAX_PATH];

            // split apart original filename
            err = _wsplitpath_s(pszCurrentFilename, Drive, _MAX_DRIVE, Dir, _MAX_DIR, Fname,
                _MAX_FNAME, Ext, _MAX_EXT);
            if (err != 0) {
                return FALSE;
            }

            err = _wmakepath_s(Directory, _MAX_PATH, Drive, Dir, L"", L"");
            if (err != 0) {
                return FALSE;
            }

            // if no directory try the global CurrentFilename for directory
            if (wcscmp(Directory, L"") == 0) {
                err = _wsplitpath_s(szCurrentFilename, Drive, _MAX_DRIVE, Dir, _MAX_DIR, Fname,
                    _MAX_FNAME, Ext, _MAX_EXT);
                if (err != 0) {
                    return FALSE;
                }

                err = _wmakepath_s(Directory, _MAX_PATH, Drive, Dir, L"", L"");
                if (err != 0) {
                    return FALSE;
                }
            }

            // if still no directory try the global strAppNameEXE for directory
            if (wcscmp(Directory, L"") == 0) {
                err = _wsplitpath_s(strAppNameEXE, Drive, _MAX_DRIVE, Dir, _MAX_DIR, Fname,
                    _MAX_FNAME, Ext, _MAX_EXT);
                if (err != 0) {
                    return FALSE;
                }

                err = _wmakepath_s(Directory, _MAX_PATH, Drive, Dir, L"", L"");
                if (err != 0) {
                    return FALSE;
                }
            }

            IShellItem* pCurFolder = NULL;
            hr = SHCreateItemFromParsingName(Directory, NULL, IID_PPV_ARGS(&pCurFolder));
            if (SUCCEEDED(hr)) {
                hr = pFileOpen->SetFolder(pCurFolder);
                hr = pFileOpen->SetDefaultFolder(pCurFolder);
                pCurFolder->Release();
            }
        }

        if (!bSelectFolder) {
            hr = pFileOpen->SetOptions(FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
            if (NumTypes != 0 && FileTypes!=NULL) {
                hr = pFileOpen->SetFileTypes(NumTypes, FileTypes);
            }
            if (szDefExt != NULL) {
                hr = pFileOpen->SetDefaultExtension(szDefExt);
            }

            if (wcscmp(pszCurrentFilename, L"") != 0)
            {
                //parse for just filename
                int err;
                WCHAR Drive[_MAX_DRIVE];
                WCHAR Dir[_MAX_DIR];
                WCHAR Fname[_MAX_FNAME];
                WCHAR Ext[_MAX_EXT];

                // split apart original filename
                err = _wsplitpath_s(pszCurrentFilename, Drive, _MAX_DRIVE, Dir, _MAX_DIR, Fname,
                    _MAX_FNAME, Ext, _MAX_EXT);
                if (err != 0) {
                    return FALSE;
                }
     
                WCHAR NewFname[_MAX_FNAME];

                swprintf_s(NewFname, _MAX_FNAME, L"%s%s", Fname, Ext);
                hr = pFileOpen->SetFileName(NewFname);
            }

        } else {
            hr = pFileOpen->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST); // This selects only a folder, not a filename
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
            if (wcscmp(pszCurrentFilename, L"") != 0) {
                //parse for just filename
                int err;
                WCHAR Drive[_MAX_DRIVE];
                WCHAR Dir[_MAX_DIR];
                WCHAR Fname[_MAX_FNAME];
                WCHAR Ext[_MAX_EXT];
                WCHAR Directory[MAX_PATH];

                // split apart original filename
                err = _wsplitpath_s(pszCurrentFilename, Drive, _MAX_DRIVE, Dir, _MAX_DIR, Fname,
                    _MAX_FNAME, Ext, _MAX_EXT);
                if (err != 0) {
                    return FALSE;
                }

                err = _wmakepath_s(Directory, _MAX_PATH, Drive, Dir, L"", L"");
                if (err != 0) {
                    return FALSE;
                }
                // if no directory try the global CurrentFilename for directory
                if (wcscmp(Directory, L"") == 0) {
                    err = _wsplitpath_s(szCurrentFilename, Drive, _MAX_DRIVE, Dir, _MAX_DIR, Fname,
                        _MAX_FNAME, Ext, _MAX_EXT);
                    if (err != 0) {
                        return FALSE;
                    }

                    err = _wmakepath_s(Directory, _MAX_PATH, Drive, Dir, L"", L"");
                    if (err != 0) {
                        return FALSE;
                    }
                }

                // if still no directory try the global strAppNameEXE for directory
                if (wcscmp(Directory, L"") == 0) {
                    err = _wsplitpath_s(strAppNameEXE, Drive, _MAX_DRIVE, Dir, _MAX_DIR, Fname,
                        _MAX_FNAME, Ext, _MAX_EXT);
                    if (err != 0) {
                        return FALSE;
                    }

                    err = _wmakepath_s(Directory, _MAX_PATH, Drive, Dir, L"", L"");
                    if (err != 0) {
                        return FALSE;
                    }
                }

                IShellItem* pCurFolder = NULL;
                hr = SHCreateItemFromParsingName(Directory, NULL, IID_PPV_ARGS(&pCurFolder));
                if (SUCCEEDED(hr)) {
                    hr = pFileSave->SetFolder(pCurFolder);
                    hr = pFileSave->SetDefaultFolder(pCurFolder);
                    pCurFolder->Release();
                }
            }

            if (!bSelectFolder) {
                hr = pFileSave->SetOptions(FOS_FORCEFILESYSTEM | FOS_OVERWRITEPROMPT | FOS_PATHMUSTEXIST);
                if (NumTypes != 0 && FileTypes != NULL) {
                    hr = pFileSave->SetFileTypes(NumTypes, FileTypes);
                }
                if (szDefExt != NULL) {
                    hr = pFileSave->SetDefaultExtension(szDefExt);
                }

                if (wcscmp(pszCurrentFilename, L"") != 0)
                {
                    //parse for just filename
                    int err;
                    WCHAR Drive[_MAX_DRIVE];
                    WCHAR Dir[_MAX_DIR];
                    WCHAR Fname[_MAX_FNAME];
                    WCHAR Ext[_MAX_EXT];

                    // split apart original filename
                    err = _wsplitpath_s(pszCurrentFilename, Drive, _MAX_DRIVE, Dir, _MAX_DIR, Fname,
                        _MAX_FNAME, Ext, _MAX_EXT);
                    if (err != 0) {
                        return FALSE;
                    }

                    WCHAR NewFname[_MAX_FNAME];

                    swprintf_s(NewFname, _MAX_FNAME, L"%s%s", Fname, Ext);
                    hr = pFileSave->SetFileName(NewFname);
                }

            } else {
                hr = pFileSave->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST); // This selects only a folder, not a filename
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
        if (hwndImage != NULL) {
            SendMessage(hwndImage, WM_COMMAND, IDC_GENERATE_BMP, 0l);
            ShowWindow(hwndImage, SW_SHOW);
            // todo:
        }
        return APP_SUCCESS;
    }

    // This should be a BMP file
    // read BMPheader to verify
    BITMAPFILEHEADER BMPheader;
    
    errno_t ErrNum;
    FILE* In;

    ErrNum = _wfopen_s(&In, Filename, L"rb");
    if (In == NULL) {
        return APPERR_FILEOPEN;
    }
    iRes = fread(&BMPheader, sizeof(BMPheader), 1, In);
    if (iRes != APP_SUCCESS) {
        fclose(In);
        return APPERR_FILETYPE;
    }
    if (BMPheader.bfType != 0x4d42 || BMPheader.bfReserved1 != 0 || BMPheader.bfReserved2 != 0) {
        fclose(In);
        return APPERR_FILETYPE;
    }
    fclose(In);

    // copy filename to szBMPFilename
    if (!CopyFile(Filename, szBMPFilename, FALSE)) {
        return APPERR_FILEREAD;
    }

    if (hwndImage != NULL) {
        SendMessage(hwndImage, WM_COMMAND, IDC_GENERATE_BMP, 0l);
        ShowWindow(hwndImage, SW_SHOW);
    }
    return APP_SUCCESS;
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

    GetPrivateProfileString(L"ExportFile", L"InputFile", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
    if (!CCFileOpen(hWnd, szString, &pszFilename, FALSE, 2, RawType, L".raw")) {
        return;
    }
    wcscpy_s(InputFile, pszFilename);
    CoTaskMemFree(pszFilename);


    if (ReadImageHeader(InputFile, &ImageHeader) != 1) {
        MessageBox(hWnd, L"Input file is not an image file", L"File incompatible", MB_OK);
        return;
    }

    if (ImageHeader.PixelSize > 2) {
        MessageBox(hWnd, L"Input file must be 1 or 2 byte per pixel", L"File incompatible", MB_OK);
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
    WritePrivateProfileString(L"ExportFile", L"InputFile", InputFile, (LPCTSTR)strAppNameINI);
    
    {
        //parse for just filename
        int err;
        WCHAR Drive[_MAX_DRIVE];
        WCHAR Dir[_MAX_DIR];
        WCHAR Fname[_MAX_FNAME];
        WCHAR Ext[_MAX_EXT];

        // split apart original filename
        err = _wsplitpath_s(InputFile, Drive, _MAX_DRIVE, Dir, _MAX_DIR, Fname,
            _MAX_FNAME, Ext, _MAX_EXT);
        if (err != 0) {
            return;
        }

        if (wmId == IDM_FILE_TOBMP) {

            // GetPrivateProfileString(L"ExportFile", L"BMPFile", L"Message.bmp", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
            // change the input file to be .bmp extension
            err = _wmakepath_s(szString, _MAX_PATH, Drive, Dir, Fname, L".bmp");
            if (err != 0) {
                return;
            }
            
            COMDLG_FILTERSPEC BMPType[] =
            {
                 { L"BMP files", L"*.bmp" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hWnd, szString, &pszFilename, FALSE, 2, BMPType, L".bmp")) {
                return;
            }
            WritePrivateProfileString(L"ExportFile", L"BMPFile", pszFilename, (LPCTSTR)strAppNameINI);

        }
        else if (wmId == IDM_FILE_TOTXT) {
            
            err = _wmakepath_s(szString, _MAX_PATH, Drive, Dir, Fname, L".txt");
            if (err != 0) {
                return;
            }

            COMDLG_FILTERSPEC TXTType[] =
            {
                 { L"Image files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hWnd, szString, &pszFilename, FALSE, 2, TXTType, L".txt")) {
                return;
            }
            WritePrivateProfileString(L"ExportFile", L"TextFile", pszFilename, (LPCTSTR)strAppNameINI);

        }
    }
    wcscpy_s(szString, pszFilename);
    CoTaskMemFree(pszFilename);

    // convert to file
    if (wmId == IDM_FILE_TOBMP) {
        SaveBMP(szString, InputFile,RGBframes, AutoScale);
        wcscpy_s(szCurrentFilename, szString);
        if (DisplayResults) {
            DisplayImage(szCurrentFilename);
        }
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
//  Input image pixel supported 8,16,32 bit
// 
//  2 types of output BMP files are supported:
//
//  Input parameter RBGframes FALSE 
//  8 bit per pixel, first frame from file
//      AutoScale parameter TRUE, auto scale as greyscale image, 0 to 255
//      AutoScale parameter only applies to 8 bit input image files.
//      16 and 24 bit images are always automatically scaled as greyscale
//      image, 0 to 255.  This is because there is no BMP file format for 
//      16 or 32 bit monochrome images
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
    RGBQUAD* ColorTable = NULL;
    BYTE* BMPimage;

    iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
    if (iRes != APP_SUCCESS) {
        return iRes;
    }

    if (ImageHeader.PixelSize > 2) {
        return APPERR_PARAMETER;
    }

    if (ImageHeader.Xsize > 8192) {
        return APPERR_PARAMETER;
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
            return APPERR_MEMALLOC;
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
        // convert image data to 8 bit DIB/BMP format
        // 
        // If xsize is odd add pad to even size.
        // Pad as required in BMP 'stride' size.
        //  
        // only first frame file saved.
        //
        // 8 bit input image, scale display using RGBQUAD color map, do not scale image data
        // 16 or 32 bit, RGBQUAD color map is scaled 0 to 255 greyscale, input image data scaled to 8 bits
        //
        int PixelMin, PixelMax;
        int InputFrameSize;
        int Stride;
        float Scale, Offset;
        int MaxPixel = 255;
        
        if (ImageHeader.PixelSize > 1) {
            AutoScale = 1;
        }

        if (InputImage[0] < 0) {
            PixelMin = PixelMax = 0;
        }
        PixelMin = PixelMax = InputImage[0];

        InputFrameSize = ImageHeader.Xsize * ImageHeader.Ysize;
        // scan image for scaling
        for (int i=0; i < InputFrameSize; i++) {
            if (InputImage[i] < 0) {
                InputImage[i] = 0;
            }
            if (InputImage[i] < PixelMin) {
                PixelMin = InputImage[i];
            }
            if (InputImage[i] > PixelMax) {
                PixelMax = InputImage[i];
            }
        }

        if (AutoScale) {
            // compute scaling: Scale, Offset
            // this is only used in the RGBQUAD color map for the image
            // It does not change the pixel value
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
            // this can only happens if input image is 8 bit pixels
            Offset = 0.0;
            Scale = 1.0;
        }

        // correct for odd column size
        biWidth = ImageHeader.Xsize;
        if (biWidth % 2 != 0) {
            // make sure bitmap width is even
            biWidth++;
        }

        // BMP files have a specific requirement for # of bytes per line
        // This is called stride.  The formula used is from the specification.
        Stride = ((((biWidth * 8) + 31) & ~31) >> 3); // 8 bpp
        BMPimageBytes = Stride * ImageHeader.Ysize; // size of image in pixels, 8bpp

        // allocate zero paddded image array
        BMPimage = (BYTE*) calloc(BMPimageBytes, 1);
        if (BMPimage == NULL) {
            delete[] InputImage;
            return APPERR_MEMALLOC;
        }

        int InputOffset;
        int BMPOffset;
        int ImagePixel;

        // copy image to BMPimage
        for (int y = 0; y < ImageHeader.Ysize; y++) {
            InputOffset = y * ImageHeader.Xsize;
            BMPOffset = y * Stride;
            for (int x = 0; x < ImageHeader.Xsize; x++) {
                ImagePixel = InputImage[InputOffset + x];
                if (ImageHeader.PixelSize > 1) {
                    ImagePixel = (int)(Scale * (float)ImagePixel + Offset + 0.5);
                }
                // copy results into BMPimage
                BMPimage[BMPOffset + x] = (BYTE)ImagePixel;
            }
        }

        // generate RGBDQUAD colormaps
        int k;
        ColorTable = new RGBQUAD[256];
        if (ImageHeader.PixelSize == 1) {
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
        else {
            for (int i = 0; i <= 255; i++) {
                ColorTable[i].rgbBlue = i;
                ColorTable[i].rgbGreen = i;
                ColorTable[i].rgbRed = i;
                ColorTable[i].rgbReserved = 0;
            }

        }
    }
    delete[] InputImage;

    // fill in BMPheader

    BMPheader.bfType = 0x4d42;  // required ID
    if (!RGBframes) {
        // 8 bpp colormap
        BMPheader.bfSize = (DWORD)(sizeof(BMPheader) + sizeof(BMPinfoheader) + sizeof(RGBQUAD) * 256) + BMPimageBytes;
    }
    else {
        // 24 bit bpp does not have a colormap
        BMPheader.bfSize = (DWORD)(sizeof(BMPheader) + sizeof(BMPinfoheader)) + BMPimageBytes;
    }
    BMPheader.bfReserved1 = 0;
    BMPheader.bfReserved2 = 0;
    if (!RGBframes) {
        // 8 bpp colormap
        BMPheader.bfOffBits = (DWORD)(sizeof(BMPheader) + sizeof(BMPinfoheader) + sizeof(RGBQUAD) * 256);
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
        return APPERR_FILEOPEN;
    }

    // write the BMPheader
    fwrite(&BMPheader, sizeof(BMPheader), 1, Out);

    // write the BMPinfoheader
    fwrite(&BMPinfoheader, sizeof(BMPinfoheader), 1, Out);

    // write color map only if greyscale image
    if (!RGBframes) {
        fwrite(ColorTable, sizeof(RGBQUAD), 256, Out);
        delete[] ColorTable;
    }

    // write the image data
    int Address = 0;
    for (int i = 0; i < (int)BMPimageBytes; i++) {
        fwrite(&BMPimage[i], 1, 1, Out);
        Address++;
    }

    free(BMPimage);
    fclose(Out);

    if (AutoPNG) {
        if (SaveBMP2PNG(Filename) != 1) {
            return APPERR_PARAMETER;
        }
    }

    return APP_SUCCESS;
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
if (iRes != APP_SUCCESS) {
    return iRes;
}

FILE* Out;
errno_t ErrNum;
int Address;

ErrNum = _wfopen_s(&Out, Filename, L"w");
if (Out == NULL) {
    delete[] InputImage;
    return APPERR_FILEOPEN;
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

return APP_SUCCESS;
}

//****************************************************************
//
//  ImportBMP
// 
//****************************************************************
int  ImportBMP(HWND hWnd)
{
    WCHAR InputFilename[MAX_PATH];
    WCHAR OutputFilename[MAX_PATH];

    GetPrivateProfileString(L"ImportBMP", L"InputFile", L"*.bmp", InputFilename, MAX_PATH, (LPCTSTR)strAppNameINI);
    GetPrivateProfileString(L"ImportBMP", L"OutputFile", L"*.raw", OutputFilename, MAX_PATH, (LPCTSTR)strAppNameINI);

    PWSTR pszFilename;
    COMDLG_FILTERSPEC BMPType[] =
    {
         { L"BMP files", L"*.bmp" },
         { L"All Files", L"*.*" }
    };

    COMDLG_FILTERSPEC IMGType[] =
    {
         { L"Image files", L"*.raw" },
         { L"All Files", L"*.*" }
    };

    if (!CCFileOpen(hWnd, InputFilename, &pszFilename, FALSE, 2, BMPType, L".bmp")) {
        return APP_SUCCESS;
    }
    wcscpy_s(InputFilename, pszFilename);
    CoTaskMemFree(pszFilename);

    if (!CCFileSave(hWnd, OutputFilename, &pszFilename, FALSE, 2, IMGType, L".bmp")) {
        return APP_SUCCESS;
    }
    wcscpy_s(OutputFilename, pszFilename);
    CoTaskMemFree(pszFilename);

    WritePrivateProfileString(L"ImportBMP", L"InputFile", InputFilename, (LPCTSTR)strAppNameINI);
    WritePrivateProfileString(L"ImportBMP", L"OutputFile", OutputFilename, (LPCTSTR)strAppNameINI);

    int Invert=0;
    if (MessageBox(hWnd, L"Invert Image?", L"Conversion parameter", MB_YESNO) == IDYES) {
        Invert = 1;
    }

    // open BMP file
    FILE* BMPfile;
    errno_t ErrNum;
    int iRes;

    ErrNum = _wfopen_s(&BMPfile, InputFilename, L"rb");
    if (!BMPfile) {
        return APPERR_FILEOPEN;
    }

    // read BMP headers
    BITMAPFILEHEADER BMPheader;
    BITMAPINFOHEADER BMPinfoheader;
    int StrideLen;
    int* Image;
    BYTE* Stride;

    iRes = (int)fread(&BMPheader, sizeof(BITMAPFILEHEADER), 1, BMPfile);
    if (iRes != APP_SUCCESS) {
        fclose(BMPfile);
        return APPERR_FILETYPE;
    }

    iRes = (int)fread(&BMPinfoheader, sizeof(BITMAPINFOHEADER), 1, BMPfile);
    if (iRes != APP_SUCCESS) {
        fclose(BMPfile);
        return APPERR_FILETYPE;
    }

    // verify this type of file can be imported
    if (BMPheader.bfType != 0x4d42 || BMPheader.bfReserved1 != 0 || BMPheader.bfReserved2 != 0) {
        // this is not a BMP file
        fclose(BMPfile);
        return APPERR_FILETYPE;
    }
    if (BMPinfoheader.biSize != sizeof(BITMAPINFOHEADER)) {
        // this is not a BMP file
        fclose(BMPfile);
        return APPERR_FILETYPE;
    }

    if (BMPinfoheader.biCompression != BI_RGB) {
        // this is wrong type of BMP file
        fclose(BMPfile);
        return APPERR_PARAMETER;
    }
    if (BMPinfoheader.biBitCount != 1 && BMPinfoheader.biBitCount != 8 && 
         BMPinfoheader.biBitCount != 24 && BMPinfoheader.biPlanes!=1) {
        // this is wrong type of BMP file
        fclose(BMPfile);
        return APPERR_PARAMETER;
    }

    // read in image
    int BMPimageBytes;
    int TopDown = 1;
    if (BMPinfoheader.biHeight < 0) {
        TopDown = 0;
        BMPinfoheader.biHeight = -BMPinfoheader.biHeight;
    }

    // BMP files have a specific requirement for # of bytes per line
    // This is called stride.  The formula used is from the specification.
    StrideLen = ((((BMPinfoheader.biWidth * BMPinfoheader.biBitCount) + 31) & ~31) >> 3); // 24 bpp
    BMPimageBytes = StrideLen * BMPinfoheader.biHeight; // size of image in bytes

    // allocate stride
    Stride = new BYTE[(size_t)StrideLen];
    if (Stride == NULL) {
        fclose(BMPfile);
        return APPERR_MEMALLOC;
    }

    int NumFrames = 1;

    if (BMPinfoheader.biBitCount == 1) {
        // This is bit image, has color table, 2 entries
        // Skip the 2 RGBQUAD entries
        if (fseek(BMPfile, sizeof(RGBQUAD) * 2, SEEK_CUR) != 0) {
            delete[] Stride;
            fclose(BMPfile);
            return APPERR_FILETYPE;
        }
        int BitCount;
        int StrideIndex;
        int Offset;

        // allocate Image
        // alocate array of 'int's to receive image
        Image = new int[(size_t)BMPinfoheader.biWidth * (size_t)BMPinfoheader.biHeight];
        if (Image == NULL) {
            delete[] Stride;
            fclose(BMPfile);
            return APPERR_MEMALLOC;
        }

        // BMPimage of BMPimageBytes
        for (int y = 0; y < BMPinfoheader.biHeight; y++) {
            // read stride
            iRes = (int)fread(Stride, 1, StrideLen, BMPfile);
            if (iRes != StrideLen) {
                delete[] Image;
                delete[] Stride;
                fclose(BMPfile);
                return APPERR_FILETYPE;
            }
            BitCount = 0;
            StrideIndex = 0;
            if (TopDown) {
                Offset = ((BMPinfoheader.biHeight-1) - y) * BMPinfoheader.biWidth;
            }
            else {
                Offset = y * BMPinfoheader.biWidth;
            }
            for (int x = 0; x < BMPinfoheader.biWidth; x++) {
                // split out bit by bit
                Image[Offset + x] = Stride[StrideIndex] & (0x80 >> BitCount);
                if (Image[Offset + x]!=0) {
                    if (Invert == 0) {
                        Image[Offset + x] = 1;
                    }
                    else {
                        Image[Offset + x] = 0;
                    }
                }
                else {
                    if (Invert == 0) {
                        Image[Offset + x] = 0;
                    }
                    else {
                        Image[Offset + x] = 1;
                    }
                }
                BitCount++;
                if (BitCount == 8) {
                    BitCount = 0;
                    StrideIndex++;
                }
            }
        }
    }
    else if(BMPinfoheader.biBitCount == 8){
        // this is a byte image, has color table, 256 entries
        // Skip the 256 RGBQUAD entries
        if (fseek(BMPfile, sizeof(RGBQUAD) * 256, SEEK_CUR) != 0) {
            delete[] Stride;
            fclose(BMPfile);
            return APPERR_FILETYPE;
        }

        // allocate Image
        // alocate array of 'int's to receive image
        Image = new int[(size_t)BMPinfoheader.biWidth * (size_t)BMPinfoheader.biHeight];
        if (Image == NULL) {
            delete[] Stride;
            fclose(BMPfile);
            return APPERR_MEMALLOC;
        }

        // BMPimage of BMPimageBytes
        int Offset;

        for (int y = 0; y < BMPinfoheader.biHeight; y++) {
            // read stride
            iRes = (int)fread(Stride, 1, StrideLen, BMPfile);
            if (iRes != StrideLen) {
                delete[] Image;
                delete[] Stride;
                fclose(BMPfile);
                return APPERR_FILETYPE;
            }
            if (TopDown) {
                Offset = ((BMPinfoheader.biHeight - 1) - y) * BMPinfoheader.biWidth;
            }
            else {
                Offset = y * BMPinfoheader.biWidth;
            }

            for (int x = 0; x < BMPinfoheader.biWidth; x++) {
                Image[Offset + x] = Stride[x];
            }
        }
    }
    else {
        // this is a 24 bit, RGB image
        // The color table is biClrUsed long
        NumFrames = 3;
        if (BMPinfoheader.biClrUsed != 0) {
            // skip the color table if present
            if (fseek(BMPfile, sizeof(RGBQUAD) * BMPinfoheader.biClrUsed, SEEK_CUR) != 0) {
                delete[] Stride;
                fclose(BMPfile);
                return APPERR_FILETYPE;
            }
        }
        
        Image = new int[(size_t)BMPinfoheader.biWidth * (size_t)BMPinfoheader.biHeight * (size_t)3];
        if (Image == NULL) {
            delete[] Stride;
            fclose(BMPfile);
            return APPERR_MEMALLOC;
        }

        // BMPimage of BMPimageBytes
        int Offset;
        int RedFrame = 0;
        int GreenFrame = BMPinfoheader.biHeight * BMPinfoheader.biWidth;
        int BlueFrame = BMPinfoheader.biHeight * BMPinfoheader.biWidth * 2;

        for (int y = 0; y < BMPinfoheader.biHeight; y++) {
            // read stride
            iRes = (int)fread(Stride, 1, StrideLen, BMPfile);
            if (iRes != StrideLen) {
                delete[] Image;
                delete[] Stride;
                fclose(BMPfile);
                return APPERR_FILETYPE;
            }
            if (TopDown) {
                Offset = ((BMPinfoheader.biHeight - 1) - y) * BMPinfoheader.biWidth;
            }
            else {
                Offset = y * BMPinfoheader.biWidth;
            }

            for (int x = 0; x < BMPinfoheader.biWidth; x++) {
                Image[BlueFrame + Offset + x] = Stride[x * 3 + 0];
                Image[GreenFrame + Offset + x] = Stride[x * 3 + 1];
                Image[RedFrame + Offset + x] = Stride[x * 3 + 2];
            }
        }
    }

    delete[] Stride;
    fclose(BMPfile);

    // save image
    IMAGINGHEADER ImgHeader;
    ImgHeader.Endian = (short)-1;  // PC format
    ImgHeader.HeaderSize = (short)sizeof(IMAGINGHEADER);
    ImgHeader.ID = (short)0xaaaa;
    ImgHeader.Version = (short)1;
    ImgHeader.NumFrames = (short)NumFrames;
    ImgHeader.PixelSize = (short)1;
    ImgHeader.Xsize = BMPinfoheader.biWidth;
    ImgHeader.Ysize = BMPinfoheader.biHeight;
    ImgHeader.Padding[0] = 0;
    ImgHeader.Padding[1] = 0;
    ImgHeader.Padding[2] = 0;
    ImgHeader.Padding[3] = 0;
    ImgHeader.Padding[4] = 0;
    ImgHeader.Padding[5] = 0;

    FILE* ImgFile;
    ErrNum = _wfopen_s(&ImgFile, OutputFilename, L"wb");
    if (ImgFile==NULL) {
        delete[] Image;
        return APPERR_FILEOPEN;
    }

    BYTE Pixel;
    fwrite(&ImgHeader, sizeof(IMAGINGHEADER), 1, ImgFile);

    for (int i = 0; i < (ImgHeader.Xsize*ImgHeader.Ysize * NumFrames) ; i++) {
        if (Image[i] <= 0) {
            Pixel = 0;
        }
        else if(Image[i] > 255) {
            Pixel = 255;
        }
        else {
            Pixel = Image[i];
        }
        fwrite(&Pixel, 1, 1, ImgFile);
    }
    fclose(ImgFile);
    delete[] Image;

    wcscpy_s(szCurrentFilename, OutputFilename);

    if (DisplayResults) {
        DisplayImage(OutputFilename);
    }

    return APP_SUCCESS;
}

//****************************************************************
//
//  Hex2Binary
// 
//****************************************************************
int HEX2Binary(HWND hWnd)
{
    WCHAR InputFilename[MAX_PATH];
    WCHAR OutputFilename[MAX_PATH];

    GetPrivateProfileString(L"Hex2Binary", L"InputFile", L"*.txt", InputFilename, MAX_PATH, (LPCTSTR)strAppNameINI);
    GetPrivateProfileString(L"Hex2Binary", L"OutputFile", L"", OutputFilename, MAX_PATH, (LPCTSTR)strAppNameINI);

    PWSTR pszFilename;
    COMDLG_FILTERSPEC txtType[] =
    {
         { L"Text files", L"*.txt" },
         { L"All Files", L"*.*" }
    };

    COMDLG_FILTERSPEC AllType[] =
    {
         { L"BMP files", L"*.bmp" },
         { L"Image files", L"*.raw" },
         { L"All Files", L"*.*" }
    };

    if (!CCFileOpen(hWnd, InputFilename, &pszFilename, FALSE, 2, txtType, L".bmp")) {
        return APP_SUCCESS;
    }
    wcscpy_s(InputFilename, pszFilename);
    CoTaskMemFree(pszFilename);

    if (!CCFileSave(hWnd, OutputFilename, &pszFilename, FALSE, 3, AllType, L"")) {
        return APP_SUCCESS;
    }
    wcscpy_s(OutputFilename, pszFilename);
    CoTaskMemFree(pszFilename);

    WritePrivateProfileString(L"Hex2Binary", L"InputFile", InputFilename, (LPCTSTR)strAppNameINI);
    WritePrivateProfileString(L"Hex2Binary", L"OutputFile", OutputFilename, (LPCTSTR)strAppNameINI);

    FILE* Input;
    FILE* Output;
    errno_t ErrNum;
    BYTE HexValue;
    int HexRead;
    int iRes;

    ErrNum = _wfopen_s(&Input, InputFilename, L"rb");
    if (Input ==NULL) {
        return APPERR_FILEOPEN;
    }

    ErrNum = _wfopen_s(&Output, OutputFilename, L"wb");
    if (Output == NULL) {
        return APPERR_FILEOPEN;
    }

    while (!feof(Input)) {
        iRes = fscanf_s(Input, "%2x", &HexRead);
        if (iRes != APP_SUCCESS) {
            break;
        }
        HexValue = (BYTE)HexRead;
        fwrite(&HexValue, 1, 1, Output);
    }

    fclose(Output);
    fclose(Input);

    return APP_SUCCESS;
}

//****************************************************************
//
//  CamIRaImport
// 
//****************************************************************
int CamIRaImport(HWND hWnd)
{
    // This structure must be exact since it is saved in a file.
#pragma pack(1)
    struct IMAGE_HEADER {
        short FirstWord;	// -1
        short Xsize;		// number of pixels in a line (# of columns)
        short Ysize;		// number of lines (# of rows)
        short PixelSize;	// # of bytes/pixel
        short Dummy1[16];   // ignore these
        short SelectFrames;	// <-10 use NumFrames1,  >10 use NumFrames2
        short NumFrames1;	// # of image frames in file. if SelectFrames <= 10 use this
        short Dummy2[206];  // ignore these
        INT32 NumFrames2;	// # of frames in file. If SelectFrames > 10 use this
        short Dummy3[26];   // ignore these
    } CamIRaHeader;
#pragma pack()

    WCHAR InputFilename[MAX_PATH];
    WCHAR OutputFilename[MAX_PATH];

    GetPrivateProfileString(L"CamIRaImport", L"InputFile", L"*.img", InputFilename, MAX_PATH, (LPCTSTR)strAppNameINI);
    GetPrivateProfileString(L"CamIRaImport", L"OutputFile", L"", OutputFilename, MAX_PATH, (LPCTSTR)strAppNameINI);

    PWSTR pszFilename;
    COMDLG_FILTERSPEC txtType[] =
    {
         { L"Text files", L"*.img" },
         { L"All Files", L"*.*" }
    };

    COMDLG_FILTERSPEC AllType[] =
    {
         { L"Image files", L"*.raw" },
         { L"All Files", L"*.*" }
    };

    if (!CCFileOpen(hWnd, InputFilename, &pszFilename, FALSE, 2, txtType, L".img")) {
        return APP_SUCCESS;
    }
    wcscpy_s(InputFilename, pszFilename);
    CoTaskMemFree(pszFilename);

    if (!CCFileSave(hWnd, OutputFilename, &pszFilename, FALSE, 2, AllType, L".raw")) {
        return APP_SUCCESS;
    }
    wcscpy_s(OutputFilename, pszFilename);
    CoTaskMemFree(pszFilename);

    WritePrivateProfileString(L"CamIRaImport", L"InputFile", InputFilename, (LPCTSTR)strAppNameINI);
    WritePrivateProfileString(L"CamIRaImport", L"OutputFile", OutputFilename, (LPCTSTR)strAppNameINI);

    FILE* Input;
    FILE* Output;
    errno_t ErrNum;

    ErrNum = _wfopen_s(&Input, InputFilename, L"rb");
    if (Input == NULL) {
        return APPERR_FILEOPEN;
    }

    int iRes;
    int HeaderLen;
    HeaderLen = sizeof(CamIRaHeader);
    iRes = (int) fread(&CamIRaHeader, HeaderLen, 1, Input);
    if (iRes != APP_SUCCESS) {
        fclose(Input);
        return APPERR_FILETYPE;
    }

    if (CamIRaHeader.FirstWord != -1 && CamIRaHeader.PixelSize!=1 && CamIRaHeader.PixelSize!=2) {
        fclose(Input);
        return APPERR_FILETYPE;
    }
    
    int NumFrames;
    if (CamIRaHeader.SelectFrames <= 10 && CamIRaHeader.SelectFrames!=0) {
        NumFrames = CamIRaHeader.NumFrames1;
    }
    else {
        NumFrames = CamIRaHeader.NumFrames2;
    }
    if (NumFrames > 32767 || CamIRaHeader.PixelSize > 2) {
        fclose(Input);
        return APPERR_FILETYPE;
    }

    // create ImageHeader
    IMAGINGHEADER ImgHeader;

    ImgHeader.Endian = (short)-1;  // PC format
    ImgHeader.HeaderSize = (short)sizeof(IMAGINGHEADER);
    ImgHeader.ID = (short)0xaaaa;
    ImgHeader.Version = (short)1;
    ImgHeader.NumFrames = (short)NumFrames;
    ImgHeader.PixelSize = CamIRaHeader.PixelSize;
    ImgHeader.Xsize = CamIRaHeader.Xsize;
    ImgHeader.Ysize = CamIRaHeader.Ysize;
    ImgHeader.Padding[0] = 0;
    ImgHeader.Padding[1] = 0;
    ImgHeader.Padding[2] = 0;
    ImgHeader.Padding[3] = 0;
    ImgHeader.Padding[4] = 0;
    ImgHeader.Padding[5] = 0;

    // open output file
    ErrNum = _wfopen_s(&Output, OutputFilename, L"wb");
    if (Output == NULL) {
        fclose(Input);
        return APPERR_FILEOPEN;
    }

    // write ImageHeader
    fwrite(&ImgHeader, sizeof(ImgHeader), 1, Output);

    // copy Input file to output file
    for (int Frame = 0; Frame < NumFrames; Frame++) {
        for (int y = 0; y < ImgHeader.Ysize; y++) {
            for (int x = 0; x < ImgHeader.Xsize; x++) {
                if (ImgHeader.PixelSize == 1) {
                    BYTE Pixel;
                    iRes = (int) fread(&Pixel, 1, 1, Input);
                    if (iRes != APP_SUCCESS) {
                        fclose(Input);
                        fclose(Output);
                        return APPERR_FILETYPE;
                    }
                    fwrite(&Pixel, 1, 1, Output);
                }
                else {
                    short Pixel;
                    iRes = (int) fread(&Pixel, 2, 1, Input);
                    if (iRes != APP_SUCCESS) {
                        fclose(Input);
                        fclose(Output);
                        return APPERR_FILETYPE;
                    }
                    fwrite(&Pixel, 2, 1, Output);
                }
            }
        }
    }
    wcscpy_s(szCurrentFilename, OutputFilename);
    fclose(Output);
    fclose(Input);
    return APP_SUCCESS;
}

//****************************************************************
//
//  GetFileSize
// 
//****************************************************************
int GetFileSize(WCHAR* szString)
{
    int FileSize = 0;
    int iRes;
    BYTE Junk;
    FILE* In;
    errno_t ErrNum;

    ErrNum = _wfopen_s(&In, szString, L"rb");
    if (In == NULL) {
        return APPERR_FILEOPEN;
    }

    while (!feof(In)) {
        iRes = (int) fread(&Junk, 1, 1, In);
        if (iRes != APP_SUCCESS) {
            break;
        }
        FileSize++;
    }

    fclose(In);
    return FileSize;
}

//****************************************************************
//
//  GetEncoderClsid
// 
// This is based on a solution provided at:
// https://learn.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-retrieving-the-class-identifier-for-an-encoder-use
// 
//****************************************************************
// using namespace Gdiplus;
INT GetEncoderClsid(const WCHAR* format, CLSID* pClsid);  // helper function

INT GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

//****************************************************************
//
//  SaveBMP2PNG
// 
// This is based on a solution provided at:
// https://learn.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-converting-a-bmp-image-to-a-png-image-use
// 
//****************************************************************
// using namespace Gdiplus;

int SaveBMP2PNG(WCHAR* Filename)
{
    // Initialize GDI+.
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    CLSID   encoderClsid;
    Gdiplus::Status  stat;

    // load BMP file to image
    Gdiplus::Image* image = new Gdiplus::Image(Filename);

    // Get the CLSID of the PNG encoder.
    GetEncoderClsid(L"image/png", &encoderClsid);
    // generate .png name version of Filename
    {
        //parse for just filename
        int err;
        WCHAR Drive[_MAX_DRIVE];
        WCHAR Dir[_MAX_DIR];
        WCHAR Fname[_MAX_FNAME];
        WCHAR Ext[_MAX_EXT];
        WCHAR PNGfilename[MAX_PATH];

        // split apart original filename
        err = _wsplitpath_s(Filename, Drive, _MAX_DRIVE, Dir, _MAX_DIR, Fname,
            _MAX_FNAME, Ext, _MAX_EXT);
        if (err != 0) {
            return APPERR_PARAMETER;
        }

        err = _wmakepath_s(PNGfilename, _MAX_PATH, Drive, Dir, Fname, L".png");
        if (err != 0) {
            return APPERR_PARAMETER;
        }

        stat = image->Save(PNGfilename, &encoderClsid, NULL);
    }
    delete image;
    Gdiplus::GdiplusShutdown(gdiplusToken);

    if (stat == Gdiplus::Ok) {
        return APP_SUCCESS;
    }

    return APPERR_PARAMETER;
}
