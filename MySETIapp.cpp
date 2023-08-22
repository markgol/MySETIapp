//
// MySETIapp, a set tools for decoding bitstreams into various formats and manipulating those files
// MySETIapp.cpp
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
// Background:
// From the 'A Sign in Space' website, https://asignin.space/:
// A Sign in Space is an interdisciplinary project by media artist Daniela de Paulis,
// in collaboration with the SETI Institute, the European Space Agency,
// the Green Bank Observatory and INAF, the Italian National Institute for Astrophysics.
// The project consists in transmitting a simulated extraterrestrial message as part
// of a live performance, using an ESA spacecraft as celestial source.The objective
// of the project is to involve the world - wide Search for Extraterrestrial
// Intelligence community, professionals from different fieldsand the broader public
// in the reception, decodingand interpretation of the message.This process will
// require global cooperation, bridging a conversation around the topics of SETI,
// space researchand society, across multiple culturesand fields of expertise.
// https://www.seti.org/event/sign-space
// 
// The message was transmitted from the ESA's ExoMars Trace Gas Orbiter (TGO)
//   on May 24 at 19:16 UTC/12:15 pm PDT.
// 
// It was received by three radio telescopes on earth May 24,2023.
// A group of individuals in the Discord 'A Sign in Space' channel
// unscrambled the message from the radio telemetry.
// The message published as Data17.bin is the correctly transcribed
// bitstream of the message payload given to ESA.
// 
// The next step in the problem is the decoding of the payload bitstream into
// the next level of the message.
// 
// The types of programs that the Discord group is using is quite varied like:
// Excel, Photoshop, GIMP, Java, Python and c/c++.  A number of
// poeple are also using online tools that use file uploads, typically text based.
// Excel uses text files for import like CSV.
// Photoshop and GIMP can use raw binary files typically 8 bit, 16 bit or 32 bit per element.
// None of these use bit packed binary input. So the first step is to translate the
// bitstream message into a format that can be used by the various tools. The next
// steps involve examination of the data to solve how to decode it.
// 
// 
// The programs is a set of tools to help in this process.
// 
// These covers many of the requests people have had in the Discord group
//
// V1.0.0.1 2023-08-20, Initial Release
// V1.1.0.1 2023-08-22, Added file type specifications to open/save dialogs
//                      Added Image Decimation
//                      Added Resize image file
//                      Interim display solution using external viewer
//
// MySETIapp.cpp : Defines the entry point for the application.
//
//  This appliction stores user parameters in a Windows style .ini file
//  The MySETIapp.ini file must be in the same directory as the exectable
//
//  A lot of this application code is really to support the framework
//  of the application for the user interface.
// 
// New dialogs for the image tools menu should be added to this module.
// The functions that actually perform the actions should be put in
// imaging.cpp and imaging.h
//
// New dialogs for the bit tools menu should be added to this module.
// The functions that actually perform the actions should be put in
// bitstream.cpp and bitstream.h
//

#include "framework.h"
#include <atlstr.h>
#include <strsafe.h>
#include "atlpath.h"
#include "string.h"
#include "MySETIapp.h"
#include "FileFunctions.h"
#include "imaging.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

WCHAR szBMPFilename[MAX_PATH] = L"";    // used to save last results file
WCHAR szCurrentFilename[MAX_PATH] = L"";    // used to save last results file
int DisplayResults = 0;
int AutoScaleResults = 0;
int DefaultRBG = 0;
int AutoSize = 0;

// The following is from the version information in the resource file
CString strProductName;
CString strProductVersion;
CString strName;
CString strCopyright;
CString strAppNameEXE;
CString strAppNameINI;

// handles for modeless dialogs and windows
HWND hwndImageDisplay = NULL;  // handle for modeless Image window

// handle to the main application window
//  needed to do things like check or uncheck a menu item in the main app
HWND hwndMain = NULL;
HWND hwndImage = NULL;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// Declaration for callback dialog procedures in other modules

INT_PTR CALLBACK    AboutDlg(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    BitTextStreamDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    BitHexDumpDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    BitExtractDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    BitDistancesDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    BitSequencesDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    BitStatsDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    BitReorderDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    BitImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ExtractImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AppendEndImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AppendRightImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ReorderImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    FoldLeftImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    FoldRightImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    FoldUpImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    FoldDownImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    LeftAccordionImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    RightAccordionImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    LeftShiftRowsImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ConvolutionImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AddImagesImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    RotateDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MirrorDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DecimationDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ResizeDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    GlobalSettingsDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ImageDlg(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    // HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    // 
    // Get version information, executable name including path to executable location
    if (!GetProductAndVersion(&strProductName,
        &strProductVersion,
        &strName,
        &strCopyright,
        &strAppNameEXE))
        return FALSE;

    // create INI file name for application settings
    // The INI file has the same name as the executable.
    // It is located in the same directory as the exectable
    CPath path(strAppNameEXE);
    path.RenameExtension(_T(".ini"));
    strAppNameINI.SetString(path);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MYSETIAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MYSETIAPP));

    MSG msg;
    BOOL bRet;

    // Main message loop:
    while ((bRet = GetMessage(&msg, nullptr, 0, 0))!=0)
    {
        if (bRet == -1) {
            // error handling if required
        }
        else {
            // Modeless dialog need to process messages intended for the modeless dialog only.
            // Check that message is not for each modeless dialog box.
            //
            if (IsWindow(hwndImage) && IsDialogMessage(hwndImage, &msg)) {
                continue;
            }
            // // Modeless dialog B
            //if (IsWindow(hwndBDlg) && IsDialogMessage(hwndBtDlg, &msg)) {
            //    continue;
            //}

            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYSETIAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MYSETIAPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MYSETIAPP));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // restore main window position from last execution
   CString csString = L"MainWindow";
   RestoreWindowPlacement(hWnd, csString);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // load globals
   hwndMain = hWnd;
   WCHAR szString[MAX_PATH];
   GetPrivateProfileString(L"GlobalSettings", L"BMPresults", szBMPFilename, szString, MAX_PATH, (LPCTSTR)strAppNameINI);
   wcscpy_s(szBMPFilename, szString);

   DisplayResults = GetPrivateProfileInt(L"GlobalSettings", L"DisplayResults", 0, (LPCTSTR)strAppNameINI);
   AutoScaleResults = GetPrivateProfileInt(L"GlobalSettings", L"AutoScaleResults", 0, (LPCTSTR)strAppNameINI);
   DefaultRBG = GetPrivateProfileInt(L"GlobalSettings", L"DefaultRBG", 0, (LPCTSTR)strAppNameINI);
   AutoSize = GetPrivateProfileInt(L"GlobalSettings", L"AutoSize", 0, (LPCTSTR)strAppNameINI);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDC_FILE_OPEN:
            {
                PWSTR pszFilename;
                COMDLG_FILTERSPEC AllType[] =
                {
                     { L"Image files", L"*.raw" },
                     { L"BMP files", L"*.bmp" },
                     { L"All Files", L"*.*" },
                };

                if (!CCFileOpen(hWnd, szCurrentFilename, &pszFilename, FALSE, 3, AllType, L".raw")) {
                    return (INT_PTR)TRUE;
                }
                {
                    wcscpy_s(szCurrentFilename, pszFilename);
                    CoTaskMemFree(pszFilename);

                    if (!IsWindow(hwndImage)) {
                        hwndImage = CreateDialog(hInst, MAKEINTRESOURCE(IDD_IMAGE), hWnd, ImageDlg);
                    }
                    int iRes;

                    iRes = DisplayImage(szCurrentFilename);
                    if (iRes != 1) {
                        MessageBox(hWnd, L"Error opening image/BMP for display\nMake sure BMP file set inProperties->Settings\n", L"File Incompatible", MB_OK);
                    }
                    else {
                        ShowWindow(hwndImage, SW_SHOW);
                    }
                }

                break;
            }

            case IDM_FILE_TOBMP:
            case IDM_FILE_TOTXT:
                ExportFile(hWnd, wmId);
                break;

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDlg);
                break;

            // Bit tools menu
            case IDM_BITTOOLS_TEXT_STREAM:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_TEXT_STREAM), hWnd, BitTextStreamDlg);
                break;

            case IDM_BITTOOLS_HEXDUMP:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_HEX_DUMP), hWnd, BitHexDumpDlg);
                break;

            case IDM_BITTOOLS_EXTRACT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_EXTRACT), hWnd, BitExtractDlg);
                break;

            case IDM_BITTOOLS_BITDISTANCES:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_DISTANCES), hWnd, BitDistancesDlg);
                break;

            case IDM_BITTOOLS_BITSEQUENCES:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_BITSEQUENCES), hWnd, BitSequencesDlg);
                break;

            case IDM_BITTOOLS_BITSTATS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_BITSTATS), hWnd, BitStatsDlg);
                break;

            case IDM_BITTOOLS_BITREORDER:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_REORDER), hWnd, BitReorderDlg);
                break;

            case IDM_BITTOOLS_BINARYIMAGE:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_BINARYIMAGE), hWnd, BitImageDlg);
                break;

            // Image tools menu
            case IDM_IMGTOOLS_PROPERTIES:
            {
                PWSTR pszFilename;
                COMDLG_FILTERSPEC ImageType[] =
                {
                     { L"Image files", L"*.raw" },
                     { L"All Files", L"*.*" },
                };
                if (!CCFileOpen(hWnd, szCurrentFilename, &pszFilename, FALSE, 2, ImageType, L".raw")) {
                    return (INT_PTR)TRUE;
                }
                {
                    wcscpy_s(szCurrentFilename, pszFilename);
                    CoTaskMemFree(pszFilename);

                    ReportImageHeader(hWnd, szCurrentFilename);
                }
                break;
            }

            case IDM_IMGTOOLS_EXTRACT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_EXTRACT), hWnd, ExtractImageDlg);
                break;

            case IDM_IMGTOOLS_APPEND_END:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_APPEND_END), hWnd, AppendEndImageDlg);
                break;

            case IDM_IMGTOOLS_APPEND_RIGHT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_APPEND_RIGHT), hWnd, AppendRightImageDlg);
                break;

            case IDM_IMGTOOLS_REORDER:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_REORDER), hWnd, ReorderImageDlg);
                break;

            case IDM_FOLDING_FOLDLEFT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_FOLDLEFT), hWnd, FoldLeftImageDlg);
                break;

            case IDM_FOLDING_FOLDRIGHT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_FOLDRIGHT), hWnd, FoldRightImageDlg);
                break;

            case IDM_FOLDING_FOLDUP:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_FOLDUP), hWnd, FoldUpImageDlg);
                break;

            case IDM_FOLDING_FOLDDOWN:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_FOLDDOWN), hWnd, FoldDownImageDlg);
                break;

            case IDM_FOLDING_LEFTACCORDION:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_LEFTACCORDION), hWnd, LeftAccordionImageDlg);
                break;

            case IDM_FOLDING_RIGHTACCORDION:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_RIGHTACCORDION), hWnd, RightAccordionImageDlg);
                break;

            case IDM_IMGTOOLS_LEFTSHIFTROWS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_LEFTSHIFTROWS), hWnd, LeftShiftRowsImageDlg);
                break;

            case IDM_IMGTOOLS_CONVOLUTION:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_CONVOLUTION), hWnd, ConvolutionImageDlg);
                break;

            case IDM_IMGTOOLS_ADDIMAGES:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_ADD_IMAGES), hWnd, AddImagesImageDlg);
                break;

            case IDM_IMAGETOOLS_ROTATE:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_ROTATE), hWnd, RotateDlg);
                break;

            case IDM_IMAGETOOLS_MIRROR:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_MIRROR), hWnd, MirrorDlg);
                break;

            case IDM_IMAGETOOLS_DECIMATION:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_DECIMATION), hWnd, DecimationDlg);
                break;

            case IDM_IMAGETOOLS_RESIZE:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_RESIZE), hWnd, ResizeDlg);
                break;

            case IDM_SETTINGS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, GlobalSettingsDlg);
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        {   // save window position/size data
            CString csString = L"MainWindow";
            SaveWindowPlacement(hWnd, csString);
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
