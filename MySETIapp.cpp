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
// V1.2.0.1 2023-08-31  Added Convert text to packed bitstream file
//                      Added image stats reporting not just image header stats
//                      Corrected error handling of reorder list, when entry in
//                      kernel is out of bounds, file closure on error.
//                      Use of Windows default viewer for BMP display works adequately.
//                      Clean up of ImageDlg to just rely on external viewer.
//                      Added batch processing for reordering,  This allows a series of
//                      reorder kernels to be used.  Each kernel adds an index number
//                      onto the output filename.
// V1.2.1.1 2023-09-06  Added import BMP to image file (.raw). Must be 1 bit, 8 bit, or 24 bit
//                      Added HEX text file to binary file
//                      Corrected ImageDlg to display results file only once.
// V1.2.2.1 2023-09-06  Path must exist check is done on selected files in dialogs.
//                      Changed default folders\filenames
//                      Changed default settings for app -> display last results, autoscale, RGB display for 3 frame files
//                      Removed Autosize flag from settings.
//                      Corrected bug introduced in V1.2.1 where incorrect frame size
//                      was used in the ImportBMP function.
// V1.2.4.1 2023-09-09  Added Add/Subtract a constant from entire image
//                      Changed Sum images to Add/Subtract images
//                      Added import of CamIRa IMG files (not applicable to A Sign in Space project)
//                      Cleanup of file open/save dialogs
// V1.2.5.1 2023-09-09  Changed handling of default directories
//                      Aded standard image decimation (summation of pixels)
//                      Corected tab order and default buttons in all dialogs
//                      Display .exe and .ini file locations in the settings dialog.
//                      The .ini location is always the same folder as the executable.
// V1.2.6.1 2023-09-24  Correction, Display Image/BMP file was not displaying BMP file
//                      Changed, display of 16, 32 bit image data, using scaled BMP file.
//                      The BMP file is still only a 8bpp file. (not applicable to A Sign in Space project)
//                      Added Algorithm driven reordering (onyl 5 algorithms to start)
//                      Added Increase image size by replication.
//                      Changed HEX dump, added skip bytes from start of file
//                      Added extract block symbols from image file
//                      (save as 2D image file, right padded with null symbols)
//                      Added temp image filename to app settings.  For use in debugging
//                      functions by saving intermediate results.
// V1.2.7.1 2023-10-01  Added invert of input bits in bitstream to the bitstream to text and
//                      bistream to image operations
//                      Added insert/add image into an existing image, to be able to recreate
//                      'A Sign in Space' starmap from the 5 sign images or to make up one's
//                      own message.
//                      Correction, 2d symbol extraction requires image y size to be
//						divisible by y symbol size.
//						Changed, block symbol extraction to allow highlighting phrases
// V1.2.8.1 2023-10-15  Added input2 image file information to dialog for Append end image operation
//                      Changed, Append image end, no longer requires Ysize to be the same 
//                      unless the frames are being added to the end of the first input image.
//                      Added Space protocol Packet extraction from a TM SPP stream file.
//                      Changed bit sequence report to also include 0s as a sequence.
//                      Changed Add/subtract constant to be add/subract/multiply/divide constant operation
//                      Added, Export Image to bitstream file
//                      Changed,  Add filesize to most bit stream operation dialogs
//                      Added 4 new algorithms to reorder using algorithm
//						    Incremental row shoft with wrap aorund
//						    n Stripes
//						    Shift (rotate) Rows
//						    Shift (rotate) Columns
//                      Correction, 16 bit image pixels were being treated as signed instead of unsigned
//                          in certain circumstances.
//                      Correction,  reshource.h files to stop ID value are not duplicated.  Duplication
//                          of ID numbers is somehting the resource editor automatically does and requires
//                          occsional cleanup to keep upwanted GUI actions and oompilation errors from occuring
//                          This included checking that a resource ID does not get assigned 65535, this is reserved
//                          for IDC_STATIC as -1 (ID #is 16 bits) 
// V1.2.9.1 2023-10-31  Added, remove NULL bytes from bitstream file
//                      Changed, added histogram of byte values in bitstream file stats report
//                      Changed, Reordering by algorithm, added MxN Block [P1,P2] output decom
//						Changed, Reordering by algorithm, Added 3rd parameter (P3) (for future use)
//                      Changed, Reordering by algorithm, Added Invert algorithm option.
//                      Correction, bug in extract image operation cortrected when extracted image
//					            was larger in the vertical direction than the source image.
//						Added,  batch input file list for reordering operation. Batch file has 
//                              both input filename and output filename
// V1.2.10.1 2023-11-2  Added, new 'Add kernel to image' dialog on a kernel based grid
//                      Changed, Text to bitstream file, added output byte Bit Order flag
//                      Changed, Bistream to image file, Added input file byte Bit Order flag
//                      Changed, Bistream to image file, Changed dialog to clarify which
//                        bit order flag applies to input file and which applies ot output file.
//                      Changed, Reordering by algorithm, added split image left/right
//                      Changed, global Setting, added auto save PNG flag when creating a BMP file  
//                      Changed, ExportBMP, added automatically saving a matching .png using a global flag
//                      Changed, all Reordering dialogs.  Added inverse transform option 
// V1.2.11.1 2023-11-7  Added, new dialog, Reorder blocks[MxM] in image
// V1.2.12.1 2023-11-20 Added prime number calculator
//                      Correction, fixed extract image when extracting multiframe files.
//                      Added, Batch extract image
// V1.2.12.2 2023-11-21 Correction, filename buffer overwrite corrected when generating batchfilelist.txt            
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
WCHAR szTempImageFilename[MAX_PATH] = L"";    // used as a temporary image file for processing
int DisplayResults = 0;
int AutoScaleResults = 0;
int DefaultRBG = 0;
int AutoSize = 0;
int AutoPNG = 0;

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
void                MessageMySETIappError(HWND hWnd, int ErrNo, const wchar_t* Title);

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
INT_PTR CALLBACK    ExtractSymbolsDlg(HWND, UINT, WPARAM, LPARAM);
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
INT_PTR CALLBACK    Text2StreamDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    MathConstantDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    StdDecimationDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ReplicationDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ReorderAlgDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    InsertImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ExtractSPPDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Image2StreamDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    RemoveNULLsDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ReorderImageBatchDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AddKernelDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ReorderBlocksImageDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    FindAPrimeDlg(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    BatchExtractImageDlg(HWND, UINT, WPARAM, LPARAM);

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
   GetPrivateProfileString(L"GlobalSettings", L"BMPresults", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
   wcscpy_s(szBMPFilename, szString);

   GetPrivateProfileString(L"GlobalSettings", L"TempImageFilename", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
   wcscpy_s(szTempImageFilename, szString);

   GetPrivateProfileString(L"GlobalSettings", L"CurrentFIlename", L"", szString, MAX_PATH, (LPCTSTR)strAppNameINI);
   wcscpy_s(szCurrentFilename, szString);

   DisplayResults = GetPrivateProfileInt(L"GlobalSettings", L"DisplayResults", 1, (LPCTSTR)strAppNameINI);
   AutoScaleResults = GetPrivateProfileInt(L"GlobalSettings", L"AutoScaleResults", 1, (LPCTSTR)strAppNameINI);
   DefaultRBG = GetPrivateProfileInt(L"GlobalSettings", L"DefaultRBG", 1, (LPCTSTR)strAppNameINI);
   AutoSize = GetPrivateProfileInt(L"GlobalSettings", L"AutoSize", 0, (LPCTSTR)strAppNameINI);
   AutoPNG = GetPrivateProfileInt(L"GlobalSettings", L"AutoPNG", 1, (LPCTSTR)strAppNameINI);

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

            case IDM_IMPORT_BMP:
            {
                int iRes;
                iRes = ImportBMP(hWnd);
                if (iRes != 1) {
                    MessageMySETIappError(hWnd, iRes,L"BMP file import");
                }
                break;
            }

            case IDM_FILE_HEX2BINARY:
                int iRes;
                iRes = HEX2Binary(hWnd);
                if (iRes != 1) {
                    MessageMySETIappError(hWnd, iRes, L"Convert HEX to Binary file");
                }
                break;

            case IDM_FILE_TOBMP:
            case IDM_FILE_TOTXT:
                ExportFile(hWnd, wmId);
                break;

            case IDM_IMPORT_CAMIRA:
            {
                int iRes;
                iRes = CamIRaImport(hWnd);
                if (iRes != 1) {
                    MessageMySETIappError(hWnd, iRes, L"CamIRa IMG file import");
                }
                break;
            }

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDlg);
                break;

            // Bit tools menu
            case IDM_BITTOOLS_REMOVENULLS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_REMOVENULLS), hWnd, RemoveNULLsDlg);
                break;

            case IDM_BITTOOLS_TEXT_STREAM:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_TEXT_STREAM), hWnd, BitTextStreamDlg);
                break;

            case IDM_BITTOOLS_HEXDUMP:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_HEX_DUMP), hWnd, BitHexDumpDlg);
                break;

            case IDM_BITTOOLS_EXTRACT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_EXTRACT), hWnd, BitExtractDlg);
                break;

            case IDM_BITTOOLS_TEXT2BITSTREAM:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_TEXT2STREAM), hWnd, Text2StreamDlg);
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

            case IDM_BITTOOLS_SPP_EXTRACT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_BITTOOLS_SPP_EXTRACT), hWnd, ExtractSPPDlg);
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

                    ReportImageProperties(hWnd, szCurrentFilename);
                }
                break;
            }

            case IDM_IMAGETOOLS_EXPORTBITSTREAM:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_IMAGE2STREAM), hWnd, Image2StreamDlg);
                break;

            case IDM_IMGTOOLS_EXTRACT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_EXTRACT), hWnd, ExtractImageDlg);
                break;

            case IDM_IMGTOOLS_EXTRACTBATCH:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_EXTRACTBATCH), hWnd, BatchExtractImageDlg);
                break;

            case IDM_IMGTOOLS_EXTRACT_PACKETS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_EXTRACT_SYMBOLS), hWnd, ExtractSymbolsDlg);
                break;

            case IDM_IMGTOOLS_APPEND_END:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_APPEND_END), hWnd, AppendEndImageDlg);
                break;

            case IDM_IMGTOOLS_APPEND_RIGHT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_APPEND_RIGHT), hWnd, AppendRightImageDlg);
                break;

            case IDM_IMGTOOLS_INSERT_IMAGE:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_INSERT_IMAGE), hWnd, InsertImageDlg);
                break;

            case IDM_IMGTOOLS_REORDER:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_REORDER), hWnd, ReorderImageDlg);
                break;

            case IDM_IMGTOOLS_REORDER_BATCH:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_REORDER_BATCH), hWnd, ReorderImageBatchDlg);
                break;

            case IDM_IMGTOOLS_REORDER_BLOCKS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_REORDER_BLOCKS), hWnd, ReorderBlocksImageDlg);
                break;

            case IDM_IMAGETOOLS_REORDER_ALG:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_REORDER_ALG), hWnd, ReorderAlgDlg);
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
                
            case IDM_IMGTOOLS_ADD_KERNEL:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_ADD_KERNEL), hWnd, AddKernelDlg);
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

            case IDM_IMAGETOOLS_STDECIMATION:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_STDDECIMATION), hWnd, StdDecimationDlg);
                break;

            case IDM_IMAGETOOLS_RESIZE:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_RESIZE), hWnd, ResizeDlg);
                break;

            case IDM_MATH_CONSTANT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_MATH_CONSTANT), hWnd, MathConstantDlg);
                break;
                
            case IDM_IMAGETOOLS_REPLICATION:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_IMGTOOLS_REPLICATION), hWnd, ReplicationDlg);
                break;

            case IDM_SETTINGS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, GlobalSettingsDlg);
                break;

            case IDM_PROPERTIES_FINDAPRIME:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_PROPERTIES_FINDAPRIME), hWnd, FindAPrimeDlg);
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
            WritePrivateProfileString(L"GlobalSettings", L"CurrentFIlename", szCurrentFilename, (LPCTSTR)strAppNameINI);
            SaveWindowPlacement(hWnd, csString);
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//******************************************************************************
//
// FoldImageRight
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
