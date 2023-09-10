//
// MySETIapp, a set tools for decoding bitstreams into various formats and manipulating those files
// Settings.cpp
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
// V1.1.0.1 2023-08-22, Added file type specifications to open/save dialogs
// V1.2.2.1 2023-09-10  Changed default folders\filenames
//                      Changed default settings for app -> display last results, autoscale, RGB display for 3 frame files
//                      Removed Autosize flag from settings.
// V1.2.5.1 2023-09-09  Display .exe and .ini file locations in the settings dialog.
//                      The .ini location is always the same folder as the executable.
//
// Global Settings dialog box handler
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

//*******************************************************************************
//
// Message handler for GlobalSettingsDlg dialog box.
// 
//*******************************************************************************
INT_PTR CALLBACK GlobalSettingsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
        WCHAR szString[MAX_PATH];

    case WM_INITDIALOG:
    {
        int iRes;
        GetPrivateProfileString(L"GlobalSettings", L"BMPresults", L"BMP files\\last.bmp", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
        SetDlgItemText(hDlg, IDC_BMP_RESULTS, szString);
        
        SetDlgItemText(hDlg, IDC_INI_FILE, strAppNameINI);
        SetDlgItemText(hDlg, IDC_EXE_FILE, strAppNameEXE);

        //IDC_SETTINGS_DISPLAY_RESULTS
        iRes = GetPrivateProfileInt(L"GlobalSettings", L"DisplayResults", 1, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_DISPLAY_RESULTS, BST_CHECKED);
        }

        // IDC_SETTINGS_AUTOSCALE_RESULTS
        iRes = GetPrivateProfileInt(L"GlobalSettings", L"AutoScaleResults", 1, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_AUTOSCALE_RESULTS, BST_CHECKED);
        }

        // IDC_SETTINGS_RGB_DISPLAY
        iRes = GetPrivateProfileInt(L"GlobalSettings", L"DefaultRBG", 1, (LPCTSTR)strAppNameINI);
        if (iRes != 0) {
            CheckDlgButton(hDlg, IDC_SETTINGS_RGB_DISPLAY, BST_CHECKED);
        }

        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BMP_RESULTS_BROWSE:
        {
            PWSTR pszFilename;
            GetDlgItemText(hDlg, IDC_IMAGE_INPUT, szString, MAX_PATH);
            COMDLG_FILTERSPEC BMPType[] =
            {
                 { L"BMP files", L"*.bmp" },
                 { L"All Files", L"*.*" },
            };
            if (!CCFileOpen(hDlg, szString, &pszFilename, FALSE, 2, BMPType, L".bmp")) {
                return (INT_PTR)TRUE;
            }
            {
                wcscpy_s(szString, pszFilename);
                CoTaskMemFree(pszFilename);
            }
            SetDlgItemText(hDlg, IDC_BMP_RESULTS, szString);

            return (INT_PTR)TRUE;
        }

        case IDOK:
            GetDlgItemText(hDlg, IDC_BMP_RESULTS, szString, MAX_PATH);
            wcscpy_s(szBMPFilename, szString);
            WritePrivateProfileString(L"GlobalSettings", L"BMPresults", szString, (LPCTSTR)strAppNameINI);

            //IDC_SETTINGS_DISPLAY_RESULTS
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_DISPLAY_RESULTS) == BST_CHECKED) {
                WritePrivateProfileString(L"GlobalSettings", L"DisplayResults", L"1", (LPCTSTR)strAppNameINI);
                DisplayResults = 1;
            }
            else {
                WritePrivateProfileString(L"GlobalSettings", L"DisplayResults", L"0", (LPCTSTR)strAppNameINI);
                DisplayResults = 0;
            }

            // IDC_SETTINGS_AUTOSCALE_RESULTS
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_AUTOSCALE_RESULTS) == BST_CHECKED) {
                WritePrivateProfileString(L"GlobalSettings", L"AutoScaleResults", L"1", (LPCTSTR)strAppNameINI);
                AutoScaleResults = 1;
            }
            else {
                WritePrivateProfileString(L"GlobalSettings", L"AutoScaleResults", L"0", (LPCTSTR)strAppNameINI);
                AutoScaleResults = 0;
            }

            // IDC_SETTINGS_RGB_DISPLAY
            if (IsDlgButtonChecked(hDlg, IDC_SETTINGS_RGB_DISPLAY) == BST_CHECKED) {
                WritePrivateProfileString(L"GlobalSettings", L"DefaultRBG", L"1", (LPCTSTR)strAppNameINI);
                DefaultRBG = 1;
            }
            else {
                WritePrivateProfileString(L"GlobalSettings", L"DefaultRBG", L"0", (LPCTSTR)strAppNameINI);
                DefaultRBG = 0;
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
