#pragma once

#pragma pack(push, 1)
typedef struct IMAGINGHEADER {
	short Endian;		// 0 MAC format, -1 PC format
	short ID;			// 0xaaaa, the header always starts with 0,ID or -1,ID
						// A file not starting with this is not the correct filetype
	short HeaderSize;	// number of bytes in header
	LONG32 Xsize;			// number of columns in image (type long allows for long linear bitstreams)
	LONG32 Ysize;			// number of rows image
	short PixelSize;	// pixel size, 1-byte (uchar), 2-uint16 (ushort), 4-int32 (int)
	short NumFrames;	// Number of image frames in the file
	short Version;		// header version  number
						// 1 - this 32 byte header
	short Padding[6];	// dummy entries reserved for other uses
} IMAGINGHEADER;
#pragma pack(pop)

union PIXEL {
	BYTE Byte[4];
	USHORT uShort;
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
				int EnableBatch, int GenerateBMP, int Invert);

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

int AddSubtractKernel(HWND hDlg, WCHAR* InputFile, WCHAR* InputFile2, WCHAR* OutputFile, int ADDflag);

int RotateImage(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int Direction);

int MirrorImage(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int Direction);

int CalculateConvPixel(int x, int y, int* Image, float* Kernel, int KernelXsize, int KernelYsize, int xsize);

void Convolve(float* Kernel, int KernelXsize, int KernelYsize, int* Image, int* NewImage, int xsize, int ysize);

int ConvertDecomList2Relative(int* DecomX, int* DecomY, int DecomXsize, int DecomYsize, int NumKernels);

int ResizeImage(WCHAR* InputFile, WCHAR* OutputFile, int Xsize, int Ysize, int PixelSize);

int DecimateImage(WCHAR* InputFile, WCHAR* TextFile, WCHAR* OutputFile, int ScalePixel);

int ReplicateImage(WCHAR* InputFile, WCHAR* OutputFile, int Xsize, int Ysize);

int StdDecimateImage(WCHAR* InputFile, WCHAR* OutputFile, int Xsize, int Ysize, int PixelSize);

int MathConstant2Image(WCHAR *InputFile, WCHAR *OutputFile, int Value, int Operation, int Warn, int* ArithmeticFlag);

int ReorderAlg(WCHAR* InputFile, WCHAR* OutputFile, int Xsize, int Ysize, int PixelSize,
	int Algorithm, int P1, int P2, int P3, int Invert);

int ExtractSymbols(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int SkipBits,
					int xsizesymbol, int ysizesymbol, int Approach, int Highlight);

int SymbolTest(int* InputImage, int xsize, int ysize, int Yoffset, int NullValue);

void SymbolCopy(int* InputImage, int* OutputImage, int xsize, int ysize,
	int YoffsetIn, int YoffsetOut, int Highlight);

void SymbolSet(int* Image, int xsize, int ysize, int Yoffset, int Value);

int InsertImage(HWND hDlg, WCHAR* ImageInputFile, WCHAR* ImageInputFile2, WCHAR* ImageOutputFile,
	int Xloc, int Yloc, int InsertAddFlag);

int Image2Stream(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int BitDepth, int Frames, int Header, int BitOrder, int Invert);

int PixelReorderBatch(HWND hDlg, WCHAR* TextInput, WCHAR* InputFile, int ScalePixel, int Lineaer, int EnableBatch,
						int GenerateBMP, int Invert);

int StringBlankorComnent(WCHAR* Line, int LineSize);

int ReadIntKernelFile(HWND hDlg, WCHAR* TextInput, int** Kernelptr, int* KernelXsize, int* KernelYsize);

int BlockReorder(HWND hDlg, WCHAR* TextInput, WCHAR* InputFile, WCHAR* OutputFile, int ScalePixel, int LinearOnly,
	int EnableBatch, int GenerateBMP, int Xsize, int Ysize, int PixelSize, int Invert);

void ComputeBlockReordering(int* DecomAddress, int xsize, int ysize, int* DecomX, int* DecomY,
	int DecomXsize, int DecomYsize, int BlockXsize, int BlockYsize);

