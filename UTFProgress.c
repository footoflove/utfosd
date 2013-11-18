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
#include "UTFProgress.h"
#include "UTFProgressPriv.h"
#include "UTFMemory.h"

/******************************************************************/
static void UTFDrawProgressFace(HUIWND hProgress, UTFRECT rcRect, DWORD info)
{
	LPUTFPROGRESS pProgress = (LPUTFPROGRESS)hProgress;
	UTFRECT wndRect = rcRect;
	UTFRECT progRect = rcRect;
	UTFCOLOR progColor, backColor;
	char bVertical;

	/* Draw background */
	if(pProgress->dwExStyle & PRGS_VER)
	{
		bVertical = TRUE;
		progRect.top = progRect.bottom - (progRect.bottom - progRect.top)*pProgress->curData/pProgress->maxData;
		wndRect.bottom = progRect.top;
	}
	else
	{
		bVertical = FALSE;
		progRect.right = progRect.left + (progRect.right - progRect.left)*pProgress->curData/pProgress->maxData;
		wndRect.left = progRect.right;
	}

	if(pProgress->dwExStyle & PRGS_IMAGE)
	{
		UTFDrawImageProgBar(pProgress->hUnitImage, pProgress->curData, pProgress->maxData, pProgress->bSpace, UTFRGB(65,95,195,255), rcRect.left, rcRect.top, rcRect.right, rcRect.bottom, bVertical);
	}
	else
	{
		if(pProgress->dwStyle & UIS_DISABLE)
		{
			progColor = UTFRGB(150,150,150,255);
			backColor = UTFRGB(100,100,100,255);
			UTFSetTextColor(UTFRGB(0,0,0,255));
		}
		else
		{
			backColor = UTFRGB(65,95,195,255);
			progColor = UTFRGB(255,100,100,255);
			UTFSetTextColor(UTFRGB(255,255,255,255));
		}
		
		UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
		UTFSetBrush(BSUI_SOLID, backColor, HSUI_HORIZONTAL);
		UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
		
		UTFSetBrush(BSUI_SOLID, progColor, HSUI_HORIZONTAL);
		UTFFillRect(progRect.left, progRect.top, progRect.right, progRect.bottom);
	}

	if(pProgress->dwExStyle & PRGS_TEXT)
	{
		UTFDrawText(pProgress->wndText, &rcRect, DTUI_CENTER|DTUI_VCENTER|DTUI_SINGLELINE);
	}
}

static void UTFProgressInvalidate(HUIWND hProgress, UTFRECT rcRect, DWORD info)
{
	LPUTFPROGRESS pProgress = (LPUTFPROGRESS)hProgress;

	if(pProgress->OnDraw)
	{
		pProgress->OnDraw(hProgress, rcRect, 0);
	}
	else
	{
		UTFDrawProgressFace(hProgress, rcRect, 0);
	}	

	pProgress->dwFlags &= ~PRGF_DRAWFRAME;
}

static int UTFProgressMsgProc(HUIWND hProgress, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFPROGRESS pProgress = (LPUTFPROGRESS)hProgress;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pProgress->hParent;
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
			pProgress->dwFlags |= UIF_REDRAW|PRGF_DRAWFRAME;
			break;

		case WMUI_ENABLE:
			if(pProgress->dwStyle & UIS_DISABLE)
			{
				pProgress->dwStyle &= ~UIS_DISABLE;
				pProgress->dwFlags |= UIF_REDRAW|PRGF_DRAWFRAME;
			}
			break;

		case WMUI_DISABLE:
			if((pProgress->dwStyle & UIS_DISABLE) != UIS_DISABLE)
			{
				pProgress->dwStyle |= UIS_DISABLE;
				pProgress->dwFlags |= UIF_REDRAW|PRGF_DRAWFRAME;
			}
			break;

		case WMUI_SHOW:
			if(pProgress->dwStyle & UIS_HIDE)
			{
				pProgress->dwStyle &= ~UIS_HIDE;
				pProgress->dwFlags |= UIF_REDRAW|PRGF_DRAWFRAME;

				UTFDialogDrawAllChildAboveCtrl(pProgress->hParent, hProgress);
			}
			break;

		case WMUI_HIDE:
			if((pProgress->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pProgress->dwStyle |= UIS_HIDE;
				pProgress->dwFlags |= UIF_REDRAW;
				pProgress->dwFlags |= DLGF_CLEANBACK;

				UTFDialogRedrawCtrlInRect(pProgress->hParent, &pProgress->rect);
			}
			break;

		case WMUI_SETRECT:
			if(param2)
			{
				LPUTFRECT lpRect = (LPUTFRECT)param2;
				UTFRECT oldRect = pProgress->rect;

				if((lpRect->right > (pDlg->rect.right-pDlg->rect.left)) || (lpRect->bottom > (pDlg->rect.bottom-pDlg->rect.top)))
					return 0;

				pProgress->dwFlags |= UIF_REDRAW|PRGF_DRAWFRAME;
				
				UTFDialogBackClean(pProgress->hParent, hProgress);
				UTFDialogRedrawCtrlInRect(pProgress->hParent, &pProgress->rect);
				
				memcpy(&pProgress->rect, lpRect, sizeof(UTFRECT));
				UTFDialogDrawAllChildAboveCtrl(pProgress->hParent, hProgress);

				if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
				{
					UTFDialogResetCtrlXYOrder(pProgress->hParent, hProgress, oldRect);
				}
			}
			else
			{
				retCode = 0;
			}
			break;

		case WMUI_SETTEXT:
		case WMUI_GETTEXT:
			break;

		case WMUI_DELETEWND:
			if(param2 == 0)
			{
				LPUTFDIALOGCTRL pCtrl;			

				if(pProgress->hPrevCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pProgress->hPrevCtrl;
					pCtrl->hNextCtrl = pProgress->hNextCtrl;
				}
				else
				{
					pDlg->pCtrl = (LPUTFDIALOGCTRL)pProgress->hNextCtrl;
				}

				if(pProgress->hNextCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pProgress->hNextCtrl;
					pCtrl->hPrevCtrl = pProgress->hPrevCtrl;
				}			
			}
			UTFFree(hProgress);
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

void UTFProgressInitialize(void)
{
	UTFRegisterWndType(WTP_PROGRESS, UTFProgressMsgProc, UTFDrawProgressFace);
}

HUIWND UTFAPI UTFCreateProgress(DWORD ctrlID, DWORD dwStyle, DWORD dwExStyle, HUIWND hDlg,
				HIMAGE hImage, WORD left, WORD top, WORD right, WORD bottom,
				DWORD maxValue, BYTE bSpace, DWORD ctrlData, UTFONDRAW OnDraw)
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
	
	if(maxValue == 0)
		return NULL;
	
	if(UTFIsDialog(hDlg) == TRUE)
	{
		HUIWND hProgress = (HUIWND)UTFMalloc(sizeof(UTFPROGRESS));
		
		if(hProgress != NULL)
		{
			LPUTFPROGRESS pProgress = (LPUTFPROGRESS)hProgress;

			memcpy(pProgress->type, WTP_PROGRESS, 4);
			pProgress->wndID	= ctrlID;
			
			pProgress->dwStyle	= dwStyle | UIS_CHILD | UIS_NOFOCUS | UIS_OWNERDRAW;
			pProgress->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST);
			pProgress->dwExStyle = dwExStyle;
			pProgress->dwFlags	= 0;
			
			pProgress->hParent	= hDlg;
			pProgress->dataEx	= ctrlData;
			pProgress->rect 	= rcRect;
			
			pProgress->DrawFace = UTFProgressInvalidate;
			pProgress->CallBack	= NULL;

			if(dwStyle & UIS_OWNERDRAW)
			{
				pProgress->OnDraw = OnDraw;
			}
			else
			{
				pProgress->OnDraw = NULL;
			}

			pProgress->maxData	= maxValue;
			pProgress->curData	= 0;
			pProgress->bSpace	= bSpace;
			pProgress->hUnitImage = hImage;
			if(pProgress->hUnitImage == NULL)
			{
				pProgress->dwExStyle &= ~PRGS_IMAGE;
			}
			else
			{
				pProgress->dwExStyle |= PRGS_IMAGE;
			}

			if(pProgress->dwExStyle & PRGS_TEXT)
			{
				sprintf((char *)pProgress->wndText, "%d%%", 100*pProgress->curData/pProgress->maxData);
			}
			else
			{
				memset((char *)pProgress->wndText, 0, TEXT_SIZE);
			}

			UTFAddCtrlToDlg(hDlg, hProgress);
		}

		return hProgress;
	}

	return NULL;
}

static int UTFIsProgress(HUIWND hWnd)
{
	return UTFWndIsType(hWnd, WTP_PROGRESS);
}

int UTFAPI UTFProgressSetPos(HUIWND hProgress, DWORD value)
{
	if(UTFIsProgress(hProgress) == TRUE)
	{
		LPUTFPROGRESS pProgress = (LPUTFPROGRESS)hProgress;
		DWORD oldValue = pProgress->curData;

		if(value > pProgress->maxData)
		{
			pProgress->curData = pProgress->maxData;
		}
		else
		{
			pProgress->curData = value;
		}
		
		if(pProgress->dwExStyle & PRGS_TEXT)
		{
			sprintf((char *)pProgress->wndText, "%d%%", 100*pProgress->curData/pProgress->maxData);
		}

		if(oldValue != pProgress->curData)
		{
			pProgress->dwFlags |= UIF_REDRAW;
		}

		return 1;
	}

	return 0;
}

