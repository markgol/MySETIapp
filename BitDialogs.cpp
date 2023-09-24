//
// MySETIapp, a set tools for decoding bitstreams into various formats and manipulating those files
// BitDialogs.cpp
// MySETIapp, tool for decoding bitstreams into various formats and manipluting those files
// BitDialogs.cpp
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
// If not, see < https://www.gnu.org/licenses/>. 
// 
// This file contains the dialog callback procedures for the bit tools menu
// 
// V1.0.0.1 2023-08-20  Initial Release
// V1.1.0.1 2023-08-22  Added file type specifications to open/save dialogs
// V1.2.0.1 2023-08-31  Added text to packed bit stream file
// V1.2.2.1 2023-09-06  Changed default folders\filenames
// V1.2.4.1 2023-09-09  Added a batch mode for bitstream to image file
// V1.2.6.1 2023-09-24  Changed HEX dump, added skip bytes from start of file
//
// Bit tools dialog box handlers
// 
// New dialogs for the bit tools menu should be added to this module.
// The functions that actually perform the actions should be put in
// bitstream.cpp and bitstream.h
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
#include "Globals.h"
#include "FileFunctions.h"
#include "BitStream.h"
#include "imaging.h"

// Add new callback prototype declarations in my MySETIapp.cpp

//*******************************************************************************
//
// Message handler for BitHexDump dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK BitHexDumpDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
        GetPrivateProfileString(L"BitHexDumpDlg", L"BinaryInput", L"OriginalSource\\data17.bin", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);

        GetPrivateProfileString(L"BitHexDumpDlg", L"TextOutput", L"data17-hex.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);

        GetPrivateProfileString(L"BitHexDumpDlg", L"xsize", L"256", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE, szString);

        GetPrivateProfileString(L"BitHexDumpDlg", L"SkipBytes", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_SKIP_BYTES, szString);

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            COMDLG_FILTERSPEC AllType[] =
            {
                 { L"bit stream files", L"*.bin" },
                 { L"image files", L"*.raw" },
                 { L"BMP files", L"*.bmp" },
                 { L"text files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 5, AllType, L".bin")) {
                return (INT_PTR)TRUE;
            }
            wcscpy_s(szString, pszFilename);
            CoTaskMemFree(pszFilename);
            
            SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);
            return (INT_PTR)TRUE;
        }
        case IDC_TEXT_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, textType, L".txt")) {
                return (INT_PTR)TRUE;
            }
            wcscpy_s(szString, pszFilename);
            CoTaskMemFree(pszFilename);
          
            SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_CONVERT:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int xsize;
            int SkipBytes;

            GetDlgItemText(hDlg, IDC_BINARY_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, OutputFile, MAX_PATH);

            xsize = GetDlgItemInt(hDlg, IDC_XSIZE, &bSuccess, TRUE);
            SkipBytes = GetDlgItemInt(hDlg, IDC_SKIP_BYTES, &bSuccess, TRUE);

            FileHexDump(hDlg, InputFile, OutputFile, xsize, SkipBytes);
            
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitHexDumpDlg", L"BinaryInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitHexDumpDlg", L"TextOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"BitHexDumpDlg", L"xsize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_SKIP_BYTES, szString, MAX_PATH);
            WritePrivateProfileString(L"BitHexDumpDlg", L"SkipBytes", szString, (LPCTSTR)strAppNameINI);

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
// Message handler for BitTextStream dialog box.
// 
// This converts a binary bitstream file (like data17.bin)
// to a CSV style file, also separating header(if not 0), block header(if not 0),
// blocks (xsize per row) for as many blocks specified and the cooler
// 
//*******************************************************************************
INT_PTR CALLBACK BitTextStreamDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
        GetPrivateProfileString(L"BitTextStreamDlg", L"BinaryInput", L"OriginalSource\\data17.bin", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);

        GetPrivateProfileString(L"BitTextStreamDlg", L"TextOutput", L"data17.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);

        GetPrivateProfileString(L"BitTextStreamDlg", L"PrologueSize", L"80", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString);

        GetPrivateProfileString(L"BitTextStreamDlg", L"BlockHeaderBits", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_HEADER_BITS, szString);

        GetPrivateProfileString(L"BitTextStreamDlg", L"BlockBits", L"65536", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_BITS, szString);

        GetPrivateProfileString(L"BitTextStreamDlg", L"xsize", L"256", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE, szString);

        GetPrivateProfileString(L"BitTextStreamDlg", L"BlockNum", L"1", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_NUM, szString);
        
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC bitType[] =
            {
                 { L"bit stream files", L"*.bin" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, bitType, L".bin")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);
            return (INT_PTR)TRUE;
        }
        case IDC_TEXT_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text stream files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_CONVERT:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int PrologueSize;
            int xsize;
            int BlockNum;
            int NumBlockBodyBits;
            int BlockHeaderBits;

            GetDlgItemText(hDlg, IDC_BINARY_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, OutputFile, MAX_PATH);

            PrologueSize = GetDlgItemInt(hDlg, IDC_PROLOGUE_SIZE, &bSuccess, TRUE);

            BlockHeaderBits = GetDlgItemInt(hDlg, IDC_BLOCK_HEADER_BITS, &bSuccess, TRUE);

            NumBlockBodyBits = GetDlgItemInt(hDlg, IDC_BLOCK_BITS, &bSuccess, TRUE);

            xsize = GetDlgItemInt(hDlg, IDC_XSIZE, &bSuccess, TRUE);

            BlockNum = GetDlgItemInt(hDlg, IDC_BLOCK_NUM, &bSuccess, TRUE);

            ExtractFromBitStreamText(hDlg, InputFile, OutputFile,
                PrologueSize, BlockHeaderBits, NumBlockBodyBits, BlockNum, xsize);

            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitTextStreamDlg", L"BinaryInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitTextStreamDlg", L"TextOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"BitTextStreamDlg", L"PrologueSize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_HEADER_BITS, szString, MAX_PATH);
            WritePrivateProfileString(L"BitTextStreamDlg", L"BlockHeaderBits", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_BITS, szString, MAX_PATH);
            WritePrivateProfileString(L"BitTextStreamDlg", L"BlockBits", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"BitTextStreamDlg", L"xsize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_NUM, szString, MAX_PATH);
            WritePrivateProfileString(L"BitTextStreamDlg", L"BlockNum", szString,(LPCTSTR)strAppNameINI);

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
// Message handler for BitExtract dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK BitExtractDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
        GetPrivateProfileString(L"BitExtractDlg", L"BinaryInput", L"OriginalSource\\data17.bin", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);

        GetPrivateProfileString(L"BitExtractDlg", L"TextOutput", L"extracted.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);

        GetPrivateProfileString(L"BitExtractDlg", L"SkipBits", L"80", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_SKIP_BITS, szString);

        GetPrivateProfileString(L"BitExtractDlg", L"CopyBits", L"65536", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_COPY_BITS, szString);

        GetPrivateProfileString(L"BitExtractDlg", L"xsize", L"256", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE, szString);

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INPUT_BROWSE:
        {
            PWSTR pszFilename;

            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC bitType[] =
            {
                 { L"bit stream files", L"*.bin" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, bitType, L".bin")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);
            return (INT_PTR)TRUE;
        }
        case IDC_TEXT_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text stream files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_EXTRACT:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int xsize;
            int CopyBits;
            int SkipBits;

            GetDlgItemText(hDlg, IDC_BINARY_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, OutputFile, MAX_PATH);

            CopyBits = GetDlgItemInt(hDlg, IDC_COPY_BITS, &bSuccess, TRUE);
            SkipBits = GetDlgItemInt(hDlg, IDC_SKIP_BITS, &bSuccess, TRUE);
            xsize = GetDlgItemInt(hDlg, IDC_XSIZE, &bSuccess, TRUE);

            ExtractBits(hDlg, InputFile, OutputFile, SkipBits, CopyBits, xsize);

            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitExtractDlg", L"BinaryInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitExtractDlg", L"TextOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_SKIP_BITS, szString, MAX_PATH);
            WritePrivateProfileString(L"BitExtractDlg", L"SkipBits", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_COPY_BITS, szString, MAX_PATH);
            WritePrivateProfileString(L"BitExtractDlg", L"CopyBits", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"BitExtractDlg", L"xsize", szString, (LPCTSTR)strAppNameINI);

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
// Message handler for BitDistancesDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK BitDistancesDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
        GetPrivateProfileString(L"BitDistancesDlg", L"BinaryInput", L"OriginalSource\\data17.bin", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);

        GetPrivateProfileString(L"BitDistancesDlg", L"TextOutput", L"data17-bit-distances.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);

        GetPrivateProfileString(L"BitDistancesDlg", L"PrologueSize", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString);

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC bitType[] =
            {
                 { L"bit stream files", L"*.bin" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, bitType, L".bin")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);
            return (INT_PTR)TRUE;
        }
        case IDC_TEXT_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text stream files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_REPORT:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int PrologueSize;

            GetDlgItemText(hDlg, IDC_BINARY_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, OutputFile, MAX_PATH);
            PrologueSize = GetDlgItemInt(hDlg, IDC_PROLOGUE_SIZE, &bSuccess, TRUE);
            BitDistance(hDlg, InputFile, OutputFile, PrologueSize);

            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitDistancesDlg", L"BinaryInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitDistancesDlg", L"TextOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"BitDistancesDlg", L"PrologueSize", szString, (LPCTSTR)strAppNameINI);

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
// Message handler for BitSequencesDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK BitSequencesDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
        GetPrivateProfileString(L"BitSequencesDlg", L"BinaryInput", L"OriginalSource\\data17.bin", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);

        GetPrivateProfileString(L"BitSequencesDlg", L"TextOutput", L"data17-bit-sequences.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);

        GetPrivateProfileString(L"BitSequencesDlg", L"PrologueSize", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString);

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC bitType[] =
            {
                 { L"bit stream files", L"*.bin" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, bitType, L".bin")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);
            return (INT_PTR)TRUE;
        }
        case IDC_TEXT_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text stream files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_REPORT:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int PrologueSize;

            GetDlgItemText(hDlg, IDC_BINARY_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, OutputFile, MAX_PATH);
            PrologueSize = GetDlgItemInt(hDlg, IDC_PROLOGUE_SIZE, &bSuccess, TRUE);
            BitSequences(hDlg, InputFile, OutputFile, PrologueSize);

            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitSequencesDlg", L"BinaryInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitSequencesDlg", L"TextOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"BitSequencesDlg", L"PrologueSize", szString, (LPCTSTR)strAppNameINI);

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
// Message handler for BitStatsDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK BitStatsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
        GetPrivateProfileString(L"BitStatsDlg", L"BinaryInput", L"OriginalSource\\data17.bin", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);

        GetPrivateProfileString(L"BitStatsDlg", L"TextOutput", L"data17-stats.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);

        GetPrivateProfileString(L"BitStatsDlg", L"PrologueSize", L"80", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString);

        GetPrivateProfileString(L"BitStatsDlg", L"BlockHeaderBits", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_HEADER_BITS, szString);

        GetPrivateProfileString(L"BitStatsDlg", L"BlockBits", L"65536", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_BITS, szString);

        GetPrivateProfileString(L"BitStatsDlg", L"BlockNum", L"1", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_NUM, szString);

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC bitType[] =
            {
                 { L"bit stream files", L"*.bin" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, bitType, L".bin")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);
            return (INT_PTR)TRUE;
        }
        case IDC_TEXT_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text stream files", L"*.txt" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, textType, L"*.txt")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }

        case IDC_REPORT:
        {
            BOOL bSuccess;
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];
            int PrologueSize;
            int BlockNum;
            int NumBlockBodyBits;
            int BlockHeaderBits;

            GetDlgItemText(hDlg, IDC_BINARY_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, OutputFile, MAX_PATH);

            PrologueSize = GetDlgItemInt(hDlg, IDC_PROLOGUE_SIZE, &bSuccess, TRUE);

            BlockHeaderBits = GetDlgItemInt(hDlg, IDC_BLOCK_HEADER_BITS, &bSuccess, TRUE);

            NumBlockBodyBits = GetDlgItemInt(hDlg, IDC_BLOCK_BITS, &bSuccess, TRUE);

            BlockNum = GetDlgItemInt(hDlg, IDC_BLOCK_NUM, &bSuccess, TRUE);

            BitStreamStats(hDlg, InputFile, OutputFile,
                PrologueSize, BlockHeaderBits, NumBlockBodyBits, BlockNum);

            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitStatsDlg", L"BinaryInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitStatsDlg", L"TextOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"BitStatsDlg", L"PrologueSize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_HEADER_BITS, szString, MAX_PATH);
            WritePrivateProfileString(L"BitStatsDlg", L"BlockHeaderBits", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_BITS, szString, MAX_PATH);
            WritePrivateProfileString(L"BitStatsDlg", L"BlockBits", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_NUM, szString, MAX_PATH);
            WritePrivateProfileString(L"BitStatsDlg", L"BlockNum", szString, (LPCTSTR)strAppNameINI);

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
// Message handler for BitReorderDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK BitReorderDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        int ScalePixel;
        IMAGINGHEADER ImageHeader;

        GetPrivateProfileString(L"BitReorderDlg", L"ImageInput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
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

        GetPrivateProfileString(L"BitReorderDlg", L"TextInput", L"Reorder\\reorder.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);

        GetPrivateProfileString(L"BitReorderDlg", L"ImageOutput", L"Reordered.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        ScalePixel = GetPrivateProfileInt(L"BitReorderDlg", L"ScalePixel", 0, (LPCTSTR)strAppNameINI);
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
                 { L"image files", L"*.raw" },
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
                if (ImageHeader.Ysize != 1) {
                    MessageBox(hDlg, L"Input image file must be linear image (Ysize=1)", L"File incompatible", MB_OK);
                }
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

            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, textType, L".txt")) {
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
                 { L"image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L".raw")) {
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

            PixelReorder(hDlg, TextInput, InputFile, OutputFile, ScalePixel, TRUE, FALSE, FALSE);

            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitReorderDlg", L"ImageInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitReorderDlg", L"TextInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitReorderDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                WritePrivateProfileString(L"BitReorderDlg", L"ScalePixel", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"BitReorderDlg", L"ScalePixel", L"0", (LPCTSTR)strAppNameINI);
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
// Message handler for BitImageDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK BitImageDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        int BitOrder;
        int BitScale;

        GetPrivateProfileString(L"BitImageDlg", L"BinaryInput", L"OriginalSource\\data17.bin", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);

        GetPrivateProfileString(L"BitImageDlg", L"ImageOutput", L"Message.raw", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString);

        GetPrivateProfileString(L"BitImageDlg", L"PrologueSize", L"80", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString);

        GetPrivateProfileString(L"BitImageDlg", L"BlockHeaderBits", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_HEADER_BITS, szString);

        GetPrivateProfileString(L"BitImageDlg", L"BlockBits", L"65536", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_BITS, szString);

        GetPrivateProfileString(L"BitImageDlg", L"xsize", L"256", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE, szString);

        GetPrivateProfileString(L"BitImageDlg", L"xsizeEnd", L"0", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_XSIZE_END, szString);

        GetPrivateProfileString(L"BitImageDlg", L"BlockNum", L"1", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BLOCK_NUM, szString);

        GetPrivateProfileString(L"BitImageDlg", L"BitDepth", L"1", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BIT_DEPTH, szString);

        BitOrder = GetPrivateProfileInt(L"BitImageDlg", L"BitOrder", 0, (LPCTSTR)strAppNameINI);
        if (!BitOrder) {
            CheckDlgButton(hDlg, IDC_BITORDER, BST_UNCHECKED);
        } else {
            CheckDlgButton(hDlg, IDC_BITORDER, BST_CHECKED);
        }

        BitScale = GetPrivateProfileInt(L"BitImageDlg", L"BitScale", 0, (LPCTSTR)strAppNameINI);
        if (!BitScale) {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_UNCHECKED);
        }
        else {
            CheckDlgButton(hDlg, IDC_SCALE_PIXEL, BST_CHECKED);
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
                 { L"bit stream files", L"*.bin" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, bitType, L"*.bin")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BINARY_INPUT, szString);
            return (INT_PTR)TRUE;
        }
        case IDC_IMAGE_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC rawType[] =
            {
                 { L"image files", L"*.raw" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, rawType, L".raw")) {
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
            int PrologueSize;
            int xsize;
            int xsizeEnd;
            int BlockNum;
            int NumBlockBodyBits;
            int BlockHeaderBits;
            int BitDepth;
            int BitOrder = 0;
            int BitScale = 0;

            GetDlgItemText(hDlg, IDC_BINARY_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, OutputFile, MAX_PATH);

            PrologueSize = GetDlgItemInt(hDlg, IDC_PROLOGUE_SIZE, &bSuccess, TRUE);

            BlockHeaderBits = GetDlgItemInt(hDlg, IDC_BLOCK_HEADER_BITS, &bSuccess, TRUE);

            NumBlockBodyBits = GetDlgItemInt(hDlg, IDC_BLOCK_BITS, &bSuccess, TRUE);

            xsize = GetDlgItemInt(hDlg, IDC_XSIZE, &bSuccess, TRUE);
            xsizeEnd = GetDlgItemInt(hDlg, IDC_XSIZE_END, &bSuccess, TRUE);

            BlockNum = GetDlgItemInt(hDlg, IDC_BLOCK_NUM, &bSuccess, TRUE);

            BitDepth = GetDlgItemInt(hDlg, IDC_BIT_DEPTH, &bSuccess, TRUE);

            if (IsDlgButtonChecked(hDlg, IDC_BITORDER) == BST_CHECKED) {
                BitOrder = 1;
            }

            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                BitScale = 1;
            }

            if (xsize >= xsizeEnd) {
                BitStream2Image(hDlg, InputFile, OutputFile,
                    PrologueSize, BlockHeaderBits, NumBlockBodyBits, BlockNum, xsize,
                    BitDepth, BitOrder, BitScale);

                wcscpy_s(szCurrentFilename, OutputFile);
            }
            else {
                BatchBitStream2Image(hDlg, InputFile, OutputFile,
                    PrologueSize, BlockHeaderBits, NumBlockBodyBits, BlockNum, xsize, xsizeEnd,
                    BitDepth, BitOrder, BitScale);
            }
            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_BINARY_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"BinaryInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_IMAGE_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"ImageOutput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_PROLOGUE_SIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"PrologueSize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_HEADER_BITS, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"BlockHeaderBits", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_BITS, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"BlockBits", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XSIZE, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"xsize", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_XSIZE_END, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"xsizeEnd", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BLOCK_NUM, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"BlockNum", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BIT_DEPTH, szString, MAX_PATH);
            WritePrivateProfileString(L"BitImageDlg", L"BitDepth", szString, (LPCTSTR)strAppNameINI);

            if (IsDlgButtonChecked(hDlg, IDC_BITORDER) == BST_CHECKED) {
                WritePrivateProfileString(L"BitImageDlg", L"BitOrder", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"BitImageDlg", L"BitOrder", L"0", (LPCTSTR)strAppNameINI);
            }

            if (IsDlgButtonChecked(hDlg, IDC_SCALE_PIXEL) == BST_CHECKED) {
                WritePrivateProfileString(L"BitImageDlg", L"BitScale", L"1", (LPCTSTR)strAppNameINI);
            }
            else {
                WritePrivateProfileString(L"BitImageDlg", L"BitScale", L"0", (LPCTSTR)strAppNameINI);
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
// Message handler for Text2StreamDlg dialog box.
// 
// This converts a text file into a binary bitstream file (like data17.bin)
// 
//*******************************************************************************
INT_PTR CALLBACK Text2StreamDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
        GetPrivateProfileString(L"Text2StreamDlg", L"TextInput", L"data17.txt", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_TEXT_INPUT, szString);

        GetPrivateProfileString(L"Text2StreamDlg", L"BinaryOutput", L"bitstream.bin", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BINARY_OUTPUT, szString);

        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_OUTPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_BINARY_OUTPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC bitType[] =
            {
                 { L"bit stream files", L"*.bin" },
                 { L"All Files", L"*.*" },
            };

            if (!CCFileSave(hDlg, szString, &pszFilename, FALSE, 2, bitType, L".bin")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BINARY_OUTPUT, szString);
            return (INT_PTR)TRUE;
        }
        case IDC_INPUT_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC textType[] =
            {
                 { L"text stream files", L"*.txt" },
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

        case IDC_CONVERT:
        {
            WCHAR InputFile[MAX_PATH];
            WCHAR OutputFile[MAX_PATH];

            GetDlgItemText(hDlg, IDC_TEXT_INPUT, InputFile, MAX_PATH);
            GetDlgItemText(hDlg, IDC_BINARY_OUTPUT, OutputFile, MAX_PATH);

            ConvertText2BitStream(hDlg, InputFile, OutputFile);

            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_TEXT_INPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"Text2StreamDlg", L"TextInput", szString, (LPCTSTR)strAppNameINI);

            GetDlgItemText(hDlg, IDC_BINARY_OUTPUT, szString, MAX_PATH);
            WritePrivateProfileString(L"Text2StreamDlg", L"BinaryOutput", szString, (LPCTSTR)strAppNameINI);

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
    }
    return (INT_PTR)FALSE;
}
