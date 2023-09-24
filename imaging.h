#pragma once

#pragma pack(push, 1)
typedef struct IMAGINGHEADER {
	short Endian;		// 0 MAC format, -1 PC format
	short ID;			// 0xaaaa, the header always starts with 0,ID or -1,ID
						// A file not starting with this is not the correct filetype
	short HeaderSize;	// number of bytes in header
	LONG32 Xsize;			// number of columns in image (type long allows for long linear bitstreams)
	LONG32 Ysize;			// number of rows image
	short PixelSize;	// pixel size, 1-byte, 2-int16 (short), 4-int32 (int)
	short NumFrames;	// Number of image frames in the file
	short Version;		// header version  number
						// 1 - this 32 byte header
	short Padding[6];	// dummy entries reserved for other uses
} IMAGINGHEADER;
#pragma pack(pop)

union PIXEL {
	BYTE Byte[4];
	SHORT Short;
	LONG Long;
};

int LoadImageFile(int** ImagePtr, WCHAR* ImagingFilename, IMAGINGHEADER* header);

void ReportImageHeader(HWND hDlg, WCHAR* szCurrentFilename);

void ReportImageProperties(HWND hDlg, WCHAR* Filename);

int ImageExtract(HWND hDlg, WCHAR* InputImageFile, WCHAR* OutputImageFile,
	int ScaleBinary, int SubimageXloc, int SubimageYloc, int StartFrame, int EndFrame,
	int SubimageXsize, int SubimageYsize, int OutputXsize, int OutputYsize, int Centered);

int ReadImageHeader(WCHAR* Filename, IMAGINGHEADER* ImageHeader);

int ImageAppendEnd(HWND hDlg, WCHAR* ImageInputFile, WCHAR* ImageInputFile2, WCHAR* ImageOutputFile, int IncrFrames);

int ImageAppendRight(HWND hDlg, WCHAR* ImageInputFile, WCHAR* ImageInputFile2, WCHAR* ImageOutputFile);

int PixelReorder(HWND hDlg, WCHAR* TextInput, WCHAR* InputFile, WCHAR* OutputFile, int ScalePixel, int LinearOnly,
				int EnableBatch, int GenerateBMP);

int ReadReoderingFile(WCHAR* TextInput, int** DecomX, int** DecomY, int* DecomXsize, int* DecomYsize, int LinearOnly,
				int EnableBatch);

void ComputeReordering(int* DecomAddress, int xsize, int ysize, int* DecomX, int* DecomY,
	int DecomXsize, int DecomYsize);

int FoldImageLeft(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int FoldColumn);

int FoldImageRight(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int FoldColumn);

int FoldImageDown(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int FoldRow);

int FoldImageUp(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int FoldRow);

int AccordionImageLeft(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int AccordionSize);

int AccordionImageRight(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int AccordionSize);

int LeftShiftImage(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile);

int ConvolveImage(HWND hDlg, WCHAR* TextInput, WCHAR* InputFile, WCHAR* OutputFile);

int AddSubtractImages(HWND hDlg, WCHAR* InputFile, WCHAR* InputFile2, WCHAR* OutputFile, int ADDflag);

int RotateImage(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int Direction);

int MirrorImage(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int Direction);

int CalculateConvPixel(int x, int y, int* Image, float* Kernel, int KernelXsize, int KernelYsize, int xsize);

void Convolve(float* Kernel, int KernelXsize, int KernelYsize, int* Image, int* NewImage, int xsize, int ysize);

int ConvertDecomList2Relative(int* DecomX, int* DecomY, int DecomXsize, int DecomYsize, int NumKernels);

int ResizeImage(WCHAR* InputFile, WCHAR* OutputFile, int Xsize, int Ysize, int PixelSize);

int DecimateImage(WCHAR* InputFile, WCHAR* TextFile, WCHAR* OutputFile, int ScalePixel);

int ReplicateImage(WCHAR* InputFile, WCHAR* OutputFile, int Xsize, int Ysize);

int StdDecimateImage(WCHAR* InputFile, WCHAR* OutputFile, int Xsize, int Ysize, int PixelSize);

int AddConstant2Image(WCHAR *InputFile, WCHAR *OutputFile, int Value);

int ReorderAlg(WCHAR* InputFile, WCHAR* OutputFile, int Xsize, int Ysize, int PixelSize, int Algorithm);

int ExtractSymbols(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int SkipBits,
					int xsizesymbol, int ysizesymbol, int Approach);

int SymbolTest(int* InputImage, int xsize, int ysize, int Yoffset);

void SymbolCopy(int* InputImage, int* OutputImage, int xsize, int ysize,
	int YoffsetIn, int YoffsetOut);

