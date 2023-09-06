//
// MySETIapp, a set tools for decoding bitstreams into various formats and manipulating those files
// imaging.cpp
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
// Common image operations
// 
// Need a set of common operations to load, transform, save image files. 
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
// V1.0.0.1	2023-08-20  Initial release
// V1.1.0.1 2023-08-22	Added Image Decimation
//                      Added Resize image file
// V1.2.0.1 2023-08-31	Added image stats reporting not just image header stats
//                      Corrected error handling of reorder list, when entry in
//                      kernel is out of bounds, file closure on error.
//                      Added batch processing for reordering,  This allows a series of
//                      reorder kernels to be used.  Each kernel adds an index number
//                      onto the output filename.
// V1.2.1.1 2023-09-06	Decimation process, corrected incorrect calculation for new Y dimension
//
#include "framework.h"
#include "stdio.h"
#include "wchar.h"
#include <atlstr.h>
#include "strsafe.h"
#include "Globals.h"
#include "Imaging.h"
#include "FileFunctions.h"

//*****************************************************************************************
//
//	LoadImageFile
// 
//	Load Image file into memory including all frames
//	The Image memory is allocated in this routine.  It must be deleted by the calling processes
//	using 'delete [] ImagePtr' after usage if completed.
//	Note: regardless of Input image PixelSize the Image memory is of type (int)
// 
// Parameters:
//	int** ImagePtr			pointer to (int) array containing input image
//	WCHAR* ImagingFilename	Image file to load
//	IMAGINGHEADER* Header	pointer to IMAGINGHEADER structure of the loaded
//							image file
// 
// return:
//	This function also checks for a valid image header from the file
// 
//	1 - success			'delete [] ImagePtr' must be used to free memory
//	error #				no memory allocated, Header contents invalid
//						see standarized app error number listed above
//
// Usage exmaple:
// 
//		#include "imaging.h"
//		int* Image1;
//		int iRes;
//		IMAGINGHEADER InputHeader;
//		iRes = LoadImageFile(&Image1, ImageInputFile, &InputHeader);
//		if (iRes != 1) {
//			MessageBox(hDlg, L"Input file read error", L"File I/O error", MB_OK);
//			return iRes;
//		}
//		int Pixel;
//		Pixel = Image1[0];
//		delete [] Image1;
//
//*****************************************************************************************
int LoadImageFile(int** ImagePtr, WCHAR* ImagingFilename, IMAGINGHEADER* Header)
{
	FILE* In;
	size_t iRead;

	_wfopen_s(&In, ImagingFilename, L"rb");
	if (In == NULL) {
		*ImagePtr = NULL;
		return -2;
	}

	iRead = fread(Header, sizeof(IMAGINGHEADER), 1, In);
	if (iRead != 1) {
		*ImagePtr = NULL;
		fclose(In);
		return -3;
	}


	if (Header->Endian != 0 && Header->Endian != -1 && Header->ID != 0xaaaa) {
		*ImagePtr = NULL;
		fclose(In);
		return 0;
	}

	if (Header->Xsize <= 0 || Header->Ysize <= 0 || Header->NumFrames <= 0) {
		*ImagePtr = NULL;
		fclose(In);
		return 0;
	}

	if (Header->PixelSize != 1 && Header->PixelSize != 2 && Header->PixelSize != 4) {
		*ImagePtr = NULL;
		fclose(In);
		return 0;
	}

	int* Image;
	int xsize;
	int ysize;
	int NumFrames;
	int PixelSize;
	int Endian;
	xsize = (int)Header->Xsize;
	ysize = (int)Header->Ysize;
	NumFrames = (int)Header->NumFrames;
	PixelSize = (int)Header->PixelSize;
	Endian = (int)Header->Endian;

	Image = new int[(size_t)xsize * (size_t)ysize * (size_t)NumFrames];  // alocate array of 'int's to receive image
	if (Image == NULL) {
		*ImagePtr = NULL;
		fclose(In);
		return -1;
	}

	*ImagePtr = Image;

	// we don't read entire file in one fread since we may need
	// to covert pixel(BYTE,SHORT or LONG) and Endian to get correct 'int'
	PIXEL Pixel;
	for (int i = 0; i < xsize * ysize * NumFrames; i++) {
		iRead = fread(&Pixel, PixelSize, 1, In);
		if (iRead != 1) {
			fclose(In);
			delete[] Image;
			*ImagePtr = NULL;
			return -3;
		}

		if (PixelSize == 1) {
			Image[i] = (int)Pixel.Byte[0];
		}
		else if (PixelSize == 2) {
			if (!Endian) {
				int swap;
				swap = Pixel.Byte[0];
				Pixel.Byte[0] = Pixel.Byte[1];
				Pixel.Byte[1] = swap;
			}
			Image[i] = (int)Pixel.Short;
		}
		else {
			if (!Endian) {
				int swap;
				swap = Pixel.Byte[0];
				Pixel.Byte[0] = Pixel.Byte[3];
				Pixel.Byte[3] = swap;
				swap = Pixel.Byte[1];
				Pixel.Byte[1] = Pixel.Byte[2];
				Pixel.Byte[2] = swap;
			}
			Image[i] = (int)Pixel.Long;
		}
	}
	fclose(In);
	// calling routine is responsible for deleting 'Image' memory

	return 1;
}

//*****************************************************************************************
//
//	ReportImageHeader
// 
//	Report the contents of the image file using its header
// 
// Parameters:
//	HWND hDlg		Handle of calling window or dialog
//	WCHAR* Filename	Image filename	
//
//*****************************************************************************************
void ReportImageHeader(HWND hDlg, WCHAR* Filename)
{
	FILE* In;
	IMAGINGHEADER ImageHeader;
	errno_t ErrNum;
	size_t iRead;

	ErrNum = _wfopen_s(&In, Filename, L"rb");
	if (In == NULL) {
		MessageBox(hDlg, L"Could not open file", L"File I/O", MB_OK);
		return;
	}

	iRead = fread(&ImageHeader, sizeof(IMAGINGHEADER), 1, In);
	if (iRead != 1) {
		MessageBox(hDlg, L"File is not a pixel image file", L"File I/O", MB_OK);
		return;
	}
	fclose(In);

	if (ImageHeader.Endian != 0 && ImageHeader.Endian != -1) {
		MessageBox(hDlg, L"File is not a pixel image file", L"File I/O", MB_OK);
		return;
	}

	if (ImageHeader.ID != (short)0xaaaa) {
		MessageBox(hDlg, L"File is not a pixel image file", L"File I/O", MB_OK);
		return;
	}

	if (ImageHeader.HeaderSize != sizeof(IMAGINGHEADER)) {
		MessageBox(hDlg, L"File is not a pixel image file", L"File I/O", MB_OK);
		return;
	}

	if (ImageHeader.PixelSize != 1 && ImageHeader.PixelSize != 2 && ImageHeader.PixelSize != 4) {
		MessageBox(hDlg, L"File is not a pixel image file", L"File I/O", MB_OK);
		return;
	}

	// report contents of image header
	TCHAR pszMessageBuf[MAX_PATH];
	StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH, TEXT("Image Pixel file Properties\n# of frames: %d\nXsize: %d\nYsize: %d\nPixelSize: %d byte(s)"),
		(int)ImageHeader.NumFrames, (int)ImageHeader.Xsize, (int)ImageHeader.Ysize, (int)ImageHeader.PixelSize);
	MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);

	return;
}

//*****************************************************************************************
//
//	ReportImageProperties
// 
//	Report the contents of the image file using its header and some image statistics
// 
// Parameters:
//	HWND hDlg		Handle of calling window or dialog
//	WCHAR* Filename	Image filename	
//
//*****************************************************************************************
void ReportImageProperties(HWND hDlg, WCHAR* Filename)
{
	IMAGINGHEADER ImageHeader;
	int* InputImage;
	int iRes;

	iRes = LoadImageFile(&InputImage, Filename, &ImageHeader);
	if (iRes != 1) {
		if (iRes == -2) {
			//     -2 open file failure
			MessageBox(hDlg, L"Could not open file", L"File error", MB_OK);
		}
		else if (iRes == -3) {
			//     -3 file read failure
			MessageBox(hDlg, L"Read failure of file", L"File error", MB_OK);
		}
		else if (iRes == -4) {
			//     -4 incorect file type
			MessageBox(hDlg, L"This is not a image file", L"File error", MB_OK);
		}
		else {
			MessageBox(hDlg, L"Problem occurred reading image file", L"File error", MB_OK);
		}
	}

	if (ImageHeader.Endian != 0 && ImageHeader.Endian != -1) {
		MessageBox(hDlg, L"File is not a pixel image file", L"File I/O", MB_OK);
		return;
	}

	if (ImageHeader.ID != (short)0xaaaa) {
		MessageBox(hDlg, L"File is not a pixel image file", L"File I/O", MB_OK);
		return;
	}

	if (ImageHeader.HeaderSize != sizeof(IMAGINGHEADER)) {
		MessageBox(hDlg, L"File is not a pixel image file", L"File I/O", MB_OK);
		return;
	}

	if (ImageHeader.PixelSize != 1 && ImageHeader.PixelSize != 2 && ImageHeader.PixelSize != 4) {
		MessageBox(hDlg, L"File is not a pixel image file", L"File I/O", MB_OK);
		return;
	}
	int NonZeroPixels = 0;
	int ImageMin = InputImage[0];
	int ImageMax = InputImage[0];
	for (int i = 0; i < (ImageHeader.Xsize * ImageHeader.Ysize * ImageHeader.NumFrames); i++) {
		if (InputImage[i] > 0) NonZeroPixels++;
		if (InputImage[i] > ImageMax) ImageMax = InputImage[i];
		if (InputImage[i] < ImageMin) ImageMin = InputImage[i];;
	}

	// report contents of image header
	TCHAR pszMessageBuf[MAX_PATH];
	StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH,
		TEXT("Image Pixel file Properties\n# of frames: %d\nXsize: %d\nYsize: %d\nPixelSize: %d byte(s)\nTotal non-zero pixels %d\nPixel range min: %d, max: %d"),
		(int)ImageHeader.NumFrames, (int)ImageHeader.Xsize, (int)ImageHeader.Ysize,
		(int)ImageHeader.PixelSize, NonZeroPixels, ImageMin, ImageMax);
	MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);

	return;
}

//*****************************************************************************************
//
// ReadImageHeader
//
// This reads the image header from a file and store it in the passed header structure.
// This also check if the header structure is valid.  If not it will return an error.
// 
// Parameters:
//	WCHAR* Filename				Filename of image file to read header from
//	IMAGINGHEADER* ImageHeader	point to IMAGINGHEADER structure. See imaging.h
//								for definition of structure
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*****************************************************************************************
int ReadImageHeader(WCHAR* Filename, IMAGINGHEADER* ImageHeader)
{
	FILE* In;
	errno_t ErrNum;
	size_t iRead;

	ErrNum = _wfopen_s(&In, Filename, L"rb");
	if (In == NULL) {
		return -1;
	}

	iRead = fread(ImageHeader, sizeof(IMAGINGHEADER), 1, In);
	if (iRead != 1) {
		return -2;
	}
	fclose(In);

	if (ImageHeader->Endian != 0 && ImageHeader->Endian != -1) {
		return 0;
	}

	if (ImageHeader->ID != (short)0xaaaa) {
		return 0;
	}

	if (ImageHeader->HeaderSize != sizeof(IMAGINGHEADER)) {
		return 0;
	}

	if (ImageHeader->PixelSize != 1 && ImageHeader->PixelSize != 2 && ImageHeader->PixelSize != 4) {
		return 0;
	}

	return 1;
}

//******************************************************************************
//
// ImageExtract
// 
// Extract a subimage from the specified image file.  The subimage can be left, top aligned
// or center aligned in the outputimage.  The image is padded as required to fill
// the output image size specification.  This function can also be used to extract
// a subset of frames from a multi-frame image file.
// The output image can also be scaled as a binary image 0,>1 -> 0,255.
// PixelSize in new image is the same as old image.
// 
// Parmeters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* InputImageFile	Input image file from which the subimage will be extracted
//	WCHAR* OutputImageFile	Output image file
//	int ScaleBinary			Scale as binary image to make it easy easier to in display
//							pixel=0 -> 0,  pixel>0 -> 255
//	int SubimageXloc		starting x position of subimage. relative to upper left corner
//							0 based
//	int SubimageYloc		starting y position of subimage. relative to upper left corner
//							0 based
//	int StartFrame			Starting frame number to extract (0 based)
//							Ignored is # frames in input file is 1
//	int EndFrame			Ending frame number to extract (0 based)
//							Ignored is # frames in input file is 1
//	int SubimageXsize		x size of subimage to extract
//							(must be smaller or equal to xsize of input image)
//	int SubimageYsize		y size of subimage to extract
//							(must be smaller or equal to ysize of input image)
//	int OutputXsize			x size of output image (0 padded if required)
//							(must be larger or equal to xsize of subimage)
//	int OutputYsize			y size of output image (0 padded if required)
//							(must be larger or equal to ysize of subimage)
//	int Centered			1 - extracted subimage to be centered in the new output image
//							0 - subimage left, top aligned in new output image
//
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//******************************************************************************
int ImageExtract(HWND hDlg, WCHAR* InputImageFile, WCHAR* OutputImageFile,
	int ScaleBinary, int SubimageXloc, int SubimageYloc, int StartFrame, int EndFrame,
	int SubimageXsize, int SubimageYsize, int OutputXsize, int OutputYsize, int Centered)
{
	FILE* In;
	FILE* Out;
	IMAGINGHEADER InputHeader;
	IMAGINGHEADER OutputHeader;
	int* Image;
	int* SubImage;
	int i;
	int Address;
	int SubAddress;
	int CopyFrames;
	int InputFrameSize;
	int OutputFrameSize;
	int InputOffset;
	int OutputOffset;
	int SkipSize;
	int iRes;
	PIXEL Pixel;
	errno_t ErrNum;
	size_t iRead;

	if (SubimageXsize > OutputXsize) {
		MessageBox(hDlg, L"Sub Image x size is larger than output image x size", L"File I/O", MB_OK);
		return 0;
	}
	if (SubimageYsize > OutputYsize) {
		MessageBox(hDlg, L"Sub Image y size is larger than output image y size", L"File I/O", MB_OK);
		return 0;
	}

	iRes = ReadImageHeader(InputImageFile, &InputHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Input file is not pixel image file", L"File I/O", MB_OK);
		return iRes;
	}
	if (InputHeader.NumFrames != 1) {
		if (StartFrame<0 || StartFrame>EndFrame ||
			StartFrame >= InputHeader.NumFrames ||
			EndFrame >= InputHeader.NumFrames)
		{
			MessageBox(hDlg, L"Start Frame or EndFrame invalid", L"File I/O", MB_OK);
			return 0;
		}
		CopyFrames = (EndFrame - StartFrame) + 1;
	}
	else {
		StartFrame = 0;
		CopyFrames = 1;
	}
	if (SubimageXsize > InputHeader.Xsize) {
		MessageBox(hDlg, L"Sub Image x size is larger than input image x size", L"File I/O", MB_OK);
		return 0;
	}
	if (SubimageYsize > InputHeader.Ysize) {
		MessageBox(hDlg, L"Sub Image y size is larger than input image y size", L"File I/O", MB_OK);
		return 0;
	}

	// read image
	ErrNum = _wfopen_s(&In, InputImageFile, L"rb");
	if (In == NULL) {
		MessageBox(hDlg, L"Could not open image input file", L"File I/O", MB_OK);
		return -2;
	}

	InputFrameSize = InputHeader.Xsize * InputHeader.Ysize;
	OutputFrameSize = OutputXsize * OutputYsize;

	Image = (int*)calloc((size_t)InputFrameSize * (size_t)CopyFrames, sizeof(int));
	if (Image == NULL) {
		fclose(In);
		MessageBox(hDlg, L"Input Image alloc failure", L"File I/O", MB_OK);
		return -1;
	}
	// initially skip frames that are not required
	if (StartFrame != 0) {
		SkipSize = (StartFrame * InputFrameSize * InputHeader.PixelSize) + InputHeader.HeaderSize;
	}
	else {
		SkipSize = InputHeader.HeaderSize;
	}
	if (fseek(In, SkipSize, SEEK_SET) != 0) {
		free(Image);
		fclose(In);
		MessageBox(hDlg, L"bad format, Image file, too small", L"File I/O", MB_OK);
		return -3;
	}

	// read rest of requested frames
	for (i = 0; i < (InputFrameSize * CopyFrames); i++) {
		iRead = fread(&Pixel, InputHeader.PixelSize, 1, In);
		if (iRead != 1) {
			free(Image);
			fclose(In);
			MessageBox(hDlg, L"bad format, Image file, too small", L"File I/O", MB_OK);
			return -3;
		}
		if (InputHeader.PixelSize == 1) {
			Image[i] = (int)Pixel.Byte[0];
		}
		else if (InputHeader.PixelSize == 2) {
			Image[i] = (int)Pixel.Short;
		}
		else {
			// Pixel size is 4
			Image[i] = (int)Pixel.Long;
		}
	}
	fclose(In);

	SubImage = (int*)calloc((size_t)SubimageXsize *
		(size_t)SubimageYsize *
		(size_t)CopyFrames,
		sizeof(int));
	if (SubImage == NULL) {
		free(Image);
		MessageBox(hDlg, L"Sub Image alloc failure", L"File I/O", MB_OK);
		return -1;
	}

	// extract sub image from input image
	int Starty;
	int Startx;
	if (!Centered) {
		Startx = SubimageXloc;
		Starty = SubimageYloc;
	}
	else {
		Startx = SubimageXloc - (SubimageXsize / 2);
		Starty = SubimageYloc - (SubimageYsize / 2);
	}

	for (int CurrentFrame = 0; CurrentFrame < CopyFrames; CurrentFrame++) {
		OutputOffset = CurrentFrame * (SubimageXsize * SubimageYsize);
		InputOffset = CurrentFrame * InputFrameSize;
		for (int i = 0, y = Starty; i < SubimageYsize; i++, y++) {
			if (y < 0) continue;
			if (y > InputHeader.Ysize) break;
			SubAddress = i * SubimageXsize + InputOffset;
			Address = y * InputHeader.Xsize + OutputOffset;
			for (int j = 0, x = Startx; j < SubimageXsize; j++, x++, SubAddress++) {
				if ((x < 0) || (x > InputHeader.Xsize)) {
					continue;
				}
				SubImage[SubAddress] = Image[Address + x];
			}
		}
	}
	if (!Centered) {
		Startx = 0;
		Starty = 0;
	}
	else {
		Startx = (OutputXsize / 2) - (SubimageXsize / 2);
		Starty = (OutputYsize / 2) - (SubimageYsize / 2);
	}

	//
	// clear Image
	for (Address = 0; Address < (OutputFrameSize * CopyFrames); Address++) {
		Image[Address] = 0;
	}
	for (int CurrentFrame = 0; CurrentFrame < CopyFrames; CurrentFrame++) {
		InputOffset = CurrentFrame * (SubimageXsize * SubimageYsize);
		OutputOffset = CurrentFrame * OutputFrameSize;
		for (int i = 0, y = Starty; i < SubimageYsize; i++, y++) {
			SubAddress = i * SubimageXsize + InputOffset;
			Address = y * OutputXsize + Startx + OutputOffset;
			for (int j = 0; j < SubimageXsize; j++) {
				Image[Address] = SubImage[SubAddress];
				SubAddress++;
				Address++;
			}
		}
	}

	// write result to output file
	ErrNum = _wfopen_s(&Out, OutputImageFile, L"wb");
	if (Out == NULL) {
		free(Image);
		free(SubImage);
		MessageBox(hDlg, L"Could not open output file", L"File I/O", MB_OK);
		return -2;
	}

	// write out upated header for output file
	OutputHeader.Endian = (short)-1;
	OutputHeader.HeaderSize = (short)sizeof(IMAGINGHEADER);
	OutputHeader.ID = (short)0xaaaa;
	OutputHeader.NumFrames = (short)CopyFrames;
	OutputHeader.Version = (short)1;
	OutputHeader.PixelSize = InputHeader.PixelSize;
	OutputHeader.Xsize = OutputXsize;
	OutputHeader.Ysize = OutputYsize;
	OutputHeader.Padding[0] = 0;
	OutputHeader.Padding[1] = 0;
	OutputHeader.Padding[2] = 0;
	OutputHeader.Padding[3] = 0;
	OutputHeader.Padding[4] = 0;
	OutputHeader.Padding[5] = 0;
	fwrite(&OutputHeader, sizeof(IMAGINGHEADER), 1, Out);

	// write to output file
	for (Address = 0; Address < (OutputFrameSize * CopyFrames); Address++) {
		if (OutputHeader.PixelSize == 1) {
			Pixel.Byte[0] = Image[Address];
			if (ScaleBinary && Pixel.Byte != 0) {
				Pixel.Byte[0] = 255;
			}
		}
		else if (OutputHeader.PixelSize == 2) {
			Pixel.Short = Image[Address];
			if (ScaleBinary && Pixel.Short != 0) {
				Pixel.Short = 255;
			}
		}
		else {
			Pixel.Long = Image[Address];
			if (ScaleBinary && Pixel.Long != 0) {
				Pixel.Long = 255;
			}
		}
		fwrite(&Pixel, OutputHeader.PixelSize, 1, Out);
	}

	fclose(Out);
	free(SubImage);
	free(Image);
	if (DisplayResults) {
		DisplayImage(OutputImageFile);
	}

	return 1;
}

//******************************************************************************
//
// ImageAppendEnd
//
// This function appends an image file to the end of another.  The image size
// must be the same for both image files.  The PixelSize does not need to be the
// same.  The larger of the input image pixel sizes will be used for the the
// new file.  You can choose to append frames in the files frame by frame.
// This makes the y image size twice as large in the output image file.
// Otherwise the frames form the second file are added onto the end for
// the first file.
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* InputImageFile	Input image file
//  WCHAR* ImageInputFile2	Image file with frames to append
//	WCHAR* ImageOutputFile	Appended image file
//	int IncrFrames			0 - append frame by frame
//							output image file frame ysize is doubled
//							1 - append frames from 2nd image file
//							to the end of the first image file
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//******************************************************************************
int ImageAppendEnd(HWND hDlg, WCHAR* ImageInputFile, WCHAR* ImageInputFile2, WCHAR* ImageOutputFile, int IncrFrames)
{
	IMAGINGHEADER InputHeader;
	IMAGINGHEADER InputHeader2;
	IMAGINGHEADER OutputHeader;
	int iRes;

	iRes = ReadImageHeader(ImageInputFile, &InputHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"First image file is not valid", L"Incompatible file type", MB_OK);
		return iRes;
	}
	iRes = ReadImageHeader(ImageInputFile2, &InputHeader2);
	if (iRes != 1) {
		MessageBox(hDlg, L"Image file to append is not valid", L"Incompatible file type", MB_OK);
		return iRes;
	}
	if (InputHeader.Xsize != InputHeader2.Xsize || InputHeader.Ysize != InputHeader2.Ysize) {
		MessageBox(hDlg, L"Input files are not the same image size", L"Incompatible file type", MB_OK);
		return -5;
	}

	if (!IncrFrames && InputHeader.NumFrames != InputHeader2.NumFrames) {
		MessageBox(hDlg, L"per frame append (ySize*2) requires both files to have same # of frames",
			L"Incompatible file type", MB_OK);
		return -5;
	}

	memcpy_s(&OutputHeader, sizeof(OutputHeader), &InputHeader, sizeof(InputHeader));

	if (IncrFrames) {
		// all frames in file1 followed by all frames in files 2
		OutputHeader.NumFrames = InputHeader.NumFrames + InputHeader2.NumFrames;
	}
	else {
		// frame by frame append (i.e frame1 file 1 + frame1 File2, frame3 file 1 + frame3 File2, ...) 
		OutputHeader.Ysize = InputHeader.Ysize * 2;
	}
	if (InputHeader.PixelSize > InputHeader2.PixelSize) {
		OutputHeader.PixelSize = InputHeader.PixelSize;
	}
	else {
		OutputHeader.PixelSize = InputHeader2.PixelSize;
	}

	int* Image1;
	int* Image2;

	iRes = LoadImageFile(&Image1, ImageInputFile, &InputHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Input file read error", L"File I/O error", MB_OK);
		return iRes;
	}
	iRes = LoadImageFile(&Image2, ImageInputFile2, &InputHeader2);
	if (iRes != 1) {
		delete[] Image1;
		MessageBox(hDlg, L"Input fileto append read error", L"File I/O error", MB_OK);
		return iRes;
	}

	FILE* Out;
	errno_t ErrNum;
	int PixelValue;

	// write result to output file
	ErrNum = _wfopen_s(&Out, ImageOutputFile, L"wb");
	if (Out == NULL) {
		delete[] Image1;
		delete[] Image2;
		MessageBox(hDlg, L"Could not open output file", L"File I/O", MB_OK);
		return -2;
	}
	// save new imahe header
	fwrite(&OutputHeader, sizeof(OutputHeader), 1, Out);

	if (IncrFrames) {
		for (int i = 0; i < (InputHeader.NumFrames * OutputHeader.Xsize * OutputHeader.Ysize); i++) {
			PixelValue = Image1[i];
			fwrite(&PixelValue, OutputHeader.PixelSize, 1, Out);
		}
		for (int i = 0; i < (InputHeader2.NumFrames * OutputHeader.Xsize * OutputHeader.Ysize); i++) {
			PixelValue = Image2[i];
			fwrite(&PixelValue, OutputHeader.PixelSize, 1, Out);
		}
	}
	else {
		for (int i = 0; i < InputHeader.NumFrames; i++) {
			int Offset;
			Offset = i * InputHeader.Xsize * InputHeader.Ysize;
			for (int j = 0; j < InputHeader.Xsize * InputHeader.Ysize; j++) {
				PixelValue = Image1[j + Offset];
				fwrite(&PixelValue, OutputHeader.PixelSize, 1, Out);
			}
			// Xsize and Ysize are same for both input images
			for (int j = 0; j < InputHeader.Xsize * InputHeader.Ysize; j++) {
				PixelValue = Image2[j + Offset];
				fwrite(&PixelValue, OutputHeader.PixelSize, 1, Out);
			}
		}
	}

	fclose(Out);
	delete[] Image1;
	delete[] Image2;

	if (DisplayResults) {
		DisplayImage(ImageOutputFile);
	}

	return 1;
}

//******************************************************************************
//
// ImageAppendRight
//
// This function appends an image file to the right side of the image.
// The y size of the two file must be the same.  The number of frames in both
// files must be the same.  The x size of the input files does not have to be
// the same. The PixelSize does not need to be the same.  The larger of the
// input image pixel sizes will be used for the the new file. 
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* InputImageFile	Input image file
//  WCHAR* ImageInputFile2	Image file, each frame from this file is
//							appended to the right side of the each
//							frame in the first image file.
//	WCHAR* ImageOutputFile	Appended image file
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//******************************************************************************//******************************************************************************
int ImageAppendRight(HWND hDlg, WCHAR* ImageInputFile, WCHAR* ImageInputFile2,
	WCHAR* ImageOutputFile)
{
	IMAGINGHEADER InputHeader;
	IMAGINGHEADER InputHeader2;
	IMAGINGHEADER OutputHeader;
	int iRes;

	iRes = ReadImageHeader(ImageInputFile, &InputHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"First image file is not valid", L"Incompatible file type", MB_OK);
		return iRes;
	}
	iRes = ReadImageHeader(ImageInputFile2, &InputHeader2);
	if (iRes != 1) {
		MessageBox(hDlg, L"Image file to append is not valid", L"Incompatible file type", MB_OK);
		return iRes;
	}
	if (InputHeader.Ysize != InputHeader2.Ysize) {
		MessageBox(hDlg, L"Input files are not the row size", L"Incompatible file type", MB_OK);
		return -5;
	}

	if (InputHeader.NumFrames != InputHeader2.NumFrames) {
		MessageBox(hDlg, L"Append right requires both files to have same # of frames",
			L"Incompatible file type", MB_OK);
		return -5;
	}

	memcpy_s(&OutputHeader, sizeof(OutputHeader), &InputHeader, sizeof(InputHeader));

	OutputHeader.Xsize = InputHeader.Xsize + InputHeader2.Xsize;

	// output file pixel size will be the larger of the input1 and input2 sizes
	if (InputHeader.PixelSize > InputHeader2.PixelSize) {
		OutputHeader.PixelSize = InputHeader.PixelSize;
	}
	else {
		OutputHeader.PixelSize = InputHeader2.PixelSize;
	}

	int* Image1;
	int* Image2;

	iRes = LoadImageFile(&Image1, ImageInputFile, &InputHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Input file read error", L"File I/O error", MB_OK);
		return iRes;
	}
	iRes = LoadImageFile(&Image2, ImageInputFile2, &InputHeader2);
	if (iRes != 1) {
		delete[] Image1;
		MessageBox(hDlg, L"Input fileto append read error", L"File I/O error", MB_OK);
		return iRes;
	}

	FILE* Out;
	errno_t ErrNum;
	int PixelValue;

	// write result to output file
	ErrNum = _wfopen_s(&Out, ImageOutputFile, L"wb");
	if (Out == NULL) {
		delete[] Image1;
		delete[] Image2;
		MessageBox(hDlg, L"Could not open output file", L"File I/O", MB_OK);
		return -2;
	}
	// save new imahe header
	fwrite(&OutputHeader, sizeof(OutputHeader), 1, Out);

	for (int i = 0; i < InputHeader.NumFrames; i++) {
		for (int j = 0; j < InputHeader.Ysize; j++) {
			int Offset1;
			int Offset2;
			Offset1 = j * InputHeader.Xsize;
			Offset2 = j * InputHeader2.Xsize;
			// Ysize are same for both input images
			for (int k = 0; k < InputHeader.Xsize; k++) {
				PixelValue = Image1[k + Offset1];
				fwrite(&PixelValue, OutputHeader.PixelSize, 1, Out);
			}
			for (int k = 0; k < InputHeader2.Xsize; k++) {
				PixelValue = Image2[k + Offset2];
				fwrite(&PixelValue, OutputHeader.PixelSize, 1, Out);
			}
		}
	}

	fclose(Out);
	delete[] Image1;
	delete[] Image2;

	if (DisplayResults) {
		DisplayImage(ImageOutputFile);
	}

	return 1;
}

//******************************************************************************
//
// PixelReorder
//
// This function appends an image file to the right side of the image.
// The y size of the two file must be the same.  The number of frames in both
// files must be the same.  The x size of the input files does not have to be
// the same. The PixelSize does not need to be the same.  The larger of the
// input image pixel sizes will be used for the the new file. 
// 
// 1D reordering
//		This is intended for flat image files (ysize=1).  The total length is divided
//		into xsize/'n' blocks.  Where 'n' is the length of the reordering kernel.
//		This requires the total length to be divisble by 'n'.
// 
//		The reordering file for 1D reordering has the following text format:
//			n,1
//			'n' values whitespace delimited
//			after the 'n' values an optional description is recommended.
//			A reordering value is relative to the its position in the
//			kernel. A 0 means the pixel is not moved
//		Example:
//			16,1
//			15 13 11 9 7 5 3 1 -1 -3 -5 -7 -9 -11 -13 -15
//			This reverses the order in the block of 16 pixels
//			0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 -> 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1
//			
// 2D reordering
//		This is intended for 2D image files.  The is divided into groups that are
//		'n' by 'm' in size.  Where 'n' is the length of the reordering kernel and
//		'm' is the height of the reordering kernel.
//		This requires the total xsize of the image to be divisble by 'n'.
//		This requires the total ysize of the image to be divisble by 'm'.
// 
//		The reordering file for 2D reordering has the following 3 text formats:
//			n,m
//			followed by n*m pairs of values with whitespace between the pairs
//			after the n*m pairs of values an optional description is recommended.
//			The reordering value is relative to the its position in the
//			kernel. A 0,0 means the pixel is not moved.  This is the most flexible
//			speficiation as it allows the replacement pixels to be outside of
//			the kernel bounds but it can be more difficult for people to visualize.
//			The other 2 formats gets translated into this form in order to be applied.
// 
//			Example:
//			8,2
//			7,1  5,1  3,1  1,1  -1,1  -3,1  -5,1  -7,1
//			7,-1 5,-1 3,-1 1,-1 -1,-1 -3,-1 -5,-1 -7,-1
// 
//			n,m,0
//			followed by n*m values with whitespace between the values
//			after the n*m values an optional description is recommended.
//			Reordering values is the linear address of replacment pixel in the 
//			the kernel, 0 based.
//			Example:
//			8,2
//			 15 14 13 12 11 10 9 8
//			  7  6  5  4  3  2 1 0
// 
//			n,m,1
//			followed by n*m values with whitespace between the values
//			after the n*m values an optional description is recommended.
//			Reordering values is the linear address of replacment pixel in the 
//			the kernel, 1 based.
//			Example:
//			8,2
//			 16 15 14 13 12 11 10 9
//			  8  7  6  5  4  3  2 1 
// 
//			These example kernals reverses the order of pixels left to right in
//			the block and swaps row so that a 8x2 block of pixel values:
//			0 1  2  3  4  5  6  7 
//			8 9 10 11 12 13 14 15
//			becomes
//			15 14 13 12 11 10 9 8
//			 7  6  5  4  3  2 1 0
//
//	Batch processing of reordering kernels
//	When the Enable Batch flag is set then multiple reodering kernels can be in a
//	reordering file.  When batch processing the kernels are listed sequentially.
//	Batch processing ends when a comment, EOF or an error in a kernel is encountered.
//	An index number starting at 1 is added to the output filename for each kernel processed.
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* TextInput		Reordering file (text file)
//	WCHAR* InputFile		Input image file
//	WCHAR* ImageOutputFile	Reordered image file
//	int ScalePixel			Scale as binary image to make it easy easier to in display
//							pixel=0 -> 0,  pixel>0 -> 255
//	int LinearOnly			1 - 1d image file pixel reorder
//							0 - 2d image file pixel reorder
//	int EnableBatch			1 - enable batch processing of kernel file
//							0 - disable batch processing of kernel file
//	int GenerateBMP			1 - Generate BMP file after each batch processing step
//							0 - Do not generate BMP during batch procssing
//							This option is only meanuingful when EnableBatch is 1.
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//						see standarized app error number listed above
//
//******************************************************************************
int PixelReorder(HWND hDlg, WCHAR* TextInput, WCHAR* InputFile, WCHAR* OutputFile,
	int ScalePixel, int LinearOnly, int EnableBatch, int GenerateBMP)
{
	FILE* Out;
	IMAGINGHEADER ImgHeader;
	int* InputImage;
	int* DecomX;
	int* DecomY;
	int* DecomAddress;
	int DecomXsize;
	int DecomYsize;
	int Offset;
	int iRes;
	int i;
	int NumKernels;
	unsigned char Pixel;
	errno_t ErrNum;

	// read input image file
	iRes = LoadImageFile(&InputImage, InputFile, &ImgHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Could not load image file", L"File I/O", MB_OK);
		return iRes;
	}

	if (LinearOnly && ImgHeader.Ysize != 1) {
		delete[] InputImage;
		MessageBox(hDlg, L"Input file requires linear image file (Ysize=1)", L"File incompatible", MB_OK);
		return -4;
	}

	NumKernels = ReadReoderingFile(TextInput, &DecomX, &DecomY, &DecomXsize, &DecomYsize, LinearOnly, EnableBatch);
	if (NumKernels <= 0) {
		delete[] InputImage;
		MessageBox(hDlg, L"Pixel reodering file read failure", L"File incompatible", MB_OK);
		return -4;
	}

	if (LinearOnly && DecomYsize != 1) {
		delete[] InputImage;
		MessageBox(hDlg, L"Reordering kernel Ysize must be 1", L"File incompatible", MB_OK);
		return -4;
	}

	if (ImgHeader.Xsize % DecomXsize != 0 || ImgHeader.Ysize % DecomYsize != 0) {
		delete[] InputImage;
		MessageBox(hDlg, L"Input image must be divisble by\nreordering list size in both x and y", L"File I/O", MB_OK);
		return 0;
	}

	int FrameSize = ImgHeader.Xsize * ImgHeader.Ysize;
	DecomAddress = new int[(size_t)FrameSize];
	if (DecomAddress == NULL) {
		delete[] DecomY;
		delete[] DecomX;
		delete[] InputImage;
		MessageBox(hDlg, L"Decom address table allocation failure", L"File I/O", MB_OK);
		return -1;
	}

	int KernelOffset;

	for (int Kernel = 0; Kernel < NumKernels; Kernel++) {
		WCHAR BMPfilename[MAX_PATH];
		WCHAR NewFilename[MAX_PATH];

		// calculate decom address table
		// A reordering list is made for an entire frame, so that it
		// applying it is just a simple lookup table
		KernelOffset = Kernel * DecomXsize * DecomYsize;

		ComputeReordering(DecomAddress, ImgHeader.Xsize, ImgHeader.Ysize, DecomX + KernelOffset, DecomY + KernelOffset, DecomXsize, DecomYsize);

		if (EnableBatch) {
			// for Batch processing, cutup filename, reassemble with index number (Kernel)
			int err;
			WCHAR Drive[_MAX_DRIVE];
			WCHAR Dir[_MAX_DIR];
			WCHAR Fname[_MAX_FNAME];
			WCHAR Ext[_MAX_EXT];

			// split apart original filename
			err = _wsplitpath_s(OutputFile, Drive, _MAX_DRIVE, Dir, _MAX_DIR, Fname,
				_MAX_FNAME, Ext, _MAX_EXT);
			if (err != 0) {
				delete[] DecomY;
				delete[] DecomX;
				delete[] InputImage;
				delete[] DecomAddress;
				MessageBox(hDlg, L"Could not creat output filename", L"Batch File I/O", MB_OK);
				return -2;
			}
			// change the fname portion to add _kernel# 1 based
			// use Kernel+1
			WCHAR NewFname[_MAX_FNAME];

			swprintf_s(NewFname, _MAX_FNAME, L"%s_%d", Fname, Kernel + 1);

			// reassemble filename
			err = _wmakepath_s(NewFilename, _MAX_PATH, Drive, Dir, NewFname, Ext);
			if (err != 0) {
				delete[] DecomY;
				delete[] DecomX;
				delete[] InputImage;
				delete[] DecomAddress;
				MessageBox(hDlg, L"Could not creat output filename", L"Batch File I/O", MB_OK);
				return -2;
			}
			// create BMP of file
			if (GenerateBMP) {
				err = _wmakepath_s(BMPfilename, _MAX_PATH, Drive, Dir, NewFname, L".bmp");
				if (err != 0) {
					delete[] DecomY;
					delete[] DecomX;
					delete[] InputImage;
					delete[] DecomAddress;
					MessageBox(hDlg, L"Could not creat output filename", L"Batch File I/O", MB_OK);
					return -2;
				}
			}


			// write result to output file
			ErrNum = _wfopen_s(&Out, NewFilename, L"wb");
			if (!Out) {
				delete[] DecomY;
				delete[] DecomX;
				delete[] InputImage;
				delete[] DecomAddress;
				MessageBox(hDlg, L"Could not open output file", L"File I/O", MB_OK);
				return -2;
			}
		}
		else {
			// write result to output file
			ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
			if (!Out) {
				delete[] DecomY;
				delete[] DecomX;
				delete[] InputImage;
				delete[] DecomAddress;
				MessageBox(hDlg, L"Could not open output file", L"File I/O", MB_OK);
				return -2;
			}
		}

		// write imgheader record out, no parameters in the header have changed
		fwrite(&ImgHeader, sizeof(ImgHeader), 1, Out);

		for (i = 0; i < ImgHeader.NumFrames; i++) {
			Offset = i * FrameSize;
			for (int j = 0; j < FrameSize; j++) {
				Pixel = InputImage[DecomAddress[j] + Offset];
				if (ScalePixel && Pixel != 0) {
					Pixel = 255;
				}
				fwrite(&Pixel, 1, 1, Out);
			}
		}
		fclose(Out);
		if (GenerateBMP) {
			SaveBMP(BMPfilename, NewFilename, FALSE, TRUE);
		}
	}
	delete[] DecomY;
	delete[] DecomX;
	delete[] InputImage;
	delete[] DecomAddress;

	if (DisplayResults && !EnableBatch) {
		DisplayImage(OutputFile);
	}

	return 1;
}

//******************************************************************************
//
// ReadReoderingFile
// 
// private function for PixelReorder()
//
// This reads all 3 formats for the reordering specification. For the
// linear address formats, they are translated into the relative pixel
// reorder format.
//
//*******************************************************************************
int ReadReoderingFile(WCHAR* TextInput, int** DecomXptr, int** DecomYptr,
	int* DecomXsize, int* DecomYsize, int LinearOnly,
	int EnableBatch)
{
	int iRes;
	int iFormat;
	FILE* TextIn;
	errno_t ErrNum;
	int* DecomX;
	int* DecomY;
	int LinearFormat;

	ErrNum = _wfopen_s(&TextIn, TextInput, L"r");
	if (!TextIn) {
		return -2;
	}

	// first line in the reordering file determines
	// the file format

	iFormat = fscanf_s(TextIn, "%d,%d,%d", DecomXsize, DecomYsize, &LinearFormat);
	if (iFormat != 2 && iFormat != 3) {
		fclose(TextIn);
		return -4;
	}
	if (((*DecomXsize) * (*DecomYsize)) <= 0) {
		return -4;
	}

	if (LinearOnly && (*DecomYsize != 1)) {
		return -4;
	}

	// scan file to determine number of kernels to process
	int NumTotal = 0;
	int NumKernels = 0;
	if (!EnableBatch) {
		NumKernels = 1;
	}
	else {
		while (!feof(TextIn)) {
			int x, y;
			if (iFormat == 2) {
				iRes = fscanf_s(TextIn, "%d,%d", &x, &y);
				if (iRes != 2) break;
			}
			else {
				iRes = fscanf_s(TextIn, "%d", &x);
				if (iRes != 1) break;
			}
			NumTotal++;
		}
		NumKernels = NumTotal / ((*DecomXsize) * (*DecomYsize));
		if (NumKernels == 0) {
			fclose(TextIn);
			return 0;
		}

		// reread start of file
		fseek(TextIn, SEEK_SET, 0);
		iFormat = fscanf_s(TextIn, "%d,%d,%d", DecomXsize, DecomYsize, &LinearFormat);
	}

	//
	// read rest of reordering kernel
	//
	DecomX = new int[(size_t)(*DecomXsize) * (size_t)(*DecomYsize) * (size_t)NumKernels];
	if (DecomX == NULL) {
		fclose(TextIn);
		return -1;
	}
	DecomY = new int[(size_t)(*DecomXsize) * (size_t)(*DecomYsize) * (size_t)NumKernels];
	if (DecomY == NULL) {
		delete[] DecomY;
		fclose(TextIn);
		return -1;
	}

	if (iFormat == 2) {
		for (int i = 0; i < ((*DecomXsize) * (*DecomYsize) * NumKernels); i++) {
			iRes = fscanf_s(TextIn, "%d,%d", &DecomX[i], &DecomY[i]);
			if (iRes != 2) {
				delete[] DecomY;
				delete[] DecomX;
				fclose(TextIn);
				return -4;
			}
		}
	}
	else {
		// this is alternate formats (linear , 0 based or 1 based)
		for (int i = 0; i < ((*DecomXsize) * (*DecomYsize) * NumKernels); i++) {
			DecomY[i] = 0;
			iRes = fscanf_s(TextIn, "%d", &DecomX[i]);
			if (iRes != 1) {
				delete[] DecomY;
				delete[] DecomX;
				fclose(TextIn);
				return -4;
			}
			if (LinearFormat == 1) {
				//convert to zero based
				DecomX[i] = DecomX[i] - 1;
			}
			if (DecomX[i] < 0 || DecomX[i] >= ((*DecomXsize) * (*DecomYsize))) {
				fclose(TextIn);
				delete[] DecomY;
				delete[] DecomX;
				return -4;
			}
		}
		//Convert format into relative pixel map
		iRes = ConvertDecomList2Relative(DecomX, DecomY, *DecomXsize, *DecomYsize, NumKernels);
		if (iRes != 1) {
			// report error number
			NumKernels = iRes;
		}
	}
	fclose(TextIn);
	*DecomXptr = DecomX;
	*DecomYptr = DecomY;
	return NumKernels;
}

//******************************************************************************
//
// ConvertDecomList2Relative
// 
// private function for PixelReorder()
//
//*******************************************************************************
int ConvertDecomList2Relative(int* DecomX, int* DecomY, int DecomXsize, int DecomYsize,
	int NumKernels)
{
	int* NewDecomX = new int[(size_t)DecomXsize * (size_t)DecomYsize * (size_t)NumKernels];
	if (NewDecomX == NULL) {
		return -1;
	}
	int* NewDecomY = new int[(size_t)DecomXsize * (size_t)DecomYsize * (size_t)NumKernels];
	if (NewDecomY == NULL) {
		delete[] NewDecomX;
		return -1;
	}

	int Xpos, Ypos;
	int Index;
	int Offset;

	for (int Kernel = 0; Kernel < NumKernels; Kernel++) {
		Offset = Kernel * DecomXsize * DecomYsize;
		for (int y = 0; y < DecomYsize; y++) {
			for (int x = 0; x < DecomXsize; x++) {
				Index = Offset + x + y * DecomXsize;
				if (DecomY[Index] != 0) {
					// this function called with wrong decom table format 1
					// needs to be format 2 or 3
					delete[] NewDecomX;
					delete[] NewDecomY;
					return 0;
				}
				// convert to new position relative to current position
				Xpos = DecomX[Index] % DecomXsize;
				Ypos = DecomX[Index] / DecomXsize;
				NewDecomX[Index] = Xpos - x;
				NewDecomY[Index] = Ypos - y;
			}
		}

	}

	for (int i = 0; i < DecomXsize * DecomYsize * NumKernels; i++) {
		DecomX[i] = NewDecomX[i];
		DecomY[i] = NewDecomY[i];
	}

	delete[] NewDecomX;
	delete[] NewDecomY;
	return 1;
}
//******************************************************************************
//
// ComputeReordering
// 
// private function for PixelReorder()
//
//*******************************************************************************
void ComputeReordering(int* DecomAddress, int xsize, int ysize, int* DecomX, int* DecomY,
	int DecomXsize, int DecomYsize)
{
	int x, y;
	int LinearAddress = 0;
	int CalculatedAddress = 0;
	int TotalSize;  // to make sure new address never exceeds the image size
	int Ykernel, Xkernel;
	int DecomXYindex;
	int Offset;

	TotalSize = xsize * ysize;

	// translate current x,y into Decom list position
	// subdivide the image into subgroups which are DecomXsize by DecomYsize in size
	for (y = 0; y < ysize; y++) {
		Ykernel = y % DecomYsize;  // compute y pixel position in current reorder subgroup
		for (x = 0; x < xsize; x++) {
			Xkernel = x % DecomXsize;   // compute x pixel position in current reorder subgroup
			DecomXYindex = Xkernel + (Ykernel * DecomXsize);
			Offset = DecomX[DecomXYindex] + (xsize * DecomY[DecomXYindex]);
			// calculate address offset based on Decom list entry
			CalculatedAddress = LinearAddress + Offset;
			if (CalculatedAddress < 0) CalculatedAddress = 0; // make sure address is not < 0
			DecomAddress[LinearAddress] = CalculatedAddress % TotalSize; // limit address to image size
			LinearAddress++;
		}
	}
	return;
}

//******************************************************************************
//
// FoldImageLeft
//
// This function folds an image along a vertcal axis.  It folds the right side
// of the image onto the left side. The fold axis does not have
// to be in the middle of the image. The image will be appropriately extended as
// needed. If the image is folded in the middle, for example at column 128 in a
// 256 wide image then the output file will be 128 wide.  If it is not split in
// the middle then the ouput image width will be larger to accomodate the fold.
// This is not an issue.  Think of folding a piece of paper and how it may overlap.
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* ImageFile		Input image file
//	WCHAR* OutputFile		Folded image output file
//	int FoldColumn			Column to fold at
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int FoldImageLeft(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int FoldColumn)
{
	FILE* Out;
	errno_t ErrNum;
	int* InputImage;
	int iRes;
	int InputXsize;
	int InputYsize;
	IMAGINGHEADER ImageHeader;

	iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Could not load input image, check format", L"File I/O error", MB_OK);
		return iRes;
	}

	InputXsize = ImageHeader.Xsize;
	InputYsize = ImageHeader.Ysize;

	if (InputXsize % 2) {
		delete[] InputImage;
		MessageBox(hDlg, L"xsize must be even", L"Input file incompatible", MB_OK);
		return -0;
	}

	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (!Out) {
		delete[] InputImage;
		MessageBox(hDlg, L"Could not open results file", L"File I/O", MB_OK);
		return -2;
	}

	int LeftAddress;
	int RightAddress;
	int RightPixel, LeftPixel;
	int StartXleft;
	int StartXright;
	int RightX;
	int LeftX;
	int OutputXsize;
	int Offset;
	PIXEL Pixel;
	int i;
	int y;
	// Calculations need to work for both even and odd sized image sizes
	if ((float)FoldColumn < ((float)InputXsize / 2.0)) {
		// extend image to left
		if (InputXsize % 2 == 0) {
			// This is an even sized image
			StartXleft = (FoldColumn - (InputXsize / 2)) - 1;
			StartXright = InputXsize - 1;
			OutputXsize = InputXsize / 2 + ((InputXsize / 2) - FoldColumn);
		}
		else {
			// this is an odd sized image
			// this section need verification
			StartXleft = (FoldColumn - (InputXsize / 2)) - 1;
			StartXright = InputXsize - 1;
			OutputXsize = (InputXsize / 2) + 1 + ((InputXsize / 2) - FoldColumn);
		}
	}
	else if ((float)FoldColumn > ((float)InputXsize / 2.0)) {
		// extend image to right
		if (InputXsize % 2 == 0) {
			// this is an even sized image
			StartXleft = 0;  //look at this
			StartXright = InputXsize + (FoldColumn - (InputXsize / 2));
			OutputXsize = InputXsize / 2 + (FoldColumn - (InputXsize / 2));
		}
		else {
			// this is an odd sized image
			// this section need verification
			StartXleft = 0;  //look at this
			StartXright = InputXsize + (FoldColumn - (InputXsize / 2));
			OutputXsize = (InputXsize / 2 + 1) + (FoldColumn - (int)(((float)InputXsize / 2.0) + 0.5));
		}
	}
	else {
		// fold through the center
		StartXleft = 0;
		StartXright = InputXsize - 1;
		OutputXsize = InputXsize / 2;
	}

	// write new header
	ImageHeader.Xsize = OutputXsize;
	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	// fold image
	for (int Frame = 0; Frame < ImageHeader.NumFrames; Frame++) {
		Offset = Frame * InputXsize * InputYsize;
		for (y = 0; y < InputYsize; y++) {
			for (LeftX = StartXleft, RightX = StartXright, i = 0; i < OutputXsize; i++, LeftX++, RightX--) {
				RightAddress = y * InputXsize + RightX + Offset;
				LeftAddress = y * InputXsize + LeftX + Offset;
				if (LeftX < 0) {
					// extended to left
					LeftPixel = 0;
				}
				else {
					LeftPixel = InputImage[LeftAddress];
				}
				if (RightX >= InputXsize) {
					// extended to right
					RightPixel = 0;
				}
				else {
					RightPixel = InputImage[RightAddress];
				}

				Pixel.Long = (LeftPixel + RightPixel);
				if (ImageHeader.PixelSize == 1) {
					if (Pixel.Long > 255) Pixel.Long = 255; //clip output
					fwrite(&Pixel.Byte, 1, 1, Out);
				}
				else if (ImageHeader.PixelSize == 2) {
					if (Pixel.Long > 65535) Pixel.Long = 65535; // clip output
					fwrite(&Pixel.Short, 2, 1, Out);
				}
				else {
					fwrite(&Pixel.Long, 4, 1, Out);
				}
			}
		}
	}

	fclose(Out);
	delete[] InputImage;

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	TCHAR pszMessageBuf[MAX_PATH];
	StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH, TEXT("Output image size is: %d,%d"), (int)OutputXsize, (int)InputYsize);
	MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
	return 1;
}

//******************************************************************************
//
// FoldImageRight
//
// This function folds an image along a vertcal axis.  It folds the left side
// of the image onto the right side. The fold axis does not have
// to be in the middle of the image. The image will be appropriately extended as
// needed. If the image is folded in the middle, for example at column 128 in a
// 256 wide image then the output file will be 128 wide.  If it is not split in
// the middle then the ouput image width will be larger to accomodate the fold.
// This is not an issue.  Think of folding a piece of paper and how it may overlap.
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* ImageFile		Input image file
//	WCHAR* OutputFile		Folded image output file
//	int FoldColumn			Column to fold at
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int FoldImageRight(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int FoldColumn)
{
	FILE* Out;
	errno_t ErrNum;
	int* InputImage;
	int iRes;
	int InputXsize;
	int InputYsize;
	IMAGINGHEADER ImageHeader;

	iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Could not load input image, check format", L"File I/O error", MB_OK);
		return iRes;
	}

	InputXsize = ImageHeader.Xsize;
	InputYsize = ImageHeader.Ysize;

	if (InputXsize % 2) {
		delete[] InputImage;
		MessageBox(hDlg, L"xsize must be even", L"Input file incompatible", MB_OK);
		return 0;
	}

	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (!Out) {
		delete[] InputImage;
		_fcloseall();
		MessageBox(hDlg, L"Could not open results file", L"File I/O", MB_OK);
		return -2;
	}

	int LeftAddress;
	int RightAddress;
	int RightPixel, LeftPixel;
	int StartXleft;
	int StartXright;
	int RightX;
	int LeftX;
	int OutputXsize;
	int Offset;
	PIXEL Pixel;
	int i;
	int y;

	if ((float)FoldColumn < ((float)InputXsize / 2.0)) {
		if (InputXsize % 2 == 0) {
			// extend image to left
			// This is an even sized image
			StartXleft = FoldColumn - 1;
			StartXright = FoldColumn;
			OutputXsize = InputXsize / 2 + ((InputXsize / 2) - FoldColumn);
		}
		else {
			// this is an odd sized image
			// this section need verification
			StartXleft = FoldColumn - 1;
			StartXright = FoldColumn;
			OutputXsize = InputXsize / 2 + 1 + ((InputXsize / 2) - FoldColumn); //?? this not correct
		}
	}
	else if ((float)FoldColumn > ((float)InputXsize / 2.0)) {
		// extend image to right
		if (InputXsize % 2 == 0) {
			// This is an even sized image
			StartXleft = FoldColumn - 1;
			StartXright = FoldColumn;
			OutputXsize = InputXsize / 2 + (FoldColumn - (InputXsize / 2));
		}
		else {
			// This is an odd sized image
			// this section need verification
			StartXleft = FoldColumn - 1;
			StartXright = FoldColumn;
			OutputXsize = InputXsize / 2 + (FoldColumn - (InputXsize / 2)); //?? this is not correct
		}
	}
	else {
		// fold through the center
		StartXleft = FoldColumn - 1;
		StartXright = FoldColumn;
		OutputXsize = InputXsize / 2;
	}

	// write new header
	ImageHeader.Xsize = OutputXsize;
	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	// fold image
	for (int Frame = 0; Frame < ImageHeader.NumFrames; Frame++) {
		Offset = Frame * InputXsize * InputYsize;
		for (y = 0; y < InputYsize; y++) {
			for (LeftX = StartXleft, RightX = StartXright, i = 0; i < OutputXsize; i++, LeftX--, RightX++) {
				RightAddress = y * InputXsize + RightX + Offset;
				LeftAddress = y * InputXsize + LeftX + Offset;
				if (LeftX < 0) {
					// extended to left
					LeftPixel = 0;
				}
				else {
					LeftPixel = InputImage[LeftAddress];;
				}
				if (RightX >= InputXsize) {
					// extended to right
					RightPixel = 0;
				}
				else {
					RightPixel = InputImage[RightAddress];
				}

				Pixel.Long = (LeftPixel + RightPixel);
				if (ImageHeader.PixelSize == 1) {
					if (Pixel.Long > 255) Pixel.Long = 255; //clip output
					fwrite(&Pixel.Byte, 1, 1, Out);
				}
				else if (ImageHeader.PixelSize == 2) {
					if (Pixel.Long > 65535) Pixel.Long = 65535; // clip output
					fwrite(&Pixel.Short, 2, 1, Out);
				}
				else {
					fwrite(&Pixel.Long, 4, 1, Out);
				}
			}
		}
	}

	fclose(Out);
	delete[] InputImage;

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	TCHAR pszMessageBuf[MAX_PATH];
	StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH, TEXT("Output image size is: %d,%d"), (int)OutputXsize, (int)InputYsize);
	MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
	return 1;
}


//*******************************************************************************
//
// FoldImageDown
//
// This function folds an image along a horizontal axis.  It folds the top side
// of the image onto the bottom side. The fold axis does not have
// to be in the middle of the image. The image will be appropriately extended as
// needed. If the image is folded in the middle, for example at row 128 in a
// 256 long image then the output file will be 128 long.  If it is not split in
// the middle then the ouput image width will be larger to accomodate the fold.
// This is not an issue.  Think of folding a piece of paper and how it may overlap.
// 
// Restrictions: The input image file must be even in ysize.  This restriction
// may be removed after testing but the current algorithm has not been verified
// for correct operation.
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* ImageFile		Input image file
//	WCHAR* OutputFile		Folded image output file
//	int FoldColumn			Column to fold at
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int FoldImageDown(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int FoldRow)
{
	FILE* Out;
	errno_t ErrNum;
	int* InputImage;
	int iRes;
	int InputXsize;
	int InputYsize;
	IMAGINGHEADER ImageHeader;

	iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Could not load input image, check format", L"File I/O error", MB_OK);
		return iRes;
	}

	InputXsize = ImageHeader.Xsize;
	InputYsize = ImageHeader.Ysize;

	if (InputYsize % 2) {
		delete[] InputImage;
		MessageBox(hDlg, L"ysize must be even", L"Input file incompatible", MB_OK);
		return 0;
	}

	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (!Out) {
		delete[] InputImage;
		_fcloseall();
		MessageBox(hDlg, L"Could not open results file", L"File I/O", MB_OK);
		return -2;
	}

	int TopAddress;
	int BotAddress;
	int TopPixel, BotPixel;
	int StartYtop;
	int StartYbot;
	int TopY;
	int BotY;
	int OutputYsize;
	int Offset;
	PIXEL Pixel;
	int i;
	int x;

	if (FoldRow < (InputYsize / 2)) {
		// extend image to left
		StartYtop = FoldRow - 1;
		StartYbot = FoldRow;
		OutputYsize = InputYsize / 2 + ((InputYsize / 2) - FoldRow);
	}
	else if (FoldRow > (InputYsize / 2)) {
		// extend image to right
		StartYtop = FoldRow - 1;
		StartYbot = FoldRow;
		OutputYsize = InputYsize / 2 + (FoldRow - (InputYsize / 2));
	}
	else {
		// fold through the center
		StartYtop = FoldRow - 1;
		StartYbot = FoldRow;
		OutputYsize = InputYsize / 2;
	}


	// write new header
	ImageHeader.Ysize = OutputYsize;
	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	// fold image
	for (int Frame = 0; Frame < ImageHeader.NumFrames; Frame++) {
		Offset = Frame * InputXsize * InputYsize;
		for (TopY = StartYtop, BotY = StartYbot, i = 0; i < OutputYsize; i++, TopY--, BotY++) {
			for (x = 0; x < InputXsize; x++) {
				TopAddress = TopY * InputXsize + x + Offset;
				BotAddress = BotY * InputXsize + x + Offset;
				if (TopY < 0) {
					// extended top
					TopPixel = 0;
				}
				else {
					TopPixel = InputImage[TopAddress];
				}
				if (BotY >= InputYsize) {
					// extended bottom
					BotPixel = 0;
				}
				else {
					BotPixel = InputImage[BotAddress];
				}

				Pixel.Long = (TopPixel + BotPixel);
				if (ImageHeader.PixelSize == 1) {
					if (Pixel.Long > 255) Pixel.Long = 255; //clip output
					fwrite(&Pixel.Byte, 1, 1, Out);
				}
				else if (ImageHeader.PixelSize == 2) {
					if (Pixel.Long > 65535) Pixel.Long = 65535; // clip output
					fwrite(&Pixel.Short, 2, 1, Out);
				}
				else {
					fwrite(&Pixel.Long, 4, 1, Out);
				}
			}
		}
	}

	fclose(Out);
	delete[] InputImage;

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	TCHAR pszMessageBuf[MAX_PATH];
	StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH, TEXT("Output image size is: %d,%d"), (int)InputXsize, (int)OutputYsize);
	MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
	return 1;
}

//*******************************************************************************
//
// FoldImageUp
//
// This function folds an image along a horizontal axis.  It folds the bottom side
// of the image onto the top side. The fold axis does not have
// to be in the middle of the image. The image will be appropriately extended as
// needed. If the image is folded in the middle, for example at row 128 in a
// 256 long image then the output file will be 128 long.  If it is not split in
// the middle then the ouput image width will be larger to accomodate the fold.
// This is not an issue.  Think of folding a piece of paper and how it may overlap.
// 
// Restrictions: The input image file must be even in ysize.  This restriction
// may be removed after testing but the current algorithm has not been verified
// for correct operation.
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* ImageFile		Input image file
//	WCHAR* OutputFile		Folded image output file
//	int FoldColumn			Column to fold at
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int FoldImageUp(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int FoldRow)
{
	FILE* Out;
	errno_t ErrNum;
	int* InputImage;
	int iRes;
	int InputXsize;
	int InputYsize;
	IMAGINGHEADER ImageHeader;

	iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Could not load input image, check format", L"File I/O error", MB_OK);
		return iRes;
	}

	InputXsize = ImageHeader.Xsize;
	InputYsize = ImageHeader.Ysize;

	if (InputYsize % 2) {
		delete[] InputImage;
		MessageBox(hDlg, L"ysize must be even", L"Input file incompatible", MB_OK);
		return 0;
	}

	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (!Out) {
		delete[] InputImage;
		_fcloseall();
		MessageBox(hDlg, L"Could not open results file", L"File I/O", MB_OK);
		return -2;
	}

	int TopAddress;
	int BotAddress;
	int TopPixel, BotPixel;
	int StartYtop;
	int StartYbot;
	int TopY;
	int BotY;
	int OutputYsize;
	int Offset;
	PIXEL Pixel;
	int i;
	int x;

	if (FoldRow < (InputYsize / 2)) {
		// extend image to left
		StartYtop = (FoldRow - (InputYsize / 2)) - 1;
		StartYbot = InputYsize - 1;
		OutputYsize = InputYsize / 2 + ((InputYsize / 2) - FoldRow);
	}
	else if (FoldRow > (InputYsize / 2)) {
		// extend image to right
		StartYtop = 0;
		StartYbot = InputYsize + (FoldRow - (InputYsize / 2));
		OutputYsize = InputYsize / 2 + (FoldRow - (InputYsize / 2));
	}
	else {
		// fold through the center
		StartYtop = 0;
		StartYbot = InputYsize - 1;
		OutputYsize = InputYsize / 2;
	}



	// write new header
	ImageHeader.Ysize = OutputYsize;
	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	// fold image
	for (int Frame = 0; Frame < ImageHeader.NumFrames; Frame++) {
		Offset = Frame * InputXsize * InputYsize;
		for (TopY = StartYtop, BotY = StartYbot, i = 0; i < OutputYsize; i++, TopY++, BotY--) {
			for (x = 0; x < InputXsize; x++) {
				TopAddress = TopY * InputXsize + x + Offset;
				BotAddress = BotY * InputXsize + x + Offset;
				if (TopY < 0) {
					// extended top
					TopPixel = 0;
				}
				else {
					TopPixel = InputImage[TopAddress];
				}
				if (BotY >= InputYsize) {
					// extended bottom
					BotPixel = 0;
				}
				else {
					BotPixel = InputImage[BotAddress];
				}

				Pixel.Long = (TopPixel + BotPixel);
				if (ImageHeader.PixelSize == 1) {
					if (Pixel.Long > 255) Pixel.Long = 255; //clip output
					fwrite(&Pixel.Byte, 1, 1, Out);
				}
				else if (ImageHeader.PixelSize == 2) {
					if (Pixel.Long > 65535) Pixel.Long = 65535; // clip output
					fwrite(&Pixel.Short, 2, 1, Out);
				}
				else {
					fwrite(&Pixel.Long, 4, 1, Out);
				}
			}
		}
	}

	fclose(Out);
	delete[] InputImage;

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	TCHAR pszMessageBuf[MAX_PATH];
	StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH, TEXT("Output image size is: %d,%d"), (int)InputXsize, (int)OutputYsize);
	MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
	return 1;
}

//******************************************************************************
//
// AccordionImageLeft
//
// This function does an acoordion fold an image along a vertical axis.
// It folds the right side of the accordion fold to the left side of the fold.
// The folded file is 1/2 the width of the unfolded image. The width of
// the input image must be divisble by the accordion size. 
// The input image file X size must be even.  The accordion
// size must also be even.  Think paper being cut into strips that are
// the width of the accordion size.  Then the strip is folded and the folded
// strips stuck back together.
//  
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* ImageFile		Input image file
//	WCHAR* OutputFile		Folded image output file
//	int AccordionSize		width of the unfolded accordion size
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int AccordionImageLeft(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int AccordionSize)
{
	FILE* Out;
	errno_t ErrNum;
	int* InputImage;
	int* OutputImage;
	int iRes;
	int InputXsize;
	int InputYsize;
	IMAGINGHEADER ImageHeader;

	iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Could not load input image, check format", L"File I/O error", MB_OK);
		return iRes;
	}

	InputXsize = ImageHeader.Xsize;
	InputYsize = ImageHeader.Ysize;

	if (InputXsize % 2) {
		delete[] InputImage;
		MessageBox(hDlg, L"xsize must be even", L"Input file incompatible", MB_OK);
		return 0;
	}

	if (InputXsize % AccordionSize) {
		delete[] InputImage;
		MessageBox(hDlg, L"ysize must be divisble by accordion size", L"Input file incompatible", MB_OK);
		return 0;
	}

	int LeftAddress;
	int RightAddress;
	int Address;
	int RightPixel, LeftPixel;
	int StartXleft;
	int StartXright;
	int RightX;
	int LeftX;
	int OutputXsize;
	int NumFolds;
	int FoldSize;
	int InputOffset;
	int OutputOffset;
	PIXEL Pixel;
	int i;
	int y;

	NumFolds = InputXsize / AccordionSize; // number of folds in accordion
	FoldSize = InputXsize / NumFolds;
	OutputXsize = InputXsize / 2;

	OutputImage = (int*)calloc((size_t)OutputXsize * (size_t)InputYsize * (size_t)ImageHeader.NumFrames, sizeof(int));
	if (OutputImage == NULL) {
		delete[] InputImage;
		MessageBox(hDlg, L"Output Image alloc failure", L"File I/O", MB_OK);
		return -1;
	}

	// fold accordian
	for (int Frame = 0; Frame < ImageHeader.NumFrames; Frame++) {
		InputOffset = Frame * InputXsize * InputYsize;
		OutputOffset = Frame * OutputXsize * InputYsize;
		for (y = 0; y < InputYsize; y++) {
			for (int j = 0; j < NumFolds; j++) {
				Address = (y * OutputXsize) + (j * (FoldSize / 2)) + OutputOffset;
				StartXleft = j * FoldSize;
				StartXright = (j * FoldSize) + FoldSize - 1;
				for (LeftX = StartXleft, RightX = StartXright, i = 0; i < FoldSize / 2; i++, LeftX++, RightX--, Address++) {
					RightAddress = y * InputXsize + RightX + InputOffset;
					LeftAddress = y * InputXsize + LeftX + InputOffset;
					LeftPixel = InputImage[LeftAddress];;
					RightPixel = InputImage[RightAddress];
					OutputImage[Address] = (int)(LeftPixel + RightPixel);
				}
			}
		}
	}
	delete[] InputImage;

	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (Out == NULL) {
		free(OutputImage);
		MessageBox(hDlg, L"Could not open results file", L"File I/O", MB_OK);
		return -2;
	}

	// write new header
	ImageHeader.Xsize = OutputXsize;
	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	// write file
	for (Address = 0; Address < (OutputXsize * InputYsize); Address++) {
		Pixel.Long = OutputImage[Address];
		if (ImageHeader.PixelSize == 1) {
			if (Pixel.Long > 255) Pixel.Long = 255; //clip output
			fwrite(&Pixel.Byte, 1, 1, Out);
		}
		else if (ImageHeader.PixelSize == 2) {
			if (Pixel.Long > 65535) Pixel.Long = 65535; // clip output
			fwrite(&Pixel.Short, 2, 1, Out);
		}
		else {
			fwrite(&Pixel.Long, 4, 1, Out);
		}
	}

	fclose(Out);
	free(OutputImage);

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	TCHAR pszMessageBuf[MAX_PATH];
	StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH, TEXT("Output image size is: %d,%d"), (int)OutputXsize, (int)InputYsize);
	MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
	return 1;
}

//******************************************************************************
//
// AccordionImageRight
//
// This function does an acoordion fold an image along a vertical axis.
// It folds the left side of the accordion fold to the right side of the fold.
// The folded file is 1/2 the width of the unfolded image. The width of
// the input image must be divisble by the accordion size. 
// The input image file must be even in xsize.  The accordion
// size must also be even.  Think paper being cut into strips that are
// the width of the accordion size.  Then the strip is folded and the folded
// strips stuck back together.
//  
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* ImageFile		Input image file
//	WCHAR* OutputFile		Folded image output file
//	int AccordionSize		width of the unfolded accordion size
// 
// return:
//	This function also checks for a valid image header from the file
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int AccordionImageRight(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int AccordionSize)
{
	FILE* Out;
	errno_t ErrNum;
	int* InputImage;
	int* OutputImage;
	int iRes;
	int InputXsize;
	int InputYsize;
	IMAGINGHEADER ImageHeader;

	iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Could not load input image, check format", L"File I/O error", MB_OK);
		return iRes;
	}

	InputXsize = ImageHeader.Xsize;
	InputYsize = ImageHeader.Ysize;

	if (InputXsize % 2) {
		delete[] InputImage;
		MessageBox(hDlg, L"xsize must be even", L"Input file incompatible", MB_OK);
		return 0;
	}

	if (InputXsize % AccordionSize) {
		delete[] InputImage;
		MessageBox(hDlg, L"ysize must be divisble by accordion size", L"Input file incompatible", MB_OK);
		return 0;
	}

	int LeftAddress;
	int RightAddress;
	int Address;
	int RightPixel, LeftPixel;
	int StartXleft;
	int StartXright;
	int RightX;
	int LeftX;
	int OutputXsize;
	int NumFolds;
	int FoldSize;
	int InputOffset;
	int OutputOffset;
	PIXEL Pixel;
	int i;
	int y;

	NumFolds = InputXsize / AccordionSize; // number of folds in accordion
	FoldSize = InputXsize / NumFolds;
	OutputXsize = InputXsize / 2;

	OutputImage = (int*)calloc((size_t)OutputXsize * (size_t)InputYsize * (size_t)ImageHeader.NumFrames, sizeof(int));
	if (OutputImage == NULL) {
		delete[] InputImage;
		MessageBox(hDlg, L"Output Image alloc failure", L"File I/O", MB_OK);
		return -1;
	}

	// fold accordian
	for (int Frame = 0; Frame < ImageHeader.NumFrames; Frame++) {
		InputOffset = Frame * InputXsize * InputYsize;
		OutputOffset = Frame * OutputXsize * InputYsize;
		for (y = 0; y < InputYsize; y++) {
			for (int j = 0; j < NumFolds; j++) {
				Address = (y * OutputXsize) + (j * (FoldSize / 2)) + OutputOffset;
				StartXleft = j * FoldSize + ((FoldSize / 2) - 1);
				StartXright = (j * FoldSize) + FoldSize / 2;
				for (LeftX = StartXleft, RightX = StartXright, i = 0; i < FoldSize / 2; i++, LeftX--, RightX++, Address++) {
					RightAddress = y * InputXsize + RightX + InputOffset;
					LeftAddress = y * InputXsize + LeftX + InputOffset;
					LeftPixel = InputImage[LeftAddress];
					RightPixel = InputImage[RightAddress];
					OutputImage[Address] = LeftPixel + RightPixel;
				}
			}
		}
	}
	delete[] InputImage;

	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (Out == NULL) {
		free(OutputImage);
		MessageBox(hDlg, L"Could not open results file", L"File I/O", MB_OK);
		return -2;
	}

	// write new header
	ImageHeader.Xsize = OutputXsize;
	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	// write file
	for (Address = 0; Address < (OutputXsize * InputYsize); Address++) {
		Pixel.Long = OutputImage[Address];
		if (ImageHeader.PixelSize == 1) {
			if (Pixel.Long > 255) Pixel.Long = 255; //clip output
			fwrite(&Pixel.Byte, 1, 1, Out);
		}
		else if (ImageHeader.PixelSize == 2) {
			if (Pixel.Long > 65535) Pixel.Long = 65535; // clip output
			fwrite(&Pixel.Short, 2, 1, Out);
		}
		else {
			fwrite(&Pixel.Long, 4, 1, Out);
		}
	}

	fclose(Out);
	free(OutputImage);

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	TCHAR pszMessageBuf[MAX_PATH];
	StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH, TEXT("Output image size is: %d,%d"), (int)OutputXsize, (int)InputYsize);
	MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
	return 1;
}

//******************************************************************************
//
// LeftShiftImage
//
// This function treats an image like a serial protocol which requires
// a start bit were each row is a transmission.  The leading zeros
// in  row are ignored (deleted) until the first start bit.  The new 
// row in the output length is row shifted left to the first 1 bit
// Rows with no start bit in it are skipped.  The output image is the same size
// as the input file but data has been left shifted and shifted up.
//  
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* ImageFile		Input image file
//	WCHAR* OutputFile		Folded image output file
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int LeftShiftImage(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile)
{
	FILE* Out;
	errno_t ErrNum;
	int NumSkipped;
	int Skipped;
	int RowsSkipped = 0;
	PIXEL Pixel;
	int* InputImage;
	IMAGINGHEADER ImageHeader;
	int x, y;
	int iRes;

	iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Error reading input file", L"File I/O", MB_OK);
		return iRes;
	}

	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (!Out) {
		MessageBox(hDlg, L"Could not open output file", L"File I/O", MB_OK);
		return -2;
	}

	// do not change image size, right pad row to make up for skipped pixels at be start of row
	// Add skipped lines to end of frame to keep image same size as input image
	// 
	// write out image header
	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	int Address = 0;

	for (int Frame = 0; Frame < ImageHeader.NumFrames; Frame++) {
		for (int y = 0; y < ImageHeader.Ysize; y++) {
			Skipped = 0;
			NumSkipped = 0;
			for (x = 0; x < ImageHeader.Xsize; x++) {
				Pixel.Long = InputImage[Address];
				Address++;
				if (Pixel.Long >= 1 || Skipped == 1) {
					if (ImageHeader.PixelSize == 1) {
						fwrite(&Pixel.Byte, 1, 1, Out);
					}
					else if (ImageHeader.PixelSize == 2) {
						fwrite(&Pixel.Short, 2, 1, Out);
					}
					else {
						fwrite(&Pixel.Long, 4, 1, Out);
					}
					Skipped = 1;
				}
				else {
					NumSkipped++;
				}
			}
			Pixel.Long = 0;
			if (NumSkipped >= ImageHeader.Xsize) {
				RowsSkipped++;
				continue;
			}
			// pad out row
			for (x = 0; x < NumSkipped; x++) {
				if (ImageHeader.PixelSize == 1) {
					fwrite(&Pixel.Byte, 1, 1, Out);
				}
				else if (ImageHeader.PixelSize == 2) {
					fwrite(&Pixel.Short, 2, 1, Out);
				}
				else {
					fwrite(&Pixel.Long, 4, 1, Out);
				}
			}
		}
	}
	delete[] InputImage;

	Pixel.Long = 0;
	for (y = 0; y < RowsSkipped; y++) {
		for (x = 0; x < ImageHeader.Xsize; x++) {
			if (ImageHeader.PixelSize == 1) {
				fwrite(&Pixel.Byte, 1, 1, Out);
			}
			else if (ImageHeader.PixelSize == 2) {
				fwrite(&Pixel.Short, 2, 1, Out);
			}
			else {
				fwrite(&Pixel.Long, 4, 1, Out);
			}
		}
	}

	fclose(Out);

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	return 1;
}

//******************************************************************************
//
// ConvolveImage
//
// This function does a kernel convolution on the input image.  It does not scale
// the results afterwards.  The convolution kernel is read in from a text file.
// The kernel weights are floating point numbers and can be < 0.0. Kernel sizes
// do not have to be square and can even be linear (such as a convolution kernel
// for a 1D image file).  If you are not familiar with convolution it is recommend
// that some online research or image processing text books be consulted.  The
// purpose of this function is to do the convolution arithmetic note explain it.
// 
// The border of an image that has been convolved may be missing data due to the
// convolution.  size of the kernel will dictate how many rows and columns on the
// border of the image are not used.  These will be 0 filled in the output file.
// 
// Kernel file format:
//	n,m				Kernel size, n wide by m long
//	w1 w2 w3 ....	List n*m long of weights
// optional description
//	
// example:
//	3,3
//	0.071428571 0.142857143 0.071428571
//	0.142857143 0.142857143	0.142857143
//  0.071428571 0.142857143 0.071428571 
// 3x3 kernel, smoothing using weighted average 
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* TextInput		Text file containing the kernel definition
//	WCHAR* ImageFile		Input image file
//	WCHAR* OutputFile		Convolved image output file
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int ConvolveImage(HWND hDlg, WCHAR* TextInput, WCHAR* InputFile, WCHAR* OutputFile)
{
	int iRes;
	int* InputImage;
	int* OutputImage;
	float* Kernel;
	int KernelXsize;
	int KernelYsize;
	PIXEL Pixel;
	FILE* TextIn;
	FILE* Out;
	IMAGINGHEADER ImageHeader;


	iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Could not load input image", L"File I/O error", MB_OK);
		return iRes;
	}

	OutputImage = (int*)calloc((size_t)ImageHeader.Xsize * (size_t)ImageHeader.Ysize * (size_t)ImageHeader.NumFrames, sizeof(int));
	if (OutputImage == NULL) {
		return -1;
	}
	// read in convolution kernel
	errno_t ErrNum;

	ErrNum = _wfopen_s(&TextIn, TextInput, L"r");
	if (!TextIn) {
		MessageBox(hDlg, L"Could not open decom file", L"File I/O", MB_OK);
		return -2;
	}

	iRes = fscanf_s(TextIn, "%d,%d", &KernelXsize, &KernelYsize);
	if (iRes != 2) {
		fclose(TextIn);
		delete[] InputImage;
		free(OutputImage);
		MessageBox(hDlg, L"bad format, kernel file", L"File I/O", MB_OK);
		return -3;
	}

	Kernel = new float[(size_t)KernelXsize * (size_t)KernelYsize];
	if (Kernel == NULL) {
		fclose(TextIn);
		delete[] InputImage;
		free(OutputImage);
		MessageBox(hDlg, L"Kernel allocation failure", L"File I/O", MB_OK);
		return -1;
	}

	for (int i = 0; i < (KernelXsize * KernelYsize); i++) {
		Kernel[i] = 0;
		iRes = fscanf_s(TextIn, "%f", &Kernel[i]);
		if (iRes != 1) {
			delete[] Kernel;
			fclose(TextIn);
			delete[] InputImage;
			free(OutputImage);
			MessageBox(hDlg, L"bad format or too small, Kernel file", L"File I/O", MB_OK);
			return -3;
		}
	}
	fclose(TextIn);

	int FrameOffset;
	for (int Frame = 0; Frame < ImageHeader.NumFrames; Frame++) {
		FrameOffset = Frame * ImageHeader.Xsize * ImageHeader.Ysize;
		Convolve(Kernel, KernelXsize, KernelYsize, InputImage + FrameOffset, OutputImage + FrameOffset,
			ImageHeader.Xsize, ImageHeader.Ysize);
	}
	delete[] InputImage;
	delete[] Kernel;

	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (Out == NULL) {
		free(OutputImage);
		MessageBox(hDlg, L"Could not open output file", L"File I/O", MB_OK);
		return -2;
	}

	// write out image header
	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	// write image
	for (int i = 0; i < (ImageHeader.Xsize * ImageHeader.Ysize * ImageHeader.NumFrames); i++) {
		Pixel.Long = OutputImage[i];
		if (ImageHeader.PixelSize == 1) {
			fwrite(&Pixel.Byte, 1, 1, Out);
		}
		else if (ImageHeader.PixelSize == 2) {
			fwrite(&Pixel.Short, 2, 1, Out);
		}
		else {
			fwrite(&Pixel.Long, 4, 1, Out);
		}
	}
	fclose(Out);
	free(OutputImage);

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	return 1;
}

//******************************************************************************
//
// Convolve
// 
// Private function for ConvolveImage()
//
//*******************************************************************************
void Convolve(float* Kernel, int KernelXsize, int KernelYsize, int* Image, int* NewImage, int xsize, int ysize) {
	int x, y;
	int Address;
	int StartX, EndX;
	int StartY, EndY;

	if (KernelXsize % 2) {
		// even kernel X size
		StartX = KernelXsize / 2 + 1;
		EndX = xsize - StartX;
	}
	else {
		// odd kernel X size
		StartX = KernelXsize / 2;
		EndX = xsize - StartX;
	}

	if (KernelYsize % 2) {
		// even kernel Y size
		StartY = KernelYsize / 2 + 1;
		EndY = ysize - StartY;
	}
	else {
		// odd kernel Y size
		StartY = KernelYsize / 2;
		EndY = ysize - StartY;
	}

	for (y = StartY; y < EndY; y++) {
		for (x = StartX; x < EndX; x++) {
			Address = x + (y * xsize);
			NewImage[Address] = CalculateConvPixel(x, y, Image, Kernel, KernelXsize, KernelYsize, xsize);
		}
	}

	return;
}

//******************************************************************************
//
// CalculateConvPixel
//
// Private function for Convolve()
//
// 
//*******************************************************************************
int CalculateConvPixel(int x, int y, int* Image, float* Kernel, int KernelXsize, int KernelYsize, int xsize)
{
	float Summation;
	int KernelAddress;
	int PixelAddress;
	int Pixel;

	Summation = 0.0;
	for (int i = 0; i < KernelYsize; i++) {
		for (int j = 0; j < KernelXsize; j++) {
			KernelAddress = j + (i * KernelXsize);
			PixelAddress = ((x - KernelXsize / 2) + j) + (((y - KernelYsize / 2) + i) * xsize);
			Summation += Kernel[KernelAddress] * (float)Image[PixelAddress];
		}
	}
	if (Summation < 0.0) {
		Pixel = 0;
	}
	else {
		Pixel = (int)(Summation + 0.5);
	}
	return Pixel;
}

//******************************************************************************
//
// AddImages
// 
// This function adds 2 image files together frame by frame.
// The 2 files must be the same size and number of frames.
// No scaling of the result is done.  Image is clipped based
// on the PixelSize.  If result is <0 then 0.  If result is larger than
// maximum value based on pixel size then maximum value for pixel size.
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* InputFile		Image file to sum next wnext file
//	WCHAR* InputFile2		Image file to sum
//	WCHAR* OutputFile		Summed image file
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int AddImages(HWND hDlg, WCHAR* InputFile, WCHAR* InputFile2, WCHAR* OutputFile)
{
	int iRes;
	int* InputImage1;
	int* InputImage2;
	int* OutputImage;
	errno_t ErrNum;
	PIXEL Pixel;
	FILE* Out;
	IMAGINGHEADER Input1Header;
	IMAGINGHEADER Input2Header;

	iRes = LoadImageFile(&InputImage1, InputFile, &Input1Header);
	if (iRes != 1) {
		MessageBox(hDlg, L"Could not load first input image", L"File I/O error", MB_OK);
		return iRes;
	}

	iRes = LoadImageFile(&InputImage2, InputFile2, &Input2Header);
	if (iRes != 1) {
		delete[] InputImage1;
		MessageBox(hDlg, L"Could not load szecond input image", L"File I/O error", MB_OK);
		return iRes;
	}

	if (Input1Header.Xsize != Input2Header.Xsize || Input1Header.Ysize != Input2Header.Ysize ||
		Input1Header.NumFrames != Input2Header.NumFrames) {
		MessageBox(hDlg, L"Input files must be same xsize, ysize, and # of frames", L"Files incomptaible", MB_OK);
		return 0;
	}
	OutputImage = new int[(size_t)Input1Header.Xsize * (size_t)Input1Header.Ysize * (size_t)Input1Header.NumFrames];
	if (OutputImage == NULL) {
		delete[] InputImage1;
		delete[] InputImage2;
		MessageBox(hDlg, L"Could not allocate output image", L"File I/O error", MB_OK);
		return 0;
	}

	for (int i = 0; i < (Input1Header.Xsize * Input1Header.Ysize * Input1Header.NumFrames); i++) {
		OutputImage[i] = InputImage1[i] + InputImage2[i];
		if (OutputImage[i] < 0) OutputImage[i] = 0;
	}

	delete[] InputImage1;
	delete[] InputImage2;

	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (Out == NULL) {
		delete[] OutputImage;
		MessageBox(hDlg, L"Could not open output file", L"File I/O", MB_OK);
		return -2;
	}

	//write output image
	fwrite(&Input1Header, sizeof(Input1Header), 1, Out);

	// write image
	for (int i = 0; i < (Input1Header.Xsize * Input1Header.Ysize * Input1Header.NumFrames); i++) {
		Pixel.Long = OutputImage[i];
		if (Input1Header.PixelSize == 1) {
			if (Pixel.Long > 255) Pixel.Long = 255;
			fwrite(&Pixel.Byte, 1, 1, Out);
		}
		else if (Input1Header.PixelSize == 2) {
			if (Pixel.Long > 65535) Pixel.Long = 65535;
			fwrite(&Pixel.Short, 2, 1, Out);
		}
		else {
			fwrite(&Pixel.Long, 4, 1, Out);
		}
	}

	delete[] OutputImage;
	fclose(Out);

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	return 1;
}

//******************************************************************************
//
// RotateImage
// 
// This function Mirrors an image either clockwise or counter clockwise.
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* InputFile		Image file to sum next wnext file
//	WCHAR* OutputFile		Summed image file
//	int Direction			0 - rotate image counter clockwise
//							1 - rotate image clockwise
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int RotateImage(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int Direction)
{
	int iRes;
	int* InputImage;
	int* OutputImage;
	errno_t ErrNum;
	PIXEL Pixel;
	FILE* Out;
	IMAGINGHEADER ImageHeader;

	iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Could not load first input image", L"File I/O error", MB_OK);
		return iRes;
	}

	int InputXsize = ImageHeader.Xsize;
	int InputYsize = ImageHeader.Ysize;
	int NumFrames = ImageHeader.NumFrames;

	OutputImage = new int[(size_t)ImageHeader.Xsize * (size_t)ImageHeader.Ysize * (size_t)ImageHeader.NumFrames];
	if (OutputImage == NULL) {
		delete[] InputImage;
		MessageBox(hDlg, L"Could not allocate output image", L"File I/O error", MB_OK);
	}

	int x;
	int y;
	int Offset;
	int InputAddress;
	int OutputAddress;
	int InputPixel;
	int FrameSize;
	int inputXsize;
	int inputYsize;

	inputXsize = ImageHeader.Xsize;
	inputYsize = ImageHeader.Ysize;
	FrameSize = ImageHeader.Xsize * ImageHeader.Ysize;

	if (!Direction) {
		// rotate counter clockwise
		for (int Frame = 0; Frame < NumFrames; Frame++) {
			Offset = Frame * FrameSize;
			for (y = 0; y < inputYsize; y++) {
				InputAddress = y * inputXsize + Offset;
				for (x = 0; x < inputXsize; x++) {
					OutputAddress = ((inputXsize - 1) - x) * inputYsize + Offset + y;
					InputPixel = InputImage[InputAddress + x];
					OutputImage[OutputAddress] = InputPixel;
				}
			}
		}
	}
	else {
		// rotate clockwise
		for (int Frame = 0; Frame < NumFrames; Frame++) {
			Offset = Frame * FrameSize;
			for (y = 0; y < inputYsize; y++) {
				InputAddress = y * inputXsize + Offset;
				for (x = 0; x < inputXsize; x++) {
					OutputAddress = Offset + ((inputYsize - 1) - y) + (inputYsize * x);
					InputPixel = InputImage[InputAddress + x];
					OutputImage[OutputAddress] = InputPixel;
				}
			}
		}
	}

	delete[] InputImage;

	ImageHeader.Xsize = inputYsize;
	ImageHeader.Ysize = inputXsize;

	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (Out == NULL) {
		delete[] OutputImage;
		MessageBox(hDlg, L"Could not open output file", L"File I/O", MB_OK);
		return -2;
	}

	//write output image
	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	// write image
	for (int i = 0; i < (ImageHeader.Xsize * ImageHeader.Ysize * ImageHeader.NumFrames); i++) {
		Pixel.Long = OutputImage[i];
		if (ImageHeader.PixelSize == 1) {
			if (Pixel.Long > 255) Pixel.Long = 255;
			fwrite(&Pixel.Byte, 1, 1, Out);
		}
		else if (ImageHeader.PixelSize == 2) {
			if (Pixel.Long > 65535) Pixel.Long = 65535;
			fwrite(&Pixel.Short, 2, 1, Out);
		}
		else {
			fwrite(&Pixel.Long, 4, 1, Out);
		}
	}

	delete[] OutputImage;
	fclose(Out);

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	return 1;
}

//******************************************************************************
//
// MirrorImage
// 
// This function Mirrors an image either vertically or horzontally.
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* InputFile		Image file to sum next wnext file
//	WCHAR* OutputFile		Summed image file
//	int Direction			0 - mirror around horizontal axis
//							1 - mirror around vertical axis
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int MirrorImage(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int Direction)
{
	int iRes;
	int* InputImage;
	int* OutputImage;
	errno_t ErrNum;
	PIXEL Pixel;
	FILE* Out;
	IMAGINGHEADER ImageHeader;

	iRes = LoadImageFile(&InputImage, InputFile, &ImageHeader);
	if (iRes != 1) {
		MessageBox(hDlg, L"Could not load first input image", L"File I/O error", MB_OK);
		return iRes;
	}

	int InputXsize = ImageHeader.Xsize;
	int InputYsize = ImageHeader.Ysize;
	int NumFrames = ImageHeader.NumFrames;

	OutputImage = new int[(size_t)ImageHeader.Xsize * (size_t)ImageHeader.Ysize * (size_t)ImageHeader.NumFrames];
	if (OutputImage == NULL) {
		delete[] InputImage;
		MessageBox(hDlg, L"Could not allocate output image", L"File I/O error", MB_OK);
	}

	int x;
	int y;
	int Offset;
	int InputAddress;
	int OutputAddress;
	int InputPixel;
	int FrameSize;

	FrameSize = ImageHeader.Xsize * ImageHeader.Ysize;

	if (Direction) {
		// mirror around vertical axis
		for (int Frame = 0; Frame < NumFrames; Frame++) {
			Offset = Frame * FrameSize;
			for (y = 0; y < ImageHeader.Ysize; y++) {
				OutputAddress = y * ImageHeader.Xsize + Offset;
				InputAddress = OutputAddress + (ImageHeader.Xsize - 1);
				for (x = 0; x < ImageHeader.Xsize; x++) {
					InputPixel = InputImage[InputAddress - x];
					OutputImage[OutputAddress + x] = InputPixel;
				}
			}
		}
	}
	else {
		// mirror around horizontal axis
		for (int Frame = 0; Frame < NumFrames; Frame++) {
			Offset = Frame * FrameSize;
			for (y = 0; y < ImageHeader.Ysize; y++) {
				OutputAddress = ((ImageHeader.Ysize - 1) - y) * ImageHeader.Xsize + Offset;
				InputAddress = y * ImageHeader.Xsize + Offset;
				for (x = 0; x < ImageHeader.Xsize; x++) {
					InputPixel = InputImage[InputAddress + x];
					OutputImage[OutputAddress + x] = InputPixel;
				}
			}
		}
	}

	delete[] InputImage;

	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (Out == NULL) {
		delete[] OutputImage;
		MessageBox(hDlg, L"Could not open output file", L"File I/O", MB_OK);
		return -2;
	}

	//write output image
	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	// write image
	for (int i = 0; i < (ImageHeader.Xsize * ImageHeader.Ysize * ImageHeader.NumFrames); i++) {
		Pixel.Long = OutputImage[i];
		if (ImageHeader.PixelSize == 1) {
			if (Pixel.Long > 255) Pixel.Long = 255;
			fwrite(&Pixel.Byte, 1, 1, Out);
		}
		else if (ImageHeader.PixelSize == 2) {
			if (Pixel.Long > 65535) Pixel.Long = 65535;
			fwrite(&Pixel.Short, 2, 1, Out);
		}
		else {
			fwrite(&Pixel.Long, 4, 1, Out);
		}
	}

	delete[] OutputImage;
	fclose(Out);

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	return 1;
}

//******************************************************************************
//
// ResizeImage
// 
// This function is used to be used to change the Pixel size (in bytes) from the
// old image to the new image.
//
// This can also be used to change the x,y size of the image.  This useful for
// converting a linear image file into a 2D image file.
// 
// This copies an image file in one size format to a new size format.
// 
// Limitations:  old image Xsize*Ysize must equal Xsize*Ysize of new image file
//  
// If the new Pixels size is smaller than the old pixel size then the values will
// be clipped as follows:
//		1 byte	- clipped at 255
//		2 bytes	- clipped at 65535
//		4 bytes - negative numbers will be set to 0
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* InputFile		Image file to sum next wnext file
//	WCHAR* OutputFile		Resized image file
//	int Xsize				New x size (width)
//	int Ysize				New y size (length)
//	int PixelSize			New Pixel size in bytes (must be 1,2 or 4)
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int ResizeImage(WCHAR* InputFile, WCHAR* OutputFile, int Xsize, int Ysize, int PixelSize)
{
	int* InputImage;
	IMAGINGHEADER ImageHeader;
	errno_t ErrNum;
	PIXEL Pixel;
	FILE* Out;

	LoadImageFile(&InputImage, InputFile, &ImageHeader);
	if (ImageHeader.Xsize * ImageHeader.Ysize != Xsize * Ysize) {
		delete[] InputImage;
		return 0;
	}

	ImageHeader.Xsize = Xsize;
	ImageHeader.Ysize = Ysize;
	ImageHeader.PixelSize = PixelSize;

	// write out new image file
	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (Out == NULL) {
		delete[] InputImage;
		return -2;
	}

	//write output image
	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	// write image
	for (int i = 0; i < (ImageHeader.Xsize * ImageHeader.Ysize * ImageHeader.NumFrames); i++) {
		Pixel.Long = InputImage[i];
		if (ImageHeader.PixelSize == 1) {
			if (Pixel.Long > 255) Pixel.Long = 255;
			fwrite(&Pixel.Byte, 1, 1, Out);
		}
		else if (ImageHeader.PixelSize == 2) {
			if (Pixel.Long > 65535) Pixel.Long = 65535;
			fwrite(&Pixel.Short, 2, 1, Out);
		}
		else {
			fwrite(&Pixel.Long, 4, 1, Out);
		}
	}
	fclose(Out);
	delete[] InputImage;

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	return 1;
}

//******************************************************************************
//
// DecimateImage
// 
// This copies an image file in one size romat to a new size format
// Limitations:
// Xsize and Ysize of input image must be divisible by the X,Y size of the
// decimation kernel
// The # of pixels decimated in each row in the kernel must be the same
// with the exception if the entire row is decimated.  Then the entire row is deleted
// exmaple:
// 2,2
// 0 0
// 0 1
// This deletes every even numbered pixel in a row
// and every even numbered row
// 
// Parameters:
//	HWND hDlg				Handle of calling window or dialog
//	WCHAR* InputFile		input image file
//	WCHAR* TextFile			Text file containing decimation kernel
//	WCHAR* OutputFile		Resized image file
//	int	   ScalePixel
// 
//  return value:
//  1 - Success
//  !=1 Error see standardized app error list at top of this source file
//
//*******************************************************************************
int DecimateImage(WCHAR* InputFile, WCHAR* TextFile, WCHAR* OutputFile, int ScalePixel)
{
	int* InputImage;
	int* OutputImage;
	int* Kernel;
	IMAGINGHEADER ImageHeader;
	errno_t ErrNum;
	PIXEL Pixel;
	FILE* Out;
	FILE* TextIn;
	int iRes;
	int KernelXsize;
	int KernelYsize;

	LoadImageFile(&InputImage, InputFile, &ImageHeader);

	// read decimation kernel
	ErrNum = _wfopen_s(&TextIn, TextFile, L"r");
	if (!TextIn) {
		return -2;
	}


	iRes = fscanf_s(TextIn, "%d,%d", &KernelXsize, &KernelYsize);
	if (iRes != 2) {
		fclose(TextIn);
		delete[] InputImage;
		return -3;
	}

	Kernel = new int[(size_t)KernelXsize * (size_t)KernelYsize];
	if (Kernel == NULL) {
		fclose(TextIn);
		delete[] InputImage;
		return -1;
	}

	for (int i = 0; i < (KernelXsize * KernelYsize); i++) {
		Kernel[i] = 0;
		iRes = fscanf_s(TextIn, "%d", &Kernel[i]);
		if (iRes != 1) {
			delete[] Kernel;
			fclose(TextIn);
			delete[] InputImage;
			return -3;
		}
		if (Kernel[i] < 0 || Kernel[i]>1) {
			// kernel values can only be 0 or 1
			delete[] Kernel;
			fclose(TextIn);
			delete[] InputImage;
			return 0;
		}
	}
	fclose(TextIn);

	// verify the decimation kernel
	int NumFoundinRow = -1;
	int NumBlankRows = 0;
	BOOL BlankRow;
	int Offset;
	int NumSetinRow;

	for (int y = 0; y < KernelYsize; y++) {
		Offset = y * KernelXsize;
		NumSetinRow = 0;
		BlankRow = TRUE;
		for (int x = 0; x < KernelXsize; x++) {
			if (Kernel[x + Offset] != 0) {
				NumSetinRow++;
				BlankRow = FALSE;
			}
		}
		if (!BlankRow) {
			if (NumFoundinRow < 0) {
				// set dx to the number of 
				NumFoundinRow = NumSetinRow;
			}
			else if (NumFoundinRow != NumSetinRow) {
				// the number of bits decmiated in a row must be the same size
				// for all nonblank rows
				delete[] Kernel;
				delete[] InputImage;
				return 0;
			}
		}
		else {
			NumBlankRows++; // increment blank row count
		}
	}

	if (NumFoundinRow == 0) {
		delete[] Kernel;
		delete[] InputImage;
		return 0;
	}

	int OutXsize;
	int OutYsize;

	OutXsize = NumFoundinRow * (ImageHeader.Xsize / KernelXsize);
	OutYsize = KernelYsize - NumBlankRows;
	OutYsize = OutYsize * (ImageHeader.Ysize / KernelYsize);

	// Apply decimation kernel
	OutputImage = new int[(size_t)OutXsize * (size_t)OutYsize * (size_t)ImageHeader.NumFrames];
	if (OutputImage == NULL) {
		delete[] InputImage;
		return -1;
	}

	int Kaddress;
	int i, j;
	int Keep;

	for (int FrameNum = 0; FrameNum < ImageHeader.NumFrames; FrameNum++) {
		i = 0;
		for (int y = 0; y < ImageHeader.Ysize; y++) {
			j = i * OutXsize;
			Offset = y * ImageHeader.Xsize;
			BlankRow = TRUE;
			for (int x = 0; x < ImageHeader.Xsize; x++) {
				Kaddress = (x % KernelXsize) + (y % KernelYsize) * KernelXsize;
				Keep = (int)Kernel[Kaddress];
				if (Keep == 0) continue;
				OutputImage[j] = InputImage[Offset + x];
				j++;
				BlankRow = FALSE;
			}
			if (!BlankRow) i++;

		}
	}

	delete[] InputImage;

	// update header
	ImageHeader.Xsize = OutXsize;
	ImageHeader.Ysize = OutYsize;

	//write output image
	ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
	if (Out == NULL) {
		delete[] InputImage;
		return -2;
	}

	fwrite(&ImageHeader, sizeof(ImageHeader), 1, Out);

	// write image
	for (int i = 0; i < (ImageHeader.Xsize * ImageHeader.Ysize * ImageHeader.NumFrames); i++) {
		Pixel.Long = OutputImage[i];
		if (ScalePixel && (Pixel.Long > 1)) {
			Pixel.Long = 255;
		}
		if (ImageHeader.PixelSize == 1) {
			if (Pixel.Long > 255) Pixel.Long = 255;
			fwrite(&Pixel.Byte, 1, 1, Out);
		}
		else if (ImageHeader.PixelSize == 2) {
			if (Pixel.Long > 65535) Pixel.Long = 65535;
			fwrite(&Pixel.Short, 2, 1, Out);
		}
		else {
			fwrite(&Pixel.Long, 4, 1, Out);
		}
	}

	fclose(Out);
	delete[] OutputImage;

	if (DisplayResults) {
		DisplayImage(OutputFile);
	}

	return 1;
}
