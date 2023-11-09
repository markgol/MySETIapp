//
// MySETIapp, a set tools for decoding bitstreams into various formats and manipulating those files
// ImagingDialogs.cpp
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
// If not, see < https://www.gnu.org/licenses/>. x
// 
// This file contains the dialog callback procedures for the image tools menu
// 
// V1.0.0.1 2023-08-20  Initial Release
// V1.1.0.1 2023-08-22  Added file type specifications to open/save dialogs
//                      Added Image Decimation
//                      Added Resize image file
//                      Interim display solution using external viewer
// V1.2.0.1 2023-08-31  Use of Windows default viewer for BMP display works adequately.
//                      Clean up of ImageDlg to just rely on external viewer.
//                      Added batch processing for reordering,  This allows a series of
//                      reorder kernels to be used.  Each kernel adds an index number
//                      onto the output filename.
// V1.2.1.1 2023-09-06  Corrected ImageDlg to display results file only once.
// V1.2.2.1 2023-09-06  Changed default folders\filenames
// V1.2.4.1 2023-09-09  Added Add/Subtract constant from images
//                      Changed Sum images to Add/Subtract images
// V1.2.5.1 2023-09-09  Added stanard image decimation (summation)
// V1.2.6.1 2023-09-24  Added Increase image size by replication.
//                      Added Algorithm driven reordering (onyl 5 algorithms to start)
//                      Added extract block symbols from image file
//                      (save as 2D image file, right padded with null symbols)
// V1.2.7.1 2023-10-01  Added skeleton for 5 more algorithms to reordering
//                      Added insert/add image into an existing image
// V1.2.8.1 2023-10-16  Added input2 image file information in dialog for Append end image operation
//                      Changed, Append image end no longer requires Ysize to be the same 
//                      unless the frames are being added to the end of the first input image.
//                      Changed, AddSubractConstant to MathConstant, now does +, *, or /
// V1.2.9.1 2023-10-31  Added,  batch file list for reordering dialog
//                      Changed, Added block output [P1xP2] transform to reorder by algorithm
//                      Changed, Added Invert algorithm to reorder by algorithm dialog
// V1.2.10.1 2023-11-5  Changed, Reordering by algorithm, added split image left/right
// V1.2.11.1 2023-11-7  Added, new dialog, Reorder blocks[MxM] in image using kernel
// 
// Imaging tools dialog box handlers
// 
// New dialogs for the image tools menu should be added to this module.
// The functions that actually perform the actions should be put in
// imaging.cpp and imaging.h
//
// When making a new dialog start by copying an existing one that is as close to the
// parameter set you will need.
//
#include "framework.h"
#include "MySETIapp.h"
#include <libloaderapi.h>
#include <Windows.h>
#include <shobjidl.h>
#include <winver.h>
#include <vector>
#include <atlstr.h>
#include <strsafe.h>
#include <atlstr.h>
#include "Globals.h"
#include "FileFunctions.h"
#include "Imaging.h"
#include "shellapi.h"

// Add new callback prototype declarations in my MySETIapp.cpp

//*******************************************************************************
//
// Message handler for ImageDlg dialog box.
// This window is used to display an image or BMP file
// It is a modeless dialog.
// 
//*******************************************************************************
INT_PTR CALLBACK ImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
//    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
    {
        CString csString = L"ImageWindow";
        RestoreWindowPlacement(hDlg, csString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case IDCANCEL:
            if (!DisplayResults) {
                DestroyWindow(hwndImage);
                hwndImage = NULL;
            }
            else {
                ShowWindow(hwndImage, SW_HIDE);
            }
            return (INT_PTR)TRUE;

        case IDC_GENERATE_BMP:
        {
            // Initialize COM
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

            // use the default Windows viewer for BMP file
            ShellExecute(hDlg, 0, szBMPFilename, 0, 0, SW_NORMAL);

            // release COM
            CoUninitialize();

            return (INT_PTR)TRUE;
        }

        default:
            return (INT_PTR)FALSE;
        }
    case WM_PAINT:
    {
        // redraw iage as required
        PAINTSTRUCT ps;
        RECT rc;
        HDC hDC;
        if (GetUpdateRect(hDlg, &rc, 0)) {
            hDC = BeginPaint(hDlg, &ps);
            if (hDC == NULL) {
                break;
            }
            TextOut(hDC,40,20, szBMPFilename, (int) wcslen(szBMPFilename));
            TextOut(hDC, 40, 40, L"using external BMP viewer", 25);
            EndPaint(hDlg, &ps);
        }
        break;
    }

    case WM_DESTROY:
    {
        // save window position/size data
        CString csString = L"ImageWindow";
        SaveWindowPlacement(hDlg, csString);
        break;
    }

    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for ExtractImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK ExtractImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
        IMAGINGHEADER ImageHeader;
        GetPrivateProfileString(L"ExtractImageDlg", L"ImageInput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"ExtractImageDlg", L"ImageOutput", L"Extracted.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"ExtractImageDlg", L"xsize", L"256", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE, szString);

        GetPrivateProfileString(L"ExtractImageDlg", L"ysize", L"256", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_YSIZE, szString);

        GetPrivateProfileString(L"ExtractImageDlg", L"StartFrame", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_START_FRAME, szString);

        GetPrivateProfileString(L"ExtractImageDlg", L"EndFrame", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_END_FRAME, szString);

        GetPrivateProfileString(L"ExtractImageDlg", L"xloc", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XLOC, szString);

        GetPrivateProfileString(L"ExtractImageDlg", L"yloc", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_YLOC, szString);

        GetPrivateProfileString(L"ExtractImageDlg", L"OutputXsize", L"256", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_OUTPUT_XSIZE, szString);

        GetPrivateProfileString(L"ExtractImageDlg", L"OutputYsize", L"256", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_OUTPUT_YSIZE, szString);

        int ScalePixel;
        int Centered;

        ScalePixel = GetPrivateProfileInt(L"ExtractImageDlg", L"ScalePixel", 0, (LPCTSTR)strAppNameINI);
        if (!ScalePixel) {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_CHECKED);
        }

        Centered = GetPrivateProfileInt(L"ExtractImageDlg", L"Centered", 0, (LPCTSTR)strAppNameINI);
        if (!Centered) {
            CheckDlgButton(hDlg, IDC_CENTERED, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_CENTERED, BST_CHECKED);
        }

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
                SetDlgItemInt(hDlg, IDC_START_FRAME, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_END_FRAME, ImageHeader.NumFrames-1, TRUE);
            } else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }
        case IDC_EXTRACT_OUTPUT_FILE_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            }; 
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_EXTRACT:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int xsize = 0;
            int ysize = 0;
            int SubimageXloc = 0;
            int SubimageYloc = 0;
            int SubimageXsize = 0;
            int SubimageYsize = 0;
            int OutputXsize = 0;
            int OutputYsize = 0;
            int ScaleBinary = 0;
            int Centered = 0;
            int StartFrame = 0;
            int EndFrame = 0;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            SubimageXsize = GetDlgItemInt(hDlg, IDC_XSIZE, &bSuccess, TRUE);
            SubimageYsize = GetDlgItemInt(hDlg, IDC_YSIZE, &bSuccess, TRUE);
            SubimageXloc = GetDlgItemInt(hDlg, IDC_XLOC, &bSuccess, TRUE);
            SubimageYloc = GetDlgItemInt(hDlg, IDC_YLOC, &bSuccess, TRUE);
            StartFrame = GetDlgItemInt(hDlg, IDC_START_FRAME, &bSuccess, TRUE);
            EndFrame = GetDlgItemInt(hDlg, IDC_END_FRAME, &bSuccess, TRUE);
            OutputXsize = GetDlgItemInt(hDlg, IDC_OUTPUT_XSIZE, &bSuccess, TRUE);
            OutputYsize = GetDlgItemInt(hDlg, IDC_OUTPUT_YSIZE, &bSuccess, TRUE);
            if (IsDlgButtonChecked(hDlg, IDC_CENTERED) == BST_CHECKED) {
                Centered = 1;
            }
            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                ScaleBinary = 1;
            }

            ImageExtract(hDlg, InputFile, OutputFile,
                ScaleBinary, SubimageXloc, SubimageYloc, StartFrame, EndFrame,
                SubimageXsize, SubimageYsize,
                OutputXsize, OutputYsize, Centered);

            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
        {
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractImageDlg", L"xsize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_YSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractImageDlg", L"ysize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_START_FRAME, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractImageDlg", L"StartFrame", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_END_FRAME, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractImageDlg", L"EndFrame", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XLOC, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractImageDlg", L"xloc", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_YLOC, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractImageDlg", L"yloc", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_OUTPUT_XSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractImageDlg", L"OutputXsize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_OUTPUT_YSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractImageDlg", L"OutputYsize", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_CENTERED) == BST_CHECKED) {
                WritePrivateProfileString(L"ExtractImageDlg", L"Centered", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"ExtractImageDlg", L"Centered", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                WritePrivateProfileString(L"ExtractImageDlg", L"ScalePixel", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"ExtractImageDlg", L"ScalePixel", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for AppendEndImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK AppendEndImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"AppendEndImageDlg", L"ImageInput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"AppendEndImageDlg", L"ImageInput2", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI2, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI2, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES2, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI2, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI2, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES2, 0, TRUE);
        }

        GetPrivateProfileString(L"AppendEndImageDlg", L"ImageOutput", L"AppendedEnd.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
        {
            int IncrFrames;
            IncrFrames = GetPrivateProfileInt(L"AppendEndImageDlg", L"IncrFrames", 0, (LPCTSTR)strAppNameINI);
            if (IncrFrames == 0) {
                CheckDlgButton(hDlg, IDC_INCR_FRAMES, BST_UNCHECKED);
            }
            else {
                CheckDlgButton(hDlg, IDC_INCR_FRAMES, BST_CHECKED);
            }
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_INPUT_BROWSE2:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI2, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI2, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES2, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI2, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI2, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES2, 0, TRUE);
            }

            if (ReadImageHeader(szString, &ImageHeader) != 1) {
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

            return (INT_PTR)TRUE;
        }

        case IDC_APPEND:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR InputFile2[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int IncrFrames=0;
            
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, InputFile2, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            if (IsDlgButtonChecked(hDlg, IDC_INCR_FRAMES) == BST_CHECKED) {
                IncrFrames = 1;
            }

            ImageAppendEnd(hDlg, InputFile, InputFile2, OutputFile, IncrFrames);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"AppendEndImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString, MAX_PATH);
            WritePrivateProfileString(L"AppendEndImageDlg", L"ImageInput2", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"AppendEndImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_INCR_FRAMES) == BST_CHECKED) {
                WritePrivateProfileString(L"AppendEndImageDlg", L"IncrFrames", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"AppendEndImageDlg", L"IncrFrames", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
           EndDialog(hDlg, LOWORD(wParam));
           return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for AppendRightImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK AppendRightImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"AppendRightImageDlg", L"ImageInput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"AppendRightImageDlg", L"ImageInput2", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString);

        GetPrivateProfileString(L"AppendRightImageDlg", L"ImageOutput", L"AppendedRight.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_INPUT_BROWSE2:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString);

            if (ReadImageHeader(szString, &ImageHeader) != 1) {
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

            return (INT_PTR)TRUE;
        }

        case IDC_APPEND:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR InputFile2[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, InputFile2, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);

            ImageAppendRight(hDlg, InputFile, InputFile2, OutputFile);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"AppendRightImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString, MAX_PATH);
            WritePrivateProfileString(L"AppendRightImageDlg", L"ImageInput2", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"AppendRightImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }

    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for ReorderImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK ReorderImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];


    case WM_INITDIALOG: 
    {
        int ScalePixel;
        int EnableBatch;
        int Invert;
        int GenerateBMP;
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"ReorderImageDlg", L"ImageInput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"ReorderImageDlg", L"TextInput", L"Reorder\\reorder.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);

        GetPrivateProfileString(L"ReorderImageDlg", L"ImageOutput", L"Reordered.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        ScalePixel = GetPrivateProfileInt(L"ReorderImageDlg", L"ScalePixel", 0, (LPCTSTR)strAppNameINI);
        if (!ScalePixel) {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_CHECKED);
        }

        Invert = GetPrivateProfileInt(L"ReorderImageDlg", L"Invert", 0, (LPCTSTR)strAppNameINI);
        if (!Invert) {
            CheckDlgButton(hDlg, IDC_INVERT, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_INVERT, BST_CHECKED);
        }

        EnableBatch = GetPrivateProfileInt(L"ReorderImageDlg", L"EnableBatch", 0, (LPCTSTR)strAppNameINI);
        if (!EnableBatch) {
            CheckDlgButton(hDlg, IDC_BATCH, BST_UNCHECKED);
            HWND GenarateHandle = GetDlgItem(hDlg, IDC_GENERATE_BMP);
            EnableWindow(GenarateHandle, FALSE);
        }
        else {
            CheckDlgButton(hDlg, IDC_BATCH, BST_CHECKED);
            GenerateBMP = GetPrivateProfileInt(L"ReorderImageDlg", L"GenerateBMP", 0, (LPCTSTR)strAppNameINI);
            HWND GenarateHandle = GetDlgItem(hDlg, IDC_GENERATE_BMP);
            EnableWindow(GenarateHandle, TRUE);
            if (!GenerateBMP) {
                CheckDlgButton(hDlg, IDC_GENERATE_BMP, BST_UNCHECKED);
            }
            else {
                CheckDlgButton(hDlg, IDC_GENERATE_BMP, BST_CHECKED);
            }
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_TEXT_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_BATCH:
            if (IsDlgButtonChecked(hDlg, IDC_BATCH) == BST_CHECKED) {
                HWND GenerateHandle = GetDlgItem(hDlg, IDC_GENERATE_BMP);
                EnableWindow(GenerateHandle, TRUE);
            }
            else {
                HWND GenerateHandle = GetDlgItem(hDlg, IDC_GENERATE_BMP);
                EnableWindow(GenerateHandle, FALSE);
            }
            return (INT_PTR)TRUE;

        case IDC_REORDER:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR TextInput[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int ScalePixel = 0;
            int GenerateBMP = 0;
            int EnableBatch = 0;
            int Invert = 0;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, TextInput, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                ScalePixel = 1;
            }

            if (IsDlgButtonChecked(hDlg, IDC_INVERT) == BST_CHECKED) {
                Invert = 1;
            }

            if (IsDlgButtonChecked(hDlg, IDC_BATCH) == BST_CHECKED) {
                EnableBatch = 1;
                if (IsDlgButtonChecked(hDlg, IDC_GENERATE_BMP) == BST_CHECKED) {
                    GenerateBMP = 1;
                }
            }

            PixelReorder(hDlg, TextInput, InputFile, OutputFile, ScalePixel, FALSE, EnableBatch,
                            GenerateBMP, Invert);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderImageDlg", L"TextInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);
            
            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                WritePrivateProfileString(L"ReorderImageDlg", L"ScalePixel", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"ReorderImageDlg", L"ScalePixel", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_INVERT) == BST_CHECKED) {
                WritePrivateProfileString(L"ReorderImageDlg", L"Invert", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"ReorderImageDlg", L"Invert", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_BATCH) == BST_CHECKED) {
                WritePrivateProfileString(L"ReorderImageDlg", L"EnableBatch", L"1", (LPCTSTR)strAppNameINI);
                if (IsDlgButtonChecked(hDlg, IDC_GENERATE_BMP) == BST_CHECKED) {
                    WritePrivateProfileString(L"ReorderImageDlg", L"GenerateBMP", L"1", (LPCTSTR)strAppNameINI);
                }
                else {
                    WritePrivateProfileString(L"ReorderImageDlg", L"GenerateBMP", L"0", (LPCTSTR)strAppNameINI);
                }
            }
            else {
                WritePrivateProfileString(L"ReorderImageDlg", L"EnableBatch", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for ReorderBlockImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK ReorderBlocksImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];


    case WM_INITDIALOG:
    {
        int ScalePixel;
        int EnableBatch;
        int Invert;
        int GenerateBMP;
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"ReorderBlocksImageDlg", L"ImageInput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"ReorderBlocksImageDlg", L"TextInput", L"Reorder\\reorder.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);

        GetPrivateProfileString(L"ReorderBlocksImageDlg", L"ImageOutput", L"Reordered.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"ReorderBlocksImageDlg", L"Xsize", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE, szString);

        GetPrivateProfileString(L"ReorderBlocksImageDlg", L"Ysize", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_YSIZE, szString);

        GetPrivateProfileString(L"ReorderBlocksImageDlg", L"PixelSize", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_PIXEL_SIZE, szString);

        ScalePixel = GetPrivateProfileInt(L"ReorderBlocksImageDlg", L"ScalePixel", 0, (LPCTSTR)strAppNameINI);
        if (!ScalePixel) {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_CHECKED);
        }

        Invert = GetPrivateProfileInt(L"ReorderBlocksImageDlg", L"Invert", 0, (LPCTSTR)strAppNameINI);
        if (!Invert) {
            CheckDlgButton(hDlg, IDC_INVERT, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_INVERT, BST_CHECKED);
        }

        EnableBatch = GetPrivateProfileInt(L"ReorderBlocksImageDlg", L"EnableBatch", 0, (LPCTSTR)strAppNameINI);
        if (!EnableBatch) {
            CheckDlgButton(hDlg, IDC_BATCH, BST_UNCHECKED);
            HWND GenarateHandle = GetDlgItem(hDlg, IDC_GENERATE_BMP);
            EnableWindow(GenarateHandle, FALSE);
        }
        else {
            CheckDlgButton(hDlg, IDC_BATCH, BST_CHECKED);
            GenerateBMP = GetPrivateProfileInt(L"ReorderBlocksImageDlg", L"GenerateBMP", 0, (LPCTSTR)strAppNameINI);
            HWND GenarateHandle = GetDlgItem(hDlg, IDC_GENERATE_BMP);
            EnableWindow(GenarateHandle, TRUE);
            if (!GenerateBMP) {
                CheckDlgButton(hDlg, IDC_GENERATE_BMP, BST_UNCHECKED);
            }
            else {
                CheckDlgButton(hDlg, IDC_GENERATE_BMP, BST_CHECKED);
            }
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_TEXT_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_BATCH:
            if (IsDlgButtonChecked(hDlg, IDC_BATCH) == BST_CHECKED) {
                HWND GenerateHandle = GetDlgItem(hDlg, IDC_GENERATE_BMP);
                EnableWindow(GenerateHandle, TRUE);
            }
            else {
                HWND GenerateHandle = GetDlgItem(hDlg, IDC_GENERATE_BMP);
                EnableWindow(GenerateHandle, FALSE);
            }
            return (INT_PTR)TRUE;

        case IDC_REORDER:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR TextInput[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int ScalePixel = 0;
            int GenerateBMP = 0;
            int EnableBatch = 0;
            int Invert = 0;
            int Xsize;
            int Ysize;
            int PixelSize;
            int bSuccess;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, TextInput, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);

            Xsize = GetDlgItemInt(hDlg, IDC_XSIZE, &bSuccess, TRUE);
            Ysize = GetDlgItemInt(hDlg, IDC_YSIZE, &bSuccess, TRUE);
            PixelSize = GetDlgItemInt(hDlg, IDC_PIXEL_SIZE, &bSuccess, TRUE);

            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                ScalePixel = 1;
            }

            if (IsDlgButtonChecked(hDlg, IDC_INVERT) == BST_CHECKED) {
                Invert = 1;
            }

            if (IsDlgButtonChecked(hDlg, IDC_BATCH) == BST_CHECKED) {
                EnableBatch = 1;
                if (IsDlgButtonChecked(hDlg, IDC_GENERATE_BMP) == BST_CHECKED) {
                    GenerateBMP = 1;
                }
            }

            BlockReorder(hDlg, TextInput, InputFile, OutputFile, ScalePixel, FALSE, EnableBatch,
                GenerateBMP, Xsize, Ysize, PixelSize, Invert);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderBlocksImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderBlocksImageDlg", L"TextInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderBlocksImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderBlocksImageDlg", L"Xsize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_YSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderBlocksImageDlg", L"Ysize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_PIXEL_SIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderBlocksImageDlg", L"PixelSize", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                WritePrivateProfileString(L"ReorderBlocksImageDlg", L"ScalePixel", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"ReorderBlocksImageDlg", L"ScalePixel", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_INVERT) == BST_CHECKED) {
                WritePrivateProfileString(L"ReorderBlocksImageDlg", L"Invert", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"ReorderBlocksImageDlg", L"Invert", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_BATCH) == BST_CHECKED) {
                WritePrivateProfileString(L"ReorderBlocksImageDlg", L"EnableBatch", L"1", (LPCTSTR)strAppNameINI);
                if (IsDlgButtonChecked(hDlg, IDC_GENERATE_BMP) == BST_CHECKED) {
                    WritePrivateProfileString(L"ReorderBlocksImageDlg", L"GenerateBMP", L"1", (LPCTSTR)strAppNameINI);
                }
                else {
                    WritePrivateProfileString(L"ReorderBlocksImageDlg", L"GenerateBMP", L"0", (LPCTSTR)strAppNameINI);
                }
            }
            else {
                WritePrivateProfileString(L"ReorderBlocksImageDlg", L"EnableBatch", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for FoldLeftImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK FoldLeftImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"FoldLeftImageDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"FoldLeftImageDlg", L"ImageOutput", L"FoldLeft.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"FoldLeftImageDlg", L"FoldColumn", L"128", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_FOLD:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int FoldColumn;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            FoldColumn = GetDlgItemInt(hDlg, IDC_FOLD_NUMBER, &bSuccess, TRUE);

            FoldImageLeft(hDlg, InputFile, OutputFile, FoldColumn);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldLeftImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldLeftImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldLeftImageDlg", L"FoldColumn", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for FoldRightImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK FoldRightImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"FoldRightImageDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"FoldRightImageDlg", L"ImageOutput", L"FoldRight.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"FoldRightImageDlg", L"FoldColumn", L"128", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L".raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_FOLD:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int FoldColumn;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            FoldColumn = GetDlgItemInt(hDlg, IDC_FOLD_NUMBER, &bSuccess, TRUE);

            FoldImageRight(hDlg, InputFile, OutputFile, FoldColumn);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldRightImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldRightImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldRightImageDlg", L"FoldColumn", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for FoldDownImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK FoldDownImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"FoldDownImageDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"FoldDownImageDlg", L"ImageOutput", L"FoldDown.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"FoldDownImageDlg", L"FoldRow", L"128", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_FOLD:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int FoldRow;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            FoldRow = GetDlgItemInt(hDlg, IDC_FOLD_NUMBER, &bSuccess, TRUE);
            
            FoldImageDown(hDlg, InputFile, OutputFile, FoldRow);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldDownImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldDownImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldDownImageDlg", L"FoldRow", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for FoldUpImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK FoldUpImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"FoldUpImageDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"FoldUpImageDlg", L"ImageOutput", L"FoldUp.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"FoldUpImageDlg", L"FoldRow", L"128", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_FOLD:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int FoldRow;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            FoldRow = GetDlgItemInt(hDlg, IDC_FOLD_NUMBER, &bSuccess, TRUE);
            
            FoldImageUp(hDlg, InputFile, OutputFile, FoldRow);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldUpImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldUpImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString, MAX_PATH);
            WritePrivateProfileString(L"FoldUpImageDlg", L"FoldRow", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for LeftAccordionImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK LeftAccordionImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"LeftAccordionImageDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"LeftAccordionImageDlg", L"ImageOutput", L"AccordionLeft.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"LeftAccordionImageDlg", L"AccordionSize", L"16", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_FOLD:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int AccordionSize;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            AccordionSize = GetDlgItemInt(hDlg, IDC_FOLD_NUMBER, &bSuccess, TRUE);
            AccordionImageLeft(hDlg, InputFile, OutputFile, AccordionSize);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"LeftAccordionImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"LeftAccordionImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString, MAX_PATH);
            WritePrivateProfileString(L"LeftAccordionImageDlg", L"AccordionSize", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for RightAccordionImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK RightAccordionImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"RightAccordionImageDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"RightAccordionImageDlg", L"ImageOutput", L"AccordionRight.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"RightAccordionImageDlg", L"AccordionSize", L"16", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_FOLD:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int AccordionSize;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            AccordionSize = GetDlgItemInt(hDlg, IDC_FOLD_NUMBER, &bSuccess, TRUE);
            AccordionImageRight(hDlg, InputFile, OutputFile, AccordionSize);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"RightAccordionImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"RightAccordionImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_FOLD_NUMBER, szString, MAX_PATH);
            WritePrivateProfileString(L"RightAccordionImageDlg", L"AccordionSize", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for LeftShiftRowsImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK LeftShiftRowsImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"LeftShiftRowsImageDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"LeftShiftRowsImageDlg", L"ImageOutput", L"ShiftLeft.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_SHIFT:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            LeftShiftImage(hDlg, InputFile, OutputFile);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"LeftShiftRowsImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"LeftShiftRowsImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for ConvolutionImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK ConvolutionImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];


    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"ConvolutionImageDlg", L"ImageInput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"ConvolutionImageDlg", L"TextInput", L"Convolution\\Kernel.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);

        GetPrivateProfileString(L"ConvolutionImageDlg", L"ImageOutput", L"Convolved.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_TEXT_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_CONVOLVE:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR TextInput[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];


            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, TextInput, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            
            ConvolveImage(hDlg, TextInput, InputFile, OutputFile);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ConvolutionImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ConvolutionImageDlg", L"TextInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ConvolutionImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for AddImagesImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK AddImagesImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"AddImagesImageDlg", L"ImageInput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"AddImagesImageDlg", L"ImageInput2", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI2, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI2, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES2, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI2, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI2, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES2, 0, TRUE);
        }

        GetPrivateProfileString(L"AddImagesImageDlg", L"ImageOutput", L"SummedImage.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"Image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_INPUT_BROWSE2:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"Image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI2, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI2, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES2, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }


            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"Image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

            return (INT_PTR)TRUE;
        }

        case IDC_ADD:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR InputFile2[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, InputFile2, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);

            AddSubtractImages(hDlg, InputFile, InputFile2, OutputFile,TRUE);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDC_SUBTRACT:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR InputFile2[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, InputFile2, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);

            AddSubtractImages(hDlg, InputFile, InputFile2, OutputFile, FALSE);

            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"AddImagesImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString, MAX_PATH);
            WritePrivateProfileString(L"AddImagesImageDlg", L"ImageInput2", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"AddImagesImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }

    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for MirrorDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK MirrorDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"MirrorDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"MirrorDlg", L"ImageOutput", L"Mirrored.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        int Direction = GetPrivateProfileInt(L"MirrorDlg", L"Direction", 0, (LPCTSTR)strAppNameINI);
        if (!Direction) {
            CheckRadioButton(hDlg, IDC_MIRROR_HORIZ, IDC_MIRROR_VERT, IDC_MIRROR_HORIZ);
        }
        else {
            CheckRadioButton(hDlg, IDC_MIRROR_HORIZ, IDC_MIRROR_VERT, IDC_MIRROR_VERT);
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"Image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"Image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_MIRROR:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            int Direction;
            if (IsDlgButtonChecked(hDlg, IDC_MIRROR_HORIZ)) {
                Direction = 0;
            }
            else {
                Direction = 1;
            }

            MirrorImage(hDlg, InputFile, OutputFile, Direction);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"MirrorDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"MirrorDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_MIRROR_HORIZ)) {
                WritePrivateProfileString(L"MirrorDlg", L"Direction", L"0", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"MirrorDlg", L"Direction", L"1", (LPCTSTR)strAppNameINI);
            }
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for RotateDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK RotateDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"RotateDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"RotateDlg", L"ImageOutput", L"Rotated.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        int Direction = GetPrivateProfileInt(L"RotateDlg", L"Direction", 0, (LPCTSTR)strAppNameINI);
        if (Direction==0) {
            CheckRadioButton(hDlg, IDC_ROTATE_CCW, IDC_ROTATE_CW, IDC_ROTATE_CCW);
        }
        else {
            CheckRadioButton(hDlg, IDC_ROTATE_CCW, IDC_ROTATE_CW, IDC_ROTATE_CW);
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"Image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"Image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_ROTATE:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            int Direction;
            if (IsDlgButtonChecked(hDlg, IDC_ROTATE_CCW)) {
                Direction = 0;
            }
            else {
                Direction = 1;
            }

            RotateImage(hDlg, InputFile, OutputFile, Direction);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"RotateDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"RotateDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_ROTATE_CCW)) {
                WritePrivateProfileString(L"RotateDlg", L"Direction", L"0", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"RotateDlg", L"Direction", L"1", (LPCTSTR)strAppNameINI);
            }
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for ResizeDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK ResizeDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"ResizeDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, ImageHeader.PixelSize, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, 0, TRUE);
        }

        GetPrivateProfileString(L"ResizeDlg", L"ImageOutput", L"Resized.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"ResizeDlg", L"Xsize", L"256", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE, szString);

        GetPrivateProfileString(L"ResizeDlg", L"Ysize", L"256", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_YSIZE, szString);

        GetPrivateProfileString(L"ResizeDlg", L"PixelSize", L"1", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_PIXEL_SIZE, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
                SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, ImageHeader.PixelSize, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_RESIZE:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int Xsize,Ysize;
            int PixelSize;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            Xsize = GetDlgItemInt(hDlg, IDC_XSIZE, &bSuccess, TRUE);
            Ysize = GetDlgItemInt(hDlg, IDC_YSIZE, &bSuccess, TRUE);
            PixelSize = GetDlgItemInt(hDlg, IDC_PIXEL_SIZE, &bSuccess, TRUE);

            if (PixelSize!=1 && PixelSize!=2 && PixelSize!=4) {
                MessageBox(hDlg, L"Pixel size must be 1, 2, or 4", L"Bad entry", MB_OK);
                return (INT_PTR)TRUE;
            }

            int iRes;
            iRes = ResizeImage(InputFile, OutputFile, Xsize, Ysize, PixelSize);
            if (iRes != 1) {
                MessageMySETIappError(hDlg, iRes, L"Resize image error");
            }
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ResizeDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ResizeDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ResizeDlg", L"Xsize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_YSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ResizeDlg", L"Ysize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_PIXEL_SIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ResizeDlg", L"PixelSize", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for ReorderAlgDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK ReorderAlgDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;
        int Algorithm;

        GetPrivateProfileString(L"ReorderAlgDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, ImageHeader.PixelSize, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, 0, TRUE);
        }

        GetPrivateProfileString(L"ReorderAlgDlg", L"ImageOutput", L"ReorderedAlg.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"ReorderAlgDlg", L"Xsize", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE, szString);

        GetPrivateProfileString(L"ReorderAlgDlg", L"Ysize", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_YSIZE, szString);

        GetPrivateProfileString(L"ReorderAlgDlg", L"PixelSize", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_PIXEL_SIZE, szString);

        GetPrivateProfileString(L"ReorderAlgDlg", L"P1", L"8", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_P1, szString);

        GetPrivateProfileString(L"ReorderAlgDlg", L"P2", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_P2, szString);

        GetPrivateProfileString(L"ReorderAlgDlg", L"P3", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_P3, szString);

        Algorithm = GetPrivateProfileInt(L"ReorderAlgDlg", L"Algorithm", 0, (LPCTSTR)strAppNameINI);
        // combo box with alogrithm list
        HWND ComboHwnd;
        ComboHwnd = GetDlgItem(hDlg, IDC_COMBO_ALG);
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"Just resize");
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"Quadrant UL,UR,LL,LR, left to right, top to bottom");
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"Quadrant UL,LL,UR,LR, left to right, top to bottom");
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"Quadrant LL,UL,LR,UR, left to right, top to bottom");
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"Quadrant LL,LR,UL,UR, left to right, top to bottom");
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"Quadrant UL,UR,LL,LR, center out");
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"Incrementally rotate rach row by P1");
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"Image in stripes, P1 # stripes");
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"Circular shift rows by P1");
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"Circular shift columns by P1");
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"MxN block decom, M = P1, N = P2");
        SendMessage(ComboHwnd, CB_ADDSTRING, 0, (LPARAM)L"Split image left/right, # pixels in split group");
        SendMessage(ComboHwnd, CB_SETCURSEL, Algorithm, 0);
        SendMessage(ComboHwnd, CB_SETTOPINDEX, Algorithm, 0);

        int Invert = GetPrivateProfileInt(L"ReorderAlgDlg", L"Invert", 0, (LPCTSTR)strAppNameINI);
        if (!Invert) {
            CheckDlgButton(hDlg, IDC_INVERT, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_INVERT, BST_CHECKED);
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_REORDER:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int Xsize, Ysize;
            int PixelSize;
            int Algorithm;
            int P1;
            int P2;
            int P3;
            int Invert = 0;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            Xsize = GetDlgItemInt(hDlg, IDC_XSIZE, &bSuccess, TRUE);
            Ysize = GetDlgItemInt(hDlg, IDC_YSIZE, &bSuccess, TRUE);
            PixelSize = GetDlgItemInt(hDlg, IDC_PIXEL_SIZE, &bSuccess, TRUE);
            P1 = GetDlgItemInt(hDlg, IDC_P1, &bSuccess, TRUE);
            P2 = GetDlgItemInt(hDlg, IDC_P2, &bSuccess, TRUE);
            P3 = GetDlgItemInt(hDlg, IDC_P3, &bSuccess, TRUE);
            if (IsDlgButtonChecked(hDlg, IDC_INVERT)) {
                Invert = 1;
            }

            if (PixelSize != 0 && PixelSize != 1 && PixelSize != 2 && PixelSize != 4) {
                MessageBox(hDlg, L"Pixel size must be 1, 2, or 4", L"Bad entry", MB_OK);
                return (INT_PTR)TRUE;
            }
            // get currently selected algorithm from combobox
            Algorithm = 0;
            HWND ComboHwnd;

            ComboHwnd = GetDlgItem(hDlg, IDC_COMBO_ALG);
            Algorithm = (int) SendMessage(ComboHwnd, CB_GETCURSEL, 0, 0);

            int iRes;
            iRes = ReorderAlg(InputFile, OutputFile, Xsize, Ysize, PixelSize, Algorithm, P1, P2, P3, Invert);
            if (iRes != 1) {
                MessageMySETIappError(hDlg, iRes, L"Algorithmic reorder image error\nCheck parameters");
            }
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderAlgDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderAlgDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderAlgDlg", L"Xsize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_YSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderAlgDlg", L"Ysize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_PIXEL_SIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderAlgDlg", L"PixelSize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_P1, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderAlgDlg", L"P1", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_P2, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderAlgDlg", L"P2", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_P3, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderAlgDlg", L"P3", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_INVERT)) {
                WritePrivateProfileString(L"ReorderAlgDlg", L"Invert", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"ReorderAlgDlg", L"Invert", L"0", (LPCTSTR)strAppNameINI);
            }

            HWND ComboHwnd;
            int Algorithm;

            ComboHwnd = GetDlgItem(hDlg, IDC_COMBO_ALG);
            Algorithm = (int)SendMessage(ComboHwnd, CB_GETCURSEL, 0, 0);

            swprintf_s(szString, MAX_PATH, L"%d", Algorithm);
            WritePrivateProfileString(L"ReorderAlgDlg", L"Algorithm", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for StdDecimationDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK StdDecimationDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"StdDecimationDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, ImageHeader.PixelSize, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, 0, TRUE);
        }

        GetPrivateProfileString(L"StdDecimationDlg", L"ImageOutput", L"Decimated.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"StdDecimationDlg", L"Xdecimate", L"2", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XDECIMATE, szString);

        GetPrivateProfileString(L"StdDecimationDlg", L"Ydecimate", L"2", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_YDECIMATE, szString);

        GetPrivateProfileString(L"StdDecimationDlg", L"PixelSize", L"1", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_PIXEL_SIZE, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_DECIMATE:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int Xsize, Ysize;
            int PixelSize;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            Xsize = GetDlgItemInt(hDlg, IDC_XDECIMATE, &bSuccess, TRUE);
            Ysize = GetDlgItemInt(hDlg, IDC_YDECIMATE, &bSuccess, TRUE);
            PixelSize = GetDlgItemInt(hDlg, IDC_PIXEL_SIZE, &bSuccess, TRUE);

            if (PixelSize != 1 && PixelSize != 2 && PixelSize != 4) {
                MessageBox(hDlg, L"Pixel size must be 1, 2, or 4", L"Bad entry", MB_OK);
                return (INT_PTR)TRUE;
            }

            int iRes;
            iRes = StdDecimateImage(InputFile, OutputFile, Xsize, Ysize, PixelSize);
            if (iRes != 1) {
                MessageMySETIappError(hDlg, iRes, L"Resize image error");
            }
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"StdDecimationDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"StdDecimationDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XDECIMATE, szString, MAX_PATH);
            WritePrivateProfileString(L"StdDecimationDlg", L"Xdecimate", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_YDECIMATE, szString, MAX_PATH);
            WritePrivateProfileString(L"StdDecimationDlg", L"Ydecimate", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_PIXEL_SIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"StdDecimationDlg", L"PixelSize", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for DecimationDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK DecimationDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];


    case WM_INITDIALOG:
    {
        int ScalePixel;
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"DecimationDlg", L"ImageInput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"DecimationDlg", L"TextInput", L"Decimation\\Decimation.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);

        GetPrivateProfileString(L"DecimationDlg", L"ImageOutput", L"Decimated.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        ScalePixel = GetPrivateProfileInt(L"DecimationDlg", L"ScalePixel", 0, (LPCTSTR)strAppNameINI);
        if (!ScalePixel) {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_CHECKED);
        }
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_TEXT_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_DECIMATE:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR TextInput[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int ScalePixel = 0;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, TextInput, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                ScalePixel = 1;
            }
            
            int iRes;
            iRes = DecimateImage(InputFile, TextInput, OutputFile, ScalePixel);
            if (iRes == 1) {
                wcscpy_s(szCurrentFilename, OutputFile);
                return (INT_PTR)TRUE;
            }
            if (iRes == 0 || iRes==-3) {
                MessageBox(hDlg, L"Problem with decimation kernel, check formatting", L"File incompatible", MB_OK);
                return (INT_PTR)TRUE;
            }
            MessageMySETIappError(hDlg, iRes, L"Add/Subtract constant error");
            
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"DecimationDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"DecimationDlg", L"TextInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"DecimationDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                WritePrivateProfileString(L"DecimationDlg", L"ScalePixel", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"DecimationDlg", L"ScalePixel", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for ReplicationDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK ReplicationDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"ReplicationDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, ImageHeader.PixelSize, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, 0, TRUE);
        }

        GetPrivateProfileString(L"ReplicationDlg", L"ImageOutput", L"replicated.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"ReplicationDlg", L"Xduplicate", L"2", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XDUPLICATE, szString);

        GetPrivateProfileString(L"ReplicationDlg", L"Yduplicate", L"2", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_YDUPLICATE, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_REPLICATE:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int Xsize, Ysize;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            Xsize = GetDlgItemInt(hDlg, IDC_XDUPLICATE, &bSuccess, TRUE);
            Ysize = GetDlgItemInt(hDlg, IDC_YDUPLICATE, &bSuccess, TRUE);

            int iRes;
            iRes = ReplicateImage(InputFile, OutputFile, Xsize, Ysize);
            if (iRes != 1) {
                MessageMySETIappError(hDlg, iRes, L"Resize image error");
            }
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReplicationDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReplicationDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XDUPLICATE, szString, MAX_PATH);
            WritePrivateProfileString(L"ReplicationDlg", L"Xduplicate", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_YDUPLICATE, szString, MAX_PATH);
            WritePrivateProfileString(L"ReplicationDlg", L"Yduplicate", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for AddConstantDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK MathConstantDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;
        int Operation;
        int Warn;

        GetPrivateProfileString(L"MathConstantDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, ImageHeader.PixelSize, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, 0, TRUE);

        }

        GetPrivateProfileString(L"MathConstantDlg", L"ImageOutput", L"Result.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"MathConstantDlg", L"Value", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_VALUE, szString);

        Operation = GetPrivateProfileInt(L"MathConstantDlg", L"Operation", 0, (LPCTSTR)strAppNameINI);
        if (Operation == 0) {
            CheckRadioButton(hDlg, IDC_ADDSUBTRACT, IDC_DIVIDE, IDC_ADDSUBTRACT);
        }
        else if(Operation ==1) {
            CheckRadioButton(hDlg, IDC_ADDSUBTRACT, IDC_DIVIDE, IDC_MULTIPLY);
        }
        else {
            CheckRadioButton(hDlg, IDC_ADDSUBTRACT, IDC_DIVIDE, IDC_DIVIDE);
        }

        Warn = GetPrivateProfileInt(L"MathConstantDlg", L"Warn", 1, (LPCTSTR)strAppNameINI);
        if (!Warn) {
            CheckDlgButton(hDlg, IDC_WARN, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_WARN, BST_CHECKED);
        }
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
                SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, ImageHeader.PixelSize, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_PERFORM:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int Value;
            int Operation;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            Value = GetDlgItemInt(hDlg, IDC_VALUE, &bSuccess, TRUE);

            if (IsDlgButtonChecked(hDlg, IDC_ADDSUBTRACT)) {
                Operation = 0;
            }
            else if (IsDlgButtonChecked(hDlg, IDC_MULTIPLY)) {
                Operation = 1;
                if (Value < 0) {
                    MessageBox(hDlg, L"Value must be >=0 for multiply operation", L"Bad math", MB_OK);
                    return (INT_PTR)TRUE;
                }
            }
            else {
                Operation = 2;
                if (Value <= 0) {
                    MessageBox(hDlg, L"Value must be >=1 for division operation", L"Bad math", MB_OK);
                    return (INT_PTR)TRUE;
                }
            }

            int Warn = 0;
            if( IsDlgButtonChecked(hDlg, IDC_WARN)) {
                Warn = 1;
            }

            int iRes;
            int ArithmeticFlag;
            iRes = MathConstant2Image(InputFile, OutputFile, Value, Operation, Warn, &ArithmeticFlag);
            if (iRes != 1) {
                    MessageMySETIappError(hDlg, iRes, L"constant math operation");
            }
            if (ArithmeticFlag) {
                MessageBox(hDlg,L"Overflow or underflow occured on at least one pixel", L"Arithmetic warning", MB_OK);
            }

            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
        {
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"MathConstantDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"MathConstantDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_VALUE, szString, MAX_PATH);
            WritePrivateProfileString(L"MathConstantDlg", L"Value", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_ADDSUBTRACT)) {
                WritePrivateProfileString(L"MathConstantDlg", L"Operation", L"0", (LPCTSTR)strAppNameINI);
            }
            else if (IsDlgButtonChecked(hDlg, IDC_MULTIPLY)) {
                WritePrivateProfileString(L"MathConstantDlg", L"Operation", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"MathConstantDlg", L"Operation", L"2", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_WARN)) {
                WritePrivateProfileString(L"MathConstantDlg", L"Warn", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"MathConstantDlg", L"Warn", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for ExtractPacketsDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK ExtractSymbolsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"ExtractSymbolsDlg", L"BinaryInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"ExtractSymbolsDlg", L"ImageOutput", L"extracted.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"ExtractSymbolsDlg", L"SkipBits", L"8", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_SKIP_BITS, szString);

        GetPrivateProfileString(L"ExtractSymbolsDlg", L"xsizesymbol", L"1", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE_SYMBOL, szString);

        GetPrivateProfileString(L"ExtractSymbolsDlg", L"ysizesymbol", L"1", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_YSIZE_SYMBOL, szString);

        int Approach = GetPrivateProfileInt(L"ExtractSymbolsDlg", L"Approach", 1, (LPCTSTR)strAppNameINI);
        if (Approach == 1) {
            CheckRadioButton(hDlg, IDC_1D, IDC_2D, IDC_1D);
        }
        else {
            // Approach 2
            CheckRadioButton(hDlg, IDC_1D, IDC_2D, IDC_2D);
        }
        int Highlight;
        Highlight = GetPrivateProfileInt(L"ExtractSymbolsDlg", L"Highlight", 1, (LPCTSTR)strAppNameINI);
        if (!Highlight) {
            CheckDlgButton(hDlg, IDC_HIGHLIGHT, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_HIGHLIGHT, BST_CHECKED);
        }
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INPUT_BROWSE:
        {
            PWSTR pszFilename;

            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC bitType[] =
            {
                 { L"image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, bitType, L".raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);

            IMAGINGHEADER ImageHeader;

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }
        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_EXTRACT:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int SkipBits;
            int xsizesymbol;
            int ysizesymbol;

            if (wcscmp(szTempImageFilename, L"") == 0) {
                MessageBox(hDlg, L"Set temporary working image filename\nin Porperty->Settings", L"Missing settings",MB_OK);
                return (INT_PTR)TRUE;
            }
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);

            SkipBits = GetDlgItemInt(hDlg, IDC_SKIP_BITS, &bSuccess, TRUE);
            xsizesymbol = GetDlgItemInt(hDlg, IDC_XSIZE_SYMBOL, &bSuccess, TRUE);
            ysizesymbol = GetDlgItemInt(hDlg, IDC_YSIZE_SYMBOL, &bSuccess, TRUE);

            int Approach;
            if (IsDlgButtonChecked(hDlg, IDC_1D)) {
                Approach = 1;
            }
            else {
                Approach = 2;
            }

            int Highlight = 0;
            if (IsDlgButtonChecked(hDlg, IDC_HIGHLIGHT)) {
                Highlight = 1;
            }
            
            ExtractSymbols(hDlg, InputFile, OutputFile, SkipBits, xsizesymbol, ysizesymbol,Approach, Highlight);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractSymbolsDlg", L"BinaryInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractSymbolsDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_SKIP_BITS, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractSymbolsDlg", L"SkipBits", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XSIZE_SYMBOL, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractSymbolsDlg", L"xsizesymbol", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_YSIZE_SYMBOL, szString, MAX_PATH);
            WritePrivateProfileString(L"ExtractSymbolsDlg", L"ysizesymbol", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_1D)) {
                WritePrivateProfileString(L"ExtractSymbolsDlg", L"Approach", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"ExtractSymbolsDlg", L"Approach", L"2", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_HIGHLIGHT) == BST_CHECKED) {
                WritePrivateProfileString(L"ExtractSymbolsDlg", L"Highlight", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"ExtractSymbolsDlg", L"Highlight", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for InsertImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK InsertImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"InsertImageDlg", L"ImageInput", L"New.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"InsertImageDlg", L"ImageInput2", L"SignA.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI2, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI2, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES2, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI2, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI2, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES2, 0, TRUE);
        }

        GetPrivateProfileString(L"InsertImageDlg", L"ImageOutput", L"NewSign.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"InsertImageDlg", L"Xloc", L"64", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XLOC, szString);

        GetPrivateProfileString(L"InsertImageDlg", L"Yloc", L"64", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_YLOC, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_INPUT_BROWSE2:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString);

            if (ReadImageHeader(szString, &ImageHeader) != 1) {
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI2, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI2, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES2, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI2, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI2, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES2, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

            return (INT_PTR)TRUE;
        }

        case IDC_ADD:
        case IDC_OVERWRITE:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR InputFile2[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int IncrFrames = 0;
            BOOL bSuccess;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, InputFile2, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            //IDC_XLOC, IDC_YLOC
            int x, y;

            x = GetDlgItemInt(hDlg, IDC_XLOC, &bSuccess, TRUE);
            if (!bSuccess) {
                MessageBox(hDlg, L"X loc invalid", L"bad parameter", MB_OK);
                return (INT_PTR)TRUE;
            }
            if (x < 0) {
                MessageBox(hDlg, L"X < 0", L"bad parameter", MB_OK);
                return (INT_PTR)TRUE;
            }

            y = GetDlgItemInt(hDlg, IDC_YLOC, &bSuccess, TRUE);
            if (!bSuccess) {
                MessageBox(hDlg, L"Y loc invalid", L"bad parameter", MB_OK);
                return (INT_PTR)TRUE;
            }
            if (y < 0) {
                MessageBox(hDlg, L"Y < 0", L"bad parameter", MB_OK);
                return (INT_PTR)TRUE;
            }

            int iRes;
            int InsertAddFlag = 0;
            if (LOWORD(wParam) == IDC_OVERWRITE) {
                InsertAddFlag = 1;
            }
            iRes = InsertImage(hDlg, InputFile, InputFile2, OutputFile, x, y, InsertAddFlag);
            if (iRes == 1) {
                wcscpy_s(szCurrentFilename, OutputFile);
            }
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"InsertImageDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT2, szString, MAX_PATH);
            WritePrivateProfileString(L"InsertImageDlg", L"ImageInput2", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"InsertImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XLOC, szString, MAX_PATH);
            WritePrivateProfileString(L"InsertImageDlg", L"Xloc", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_YLOC, szString, MAX_PATH);
            WritePrivateProfileString(L"InsertImageDlg", L"Yloc", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for Image2StreamDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK Image2StreamDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"Image2StreamDlg", L"ImageInput", L"message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, ImageHeader.PixelSize, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, 0, TRUE);
        }

        GetPrivateProfileString(L"Image2StreamDlg", L"StreamOutput", L"Resized.bin", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"Image2StreamDlg", L"Frames", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_FRAMES, szString);

        GetPrivateProfileString(L"Image2StreamDlg", L"BitDepth", L"1", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BIT_DEPTH, szString);

        int Header = GetPrivateProfileInt(L"Image2StreamDlg", L"Header", 0, (LPCTSTR)strAppNameINI);
        if (!Header) {
            CheckDlgButton(hDlg, IDC_HEADER, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_HEADER, BST_CHECKED);
        }

        int BitOrder = GetPrivateProfileInt(L"Image2StreamDlg", L"BitOrder", 0, (LPCTSTR)strAppNameINI);
        if (!BitOrder) {
            CheckDlgButton(hDlg, IDC_BITORDER, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_BITORDER, BST_CHECKED);
        }

        int Invert = GetPrivateProfileInt(L"Image2StreamDlg", L"Invert", 0, (LPCTSTR)strAppNameINI);
        if (!Invert) {
            CheckDlgButton(hDlg, IDC_INVERT, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_INVERT, BST_CHECKED);
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
                SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, ImageHeader.PixelSize, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_PIXEL_SIZEI, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.bin" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.bin")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_CONVERT:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int Header = 0;
            int Invert = 0;
            int BitOrder = 0;
            int Frames;
            int BitDepth;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            BitDepth = GetDlgItemInt(hDlg, IDC_BIT_DEPTH, &bSuccess, TRUE);
            Frames = GetDlgItemInt(hDlg, IDC_FRAMES, &bSuccess, TRUE);
            if (IsDlgButtonChecked(hDlg, IDC_HEADER)) {
                Header = 1;
            }

            if (IsDlgButtonChecked(hDlg, IDC_BITORDER)) {
                BitOrder = 1;
            }

            if (IsDlgButtonChecked(hDlg, IDC_INVERT)) {
                Invert = 1;
            }

            int iRes;
            iRes = Image2Stream(hDlg, InputFile, OutputFile, BitDepth, Frames, Header, BitOrder, Invert);
            if (iRes != 1) {
                MessageMySETIappError(hDlg, iRes, L"Image2Stream error");
            }
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"Image2StreamDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"Image2StreamDlg", L"StreamOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_FRAMES, szString, MAX_PATH);
            WritePrivateProfileString(L"Image2StreamDlg", L"Frames", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BIT_DEPTH, szString, MAX_PATH);
            WritePrivateProfileString(L"Image2StreamDlg", L"BitDepth", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_HEADER)) {
                WritePrivateProfileString(L"Image2StreamDlg", L"Header", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"Image2StreamDlg", L"Header", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_BITORDER)) {
                WritePrivateProfileString(L"Image2StreamDlg", L"BitOrder", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"Image2StreamDlg", L"BitOrder", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_INVERT)) {
                WritePrivateProfileString(L"Image2StreamDlg", L"Invert", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"Image2StreamDlg", L"Invert", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}

//*******************************************************************************
//
// Message handler for ReorderImageBatchDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK ReorderImageBatchDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];


    case WM_INITDIALOG:
    {
        int ScalePixel;
        int EnableBatch;
        int GenerateBMP;
        int Invert;

        GetPrivateProfileString(L"ReorderImageBatchDlg", L"BatchInput", L"Batch.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BATCH_INPUT, szString);

        GetPrivateProfileString(L"ReorderImageBatchDlg", L"TextInput", L"Reorder\\reorder.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);

        ScalePixel = GetPrivateProfileInt(L"ReorderImageBatchDlg", L"ScalePixel", 0, (LPCTSTR)strAppNameINI);
        if (!ScalePixel) {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_CHECKED);
        }

        Invert = GetPrivateProfileInt(L"ReorderImageBatchDlg", L"Invert", 0, (LPCTSTR)strAppNameINI);
        if (!Invert) {
            CheckDlgButton(hDlg, IDC_INVERT, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_INVERT, BST_CHECKED);
        }

        EnableBatch = GetPrivateProfileInt(L"ReorderImageBatchDlg", L"EnableBatch", 0, (LPCTSTR)strAppNameINI);
        if (!EnableBatch) {
            CheckDlgButton(hDlg, IDC_BATCH, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_BATCH, BST_CHECKED);
        }

        GenerateBMP = GetPrivateProfileInt(L"ReorderImageBatchDlg", L"GenerateBMP", 0, (LPCTSTR)strAppNameINI);
        HWND GenarateHandle = GetDlgItem(hDlg, IDC_GENERATE_BMP);
        EnableWindow(GenarateHandle, TRUE);
        if (!GenerateBMP) {
            CheckDlgButton(hDlg, IDC_GENERATE_BMP, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_GENERATE_BMP, BST_CHECKED);
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BATCH_INPUT_BROWSE:
        {
            PWSTR pszFilename;

            GetDlgItemText(hDlg, IDC_BATCH_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"text files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BATCH_INPUT, szString);

            return (INT_PTR)TRUE;
        }

        case IDC_TEXT_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_REORDER:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR TextInput[MAX_PATH];
            int ScalePixel = 0;
            int GenerateBMP = 0;
            int EnableBatch = 0;
            int Invert = 0;

            GetDlgItemText(hDlg, IDC_BATCH_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, TextInput, MAX_PATH);
            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                ScalePixel = 1;
            }
            if (IsDlgButtonChecked(hDlg, IDC_BATCH) == BST_CHECKED) {
                EnableBatch = 1;
            }
            if (IsDlgButtonChecked(hDlg, IDC_INVERT) == BST_CHECKED) {
                Invert = 1;
            }
            if (IsDlgButtonChecked(hDlg, IDC_GENERATE_BMP) == BST_CHECKED) {
                GenerateBMP = 1;
            }

            PixelReorderBatch(hDlg, TextInput, InputFile, ScalePixel, FALSE, EnableBatch,
                                GenerateBMP, Invert);
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_BATCH_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderImageBatchDlg", L"BatchInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"ReorderImageBatchDlg", L"TextInput", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                WritePrivateProfileString(L"ReorderImageBatchDlg", L"ScalePixel", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"ReorderImageBatchDlg", L"ScalePixel", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_INVERT) == BST_CHECKED) {
                WritePrivateProfileString(L"ReorderImageBatchDlg", L"Invert", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"ReorderImageBatchDlg", L"Invert", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_BATCH) == BST_CHECKED) {
                WritePrivateProfileString(L"ReorderImageBatchDlg", L"EnableBatch", L"1", (LPCTSTR)strAppNameINI);
                if (IsDlgButtonChecked(hDlg, IDC_GENERATE_BMP) == BST_CHECKED) {
                    WritePrivateProfileString(L"ReorderImageBatchDlg", L"GenerateBMP", L"1", (LPCTSTR)strAppNameINI);
                }
                else {
                    WritePrivateProfileString(L"ReorderImageBatchDlg", L"GenerateBMP", L"0", (LPCTSTR)strAppNameINI);
                }
            }
            else {
                WritePrivateProfileString(L"ReorderImageBatchDlg", L"EnableBatch", L"0", (LPCTSTR)strAppNameINI);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}


//*******************************************************************************
//
// Message handler for AddKernelDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK AddKernelDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"AddKernelDlg", L"ImageInput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);
        if (ReadImageHeader(szString, &ImageHeader) == 1) {
            SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
        }
        else {
            SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
            SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
        }

        GetPrivateProfileString(L"AddKernelDlg", L"Kernel", L"kernel.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);
        
        GetPrivateProfileString(L"AddKernelDlg", L"ImageOutput", L"TestKernel.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_IMAGE_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            IMAGINGHEADER ImageHeader;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"Image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString);

            if (ReadImageHeader(szString, &ImageHeader) == 1) {
                SetDlgItemInt(hDlg, IDC_XSIZEI, ImageHeader.Xsize, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, ImageHeader.Ysize, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, ImageHeader.NumFrames, TRUE);
            }
            else {
                SetDlgItemInt(hDlg, IDC_XSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_YSIZEI, 0, TRUE);
                SetDlgItemInt(hDlg, IDC_NUM_FRAMES, 0, TRUE);
                MessageBox(hDlg, L"Selected file is not an image file", L"File incompatible", MB_OK);
            }

            return (INT_PTR)TRUE;
        }

        case IDC_TEXT_INPUT_BROWSE:
        {
            PWSTR pszFilename;

            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"Image files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);

            return (INT_PTR)TRUE;
        }

        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"Image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L"*.raw")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

            return (INT_PTR)TRUE;
        }

        case IDC_ADD:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR TextFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, TextFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);

            AddSubtractKernel(hDlg, InputFile, TextFile, OutputFile, TRUE);
            wcscpy_s(szCurrentFilename, OutputFile);
            return (INT_PTR)TRUE;
        }

        case IDC_SUBTRACT:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR TextFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, TextFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);

            AddSubtractKernel(hDlg, InputFile, TextFile, OutputFile, FALSE);

            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"AddKernelDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"AddKernelDlg", L"Kernel", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"AddKernelDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }

    return (INT_PTR)FALSE;
}

