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

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, TextInput, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                ScalePixel = 1;
            }
            if (IsDlgButtonChecked(hDlg, IDC_BATCH) == BST_CHECKED) {
                EnableBatch = 1;
                if (IsDlgButtonChecked(hDlg, IDC_GENERATE_BMP) == BST_CHECKED) {
                    GenerateBMP = 1;
                }
            }

            PixelReorder(hDlg, TextInput, InputFile, OutputFile, ScalePixel, FALSE, EnableBatch, GenerateBMP);

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
            } else {
                WritePrivateProfileString(L"ReorderImageDlg", L"ScalePixel", L"0", (LPCTSTR)strAppNameINI);
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

            AddImages(hDlg, InputFile, InputFile2, OutputFile);

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

        GetDlgItemText(hDlg, IDC_XSIZEI, szString, MAX_PATH);
        GetPrivateProfileString(L"ResizeDlg", L"Xsize", szString, szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE, szString);

        GetDlgItemText(hDlg, IDC_YSIZEI, szString, MAX_PATH);
        GetPrivateProfileString(L"ResizeDlg", L"Ysize", szString, szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_YSIZE, szString);

        GetDlgItemText(hDlg, IDC_PIXEL_SIZEI, szString, MAX_PATH);
        GetPrivateProfileString(L"ResizeDlg", L"PixelSize", szString, szString, MAX_PATH, (LPCTSTR)strAppNameINI);
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

        case IDC_RESIZE:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int Xsize,Ysize;
            int XsizeI, YsizeI;
            int PixelSize;

            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);
            Xsize = GetDlgItemInt(hDlg, IDC_XSIZE, &bSuccess, TRUE);
            Ysize = GetDlgItemInt(hDlg, IDC_YSIZE, &bSuccess, TRUE);
            XsizeI = GetDlgItemInt(hDlg, IDC_XSIZEI, &bSuccess, TRUE);
            YsizeI = GetDlgItemInt(hDlg, IDC_YSIZEI, &bSuccess, TRUE);
            PixelSize = GetDlgItemInt(hDlg, IDC_PIXEL_SIZE, &bSuccess, TRUE);

            if ((Xsize * Ysize) != (XsizeI * YsizeI)) {
                MessageBox(hDlg, L"New Xsize*Ysize must equal old Xsize*Ysize", L"Bad entry", MB_OK);
                return (INT_PTR)TRUE;
            }

            if (PixelSize!=1 && PixelSize!=2 && PixelSize!=4) {
                MessageBox(hDlg, L"Pixel size must be 1, 2, or 4", L"Bad entry", MB_OK);
                return (INT_PTR)TRUE;
            }

            int iRes;
            iRes = ResizeImage(InputFile, OutputFile, Xsize, Ysize, PixelSize);
            if (iRes != 1) {
                MessageBox(hDlg, L"Error writing new image file", L"File I/O error", MB_OK);
            }
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
                return (INT_PTR)TRUE;
            }
            if (iRes == 0 || iRes==-3) {
                MessageBox(hDlg, L"Problem with decimation kernel, check formatting", L"File incompatible", MB_OK);
                return (INT_PTR)TRUE;
            }
            MessageBox(hDlg, L"Error, could not process", L"I/O error", MB_OK);

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

