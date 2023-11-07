#pragma once

#include "SPP.h"

void ExtractFromBitStreamText(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int NumBlockHeaderBits, int NumBlockBodyBits, int NumBlocks,
    int xsize, int Invert, int BitOrder);

void BitDistance(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int SkipSize,
    int BitOrder);

void BitSequences(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int PrologueSize,
    int BitOrder);

void FileHexDump(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int xsize, int SkipBytes);

void BitStreamStats(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits,
    int BlockNum, int BitOrder);

void ExtractBits(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int SkipSize, int CopyBits, int xsize, int Invert, int BitOrder);

int BitStream2Image(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits, int BlockNum, int xsize,
    int BitDepth, int BitOrder, int BitScale, int Invert, int InputBitOrder);

void BatchBitStream2Image(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits, int BlockNum, int xsize,
    int xsizeEnd, int BitDepth, int BitOrder, int BitScale, int Invert, int InputBitOrder);

int ConvertText2BitStream(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int BitOrder);

int ExtractSPP(HWND hDlg, WCHAR* APIDoutputFile, WCHAR* InputFile, WCHAR* OutputFile,
                int APID, int SkipBytes, int SecondaryHeaderSize,
                int Strict, int SaveSPP);

int DecodeSPP(SPP_PRIMARY_HEADER* PackedPriHeader, SPP_UNPACKED_PRIMARY_HEADER* PriHeader, int Strict);

uint16_t ByteSwap(uint16_t Value);

void RemoveNULLbytes(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int NULLvalue,
                    int NullLength, int SkipBytes);
