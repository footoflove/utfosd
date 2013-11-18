/*
	Copyright (C) shenzhen sowell technology CO.,LTD
*/
/* This is a copy of UTF Tool source code and you should have a copy
** of sowell license to permit use this program.
** 
** This source code can create UI enviroment and manage message of
** each window, you can easy to draw window face and process message
** with it, 
** 
** This code realsed to dareglobal CO.,LTD shanghai
*/

#include "stdafx.h"
#include "UTFTypeDef.h"
#include "UTFWndDef.h"

int UTFAPI UTFRectOver(const LPUTFRECT lprcSrc, const LPUTFRECT lprcDst)
{
	if((lprcSrc != NULL) && (lprcDst != NULL))
	{
		if((lprcSrc->left <= lprcDst->left) &&
			(lprcSrc->top <= lprcDst->top) &&
			(lprcSrc->right >= lprcDst->right) &&
			(lprcSrc->bottom >= lprcDst->bottom))
		{
			return 1;
		}
	}

	return 0;
}

int UTFAPI UTFInflateRect(LPUTFRECT lpRect, int left, int top, int right, int bottom)
{
	if(lpRect == NULL)
		return 0;

	lpRect->left -= left;
	lpRect->top -= top;
	lpRect->right += right;
	lpRect->bottom += bottom;

	return 1;
}

int UTFAPI UTFDeflateRect(LPUTFRECT lpRect, int left, int top, int right, int bottom)
{
	if(lpRect == NULL)
		return 0;

	lpRect->left += left;
	lpRect->top += top;
	lpRect->right -= right;
	lpRect->bottom -= bottom;

	return 1;
}

int UTFAPI UTFNormalizeRect(LPUTFRECT lpRect)
{
	WORD temp;

	if(lpRect == NULL)
		return 0;

	if(lpRect->left > lpRect->right)
	{
		temp = lpRect->left;
		lpRect->left = lpRect->right;
		lpRect->right = temp;
	}

	if(lpRect->top > lpRect->bottom)
	{
		temp = lpRect->top;
		lpRect->top = lpRect->bottom;
		lpRect->bottom = temp;
	}

	return 1;
}

int UTFAPI UTFUnionRect(LPUTFRECT lprcDst, const LPUTFRECT lprcSrc1, const LPUTFRECT lprcSrc2)
{
	UTFRECT rcRect;

	if((lprcDst == NULL) || (lprcSrc1 == NULL) || (lprcSrc2 == NULL))
		return 0;

	if(lprcSrc2->left < lprcSrc1->left)
	{
		rcRect.left = lprcSrc2->left;
	}
	else
	{
		rcRect.left = lprcSrc1->left;
	}

	if(lprcSrc2->top < lprcSrc1->top)
	{
		rcRect.top = lprcSrc2->top;
	}
	else
	{
		rcRect.top = lprcSrc1->top;
	}

	if(lprcSrc2->right > lprcSrc1->right)
	{
		rcRect.right = lprcSrc2->right;
	}
	else
	{
		rcRect.right = lprcSrc1->right;
	}

	if(lprcSrc2->bottom > lprcSrc1->bottom)
	{
		rcRect.bottom = lprcSrc2->bottom;
	}
	else
	{
		rcRect.bottom = lprcSrc1->bottom;
	}

	memcpy(lprcDst, &rcRect, sizeof(UTFRECT));

	return 1;
}

int UTFAPI UTFIntersectRect(LPUTFRECT lprcDst, const LPUTFRECT lprcSrc1, const LPUTFRECT lprcSrc2)
{
	UTFRECT rcRect;
	int ret = 1;

	if((lprcSrc1 == NULL) || (lprcSrc2 == NULL))
		return 0;

	// get max window left
	if(lprcSrc2->left < lprcSrc1->left)
	{
		rcRect.left = lprcSrc1->left;
	}
	else
	{
		rcRect.left = lprcSrc2->left;
	}

	// get min window right
	if(lprcSrc2->right < lprcSrc1->right)
	{
		rcRect.right = lprcSrc2->right;
	}
	else
	{
		rcRect.right = lprcSrc1->right;
	}

	// get max window top
	if(lprcSrc2->top < lprcSrc1->top)
	{
		rcRect.top = lprcSrc1->top;
	}
	else
	{
		rcRect.top = lprcSrc2->top;
	}

	// get min window bottom
	if(lprcSrc2->bottom < lprcSrc1->bottom)
	{
		rcRect.bottom = lprcSrc2->bottom;
	}
	else
	{
		rcRect.bottom = lprcSrc1->bottom;
	}

	if((rcRect.right < rcRect.left) || (rcRect.bottom < rcRect.top))
	{
		rcRect.left = 0;
		rcRect.top = 0;
		rcRect.right = 0;
		rcRect.bottom = 0;

		ret = 0;
	}

	if(lprcDst != NULL)
	{
		memcpy(lprcDst, &rcRect, sizeof(UTFRECT));
	}

	return ret;
}

int UTFAPI UTFIsRectEmpty(const LPUTFRECT lpRect)
{
	if(lpRect == NULL)
		return 1;

	if((lpRect->right <= lpRect->left) || (lpRect->bottom <= lpRect->top))
		return 1;
		
	return 0;
}

int UTFAPI UTFIsRectEqual(const LPUTFRECT lpRect1, const LPUTFRECT lpRect2)
{
	if((lpRect1 != NULL) && (lpRect2 != NULL))
	{
		if((lpRect1->left == lpRect2->left) &&
			(lpRect1->top == lpRect2->top) &&
			(lpRect1->right == lpRect2->right) &&
			(lpRect1->bottom == lpRect2->bottom))
		{
			return 1;
		}
	}

	return 0;
}

int UTFAPI UTFSetRect(LPUTFRECT lpRect, WORD left, WORD top, WORD right, WORD bottom)
{
	if(lpRect == NULL)
		return 0;

	lpRect->left = left;
	lpRect->top = top;
	lpRect->right = right;
	lpRect->bottom = bottom;

	return 1;
}

int UTFAPI UTFOffsetRect(LPUTFRECT lpRect, int xDelta, int yDelta)
{
	if(lpRect == NULL)
		return 0;

	lpRect->left += xDelta;
	lpRect->right += xDelta;
	lpRect->top += yDelta;
	lpRect->bottom += yDelta;

	return 1;
}

int UTFAPI UTFClientToScreen(HUIWND hWnd, LPUTFRECT lpRect)
{
	UTFRECT rcRectMain;
	
	if((hWnd == NULL) || (lpRect == NULL))
		return 0;

	UTFGetWindowRect(hWnd, &rcRectMain);
	
	lpRect->left += rcRectMain.left;
	lpRect->top += rcRectMain.top;
	lpRect->right += rcRectMain.left;
	lpRect->bottom += rcRectMain.top;
	
	return 1;
}

int UTFAPI UTFScreenToClient(HUIWND hWnd, LPUTFRECT lpRect)
{
	UTFRECT rcRectMain;
	UTFRECT rcRect;
	
	if((hWnd == NULL) || (lpRect == NULL))
		return 0;

	UTFGetWindowRect(hWnd, &rcRectMain);
	
	if( !UTFIntersectRect(&rcRect, &rcRectMain, lpRect) )
		return 0;
	
	rcRect.left -= rcRectMain.left;
	rcRect.top -= rcRectMain.top;
	rcRect.right -= rcRectMain.left;
	rcRect.bottom -= rcRectMain.top;

	memcpy(lpRect, &rcRect, sizeof(UTFRECT));
	
	return 1;
}

