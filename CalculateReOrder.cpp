#include "CalculateReOrder.h"

int CalcQuad_UL_UR_LL_LR_l2r_t2b(int x, int y, int Xsize, int Ysize);
int CalcQuad_UL_LL_UR_LR_l2r_t2b(int x, int y, int Xsize, int Ysize);
int CalcQuad_LL_UL_LR_UR_l2r_t2b(int x, int y, int Xsize, int Ysize);
int CalcQuad_LL_LR_UL_UR_l2r_t2b(int x, int y, int Xsize, int Ysize);
int CalcQuad_UL_UR_LL_LR_center_out(int x, int y, int Xsize, int Ysize);

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
int CalculateReOrder(int x, int y, int Xsize, int Ysize, int Algorithm, int* ResizeFlag)
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
