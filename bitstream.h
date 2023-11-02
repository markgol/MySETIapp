#pragma once

#include "SPP.h"

void ExtractFromBitStreamText(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int NumBlockHeaderBits, int NumBlockBodyBits, int NumBlocks,
    int xsize, int Invert);

void BitDistance(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int SkipSize);

void BitSequences(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int PrologueSize);

void FileHexDump(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int xsize, int SkipBytes);

void BitStreamStats(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits, int BlockNum);

void ExtractBits(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int SkipSize, int CopyBits, int xsize);

int BitStream2Image(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits, int BlockNum, int xsize,
    int BitDepth, int BitOrder, int BitScale, int Invert);

void BatchBitStream2Image(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits, int BlockNum, int xsize,
    int xsizeEnd, int BitDepth, int BitOrder, int BitScale, int Invert);

int ConvertText2BitStream(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile);

int ExtractSPP(HWND hDlg, WCHAR* APIDoutputFile, WCHAR* InputFile, WCHAR* OutputFile,
                int APID, int SkipBytes, int SecondaryHeaderSize,
                int Strict, int SaveSPP);

int DecodeSPP(SPP_PRIMARY_HEADER* PackedPriHeader, SPP_UNPACKED_PRIMARY_HEADER* PriHeader, int Strict);

uint16_t ByteSwap(uint16_t Value);

void RemoveNULLbytes(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int NULLvalue,
                    int NullLength, int SkipBytes);
