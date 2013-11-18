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
#include "UTFDialog.h"
#include "UTFDialogPriv.h"
#include "UTFWndManage.h"
#include "UTFWndDefPriv.h"
#include "UTFMsgDefPriv.h"
#include "UTFDrawAPI.h"
#include "UTFDrawText.h"
#include "UTFFont.h"
#include "UTFImage.h"
#include "UTFImagePriv.h"
#include "UTFMemory.h"

/******************************************************************/
static void UTFDrawImageFace(HUIWND hImage, UTFRECT rcRect, DWORD info)
{
	LPUTFIMAGE pImage = (LPUTFIMAGE)hImage;
	UTFRECT wndRect = rcRect;

	/* Draw background */	
	UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
	UTFSetBrush(BSUI_SOLID, UTFGetDialogBkColor(pImage->hParent), HSUI_HORIZONTAL);
	UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);

	UTFDrawImage(pImage->hImage, wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
}

static int UTFImageMsgProc(HUIWND hImage, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFIMAGE pImage = (LPUTFIMAGE)hImage;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pImage->hParent;
	int retCode = 1;

	switch(uMsg)
	{
	case WMUI_PUBLIC:
		switch(param1)
		{
		case WMUI_CREATE:
			break;

		case WMUI_DESTROY:
			break;

		default:
			retCode = 0;
			break;
		}
		break;

	case WMUI_INTER:
		switch(param1)
		{
		case WMUI_PAINT:
			pImage->dwFlags |= UIF_REDRAW;
			break;

		case WMUI_SHOW:
			if(pImage->dwStyle & UIS_HIDE)
			{
				pImage->dwStyle &= ~UIS_HIDE;
				pImage->dwFlags |= UIF_REDRAW;

				UTFDialogDrawAllChildAboveCtrl(pImage->hParent, hImage);
			}
			break;

		case WMUI_HIDE:
			if((pImage->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pImage->dwStyle |= UIS_HIDE;
				pImage->dwFlags |= UIF_REDRAW;
				pImage->dwFlags |= DLGF_CLEANBACK;

				UTFDialogRedrawCtrlInRect(pImage->hParent, &pImage->rect);
			}
			break;

		case WMUI_SETRECT:
			if(param2)
			{
				LPUTFRECT lpRect = (LPUTFRECT)param2;
				UTFRECT oldRect = pImage->rect;

				if((lpRect->right > (pDlg->rect.right-pDlg->rect.left)) || (lpRect->bottom > (pDlg->rect.bottom-pDlg->rect.top)))
					return 0;

				pImage->dwFlags |= UIF_REDRAW;
				
				UTFDialogBackClean(pImage->hParent, hImage);
				UTFDialogRedrawCtrlInRect(pImage->hParent, &pImage->rect);
				
				memcpy(&pImage->rect, lpRect, sizeof(UTFRECT));
				UTFDialogDrawAllChildAboveCtrl(pImage->hParent, hImage);

				if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
				{
					UTFDialogResetCtrlXYOrder(pImage->hParent, hImage, oldRect);
				}
			}
			else
			{
				retCode = 0;
			}
			break;

		case WMUI_ENABLE:
		case WMUI_DISABLE:
		case WMUI_SETTEXT:
		case WMUI_GETTEXT:
			break;	

		case WMUI_DELETEWND:
			if(param2 == 0)
			{
				LPUTFDIALOGCTRL pCtrl;			

				if(pImage->hPrevCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pImage->hPrevCtrl;
					pCtrl->hNextCtrl = pImage->hNextCtrl;
				}
				else
				{
					pDlg->pCtrl = (LPUTFDIALOGCTRL)pImage->hNextCtrl;
				}

				if(pImage->hNextCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pImage->hNextCtrl;
					pCtrl->hPrevCtrl = pImage->hPrevCtrl;
				}			
			}
			UTFFree(hImage);
			break;

		default:
			retCode = 0;
			break;
		}
		break;

	default:
		retCode = 0;
		break;
	}

	return retCode;
}

void UTFImageInitialize(void)
{
	UTFRegisterWndType(WTP_IMAGE, UTFImageMsgProc, UTFDrawImageFace);
}

HUIWND UTFAPI UTFCreateImage(DWORD ctrlID, DWORD dwStyle, DWORD dwExStyle, HUIWND hDlg,
					HIMAGE hImage, WORD left, WORD top, WORD right, WORD bottom, 
					DWORD ctrlData, UTFONDRAW OnDraw)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	UTFRECT rcRect;

	if(hDlg == NULL)
		return NULL;

	rcRect.left	= left;
	rcRect.top	= top;
	rcRect.right = right;
	rcRect.bottom = bottom;
	UTFNormalizeRect(&rcRect);
	
	if(UTFIsRectEmpty(&rcRect))
		return NULL;
	
	if((rcRect.right > (pDlg->rect.right-pDlg->rect.left)) || (rcRect.bottom > (pDlg->rect.bottom-pDlg->rect.top)))
		return NULL;
	
	if(UTFIsDialog(hDlg) == TRUE)
	{
		HUIWND hCtrl = (HUIWND)UTFMalloc(sizeof(UTFIMAGE));
		
		if(hCtrl != NULL)
		{
			LPUTFIMAGE pImage = (LPUTFIMAGE)hCtrl;

			memcpy(pImage->type, WTP_IMAGE, 4);
			pImage->wndID	= ctrlID;
			
			pImage->dwStyle	= dwStyle | UIS_CHILD | UIS_NOFOCUS;
			pImage->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST);
			pImage->dwExStyle = dwExStyle;
			pImage->dwFlags	= 0;
			
			pImage->hParent	= hDlg;
			pImage->hImage 	= hImage;
			pImage->dataEx	= ctrlData;
			pImage->rect 	= rcRect;
			
			pImage->OnDraw = OnDraw;
			pImage->CallBack = NULL;

			UTFAddCtrlToDlg(hDlg, hCtrl);
		}

		return hCtrl;
	}

	return NULL;
}

int UTFAPI UTFImageSetHandle( HUIWND hCtrl, HIMAGE hImage )
{
	if(hCtrl == NULL)
		return 0;

	if(UTFWndIsType(hCtrl, WTP_IMAGE) == TRUE)
	{
		LPUTFIMAGE pImage = (LPUTFIMAGE)hCtrl;

		if(pImage->hImage != hImage)
		{
			pImage->hImage = hImage;
			pImage->dwFlags |= UIF_REDRAW;
		}
	}

	return 1;
}

