//
// MySETIapp, a set tools for decoding bitstreams into various formats and manipulating those files
// BitStream.cpp
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
// This file contains the processing routines that acutally perform the bit stream operations
// selected under the BitStream menu selection 
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
// V1.0.0.1 2023-08-20  Initial Release
// V1.2.0.1 2023-08-30  Added Convert text to packed bitstream file
// V1.2.6.1 2023-09-24  Added Skip bytes to HEX dump
// V1.2.7.1 2023-10-01  Added invert of input bits in bitstream to the bitstream to text and
//                      bistream to image operations: ExtractFromBitStreamText() and
//                      BitStream2Image(), BatchBitStream2Image()
// V1.2.8.1 2023-10-5   Added SPP extraction from a TM SPP stream file.
//                      Changed, Bit sequences report to include 0 sequences.
//
#include "framework.h"
#include <windowsx.h>
#include <time.h>
#include <stdio.h>
#include <atlstr.h>
#include "globals.h"
#include "BitStream.h"
#include "imaging.h"
#include "FileFunctions.h"
#include "strsafe.h"

//*******************************************************************
//
// ExtractFromBitStreamText
// 
// Extract packed BitStream binary file to text file
// 
// model for bitstream
//      prologue (optional)
//      blocks (1 or more blocks) consisiting of:
//          Block header(optional)
//          Body (fixed size)
//      Epilogue/footer (bits left over not belonging to a block)
// 
// Parameters:
//  HWND hDlg               handle of calling window/dialog
//  WCHAR* InputFile        Name of packed binary bitstream
//                          (file is multiple always of 8 bits)
//  WCHAR* OutputFile       Name of text output file
//  int PrologueSize        # of bits inprologue (0 - no prologue)      
//  int NumBlockHeaderBits  # of bits in block header (0 - no block header)
//  int NumBlockBodyBits    # of bits in block body (>=1)
//  int NumBlocks           # of blocks (>=1)
//  int xsize               # of bits to report on a line of text (>=1)
// 
// The text file output is:
//      Prologue line
//      for number NumBlocks
//          Block header line
//          Block with xsize values per line
//          blank line
//      Footer line
//
//*******************************************************************
void ExtractFromBitStreamText(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int NumBlockHeaderBits, int NumBlockBodyBits, int NumBlocks,
    int xsize, int Invert)
{
    // This function is large because of formatting the output
    //  text file into header, {block header, block}, footer
    FILE* In;
    FILE* Out;
    int CurrentBit = 0;
    int CurrentPrologueBit = 0;
    int CurrentFooterBit = 0;
    int CurrentByteBit = 0;
    int CurrentBlock = 0;
    int CurrentHeaderBit = 0;
    int CurrentBlockBit = 0;
    int CurrentBlockCol = 0;
    int CurentX = 0;
    int CurrentY = 0;
    errno_t ErrNum;

    ErrNum = _wfopen_s(&In, InputFile, L"rb");
    if (In==NULL) {
        MessageBox(hDlg, L"Could not open input file", L"File I/O", MB_OK);
        return;
    }

    // this is a text file
    ErrNum = _wfopen_s(&Out, OutputFile, L"w");
    if (Out==NULL) {
        fclose(In);
        MessageBox(hDlg, L"Could not open text output file", L"File I/O", MB_OK);
        return;
    }

    while (!feof(In)) {
        char CurrentByte;
        int BitValue;
        size_t NumRead;
        CurrentByteBit = 0;

        NumRead = fread(&CurrentByte, 1, 1, In);
        if (NumRead != 1) {
            break;
        }
        // process input file bit by bit
        // while extracting use selected bit order in byte, LSB to MSB or MSB to LSB
        // variable Endian is really referring to bit order in byte not byte order in multiple byte sequence
        while (CurrentByteBit < 8) {
            // process data bit by bit in the order of the bit transmission message
            // input file is byte oriented MSB to LSB representing the bit order that the message was received
            // This does not imply any bit ordering in the message itself.
            BitValue = CurrentByte & (0x80 >> CurrentByteBit);
            if (Invert == 1) {
                if (BitValue == 0) {
                    BitValue = 1;
                }
                else {
                    BitValue = 0;
                }
            }
            // allow for message prolouge/header at the beginning of the message 
            if (PrologueSize > 0) {
                if (CurrentPrologueBit < PrologueSize) {
                    // process header bits
                    if (!BitValue) {
                        if (CurrentPrologueBit == PrologueSize - 1) {
                            fprintf(Out, "0\n");
                        } else {
                            fprintf(Out, "0,");
                        }
                    }
                    else {
                        if (CurrentPrologueBit == PrologueSize - 1) {
                            fprintf(Out, "1\n");
                        }
                        else {
                            fprintf(Out, "1,");
                        }
                    }
                    CurrentByteBit++;
                    CurrentPrologueBit++;
                    CurrentBit++;
                    continue;
                } else {
                    if (CurrentPrologueBit == PrologueSize) {
                        CurrentPrologueBit++; // do not process anymore prologue bits
                        fprintf(Out, "\n");
                    }
                }
            }

            // allow there to be a multibit header at start of a block
            if (NumBlockHeaderBits > 0) {
                if (CurrentHeaderBit < NumBlockHeaderBits) {
                    // process header bits
                    if (!BitValue) {
                        if (CurrentHeaderBit == NumBlockHeaderBits - 1) {
                            fprintf(Out, "0\n");
                        }
                        else {
                            fprintf(Out, "0,");
                        }
                    } else {
                        if (CurrentHeaderBit == NumBlockHeaderBits - 1) {
                            fprintf(Out, "1\n");
                        }
                        else {
                            fprintf(Out, "1,");
                        }
                    }
                    CurrentByteBit++;
                    CurrentHeaderBit++;
                    CurrentBit++;
                    continue;
                } else {
                    if (CurrentBlockBit == 0) {
                        fprintf(Out, "\n");
                    }
                }
            }

            // process a block of a specified number of bits
            if (CurrentBlock < NumBlocks) {
                if (CurrentBlockBit < NumBlockBodyBits) {
                    if (!BitValue) {
                        if (CurrentBlockCol == (xsize - 1)) {
                            fprintf(Out, "0\n"); // for 0
                        }
                        else {
                            fprintf(Out, "0,"); // for 0
                        }
                    } else {
                        if (CurrentBlockCol == (xsize - 1)) {
                            fprintf(Out, "1\n"); // for 1
                        }
                        else {
                            fprintf(Out, "1,"); // for 1
                        }
                    }
                    CurrentBlockCol++;
                    // assume block is in raster format with xsize bits on a line
                    if (CurrentBlockCol >= xsize) {
                        CurrentBlockCol = 0;
                    }
                    CurrentByteBit++;
                    CurrentBlockBit++;
                    CurrentBit++;
                    continue;
                }
                else {
                    // start on next block
                    fprintf(Out, "\n");
                    CurrentHeaderBit = 0;
                    CurrentBlockBit = 0;
                    CurrentBlock++;
                }
                // don't increment CurrentByteBit, the current bit is the first bit of next block
                continue;
            }
            // only get here if processing footer
            // process footer bits
            if (!BitValue) {
                if (CurrentFooterBit == 0) {
                    fprintf(Out, "0");
                } else {
                    fprintf(Out, ",0");
                }
            } else {
                if (CurrentFooterBit == 0) {
                    fprintf(Out, "1");
                }
                else {
                    fprintf(Out, ",1");
                }
            }
            CurrentFooterBit++;
            CurrentByteBit++;
            CurrentBit++;
        }
        //fprintf(Out, "\n");
    }
    fclose(In);
    fclose(Out);

    return;
}

//*******************************************************************
//
//  BitDistance
// 
// Generates a csv style file listing the position of each bit value
// of 1 and the distance from the last 1 bit
// 
// Parameters:
// HWND hDlg            Handle of calling window/dialog
// WCHAR* InputFile     Packed binary bitstream file
// WCHAR* OutputFile    CSV text output file
// int SkipSize         # of bits to skip before starting (typically the prologue)
//
//*******************************************************************
void BitDistance(HWND hDlg, WCHAR* InputFile,WCHAR* OutputFile, int SkipSize)
{
    FILE* In;
    FILE* Out;
    int CurrentBit;
    int CurrentPrologueBit;
    int CurrentByteBit;
    int CurrentPage;
    int CurrentHeaderBit;
    int CurrentPageBit;
    int CurrentPageCol;
    int CurentX;
    int CurrentY;
    errno_t ErrNum;
    int NumOnes;
    int Distance;
    int LastOne;

    ErrNum = _wfopen_s(&In, InputFile, L"rb");
    if (In==NULL) {
        MessageBox(hDlg, L"Could not open input file", L"File I/O", MB_OK);
        return;
    }

    ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
    if (Out==NULL) {
        fclose(In);
        MessageBox(hDlg, L"Could not open text output file", L"File I/O", MB_OK);
        return;
    }

    CurrentBit = 0;
    CurrentPage = 0;
    CurrentHeaderBit = 0;
    CurrentPageBit = 0;
    CurentX = 0;
    CurrentY = 0;
    CurrentPrologueBit = 0;
    CurrentPageCol = 0;
    Distance = 0;
    LastOne = -1;
    NumOnes = 0;

    while (!feof(In)) {
        char CurrentByte;
        int BitValue;
        size_t NumRead;

        CurrentByteBit = 0;

        NumRead = fread(&CurrentByte, 1, 1, In);
        if (NumRead <= 0) {
            break;
        }

        // process data bit by bit in the order of the bit transmission message
        // input file is byte oriented MSB to LSB representing the bit order that the message was received
        // This does not imply any bit ordering in the message itself.
        while (CurrentByteBit < 8) {
            if (CurrentBit < SkipSize) {
                CurrentByteBit++;
                CurrentBit++;
                continue;
            }
            else if (CurrentBit == SkipSize) {
                LastOne = CurrentBit - 1;
            }
            BitValue = CurrentByte & (0x80 >> CurrentByteBit);
            if (BitValue) {
                NumOnes++;
                Distance = CurrentBit - LastOne;
                fprintf(Out, "%5d,%5d\n", CurrentBit - SkipSize, Distance);
                LastOne = CurrentBit;
            }
            CurrentByteBit++;
            CurrentBit++;
        }
    }
    fprintf(Out, "Number of ones: %5d\n", NumOnes);
    fclose(In);
    fclose(Out);

    return;
}

//*******************************************************************
//
//  BitSequences
// 
// Generates a csv style file listing the position of each bit value
// of 1 sequence and the # of 1 bits int he sequence
// 
// Parameters:
// HWND hDlg            Handle of calling window/dialog
// WCHAR* InputFile     Packed binary bitstream file
// WCHAR* OutputFile    CSV text output file
// int SkipSize         # of bits to skip before starting (typically the prologue)
//
//*******************************************************************
void BitSequences(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int SkipSize)
{
    FILE* In;
    FILE* Out;
    errno_t ErrNum;
    int CurrentBit = 0;
    int CurrentPrologueBit = 0;
    int CurrentByteBit = 0;
    int NumOnes = 0;
    int NumZeros = 0;
    int LastBit = 0;
    int BitCounter = 0;
    int OneLength = 0;
    int ZeroLength = 0;
    int NumSequences = 0;
    int CurrentState = 0; // 0 - this is first bit in file, 1 - counting 1 sequence, 2 counting 0 sequence

    ErrNum = _wfopen_s(&In, InputFile, L"rb");
    if (In==NULL) {
        MessageBox(hDlg, L"Could not open input file", L"File I/O", MB_OK);
        return;
    }

    ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
    if (Out==NULL) {
        fclose(In);
        MessageBox(hDlg, L"Could not open text output file", L"File I/O", MB_OK);
        return;
    }

    fprintf(Out, "  Seq#,Bit, length\n");

    while (!feof(In)) {
        char CurrentByte;
        int BitValue;
        size_t NumRead;

        CurrentByteBit = 0;

        NumRead = fread(&CurrentByte, 1, 1, In);
        if (NumRead <= 0) {
            break;
        }

        // process data bit by bit in the order of the bit transmission message
        // input file is byte oriented MSB to LSB representing the bit order that the message was received
        // This does not imply any bit ordering in the message itself.

        while (CurrentByteBit < 8) {
            // skip Prologue bits
            if (CurrentBit < SkipSize) {
                CurrentByteBit++;
                CurrentBit++;
                continue;
            }

            // current bit
            BitValue = CurrentByte & (0x80 >> CurrentByteBit);

            // process this bit
            // possible states:
            //      first bit
            //      counting zeros
            //      counting ones
            // 
            switch (CurrentState) {
            case 0:
            {
                if (BitValue) {
                    CurrentState = 1;
                    NumOnes = 1;
                    OneLength = 1;
                }
                else {
                    CurrentState = 2;
                    NumZeros = 1;
                    ZeroLength = 1;
                }
                break;
            }

            case 1:
            {
                if (BitValue) {
                    // another 1
                    NumOnes++;
                    OneLength++;
                    break;
                }

                // this is a 0, so change states
                // save result of 1 sequence
                NumSequences++;
                fprintf(Out, "%6d, 1 ,%6d\n", NumSequences, OneLength);
                ZeroLength = 1;
                NumZeros++;
                CurrentState = 2;
                break;
            }

            case 2:
            {
                if (BitValue == 0) {
                    // another 0
                    NumZeros++;
                    ZeroLength++;
                    break;
                }

                // this is a 1, so change states
                // save result of 0 sequence
                NumSequences++;
                fprintf(Out, "%6d, 0 ,%6d\n", NumSequences, ZeroLength);
                OneLength = 1;
                NumOnes++;
                CurrentState = 1;
                break;
            }

            default:
                // should never get here
                break;
            };

            CurrentByteBit++;
            CurrentBit++;
        }
    }

    // save last state results
    if (CurrentState==1) {
        NumSequences++;
        fprintf(Out, "%6d, 1 ,%6d\n", NumSequences, OneLength);
    }
    else if (CurrentState == 2) {
        NumSequences++;
        fprintf(Out, "%6d, 0 ,%6d\n", NumSequences, OneLength);
    }

    fprintf(Out, "Number of ones: %5d\nNumber of Zeros: %5d\n#sequences: %5d", NumOnes, NumZeros,NumSequences);
    fclose(In);
    fclose(Out);

    return;
}

//******************************************************************************
//
// FileHexDump
//
//  Text file HEXDUMP utility
//  Input file type does not matter
// 
// HWND hDlg            Handle of calling window or dialog
// WCHAR* InputFile     InputFile is treated as a raw byte sized file (8 bit per byte)
// WCHAR* OutputFile    OutputFile is the text file that will contain the hex dump
// int xsize            how many hex coded bytes on a line
//                      0 - will result in all values on 1 line without eol in file
//  
//******************************************************************************
void FileHexDump(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile, int xsize, int SkipBytes)
{
    FILE* In;
    FILE* Out;
    errno_t ErrNum;
    int Current;
    unsigned char Pixel;
    size_t iRead;

    if (xsize < 0) {
        MessageBox(hDlg, L"xsize must be >= 0", L"xsize (pixels) setting", MB_OK);
        return;
    }

    ErrNum = _wfopen_s(&In, InputFile, L"rb");
    if (In==NULL) {
        MessageBox(hDlg, L"Could not open input file", L"File I/O", MB_OK);
        return;
    }

    ErrNum = _wfopen_s(&Out, OutputFile, L"w");
    if (Out==NULL) {
        fclose(In);
        MessageBox(hDlg, L"Could not open temp output file", L"File I/O", MB_OK);
        return;
    }

    if (fseek(In, SkipBytes, SEEK_SET) != 0) {
        fclose(In);
        MessageBox(hDlg, L"bad format, file, too small", L"File I/O", MB_OK);
        return;
    }

    Current = 0;
    while (!feof(In))
    {
        iRead = fread(&Pixel, 1, 1, In);
        if (iRead == 0) break; // eof
        if (iRead != 1) {
            MessageBox(hDlg, L"Read error, input file", L"Raw Input File", MB_OK);
            fclose(In);
            fclose(Out);
            return;
        }
        fprintf(Out, "%02x ", Pixel);
        Current++;
        if ((xsize != 0) && (Current >= xsize)) {
            fprintf(Out, "\n");
            Current = 0;
        }
    }
    if ((Current != 0) && (xsize != 0)) {
        fprintf(Out, "\n");
    }
    fclose(In);
    fclose(Out);

    return;
}

//******************************************************************************
//
// Report bit stats in binary packed bitstream file
//
// Report the number of bits set to 1 in each specified section of the 
// bitsream file
//
// Parameters:
//  HWND hDlg               handle of calling window/dialog
//  WCHAR* InputFile        Name of packed binary bitstream
//                          (file is multiple always of 8 bits)
//  WCHAR* OutputFile       Name of text output file
//  int PrologueSize        # of bits inprologue (0 - no prologue)      
//  int NumBlockHeaderBits  # of bits in block header (0 - no block header)
//  int NumBlockBodyBits    # of bits in block body (>=1)
//  int NumBlocks           # of blocks (>=1)
// 
//******************************************************************************
void BitStreamStats(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int NumBlockHeaderBits, int NumBlockBodyBits, int BlockNum)
{
    FILE* In;
    FILE* Out;
    int CurrentBit = 0;
    int CurrentPrologueBit = 0;
    int CurrentFooterBit = 0;
    int CurrentByteBit = 0;
    int CurrentBlock = 0;
    int CurrentHeaderBit = 0;
    int CurrentBlockBit = 0;
    int CurentX = 0;
    int CurrentY = 0;
    int NumberOfOnes = 0;
    int TotalBits = 0;
    errno_t ErrNum;

    ErrNum = _wfopen_s(&In, InputFile, L"rb");
    if (In==NULL) {
        MessageBox(hDlg, L"Could not open input file", L"File I/O", MB_OK);
        return;
    }

    // this is a text file
    ErrNum = _wfopen_s(&Out, OutputFile, L"w");
    if (Out==NULL) {
        fclose(In);
        MessageBox(hDlg, L"Could not open text output file", L"File I/O", MB_OK);
        return;
    }
    fprintf(Out, "Bitstream file stats\n");
    fprintf(Out, "File report settings:\nHeader size:%d\nNumber of Blocks:%d\n",
        PrologueSize, BlockNum);
    fprintf(Out, "Header size per block:%d\nBlock size:%d\n\n",
        NumBlockHeaderBits, NumBlockBodyBits);
    fprintf(Out, "Bit stats:\n");

    while (!feof(In)) {
        char CurrentByte;
        int BitValue;
        size_t NumRead;
        CurrentByteBit = 0;

        NumRead = fread(&CurrentByte, 1, 1, In);
        if (NumRead != 1) {
            break;
        }
        // process input file bit by bit
        // while extracting use selected bit order in byte, LSB to MSB or MSB to LSB
        // variable Endian is really referring to bit order in byte not byte order in multiple byte sequence
        while (CurrentByteBit < 8) {
            // process data bit by bit in the order of the bit transmission message
            // input file is byte oriented MSB to LSB representing the bit order that the message was received
            // This does not imply any bit ordering in the message itself.
            BitValue = CurrentByte & (0x80 >> CurrentByteBit);

            // allow for message prolouge/header at the beginning of the message 
            if (PrologueSize > 0) {
                if (CurrentPrologueBit < PrologueSize) {
                    // process header bits
                    if (BitValue) {
                        NumberOfOnes++;
                        TotalBits++;
                    }
                    CurrentByteBit++;
                    CurrentPrologueBit++;
                    CurrentBit++;
                    continue;
                } else {
                    if (CurrentPrologueBit == PrologueSize) {
                        fprintf(Out, "Number of bits set in prologue (header): %6d, %5.1f%%\n", NumberOfOnes,
                            100.0*(float)NumberOfOnes / (float)PrologueSize);
                        NumberOfOnes = 0;
                        CurrentPrologueBit++; // do not process anymore prologue bits
                    }
                }
            }

            // allow there to be a multibit header at start of a block
            if (NumBlockHeaderBits > 0) {
                if (CurrentHeaderBit < NumBlockHeaderBits) {
                    // process header bits
                    if (BitValue) {
                        NumberOfOnes++;
                        TotalBits++;
                     }
                    CurrentByteBit++;
                    CurrentHeaderBit++;
                    CurrentBit++;
                    continue;
                } else {
                    if (CurrentBlockBit == 0) {
                        fprintf(Out, "Number of bits set in header, block %3d: %6d, %5.1f%%\n",
                            CurrentBlock, NumberOfOnes,
                            100.0*(float)NumberOfOnes / (float)NumBlockHeaderBits);
                        NumberOfOnes = 0;
                    }
                }
            }

            // process a block of a specified number of bits
            if (CurrentBlock < BlockNum) {
                if (CurrentBlockBit < NumBlockBodyBits) {
                    if (BitValue) {
                        NumberOfOnes++;
                        TotalBits++;
                    }
                    CurrentByteBit++;
                    CurrentBlockBit++;
                    CurrentBit++;
                    continue;
                } else {
                    // start on next block
                    fprintf(Out, "Number of bits set in body, block %3d: %6d, %5.1f%%\n",
                        CurrentBlock, NumberOfOnes,
                        100.0*(float)NumberOfOnes / (float)NumBlockBodyBits);
                    NumberOfOnes = 0;
                    CurrentHeaderBit = 0;
                    CurrentBlockBit = 0;
                    CurrentBlock++;
                }
                // don't increment CurrentByteBit, the current bit is the first bit of next block
                continue;
            }
            // only get here if processing footer
            // process footer bits
            if (BitValue) {
                NumberOfOnes++;
                TotalBits++;
            }
            CurrentFooterBit++;
            CurrentByteBit++;
            CurrentBit++;
        }
        
    }
    if (CurrentFooterBit != 0) {
        fprintf(Out, "Number of bits found in footer: %6d\n", CurrentFooterBit);
        fprintf(Out, "Number of bits set in footer: %6d, %5.1f%%\n",
            NumberOfOnes, 100.0*(float)NumberOfOnes / (float)CurrentFooterBit);
    } else {
        if (CurrentBlockBit >= NumBlockBodyBits) {
            fprintf(Out, "Number of bits set in body, block %3d: %6d, %5.1f%%\n",
                CurrentBlock, NumberOfOnes,
                100.0 * (float)NumberOfOnes / (float)NumBlockBodyBits);
        } else {
            fprintf(Out, "Inconsistent size of file with selected parameters\nFilesize smaller than expected\n");
        }
        fprintf(Out, "No footer bits\n");
    }
    
    fprintf(Out, "Total number of bits set: %d\n", TotalBits);

    fclose(In);
    fclose(Out);

    return;
}

//******************************************************************************
//
// ExtractBits
// 
// Extract specifed bits from binary packed bitstream file
// to a text file
//
// Parameters:
//  HWND hDlg           Handle of calling window or dialog
//  WCHAR* InputFile    Packed Binary Btstream input file
//  CHAR* OutputFile    Name of text file to output
//  int SkipSize        Numberof bits to skip from beginning
//  int CopyBits        Number of bits to copy
//  int xsize           Number of bit to report on a line
//                      (if 0 then all bits are on one line)
// 
//******************************************************************************
void ExtractBits(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int SkipSize, int CopyBits, int xsize)
{
    FILE* In;
    FILE* Out;
    int CurrentBit = 0;
    int CurrentByteBit = 0;
    int CurrentX = 0;
    int CurrentCopyBits = 0;
    errno_t ErrNum;

    if (SkipSize<0) {
        MessageBox(hDlg, L"# of bits to skip < 0", L"Parameter error", MB_OK);
        return;
    }

    if (CopyBits <= 0) {
        MessageBox(hDlg, L"# of bits to copy <= 0", L"Parameter error", MB_OK);
        return;
    }

    ErrNum = _wfopen_s(&In, InputFile, L"rb");
    if (In==NULL) {
        MessageBox(hDlg, L"Could not open input file", L"File I/O", MB_OK);
        return;
    }

    // this is a text file
    ErrNum = _wfopen_s(&Out, OutputFile, L"w");
    if (Out==NULL) {
        fclose(In);
        MessageBox(hDlg, L"Could not open text output file", L"File I/O", MB_OK);
        return;
    }

    while (!feof(In)) {
        char CurrentByte;
        int BitValue;
        size_t NumRead;
        CurrentByteBit = 0;

        NumRead = fread(&CurrentByte, 1, 1, In);
        if (NumRead != 1) {
            break;
        }
        // process input file bit by bit
        // while extracting use selected bit order in byte, LSB to MSB or MSB to LSB
        // variable Endian is really referring to bit order in byte not byte order in multiple byte sequence
        while (CurrentByteBit < 8) {
            // process data bit by bit in the order of the bit transmission message
            // input file is byte oriented MSB to LSB representing the bit order that the message was received
            // This does not imply any bit ordering in the message itself.
            BitValue = CurrentByte & (0x80 >> CurrentByteBit);

            // Skip bits at the beginning of the message 
            if (CurrentBit< SkipSize) {
                    CurrentByteBit++;
                    CurrentBit++;
                    continue;
            }
            if (CurrentCopyBits < CopyBits) {
                // save bit
                if (xsize == 0) {
                    if (CurrentCopyBits == 0) {
                        if (!BitValue) {
                            fprintf(Out, "0");
                        }
                        else {
                            fprintf(Out, "1");
                        }
                    } else {
                        if (!BitValue) {
                            fprintf(Out, ",0");
                        }
                        else {
                            fprintf(Out, ",1");
                        }
                    }
                } else {
                    if (CurrentCopyBits % xsize == (xsize - 1)) {
                        if (!BitValue) {
                            fprintf(Out, "0\n");
                        }
                        else {
                            fprintf(Out, "1\n");
                        }
                    }
                    else {
                        if (!BitValue) {
                            fprintf(Out, "0,");
                        }
                        else {
                            fprintf(Out, "1,");
                        }
                    }
                }
            } else {
                // finshed succesfully
                fclose(In);
                fclose(Out);
                return;
            }
            // Copy the specified number of bits
            CurrentCopyBits++;
            CurrentByteBit++;
            CurrentBit++;
        }
    }

    
    if (CurrentCopyBits != CopyBits) {
        MessageBox(hDlg, L"Warning: unexpected end of input file", L"File error", MB_OK);
    }

    fclose(In);
    fclose(Out);

    return;
}

//******************************************************************************
//
// BatchBitStream2Image
// 
// Convert packed binary bitstream file to image file
// 
// Parameters:
//  HWND hDlg               Handle of calling window or dialog
//  WCHAR* InputFile        Packed Binary Btstream input file
//  CHAR* OutputFile        baseline Name of text file to output an index number is added to the filename
//                          representing the xsize value used in the file
//  int PrologueSize        # of bits to skip in prologue
//  int BlockHeaderBits     # of block header bits to skip
//  int NumBlockBodyBits    # of bits in block (each block is converted to a frame
//                          in the output image file) 
//  int BlockNum            # of block in bitstream (becomes # of frames
//                          in the output image file
//  int xsize               # of pixels in a row, starting value for batch
//  int xsizeEnd            # of pixels in a row, ending value for batch
//  int BitDepth            # of bits converted per pixel
//  int BitOrder            0 - LSB to MSB, 1 - MSB to LSB
//  int BitScale            Scale binary output, 0,1 -> 0,255
// 
//  The Ysize of the image is calculated as Ysize = NumBlockBodyBits/(xsize*bitdepth)
//
//******************************************************************************
void BatchBitStream2Image(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits, int BlockNum, int xsize, int xsizeEnd, 
    int BitDepth, int BitOrder, int BitScale, int Invert)
{
    int SaveDisplayResults;

    SaveDisplayResults = DisplayResults;
    DisplayResults = 0;

    for (int CurrentXsize = xsize; CurrentXsize <= xsizeEnd; CurrentXsize++) {
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
            MessageBox(hDlg, L"Could not creat output filename", L"Batch File I/O", MB_OK);
            DisplayResults = SaveDisplayResults;
            return;
        }
        // change the fname portion to add _kernel# 1 based
        // use Kernel+1
        WCHAR NewFname[_MAX_FNAME];
        WCHAR NewFilename[MAX_PATH];
        WCHAR BMPfilename[MAX_PATH];

        swprintf_s(NewFname, _MAX_FNAME, L"%s_%d", Fname, CurrentXsize);

        // reassemble filename
        err = _wmakepath_s(NewFilename, _MAX_PATH, Drive, Dir, NewFname, Ext);
        if (err != 0) {
            MessageBox(hDlg, L"Could not creat output filename", L"Batch File I/O", MB_OK);
            DisplayResults = SaveDisplayResults;
            return;
        }
        // create BMP of file
        if (SaveDisplayResults) {
            err = _wmakepath_s(BMPfilename, _MAX_PATH, Drive, Dir, NewFname, L".bmp");
            if (err != 0) {
                MessageBox(hDlg, L"Could not creat output filename", L"Batch File I/O", MB_OK);
                DisplayResults = SaveDisplayResults;
                return;
            }
        }

        // add CurrentXsize index to filename portion
        // reconstruct full filename
        
        err= BitStream2Image(hDlg, InputFile, NewFilename,
            PrologueSize, BlockHeaderBits, NumBlockBodyBits, BlockNum, CurrentXsize,
            BitDepth, BitOrder, BitScale, Invert);
        if (err != 1) {
            TCHAR pszMessageBuf[MAX_PATH];
            StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH,
                TEXT("Error occurred while processing batch# %d\nEror# %d\n"),
                CurrentXsize, err);
            MessageBox(hDlg, pszMessageBuf, L"Batch process Bit stream to image file", MB_OK);
            return;
        }
        if (SaveDisplayResults) {
            SaveBMP(BMPfilename, NewFilename, FALSE, AutoScaleResults);
        }
    }

    DisplayResults = SaveDisplayResults;
    return;
}

//******************************************************************************
//
// BitStream2Image
// 
// Convert packed binary bitstream file to image file
// 
// Parameters:
//  HWND hDlg               Handle of calling window or dialog
//  WCHAR* InputFile        Packed Binary Btstream input file
//  CHAR* OutputFile        Name of text file to output
//  int PrologueSize        # of bits to skip in prologue
//  int BlockHeaderBits     # of block header bits to skip
//  int NumBlockBodyBits    # of bits in block (each block is converted to a frame
//                          in the output image file) 
//  int BlockNum            # of block in bitstream (becomes # of frames
//                          in the output image file
//  int xsize               # of pixels in a row
//  int BitDepth            # of bits converted per pixel
//  int BitOrder            0 - LSB to MSB, 1 - MSB to LSB
//  int BitScale            Scale binary output, 0,1 -> 0,255
// 
//  The Ysize of the image is calculated as Ysize = NumBlockBodyBits/(xsize*bitdepth)
//
//******************************************************************************
int BitStream2Image(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile,
    int PrologueSize, int BlockHeaderBits, int NumBlockBodyBits, int BlockNum, int xsize,
    int BitDepth, int BitOrder, int BitScale, int Invert)
{
    FILE* In;
    FILE* OutRaw;
    int CurrentBit=0;
    int CurrentPrologueBit=0;
    int CurrentByteBit;
    int CurrentPage=0;
    int CurrentHeaderBit=0;
    int CurrentPageBit=0;
    int CurrentPageCol=0;
    int CurentX=0;
    int CurrentY=0;
    int CurrentPixelBit;
    errno_t ErrNum;
    union PIXEL {
        unsigned char bit8;
        unsigned short bit16;
        long bit32;
    } Pixel;

    if (xsize <= 0) {
        MessageBox(hDlg, L"x size must be >= 1", L"File I/O", MB_OK);
        return 0;
    }

    if (NumBlockBodyBits <= 0) {
        MessageBox(hDlg, L"# bits in block >= 1", L"File I/O", MB_OK);
        return 0;
    }

    if (BitDepth <= 0 || BitDepth > 32) {
        MessageBox(hDlg, L"1 <= Image bit depth <= 32", L"File I/O", MB_OK);
        return 0;
    }

    if (BitDepth != 1 && BitScale) {
        MessageBox(hDlg, L"Scale Binary can only be used if Image bit depth is 1", L"File I/O", MB_OK);
        return 0;
    }

    ErrNum = _wfopen_s(&In, InputFile, L"rb");
    if (In==NULL) {
        MessageBox(hDlg, L"Could not open input file", L"File I/O", MB_OK);
        return -2;
    }

    ErrNum = _wfopen_s(&OutRaw, OutputFile, L"wb");
    if (OutRaw==NULL) {
        fclose(In);
        MessageBox(hDlg, L"Could not open raw output file", L"File I/O", MB_OK);
        return -2;
    }

    // Initialize image file header
    IMAGINGHEADER ImgHeader;

    ImgHeader.Endian = (short)-1;  // PC format
    ImgHeader.HeaderSize = (short) sizeof(IMAGINGHEADER);
    ImgHeader.ID = (short)0xaaaa;
    ImgHeader.Version = (short)1;
    ImgHeader.NumFrames = (short)BlockNum;
    if (BitDepth <= 8) {
        ImgHeader.PixelSize = (short)1;
    } else if (BitDepth <= 16) {
        ImgHeader.PixelSize = (short)2;
    } else {
        ImgHeader.PixelSize = (short)4;
    }
    ImgHeader.Xsize = xsize;
    ImgHeader.Ysize = (NumBlockBodyBits / (xsize* BitDepth));
    ImgHeader.Padding[0] = 0;
    ImgHeader.Padding[1] = 0;
    ImgHeader.Padding[2] = 0;
    ImgHeader.Padding[3] = 0;
    ImgHeader.Padding[4] = 0;
    ImgHeader.Padding[5] = 0;

    // write header to file
    fwrite(&ImgHeader, sizeof(IMAGINGHEADER),1, OutRaw);

    CurrentPixelBit = BitDepth - 1;
    Pixel.bit32 = 0;

    while (!feof(In)) {
        char CurrentByte;
        int BitValue;
        size_t NumRead;
        CurrentByteBit = 0;

        NumRead = fread(&CurrentByte, 1, 1, In);
        if (NumRead <= 0) {
            break;
        }

        // process data bit by bit in the order of the bit transmission message
        // input file is byte oriented MSB to LSB representing the bit order that the message was received
        // This does not imply any bit ordering in the message itself.
        while (CurrentByteBit < 8) {
            // MSB to LSB in input data file 
            BitValue = CurrentByte & (0x80 >> CurrentByteBit);
            if (Invert==1) {
                if (BitValue == 0) {
                    BitValue = 1;
                }
                else {
                    BitValue = 0;
                }
            }
            // Prologue block processing
            // skip prologue, PrologueSize bits
            if (PrologueSize > 0) {  // 0 based index
                if (CurrentPrologueBit < PrologueSize) {
                    CurrentByteBit++;
                    CurrentPrologueBit++;
                    CurrentBit++;
                    continue;
                }
                else {
                    if (CurrentPrologueBit == PrologueSize) {
                        // mark so don't process in this block again
                        CurrentPrologueBit++;
                    }
                }
            }
            // end of prologque processing

            // Page header block processing, one header per page
            // skip page headers
            if (BlockHeaderBits > 0) {
                if (CurrentHeaderBit < BlockHeaderBits) {
                    // process header bits
                    CurrentByteBit++;
                    CurrentHeaderBit++;
                    CurrentBit++;
                    continue;
                }
            }
            // end of page header block processing

            // these bits belong in page/tile/image

            if (CurrentPageBit < NumBlockBodyBits) {
                // this assumes that there could be 1 to 16 bits per pixel in the resulting image
                if (BitValue) {
                    // add to image byte if if bit depth is <=8, add to image int16 is bit depth >8
                    // right now don't worry about pixel bit depth > 16
                    // PixelByte, Pixel are a union.

                    // data is always stored in resulting image MSB to LSB in byte/short
                    // since Pixel value starts as 0 only need to set bits not clear them
                    // Decode using Endian flag for LSB-MSB or MSB-LSB order
                    if (BitDepth <= 8) {
                        if (BitOrder) {
                            // bit stream order is MSB to LSB
                            Pixel.bit8 = Pixel.bit8 | (1 << CurrentPixelBit);
                        }
                        else {
                            // bit stream order is LSB to MSB
                            Pixel.bit8 = Pixel.bit8 | (1 << ((BitDepth - 1) - CurrentPixelBit));
                        }
                    }
                    else if (BitDepth <= 16) {
                        if (BitOrder) {
                            // bit stream order is MSB to LSB
                            Pixel.bit16 = Pixel.bit16 | (1 << CurrentPixelBit);
                        }
                        else {
                            // bit stream order is LSB to MSB
                            Pixel.bit16 = Pixel.bit16 | (1 << ((BitDepth - 1) - CurrentPixelBit));
                        }
                    }
                    else {
                        if (BitOrder) {
                            // bit stream order is MSB to LSB
                            Pixel.bit32 = Pixel.bit32 | (1 << CurrentPixelBit);
                        }
                        else {
                            // bit stream order is LSB to MSB
                            Pixel.bit32 = Pixel.bit32 | (1 << ((BitDepth - 1) - CurrentPixelBit));
                        }
                    }
                }
                CurrentPixelBit--;
                if (CurrentPixelBit < 0) {
                    if (BitDepth <= 8) {
                        if (BitScale && Pixel.bit8 && (BitDepth == 1)) {
                            Pixel.bit8 = 255;
                        }
                        fwrite(&Pixel.bit8, 1, 1, OutRaw);
                    }
                    else if (BitDepth <= 16) {
                        fwrite(&Pixel.bit16, 2, 1, OutRaw);
                    }
                    else {
                        fwrite(&Pixel.bit32, 4, 1, OutRaw);

                    }
                    Pixel.bit16 = 0;
                    CurrentPixelBit = BitDepth - 1;
                }
                CurrentPageCol++;
                if (CurrentPageCol >= xsize) {
                    CurrentPageCol = 0;
                }
                CurrentByteBit++;
                CurrentPageBit++;
                CurrentBit++;
                continue;
            }
            else {
                // start on next page
                CurrentHeaderBit = 0;
                CurrentPageBit = 0;
                CurrentPixelBit = BitDepth - 1;
                CurrentPage++;
                if (CurrentPage==BlockNum) {
                    fclose(In);
                    fclose(OutRaw);
                    if (DisplayResults) {
                        DisplayImage(OutputFile);
                    }
                    return 1;
                }
            }
        }
    }
    fclose(In);
    fclose(OutRaw);
    if (DisplayResults) {
        DisplayImage(OutputFile);
    }
    return 1;
}

//*******************************************************************
//
// ConvertText2BitStream
// 
// Convert textfile to a packed BitStream binary file
// 
// Parameters:
//  HWND hDlg               handle of calling window/dialog
//  WCHAR* InputFile        text file with space delmited list of values
//                          value < 0 , cause error to be returned
//                          value = 0 is taken as bit with value 0
//                          value > 0 is taken as bit with value 1
//                          (file is multiple always of 8 bits)
//  WCHAR* OutputFile       Packed Binary bit stream file
//
//*******************************************************************
int ConvertText2BitStream(HWND hDlg, WCHAR* InputFile, WCHAR* OutputFile)
{
    FILE* In;
    FILE* Out;
    int BitNumber;
    int BitValue;
    int ByteValue;
    int iRes;
    int TotalBits;
    int TotalOneBits;
    int TotalBytes;
    errno_t ErrNum;

    BitNumber = 0;
    ByteValue = 0;
    TotalBits = 0;
    TotalOneBits = 0;
    TotalBytes = 0;

    ErrNum = _wfopen_s(&In, InputFile, L"r");
    if (In == NULL) {
        MessageBox(hDlg, L"Could not open input file", L"File I/O", MB_OK);
        return -2;
    }

    ErrNum = _wfopen_s(&Out, OutputFile, L"wb");
    if (Out == NULL) {
        fclose(In);
        MessageBox(hDlg, L"Could not open raw output file", L"File I/O", MB_OK);
        return -2;
    }

    while (!feof(In)) {
        iRes = fscanf_s(In, "%d", &BitValue);
        if (iRes != 1) {
            fclose(Out);
            fclose(In);
            return -3;
        }
        if (BitValue < 0) {
            fclose(Out);
            fclose(In);
            return -3;
        }
        if (BitValue > 0) {
            ByteValue = ByteValue | (0x01 << BitNumber);
            TotalOneBits++;
        }
        TotalBits++;
        BitNumber++;
        if (BitNumber >= 8) {
            fwrite(&ByteValue, 1, 1, Out);
            ByteValue = 0;
            BitNumber = 0;
            TotalBytes++;
        }
    }

    if (BitNumber != 0) {
        fwrite(&ByteValue, 1, 1, Out);
    }

    TCHAR pszMessageBuf[MAX_PATH];
    StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH, TEXT("Bitsream properties\n# of bits: %d\n# of set bits: %d\nBytes writtten: %d"),
        TotalBits, TotalOneBits, TotalBytes);
    MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);

    fclose(In);
    fclose(Out);
    return 1;
}

//*******************************************************************
//
// ExtractSPP
// 
// Extract SPP from input stream
// The input to this is a SPP binary stream.  The stream SPP headers 
// are decoded.  IDLE packets are discarded. If SaveSPP is enabled then
// a summary file of all the non IDLE packets is saved without
// the data fields si saved.  The target APID SPPs in the summary file
// have *** at the end of end line.  If Strict flag is set then the primary
// header must be in complete conformance with the ESCC standards, ECSS-E-ST-70-41C
// or ECSS-E-70-41A.  A file for the spcified APID is saved.  This contains each
// target APID SPPs in the source file including the Primary header field decoded
// along with the packet data field which contains the secondary header, user data,
// and CRC/checksum in HEX coded bytes.
// 
// Parameters:
//  HWND hDlg               handle of calling window/dialog
//  WCHAR* InputFile        Packed Binary bit stream file
//  WCHAR* OutputFile       CSV file with SPps of the selected APID
//  int APID                APID to look for
//  int SkipBytes           Number of byte in input file to initially skip
//  int SecondaryHeaderSize The number of bytes to skip (the secondary header)
//                          at the start of the data field.
//  int strict              0 - only basic check for conforming to standards
//                          1 - must comply with the packet primary header standards
//  int SaveSPP             0 - do not save summary file
//                          1 - SPP summary file
//
//*******************************************************************
int ExtractSPP(HWND hDlg, WCHAR* InputFile, WCHAR* APIDoutputFile, WCHAR* OutputFile,
    int APID, int SkipBytes, int SecondaryHeaderSize,
    int Strict, int SaveSPP) {

    SPP_UNPACKED_PRIMARY_HEADER PriHeader;
    SPP_PRIMARY_HEADER PackedPriHeader;
    FILE* In = NULL;
    FILE* Out = NULL;
    FILE* OutAPID = NULL;
    int iRes;
    int NumPackets = 0;
    int NumIdlePackets = 0;
    int NumTMpackets = 0;
    int NumTCpackets = 0;
    int NumAPIDmatches = 0;
    int TotalBytes = 0;
    errno_t ErrNum;

    // open input file
    ErrNum = _wfopen_s(&In, InputFile, L"rb");
    if (In == NULL) {
        MessageBox(hDlg, L"Could not open input file", L"File I/O", MB_OK);
        return -2;
    }
    if (fseek(In, SkipBytes, SEEK_SET) != 0) {
        fclose(In);
        MessageBox(hDlg, L"bad format, file, too small\nNot likely SPP binary file", L"File I/O", MB_OK);
        return -3;
    }

    if (SaveSPP) {
        // open output summary file
        ErrNum = _wfopen_s(&Out, OutputFile, L"w");
        if (Out == NULL) {
            fclose(In);
            MessageBox(hDlg, L"Could not open summary output file", L"File I/O", MB_OK);
            return -2;
        }
        fprintf(Out, "   SPP#, PVN, Type, SHflag,   APID, SeqFlg, SeqCount, DataLen, Data Field ->\n");
    }

    ErrNum = _wfopen_s(&OutAPID, APIDoutputFile, L"w");
    if (OutAPID == NULL) {
        if (SaveSPP) {
            fclose(Out);
        }
        fclose(In);
        MessageBox(hDlg, L"Could not open APID output file", L"File I/O", MB_OK);
        return -2;
    }

    fprintf(OutAPID, "   SPP#, PVN, Type, SHflag,   APID, SeqFlg, SeqCount, DataLen, Data Field ->\n");

    while (!feof(In)) {
        iRes = fread(&PackedPriHeader, sizeof(PackedPriHeader), 1, In);
        if (iRes != 1) {
            if (NumPackets == 0) {
                MessageBox(hDlg, L"Input file is wrong type", L"File I/O error", MB_OK);
                if (SaveSPP) {
                    fclose(Out);
                }
                fclose(OutAPID);
                fclose(In);
                return -3;
            }
            // generate summary
            TCHAR pszMessageBuf[MAX_PATH];
            StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH,
                TEXT("Processed:\n# of total packets: %d\n# idle packets: %d\n# of telemtry packets: %d\n# of telecommand packets: %d\n# matching APID packets: %d\nTotal bytes processeed: %d"),
                NumPackets, NumIdlePackets,NumTMpackets, NumTCpackets, NumAPIDmatches, TotalBytes);
            MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
            if (SaveSPP) {
                fclose(Out);
            }
            fclose(In);
            fclose(OutAPID);
            return 1;
        }

        // SPP header read
        TotalBytes += 6;

        PackedPriHeader.ID = ByteSwap(PackedPriHeader.ID);
        PackedPriHeader.SEQ = ByteSwap(PackedPriHeader.SEQ);
        PackedPriHeader.DataLength = ByteSwap(PackedPriHeader.DataLength);

        iRes = DecodeSPP(&PackedPriHeader, &PriHeader,Strict);
        if (iRes == 0) {
            MessageBox(hDlg, L"Invalid SPP encountered", L"File I/O", MB_OK);
            TCHAR pszMessageBuf[MAX_PATH];
            StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH,
                TEXT("Processed:\n# of total packets: %d\n# idle packets: %d\n# of telemtry packets: %d\n# of telecommand packets: %d\n# matching APID packets: %d\nTotal bytes processeed: %d"),
                NumPackets, NumIdlePackets, NumTMpackets, NumTCpackets, NumAPIDmatches, TotalBytes);
            MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
            if (SaveSPP) {
                fclose(Out);
            }
            fclose(OutAPID);
            fclose(In);
            return 0;
        }

        // add SPP data field length, this includes secondary header, user data, and CRC/checksum
        TotalBytes += PriHeader.DataLength;

        NumPackets++;
        if (PriHeader.Type == 1) {
            NumTCpackets++;
        }
        else {
            NumTMpackets++;
        }
        if (PriHeader.APID == 0x7ff) {
            // This is an IDLE packet,skip
            NumIdlePackets++;
            if (fseek(In, PriHeader.DataLength, SEEK_CUR) != 0) {
                if (SaveSPP) {
                    fclose(Out);
                }
                fclose(OutAPID);
                fclose(In);
                MessageBox(hDlg, L"Incomplete packet encountered\nEOF before end of packet", L"File I/O", MB_OK);
                TCHAR pszMessageBuf[MAX_PATH];
                StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH,
                    TEXT("Processed:\n# of total packets: %d\n# idle packets: %d\n# of telemtry packets: %d\n# of telecommand packets: %d\n# matching APID packets: %d\nTotal bytes processeed: %d"),
                    NumPackets, NumIdlePackets, NumTMpackets, NumTCpackets, NumAPIDmatches, TotalBytes);
                MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
                return -3;
            }
            continue;
        }
        if (SaveSPP) {
            fprintf(Out, "%7d,   %1d,    %1d,      %1d, 0x%04x,      %1d,    %5d,   %5d", NumPackets,
                PriHeader.PVN, PriHeader.Type, PriHeader.SecHeaderFlag, PriHeader.APID,
                PriHeader.SeqFlag, PriHeader.SeqCount, PriHeader.DataLength);
        }

        if (APID != PriHeader.APID) {
            if (SaveSPP) {
                fprintf(Out, "\n");
            }
            if (fseek(In, PriHeader.DataLength, SEEK_CUR) != 0) {
                if (SaveSPP) {
                    fclose(Out);
                }
                fclose(OutAPID);
                fclose(In);
                MessageBox(hDlg, L"bad format, file, too small", L"File I/O", MB_OK);
                TCHAR pszMessageBuf[MAX_PATH];
                StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH,
                    TEXT("Processed:\n# of total packets: %d\n# idle packets: %d\n# of telemtry packets: %d\n# of telecommand packets: %d\n# matching APID packets: %d\nTotal bytes processeed: %d"),
                    NumPackets, NumIdlePackets, NumTMpackets, NumTCpackets, NumAPIDmatches, TotalBytes);
                MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
                return -3;
            }
            continue;
        }
        // found correct APID
        NumAPIDmatches++;
        if (SaveSPP) {
            fprintf(Out, ", ***\n"); // flag target APID SPP in summary file
        }

        fprintf(OutAPID, "%7d,   %1d,    %1d,      %1d, 0x%04x,      %1d,    %5d,   %5d", NumPackets,
            PriHeader.PVN, PriHeader.Type, PriHeader.SecHeaderFlag, PriHeader.APID,
            PriHeader.SeqFlag, PriHeader.SeqCount, PriHeader.DataLength);

        // read data field
        {
            unsigned char* DataField;
            int iRes;

            DataField = new unsigned char[PriHeader.DataLength];
            if (DataField == NULL) {
                delete[] DataField;
                if (SaveSPP) {
                    fclose(Out);
                }
                fclose(OutAPID);
                fclose(In);
                MessageBox(hDlg, L"Memory allocation failure", L"Program error", MB_OK);
                TCHAR pszMessageBuf[MAX_PATH];
                StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH,
                    TEXT("Processed:\n# of total packets: %d\n# idle packets: %d\n# of telemtry packets: %d\n# of telecommand packets: %d\n# matching APID packets: %d\nTotal bytes processeed: %d"),
                    NumPackets, NumIdlePackets, NumTMpackets, NumTCpackets, NumAPIDmatches, TotalBytes);
                MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
                return -3;
            }
            
            iRes = fread(DataField, 1, PriHeader.DataLength, In);
            if (iRes != PriHeader.DataLength) {
                delete[] DataField;
                if (SaveSPP) {
                    fclose(Out);
                }
                fclose(OutAPID);
                fclose(In);
                MessageBox(hDlg, L"Packet data field too short", L"File I/O", MB_OK);
                TCHAR pszMessageBuf[MAX_PATH];
                StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH,
                    TEXT("Processed:\n# of total packets: %d\n# idle packets: %d\n# of telemtry packets: %d\n# of telecommand packets: %d\n# matching APID packets: %d\nTotal bytes processeed: %d"),
                    NumPackets, NumIdlePackets, NumTMpackets, NumTCpackets, NumAPIDmatches, TotalBytes);
                MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);
                return -3;
            }

            // skip sepcifed number of bytes at the beginning of the data field, nominally the secondary header
            for (int i = SecondaryHeaderSize; i < PriHeader.DataLength; i++) {
                fprintf(OutAPID, ", %02x", (int)DataField[i]);
            }
            fprintf(OutAPID, "\n");

            delete[] DataField;
        }
    }

    TCHAR pszMessageBuf[MAX_PATH];
    StringCchPrintf(pszMessageBuf, (size_t)MAX_PATH,
        TEXT("Processed:\n# of total packets: %d\n# idle packets: %d\n# of telemtry packets: %d\n# of telecommand packets: %d\n# matching APID packets: %d\nTotal bytes processeed: %d"),
        NumPackets, NumIdlePackets, NumTMpackets, NumTCpackets, NumAPIDmatches, TotalBytes);
    MessageBox(hDlg, pszMessageBuf, L"Completed", MB_OK);

    if (SaveSPP) {
        fclose(Out);
    }
    fclose(OutAPID);
    fclose(In);

    return 1;
}

//*******************************************************************
//
//  DecodeSPP
// 
// Private function for ExtractSPP()
//
//*******************************************************************
int DecodeSPP(SPP_PRIMARY_HEADER* PackedPriHeader, SPP_UNPACKED_PRIMARY_HEADER* PriHeader,
            int Strict)
{
    PriHeader->PVN = (PackedPriHeader->ID & 0xe000) >> 13 ;

    PriHeader->Type = (PackedPriHeader->ID & 0x1000) >>12;

    PriHeader->SecHeaderFlag = (PackedPriHeader->ID & 0x0800) >> 11;

    PriHeader->APID = PackedPriHeader->ID & 0x7ff;

    PriHeader->SeqFlag = (PackedPriHeader->SEQ & 0xc000) >>14;

    PriHeader->SeqCount = PackedPriHeader->SEQ & 0x3fff;

    PriHeader->DataLength = PackedPriHeader->DataLength+1; // need to increment by 1 to be accurate

    if (PriHeader->PVN != 0) {
        return 0;
    }

    // strict application of ECSS-E-ST-70-41C or otherwise only apply CCSDS 133.0-B-1
    if (!Strict) {
        return 1;
    }

    // flag incorrect settings in SPP primary header

    // Almost all telemtry packets have a packet secondary header, IDLE and time packets do not
    if (PriHeader->Type == 0 && PriHeader->SecHeaderFlag != 1) {
        // IDLE packets and spacecraft time packets do not have secondary header
        if (PriHeader->APID != 0x7ff && PriHeader->APID !=0x000) {
            return 0;
        }
    }
    if (PriHeader->SeqFlag != 3) {
        return 0;
    }

    return 1;
}

//*******************************************************************
//
//  ByteSwap
//
//*******************************************************************
uint16_t ByteSwap(uint16_t Value) {
    uint16_t LowByte;
    uint16_t HighByte;
    uint16_t NewValue;
    LowByte = Value & 0x00ff;
    HighByte = (Value & 0xff00) >> 8;

    NewValue = HighByte | (LowByte << 8);
    return NewValue;
}