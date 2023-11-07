//******************************************************************************
//
// CalculateReOrder
// 
// This calculates the origin address in the input image
// of the specified pixel in the output space.
// 
// Parameters:
// 
//  return value:
//  Address ppixel in the input image space
//
//	ResizeFLag value   -1	Error, invalid algorithm specified
//						0	Don't resize image
//						1	resize image
//
// V1.2.6.1 2023-09-24	Initial release
// V1.2.7.1 2023-10-01	No actual changes, skeleton added for addtional 5 algorithms
// V1.2.8.1 2023-10-18	Added 4 new algorithms to reorder using algorithm
//							Incremental row shoft with wrap aorund
//							n Stripes
//							Shift (rotate) Rows
//							Shift (rotate) Columns
// V1.2.9.1	2023-10-31	Added new algorithm for reordering
//							MxN Block [P1,P2] output decom
// V1.2.10.1 2023-11-5  Added new algorithm for reordering
//							Added split image left/right                      
// 
//*******************************************************************************
#include "CalculateReOrder.h"

int CalcQuad_UL_UR_LL_LR_l2r_t2b(int x, int y, int Xsize, int Ysize);
int CalcQuad_UL_LL_UR_LR_l2r_t2b(int x, int y, int Xsize, int Ysize);
int CalcQuad_LL_UL_LR_UR_l2r_t2b(int x, int y, int Xsize, int Ysize);
int CalcQuad_LL_LR_UL_UR_l2r_t2b(int x, int y, int Xsize, int Ysize);
int CalcQuad_UL_UR_LL_LR_center_out(int x, int y, int Xsize, int Ysize);
int CalcRotateRow(int x, int y, int Xsize, int Ysize, int P1);
int StripesP1(int x, int y, int Xsize, int Ysize, int P1);
int CalcShiftRow(int x, int y, int Xsize, int Ysize, int P1);
int CalcShiftCol(int x, int y, int Xsize, int Ysize, int P1);
int NewAlg5(int x, int y, int Xsize, int Ysize);
int BlockMxNdecom(int x, int y, int Xsize, int Ysize, int P1, int P2);
int BlockMxNrecom(int x, int y, int Xsize, int Ysize, int P1, int P2);
int SplitLeftRight(int x, int y, int Xsize, int Ysize, int P1);

//******************************************************************************
//
// CalculateReOrder
// 
// This calculates the origin address in the input image
// of the specified pixel in the output space.
// 
// Parameters:
// 
//  return value:
//  Address ppixel in the input image space
//
//	ResizeFLag value   -1	Error, invalid algorithm specified
//						0	Don't resize image
//						1	resize image
//
//*******************************************************************************
int CalculateReOrder(int x, int y, int Xsize, int Ysize, int Algorithm, int P1, int P2, int P3, int* ResizeFlag)
{
	int Address;

	Address = 0;
	(*ResizeFlag) = 0;
	switch (Algorithm) {

	case 0:
		(*ResizeFlag) = 1;
		// no reorder, linear adress calculation
		Address = x + (y * Xsize * Ysize);
		break;

	case 1:
		// Quadrant UL, UR, LL, LR, left to right, top to bottom, no resize
		Address = CalcQuad_UL_UR_LL_LR_l2r_t2b(x,y,Xsize,Ysize);
		break;

	case 2:
		// Quadrant UL,LL,UR,LR, left to right, top to bottom, no resize
		Address = CalcQuad_UL_LL_UR_LR_l2r_t2b(x, y, Xsize, Ysize);
		break;

	case 3:
		// Quadrant LL,UL,LR,UR, left to right, top to bottom, no resize
		Address = CalcQuad_LL_UL_LR_UR_l2r_t2b(x, y, Xsize, Ysize);
		break;

	case 4:
		//Quadrant LL,LR,UL,UR, left to right, top to bottom, no resize
		Address = CalcQuad_LL_LR_UL_UR_l2r_t2b(x, y, Xsize, Ysize);
		break;

	case 5:
		// Quadrant UL, UR, LL, LR,  center out, no resize
		Address = CalcQuad_UL_UR_LL_LR_center_out(x, y, Xsize, Ysize);
		break;

	case 6:
		// Incrmentally rotate rows by P1
		Address = CalcRotateRow(x, y, Xsize, Ysize, P1);
		break;

	case 7:
		// Image in stripes, P1 wide
		if (Xsize % P1 != 0) {
			(*ResizeFlag) = -1; // flag error
			break;
		}
		Address = StripesP1(x, y, Xsize, Ysize, P1);
		break;

	case 8: 
		// circular shift rows by P1
		Address = CalcShiftRow(x, y, Xsize, Ysize, P1);
		break;
		break;

	case 9: 
		// circular shift rows by P1
		Address = CalcShiftCol(x, y, Xsize, Ysize, P1);
		break;

	case 10: // MxN Block output format decomutation
		// Horizontal block size = M = P1
		// Vertical block size = N = P2
		(*ResizeFlag) = 1;
		Address = BlockMxNdecom(x, y, Xsize, Ysize, P1, P2);
		break;

	case 11: // Split image left/right
		if (P1 < 1 || P1 > (Xsize / 2)) {
			(*ResizeFlag) = -1;
			return 0;
		}

		(*ResizeFlag) = 1;
		Address = SplitLeftRight(x, y, Xsize, Ysize, P1);
		break;

	case 12: // not yet implemented
		(*ResizeFlag) = -1;
		Address = NewAlg5(x, y, Xsize, Ysize);
		break;

	default:
		(*ResizeFlag) = -1;
		break;
	}

	return Address;
}

//******************************************************************************
//
// CalcQuad_UL_UR_LL_LR_l2r_t2b
// 
//******************************************************************************
int CalcQuad_UL_UR_LL_LR_l2r_t2b(int x, int y, int Xsize, int Ysize) {
	int Address;

	if (x < (Xsize / 2)) {
		if (y < (Ysize / 2)) {
			// UL quadrant, 0
			Address = x * 4 + (y * Xsize * 2) + 0;
		}
		else {
			// LL quadrant, 2
			Address = x * 4 + ((y - Ysize / 2) * Xsize * 2) + 2;
		}
	}
	else {
		if (y < (Ysize / 2)) {
			// UR quadrant, 1
			Address = (x-Xsize/2) * 4 + (y * Xsize * 2) + 1;
		}
		else {
			// LR quadrant, 3
			Address = (x-Xsize/2) * 4 + ((y-Ysize/2) * Xsize * 2) + 3;
		}
	}
	return Address;
}

//******************************************************************************
//
// CalcQuad_UL_LL_UR_LR_l2r_t2b
// 
//******************************************************************************
int CalcQuad_UL_LL_UR_LR_l2r_t2b(int x, int y, int Xsize, int Ysize) {
	int Address;

	if (x < (Xsize / 2)) {
		if (y < (Ysize / 2)) {
			// UL quadrant, 0
			Address = x * 4 + (y * Xsize * 2) + 0;
		}
		else {
			// LL quadrant, 1
			Address = x * 4 + ((y - Ysize / 2) * Xsize * 2) + 1;
		}
	}
	else {
		if (y < (Ysize / 2)) {
			// UR quadrant, 2
			Address = (x - Xsize / 2) * 4 + (y * Xsize * 2) + 2;
		}
		else {
			// LR quadrant, 3
			Address = (x - Xsize / 2) * 4 + ((y - Ysize / 2) * Xsize * 2) + 3;
		}
	}
	return Address;
}

//******************************************************************************
//
// CalcQuad_LL_UL_LR_UR_l2r_t2b
// 
//******************************************************************************
int CalcQuad_LL_UL_LR_UR_l2r_t2b(int x, int y, int Xsize, int Ysize) {
	int Address;

	if (x < (Xsize / 2)) {
		if (y < (Ysize / 2)) {
			// UL quadrant, 1
			Address = x * 4 + (y * Xsize * 2) + 1;
		}
		else {
			// LL quadrant, 0
			Address = x * 4 + ((y - Ysize / 2) * Xsize * 2) + 0;
		}
	}
	else {
		if (y < (Ysize / 2)) {
			// UR quadrant, 3
			Address = (x - Xsize / 2) * 4 + (y * Xsize * 2) + 3;
		}
		else {
			// LR quadrant, 2
			Address = (x - Xsize / 2) * 4 + ((y - Ysize / 2) * Xsize * 2) + 2;
		}
	}
	return Address;
}

//******************************************************************************
//
// CalcQuad_LL_LR_UL_UR_l2r_t2b
// 
//******************************************************************************
int CalcQuad_LL_LR_UL_UR_l2r_t2b(int x, int y, int Xsize, int Ysize) {
	int Address;

	if (x < (Xsize / 2)) {
		if (y < (Ysize / 2)) {
			// UL quadrant, 2
			Address = x * 4 + (y * Xsize * 2) + 2;
		}
		else {
			// LL quadrant, 0
			Address = x * 4 + ((y - Ysize / 2) * Xsize * 2) + 0;
		}
	}
	else {
		if (y < (Ysize / 2)) {
			// UR quadrant, 3
			Address = (x - Xsize / 2) * 4 + (y * Xsize * 2) + 3;
		}
		else {
			// LR quadrant, 1
			Address = (x - Xsize / 2) * 4 + ((y - Ysize / 2) * Xsize * 2) + 1;
		}
	}
	return Address;
}

//******************************************************************************
//
// CalcQuad_UL_UR_LL_LR_center_out
// 
//******************************************************************************
int CalcQuad_UL_UR_LL_LR_center_out(int x, int y, int Xsize, int Ysize) {
	int Address;

	if (x < (Xsize / 2)) {
		if (y < (Ysize / 2)) {
			// UL quadrant, 0
			Address = ((Xsize / 2 - 1) - x) * 4 + (((Ysize / 2 - 1) - y) * Xsize * 2) + 0;
		}
		else {
			// LL quadrant, 2
			Address = ((Xsize / 2 - 1) - x) * 4 + ((y - Ysize / 2) * Xsize * 2) + 2;
		}
	}
	else {
		if (y < (Ysize / 2)) {
			// UR quadrant, 1
			Address = (x - Xsize / 2) * 4 + (((Ysize / 2 - 1) - y) * Xsize * 2) + 1;
		}
		else {
			// LR quadrant, 3
			Address = (x - Xsize / 2) * 4 + ((y - Ysize / 2) * Xsize * 2) + 3;
		}
	}

	return Address;
}

//******************************************************************************
//
// CalcRotateRow
// 
//******************************************************************************
int CalcRotateRow(int x, int y, int Xsize, int Ysize, int P1) {
	int Address;

	x = x + (P1 * y);
	
	while (x < 0) {
		x = x + Xsize;
	}
	
	while (x >= Xsize) {
		x = x - Xsize;
	}

	Address = x + (y * Xsize);
	return Address;
}

//******************************************************************************
//
// StripesP1
// 
//******************************************************************************
int StripesP1(int x, int y, int Xsize, int Ysize, int P1) {
	int Address;
	int Stripe;
	int StripeWidth;
	int StripePixel;
	int Offset;

	StripeWidth = Xsize / P1;
	Stripe = x / StripeWidth;
	StripePixel = x % StripeWidth;

	Offset = StripePixel * P1 + Stripe;

	Address = Offset + (y * Xsize);

	return Address;
}

//******************************************************************************
//
// CalcShiftRow
// 
//******************************************************************************
int CalcShiftRow(int x, int y, int Xsize, int Ysize,int P1) {
	int Address;

	x = x + P1;

	while (x < 0) {
		x = x + Xsize;
	}

	while (x >= Xsize) {
		x = x - Xsize;
	}

	Address = x + (y * Xsize);
	return Address;	return Address;
}

//******************************************************************************
//
// CalcShiftCol
// 
//******************************************************************************
int CalcShiftCol(int x, int y, int Xsize, int Ysize, int P1) {
	int Address;

	y = y + P1;

	while (y < 0) {
		y = y + Ysize;
	}

	while (y >= Ysize) {
		y = y - Ysize;
	}

	Address = x + (y * Xsize);
	return Address;	return Address;
}

//******************************************************************************
//
// BlockMxNdecom
// 
//******************************************************************************
int BlockMxNdecom(int x, int y, int Xsize, int Ysize, int P1, int P2) {
	int Address;
	// M = P1
	// N = P2
	int BlockNum;
	int BlockSize;
	int Xblock;
	int Yblock;
	int BlockPos;
	int NewAddress;

	// Note:  Code is deliberately not minimized
	// It is meant to explain the transform math
	BlockSize = P1 * P2;	// number of pixels in block
	Xblock = x/P1;			// This pixel (x,y) is in this horizontal block
	Yblock = y/P2;			// This pixel (x,y) is in this vertical block
	BlockNum = Xblock + Yblock * (Xsize / P1);  // This pixel (x,y) is in this the linear block number
	BlockPos = x%P1 + (y%P2) * P1;  // This pixel (x,y) is in this linear position within a block
	NewAddress = BlockNum * BlockSize + BlockPos; // The calculated address of were this pixel(x,y)
									// is in the xsize by ysize image
	Address = NewAddress;

	return Address;
}

//******************************************************************************
//
// SplitLeftRight
// 
//******************************************************************************
int SplitLeftRight(int x, int y, int Xsize, int Ysize, int P1)
{
	// P1 = # of pixel in group to split

	int Address;
	int Middle;
	int GroupNum;
	int GroupMiddle;
	int Xpos;

	Middle = Xsize / 2;
	GroupNum = x / P1;

	GroupMiddle = Xsize / (2 * P1);

	if (GroupNum < GroupMiddle) {
		Xpos = (GroupNum * P1 * 2) + x % P1;
	}
	else {
		Xpos = ((GroupNum-GroupMiddle) * P1 * 2) + (x % P1) + P1;
	}

	Address = Xpos + (y * Xsize);

	return Address;
}

//******************************************************************************
//
// NewAlg5
// 
//******************************************************************************
int NewAlg5(int x, int y, int Xsize, int Ysize) {
	int Address;

	Address = x + (y * Xsize);

	return Address;
}
