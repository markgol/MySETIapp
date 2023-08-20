#pragma once

void ExtractFromBitStreamText(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int NumBlockHeaderBits, int NumBlockBodyBits, int NumBlocks,
    int xsize);

void BitDistance(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int SkipSize);

void BitSequences(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int PrologueSize);

void BitStreamToGreyscaleImage(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, WCHAR* OutputFileRaw,
    int PrologueSize, int NumPageHeaderBits, int NumPageBodyBits,
    int xsize, int BitDepth, BOOL BitOrder);

void FileHexDump(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int xsize);

void BitStreamStats(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits, int BlockNum);

void ExtractBits(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int SkipSize, int CopyBits, int xsize);

void BitStream2Image(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits, int BlockNum, int xsize,
    int BitDepth, int BitOrder, int BitScale);

