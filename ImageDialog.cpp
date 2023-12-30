//
// MySETIviewer, a set tools for decoding bitstreams into various formats and manipulating those files
// ImageDialog.cpp
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
// This file defines the class methods for the ImageDialog class used by ImageDlg
// 
// V1.0.1	2023-12-20	Initial release, MySETIviewer
// V1.1.1   2023-12-27  MySETIviewer
//                      Added, status bar to Image window
//                      Added, bitmap position to status bar
//                      Status bar base on example from:
//                      https://learn.microsoft.com/en-us/windows/win32/controls/create-status-bars
//                      Changed, zoom, pan behavior of bitmap
//                      Changed window resize of image display
// V1.3.1   2023-12-28  Merged into MySETIapp
// 
//
#include "framework.h"
#include <stdio.h>
#include <d2d1.h>
#include <d2d1_1.h>
#pragma comment(lib, "d2d1.lib")
#include <CommCtrl.h>
#include <math.h>
#include "AppErrors.h"
#include "ImageDialog.h"

#define MINZOOM 0.5f
#define MAXZOOM 40.0f

extern HWND hwndImage;

//*******************************************************************************
//
// 
// 
//*******************************************************************************
int ImageDialog::InitializeDirect2D(void)
{
    HRESULT hResult;
    hResult = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
    if (FAILED(hResult)) {
        return APPERR_MEMALLOC;
    }
    return APP_SUCCESS;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::ReleaseDirect2D(void)
{
    // release resources
    if (pBitmap) {
        pBitmap->Release();
        pBitmap = nullptr;
    }

    if (pRenderTarget) {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }

    // factory is last
    if (pFactory) {
        pFactory->Release();
        pFactory = nullptr;
    }
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::ReleaseBitmapRender(void)
{
    if (pBitmap) {
        pBitmap->Release();
        pBitmap = nullptr;
    }
    if (pRenderTarget) {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
BOOL ImageDialog::LoadCOLORREFimage(HWND hWnd)
{
    // delete old data first

    if (pBitmap) {
        pBitmap->Release();
        pBitmap = nullptr;
    }

    if (pRenderTarget) {
        pRenderTarget->Release();
        pRenderTarget = nullptr;
    }

    if (!ColorImage) {
        return FALSE;
    }

    // create render target
    RECT Rect;
    int xt, yt;
    GetClientRect(hWnd, &Rect);
    xt = (Rect.right - Rect.left) - 1;
    yt = (Rect.bottom - Rect.top) - 1;

    pFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(xt, yt)),
        &pRenderTarget
    );

    if (pRenderTarget) {
        // bitmap properties set to RGBA 8 bit ignore alpha,96 DPI (Windows default DPI)
        HRESULT hRes = pRenderTarget->CreateBitmap(D2D1::SizeU(DisplayXsize, DisplayYsize), ColorImage,
                                        DisplayXsize * sizeof(COLORREF),
                                        bitmapProperties, &pBitmap);
        if (FAILED(hRes)) {
            if (pBitmap) {
                pBitmap->Release();
                pBitmap = nullptr;
            }
            BitmapSize = { 0.0f, 0.0f };
            return FALSE;
        }

        if (pBitmap) {
            BitmapSize = { (float)DisplayXsize, (float)DisplayYsize };
            return TRUE;
        }
    }
    BitmapSize = { 0.0f, 0.0f };
    return FALSE;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
BOOL ImageDialog::Repaint()
{
    // only do this if target exists
    if (pRenderTarget) {
        pRenderTarget->BeginDraw();

        // set scaling and offset 
        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Scale(scaleFactor, scaleFactor,
            D2D1::Point2F(0.0f, 0.0f)));
        
        // clear background
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Bisque));
        
        //Pan bitmap in bitmap pixel steps not display pixel steps
        D2D1_SIZE_F RectSize = pBitmap->GetSize();
        int ix, iy;
        ix = (int)(panOffset.x + 0.5f);
        iy = (int)(panOffset.y + 0.5f);

        D2D1_RECT_F Rectf = D2D1::RectF((float)ix, float(iy), (float)ix + RectSize.width, float(iy) + RectSize.height);
        pRenderTarget->DrawBitmap(pBitmap, Rectf, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        
        HRESULT hr = pRenderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET) {
            // close resources so they are recreated
            ReleaseBitmapRender();
        }
        return TRUE;
    }
    return FALSE;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::Rescale(int Delta)
{
    scaleFactor += (Delta > 0) ? 0.1f : -0.1f;
    scaleFactor = max(MINZOOM , min(MAXZOOM, scaleFactor));
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::SetScale(float Scale)
{
    scaleFactor = max(MINZOOM, min(MAXZOOM, Scale));
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
float ImageDialog::GetScale()
{
    return scaleFactor;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::SetPan(float x, float y)
{
    panOffset.x = x;
    panOffset.y = y;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::GetPan(float* x, float* y)
{
    *x = panOffset.x;
    *y = panOffset.y;
}


//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::EnablePanning(BOOL Enable)
{
    isPanning = Enable;
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
BOOL ImageDialog::PanImage(HWND hwndParent, int x, int y)
{
    
    if (isPanning) {
        // scaleFactor is for the bitmap and render target

        if (x >= 0) {
            float deltaX = (float)(x - lastMousePos.x) / scaleFactor;
            panOffset.x += deltaX;
        }
        if (y >= 0) {
            float deltaY = (float)(y - lastMousePos.y) / scaleFactor;
            panOffset.y += deltaY;
        }
    }

    lastMousePos.x = x;
    lastMousePos.y = y;
 
    return isPanning;
}

//*******************************************************************************
//
// Description: 
//   Creates a status bar with one part on left hand, bottom of window
// Parameters:
//   hwndParent - parent window for the status bar.
//   idStatus - child window identifier of the status bar.
//   hinst - handle to the application instance.
// Returns:
//   The handle to the status bar.
//
//*******************************************************************************
HWND ImageDialog::CreateStatusBar(HWND hwndParent, int idStatus, HINSTANCE
    hinst)
{
    // Ensure that the common control DLL is loaded.
    InitCommonControls();

    // Create the status bar.
    hwndStatusBar = CreateWindowEx(
        0,                       // no extended styles
        STATUSCLASSNAME,         // name of status bar class
        (PCTSTR)NULL,            // no text when first created
        SBARS_SIZEGRIP |         // includes a sizing grip
        WS_CHILD | WS_VISIBLE,   // creates a visible child window
        0, 0, 0, 0,              // ignores size and position
        hwndParent,              // handle to parent window
        (HMENU) idStatus,         // child window identifier
        hinst,                   // handle to application instance
        NULL);                   // no window creation data

    if (hwndStatusBar == NULL) {
        return hwndStatusBar;
    }
    // this status bar with 1 part the full width of window
    // StatusBarRight is set to -1
    // Tell the status bar to create the window parts.
    SendMessage(hwndStatusBar, SB_SETPARTS, (WPARAM)1, (LPARAM)
        &StatusBarRight);   

    // Free the array, and return.
    return hwndStatusBar;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::DestroyStatusBar() {
    if (hwndStatusBar) {
        DestroyWindow(hwndStatusBar);
        hwndStatusBar = NULL;
    }
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
BOOL ImageDialog::StatusBarExists() {
    if (hwndStatusBar) {
        return TRUE;
    }
    return FALSE;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::ShowStatusBar(BOOL Show) {
    if (hwndStatusBar) {
        if (Show) {
            ShowWindow(hwndStatusBar, SW_SHOW);
        }
        else {
            ShowWindow(hwndStatusBar, SW_HIDE);
        }
    }
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::ResizeStatusBar(HWND ParentWindow) {
    if (hwndStatusBar) {
        RECT Rect;

        GetClientRect(ParentWindow, &Rect);
        SendMessage(hwndStatusBar, WM_SIZE, 0, MAKELPARAM(Rect.right, Rect.bottom));
    }
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::UpdateMousePos(HWND hwndParent, int x, int y)
{
    int ix, iy;

    ix = (int)(panOffset.x + 0.5);
    BitMapMousePos.x = (int)((float)x / scaleFactor - (float)ix);

    // keep bitmap mouse poistion within bitmap coordinates
    if (BitMapMousePos.x < 0) BitMapMousePos.x = 0;
    if (BitMapMousePos.x >= (long)BitmapSize.x) BitMapMousePos.x = (long)BitmapSize.x - 1;

    iy = (int)(panOffset.y + 0.5);
    BitMapMousePos.y = (int)((float)y / scaleFactor - (float)iy);
    // keep bitmap mouse poistion within bitmap coordinates
    if (BitMapMousePos.y < 0) BitMapMousePos.y = 0;
    if (BitMapMousePos.y >= (long)BitmapSize.y) BitMapMousePos.y = (long)BitmapSize.y - 1;

    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::GetBorderSize(int* x, int* y)
{
    *x = BorderX;
    *y = BorderY;
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::SetReportedWindowPos(HWND hwndWindow, WINDOWPOS* wpos)
{
    // The window position and size reported is the entire window
    //  not just the client area.  As result you must understand how 
    //  client area dimensions are affected.
    // Since the Window size must be changed in the WM_WINDOWPOSCHANGING
    //  changing message, you must understand how to reflact the difference
    //  what a client area size should be and the window size.

    WindowPos.x = wpos->x;
    WindowPos.y = wpos->y;
    WindowPos.cx = wpos->cx;
    WindowPos.cy = wpos->cy;
    WindowPos.hwnd = wpos->hwnd;
    WindowPos.hwndInsertAfter = wpos->hwndInsertAfter;
    WindowPos.flags = wpos->flags;

    // calculate Client Height Offset
    RECT Rect;
    GetClientRect(hwndWindow, &Rect);

    // Calculate window border sizes so can properly scale 
    // the client window
    // This accounts for the window frame and title bar

    BorderX = WindowPos.cx - ((Rect.right - Rect.left) + 1);
    BorderY = WindowPos.cy - ((Rect.bottom - Rect.top) + 1);

    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::UpdateStatusBar(HWND ParentWindow) {
    if (hwndStatusBar) {
        WCHAR szString[MAX_PATH];
        RECT Rect;

        if (!hwndStatusBar) {
            return;
        }

        GetClientRect(ParentWindow, &Rect);
        ICOLOR Pixel;

        if (ColorImage) {
            Pixel.ColorRef = ColorImage[BitMapMousePos.y * DisplayXsize + BitMapMousePos.x];
        }
        else {
            Pixel.ColorRef = -1;
        }

        swprintf_s(szString, MAX_PATH,
            L"(%4d,%4d) = RBG(%3d,%3d,%3d), ScaleFactor=%.3f, Image Size =(%4dH,%4dV)",
            BitMapMousePos.x, BitMapMousePos.y, 
            Pixel.Quad.rgbBlue,Pixel.Quad.rgbGreen, Pixel.Quad.rgbRed,
            scaleFactor,
            (int)BitmapSize.x,(int)BitmapSize.y);

        SendMessage(hwndStatusBar, SB_SETTEXT, MAKEWPARAM(0, SBT_POPOUT), reinterpret_cast<LPARAM>(szString));
    }
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::GetScalePos(float* Scale, float* Xoff, float* Yoff)
{
    *Scale = scaleFactor;
    *Xoff = panOffset.x;
    *Yoff = panOffset.y;
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::SetScalePos(float Scale, float Xoff, float Yoff)
{
    scaleFactor = Scale;
    panOffset.x = Xoff;
    panOffset.y = Yoff;
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::GetDisplaySize(int* x, int* y)
{
    *x = DisplayXsize;
    *y = DisplayYsize;
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
void ImageDialog::DeleteColorImage(void)
{
    if (ColorImage) {
        delete [] ColorImage;
        ColorImage = NULL;
        DisplayXsize = 0;
        DisplayYsize = 0;
    }
    return;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
COLORREF* ImageDialog::GetColorImage(void)
{
    if (!ColorImage) {
        return NULL;
    }
    return ColorImage;
}

//*******************************************************************************
//
// 
// 
//*******************************************************************************
COLORREF* ImageDialog::CreateColorImage(int xsize, int ysize, int NumFrames)
{
    if (ColorImage) {
        delete[] ColorImage;
    }

    ColorImage = new COLORREF [(size_t)xsize * (size_t)ysize * (size_t)NumFrames];
    DisplayXsize = xsize;
    DisplayYsize = ysize;

    return ColorImage;
}

//****************************************************************
//
//  LoadBMPfile
// 
//****************************************************************
int  ImageDialog::LoadBMPfile(WCHAR* InputFilename, BOOL Invert)
{
    // open BMP file
    FILE* BMPfile;
    errno_t ErrNum;
    int iRes;

    ErrNum = _wfopen_s(&BMPfile, InputFilename, L"rb");
    if (!BMPfile) {
        return APPERR_FILEOPEN;
    }

    DeleteColorImage(); // if ColorImage exists delete it

    // read BMP headers
    BITMAPFILEHEADER BMPheader;
    BITMAPINFOHEADER BMPinfoheader;
    int StrideLen;
    BYTE* Stride;

    iRes = (int)fread(&BMPheader, sizeof(BITMAPFILEHEADER), 1, BMPfile);
    if (iRes != 1) {
        fclose(BMPfile);
        return APPERR_FILETYPE;
    }

    iRes = (int)fread(&BMPinfoheader, sizeof(BITMAPINFOHEADER), 1, BMPfile);
    if (iRes != 1) {
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
        BMPinfoheader.biBitCount != 24 && BMPinfoheader.biPlanes != 1) {
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
        // alocate array to receive image
        if(!CreateColorImage(BMPinfoheader.biWidth, BMPinfoheader.biHeight, 1)) {
            delete[] Stride;
            fclose(BMPfile);
            return APPERR_MEMALLOC;
        }

        // BMPimage of BMPimageBytes
        for (int y = 0; y < BMPinfoheader.biHeight; y++) {
            // read stride
            iRes = (int)fread(Stride, 1, StrideLen, BMPfile);
            if (iRes != StrideLen) {
                DeleteColorImage();
                delete[] Stride;
                fclose(BMPfile);
                return APPERR_FILETYPE;
            }
            BitCount = 0;
            StrideIndex = 0;
            if (TopDown) {
                Offset = ((BMPinfoheader.biHeight - 1) - y) * BMPinfoheader.biWidth;
            }
            else {
                Offset = y * BMPinfoheader.biWidth;
            }
            for (int x = 0; x < BMPinfoheader.biWidth; x++) {
                int Bit;
                // split out bit by bit
                Bit = Stride[StrideIndex] & (0x80 >> BitCount);
                if (Invert) {
                    if (Bit) {
                        Bit = 0;
                    }
                    else {
                        Bit = 1;
                    }
                }
                else {
                    if (Bit) Bit = 1;
                }

                if (Bit) {
                    ColorImage[Offset + x] = 0xffffff; // set to white
                }
                else {
                    ColorImage[Offset + x] = 0;
                }
                BitCount++;
                if (BitCount == 8) {
                    BitCount = 0;
                    StrideIndex++;
                }
            }
        }
    }
    else if (BMPinfoheader.biBitCount == 8) {
        // this is a byte image, has color table, 256 entries
        // Skip the 256 RGBQUAD entries

        ICOLOR Color;
        COLORREF ColorTable[256];

        for (int i = 0; i < 256; i++) {
            iRes = (int)fread(&Color, sizeof(RGBQUAD), (size_t)1, BMPfile);
            if (iRes != 1) {
                delete[] Stride;
                fclose(BMPfile);
                return APPERR_FILETYPE;
            }

            // RGBQUAD has blue and red colors reversed fromm COLOLRREF R8GG8B8A8
            BYTE tmp;
            tmp = Color.Quad.rgbBlue;
            Color.Quad.rgbBlue = Color.Quad.rgbRed;
            Color.Quad.rgbRed = tmp;
            ColorTable[i] = Color.ColorRef;
        }

        // allocate Image
        // alocate array of 'int's to receive image
        if(!CreateColorImage(BMPinfoheader.biWidth, BMPinfoheader.biHeight,1)) {
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
                DeleteColorImage();
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
                int Pixel;
                Pixel = (int)Stride[x];
                if (!Invert) {
                    ColorImage[Offset + x] = ColorTable[Pixel];
                }
                else {
                    ColorImage[Offset + x] = ColorTable[255 - Pixel];
                }
            }
        }
    }
    else {
        // this is a 24 bit, RGB image
        // The color table is biClrUsed long
        if (BMPinfoheader.biClrUsed != 0) {
            // skip the color table if present
            if (fseek(BMPfile, sizeof(RGBQUAD) * BMPinfoheader.biClrUsed, SEEK_CUR) != 0) {
                delete[] Stride;
                fclose(BMPfile);
                return APPERR_FILETYPE;
            }
        }

        if (!CreateColorImage(BMPinfoheader.biWidth, BMPinfoheader.biHeight, 1)) {
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
                DeleteColorImage();
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
                int Red, Green, Blue;
                Red = (int)Stride[x * 3 + 2];
                Green = (int)Stride[x * 3 + 1] << 8;
                Blue = (int)Stride[x * 3 + 0] << 16;
                ColorImage[Offset + x] = Red + Green + Blue;
            }
        }
    }

    delete[] Stride;
    fclose(BMPfile);
    
    return APP_SUCCESS;
}
