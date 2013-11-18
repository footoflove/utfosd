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
#include "UTFScrollBar.h"
#include "UTFScrollBarPriv.h"
#include "UTFMemory.h"

/******************************************************************/
static void UTFDrawScrollBarFace(HUIWND hScrollBar, UTFRECT rcRect, DWORD info)
{
	LPUTFSCROLLBAR pScrollBar = (LPUTFSCROLLBAR)hScrollBar;
	SCROLLBARINFO drawInfo;
	
	UTFScrollBarGetInfo(hScrollBar, &drawInfo);

	if(pScrollBar->dwFlags & SBF_DRAWFRAME)
	{
		WORD Width,Height;
		
		Width = drawInfo.rcUp.right-drawInfo.rcUp.left;
		Height = drawInfo.rcUp.bottom-drawInfo.rcUp.top;

		/* Draw background */
		UTFSetPen(PSUI_SOLID, UTFRGB(255,255,255,255), 1);
		UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,150,255), 0);
		
		UTFFillRect(drawInfo.rcUp.left, drawInfo.rcUp.top, drawInfo.rcUp.right, drawInfo.rcUp.bottom);
		UTFDrawArrow(drawInfo.rcUp.left+2, drawInfo.rcUp.top+2, Width-4, Height-5, ASUI_UP);
		
		UTFFillRect(drawInfo.rcDown.left, drawInfo.rcDown.top, drawInfo.rcDown.right, drawInfo.rcDown.bottom);
		UTFDrawArrow(drawInfo.rcDown.left+2, drawInfo.rcDown.top+2, Width-4, Height-5, ASUI_DOWN);
		
		UTFFillRect(drawInfo.rcCenter.left, drawInfo.rcCenter.top, drawInfo.rcCenter.right, drawInfo.rcCenter.bottom);
	}

	UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
	UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,150,255), 0);
	UTFFillRect(drawInfo.rcTrackBack.left, drawInfo.rcTrackBack.top, drawInfo.rcTrackBack.right, drawInfo.rcTrackBack.bottom);

	if(pScrollBar->dwFlags & SBF_DRAWTRACK)
	{
		UTFSetBrush(BSUI_SOLID, UTFRGB(100,100,250,255), 0);
		UTFFillRect(drawInfo.rcTrack.left, drawInfo.rcTrack.top, drawInfo.rcTrack.right, drawInfo.rcTrack.bottom);
	}
}

static void UTFScrollBarInvalidate(HUIWND hScrollBar, UTFRECT rcRect, DWORD info)
{
	LPUTFSCROLLBAR pScrollBar = (LPUTFSCROLLBAR)hScrollBar;

	if(pScrollBar->OnDraw)
	{
		pScrollBar->OnDraw(hScrollBar, rcRect, 0);
	}
	else
	{
		UTFDrawScrollBarFace(hScrollBar, rcRect, 0);
	}	

	pScrollBar->dwFlags &= ~(SBF_DRAWFRAME|SBF_DRAWTRACK);
}

static int UTFScrollBarMsgProc(HUIWND hScrollBar, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFSCROLLBAR pScrollBar = (LPUTFSCROLLBAR)hScrollBar;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pScrollBar->hParent;
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
			pScrollBar->dwFlags |= UIF_REDRAW|SBF_DRAWFRAME|SBF_DRAWTRACK;
			break;

		case WMUI_ENABLE:
			if(pScrollBar->dwStyle & UIS_DISABLE)
			{
				pScrollBar->dwStyle &= ~UIS_DISABLE;
				pScrollBar->dwFlags |= UIF_REDRAW|SBF_DRAWFRAME;
			}
			break;

		case WMUI_DISABLE:
			if((pScrollBar->dwStyle & UIS_DISABLE) != UIS_DISABLE)
			{
				pScrollBar->dwStyle |= UIS_DISABLE;
				pScrollBar->dwFlags |= UIF_REDRAW|SBF_DRAWFRAME;
			}
			break;

		case WMUI_SHOW:
			if(pScrollBar->dwStyle & UIS_HIDE)
			{
				pScrollBar->dwStyle &= ~UIS_HIDE;
				pScrollBar->dwFlags |= UIF_REDRAW|SBF_DRAWFRAME;

				UTFDialogDrawAllChildAboveCtrl(pScrollBar->hParent, hScrollBar);
			}
			break;

		case WMUI_HIDE:
			if((pScrollBar->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pScrollBar->dwStyle |= UIS_HIDE;
				pScrollBar->dwFlags |= UIF_REDRAW|SBF_DRAWFRAME;
				pScrollBar->dwFlags |= DLGF_CLEANBACK;

				UTFDialogRedrawCtrlInRect(pScrollBar->hParent, &pScrollBar->rect);
			}
			break;

		case WMUI_SETRECT:
			if((param2 == 0) || (pScrollBar->dwExStyle & SBS_ATTACH_TO_CHILD))
			{
				retCode = 0;
			}
			else
			{
				LPUTFRECT lpRect = (LPUTFRECT)param2;
				UTFRECT oldRect = pScrollBar->rect;

				if((lpRect->right > (pDlg->rect.right-pDlg->rect.left)) || (lpRect->bottom > (pDlg->rect.bottom-pDlg->rect.top)))
					return 0;

				pScrollBar->dwFlags |= UIF_REDRAW|SBF_DRAWFRAME;
				
				UTFDialogBackClean(pScrollBar->hParent, hScrollBar);
				UTFDialogRedrawCtrlInRect(pScrollBar->hParent, &pScrollBar->rect);
				
				memcpy(&pScrollBar->rect, lpRect, sizeof(UTFRECT));
				UTFDialogDrawAllChildAboveCtrl(pScrollBar->hParent, hScrollBar);

				if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
				{
					UTFDialogResetCtrlXYOrder(pScrollBar->hParent, hScrollBar, oldRect);
				}
			}
			break;

		case WMUI_SETTEXT:
		case WMUI_GETTEXT:
			break;

		case WMUI_DELETEWND:
			if(param2 == 0)
			{
				LPUTFDIALOGCTRL pCtrl;			

				if(pScrollBar->hPrevCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pScrollBar->hPrevCtrl;
					pCtrl->hNextCtrl = pScrollBar->hNextCtrl;
				}
				else
				{
					pDlg->pCtrl = (LPUTFDIALOGCTRL)pScrollBar->hNextCtrl;
				}

				if(pScrollBar->hNextCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pScrollBar->hNextCtrl;
					pCtrl->hPrevCtrl = pScrollBar->hPrevCtrl;
				}
			}
			UTFFree(hScrollBar);
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

void UTFScrollBarInitialize(void)
{
	UTFRegisterWndType(WTP_SCROLLBAR, UTFScrollBarMsgProc, NULL);
}

HUIWND UTFAPI UTFCreateScrollBar(DWORD ctrlID, DWORD dwStyle, DWORD dwExStyle, HUIWND hDlg, HIMAGE hImage,
				WORD left, WORD top, WORD right, WORD bottom, DWORD dwTotal, DWORD dwPageSize,
				DWORD dwPos, WORD ArrowHeight, DWORD ctrlData, UTFONDRAW OnDraw)
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

	if((ArrowHeight*2+14) > (rcRect.bottom-rcRect.top))
		return NULL;
	
	if(UTFIsDialog(hDlg) == TRUE)
	{
		HUIWND hScrollBar = (HUIWND)UTFMalloc(sizeof(UTFSCROLLBAR));
		
		if(hScrollBar != NULL)
		{
			LPUTFSCROLLBAR pScrollBar = (LPUTFSCROLLBAR)hScrollBar;

			memcpy(pScrollBar->type, WTP_SCROLLBAR, 4);
			pScrollBar->wndID	= ctrlID;
			
			pScrollBar->dwStyle	= dwStyle | UIS_CHILD | UIS_NOFOCUS | UIS_OWNERDRAW;
			pScrollBar->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST);
			pScrollBar->dwExStyle = dwExStyle;
			pScrollBar->dwFlags	= 0;
			
			pScrollBar->hParent	= hDlg;
			pScrollBar->dataEx	= ctrlData;
			pScrollBar->rect 	= rcRect;

			pScrollBar->CallBack = NULL;
			pScrollBar->DrawFace = UTFScrollBarInvalidate;
			pScrollBar->hImage = hImage;
			if(dwStyle & UIS_OWNERDRAW)
			{
				pScrollBar->OnDraw = OnDraw;
			}
			else
			{
				pScrollBar->OnDraw = NULL;
			}
			
			pScrollBar->dwTotal = dwTotal;
			pScrollBar->dwPageSize = dwPageSize;
			pScrollBar->dwArrowHeight = ArrowHeight;
			pScrollBar->dwPos = dwPos;
			if(pScrollBar->dwTotal == 0)
			{
				pScrollBar->dwPos = 0;
			}
			else if(pScrollBar->dwPos >= pScrollBar->dwTotal)
			{
				pScrollBar->dwPos = pScrollBar->dwTotal-1;
			}

			UTFAddCtrlToDlg(hDlg, hScrollBar);
		}

		return hScrollBar;
	}

	return NULL;
}

void UTFAPI UTFScrollBarGetInfo(HUIWND hScrollBar, LPSCROLLBARINFO lpInfo)
{
	LPUTFSCROLLBAR pScrollBar = (LPUTFSCROLLBAR)hScrollBar;
	LPUTFDIALOG pDlg;
	UTFRECT rcScrollBar;

	if(lpInfo == NULL)
		return;

	if(UTFWndIsType(hScrollBar, WTP_SCROLLBAR) == FALSE)
		return;

	pDlg = (LPUTFDIALOG)pScrollBar->hParent;
	rcScrollBar = pScrollBar->rect;
	UTFOffsetRect(&rcScrollBar, pDlg->rect.left, pDlg->rect.top);

	lpInfo->rcUp = lpInfo->rcDown = rcScrollBar;
	lpInfo->rcUp.bottom = lpInfo->rcUp.top+pScrollBar->dwArrowHeight;
	lpInfo->rcDown.top = lpInfo->rcDown.bottom-pScrollBar->dwArrowHeight;
	
	if(pScrollBar->hImage)
	{
		lpInfo->rcCenter.left = rcScrollBar.left;
		lpInfo->rcCenter.right = rcScrollBar.right;
		lpInfo->rcCenter.top = rcScrollBar.top+pScrollBar->dwArrowHeight+1;
		lpInfo->rcCenter.bottom = rcScrollBar.bottom-pScrollBar->dwArrowHeight-1;

		lpInfo->rcTrackBack.left = lpInfo->rcCenter.left+1;
		lpInfo->rcTrackBack.right = lpInfo->rcCenter.right-1;
		lpInfo->rcTrackBack.top = lpInfo->rcCenter.top+1;
		lpInfo->rcTrackBack.bottom = lpInfo->rcCenter.bottom-1;

		lpInfo->rcTrack.left = lpInfo->rcTrackBack.left;
		lpInfo->rcTrack.right = lpInfo->rcTrackBack.right;
		if((pScrollBar->dwTotal == 0) || (pScrollBar->dwPageSize == 0) ||
			(pScrollBar->dwPageSize >= pScrollBar->dwTotal))
		{
			lpInfo->rcTrack.top = lpInfo->rcTrackBack.top;
			lpInfo->rcTrack.bottom = lpInfo->rcTrackBack.bottom;

			pScrollBar->dwFlags &= ~SBF_DRAWTRACK;
		}
		else
		{
			UTFIMAGEINFO bmpInfo;
			WORD TotalHeight;
			WORD TrackHeight;
			WORD TotalPage;
			WORD CurrentPage;
			WORD TopPos;

			TotalHeight = lpInfo->rcTrackBack.bottom-lpInfo->rcTrackBack.top;

			UTFGetImageInfo(pScrollBar->hImage, &bmpInfo);
			TrackHeight = bmpInfo.dwHeight;
			if(TrackHeight < 4)
			{
				TrackHeight = 4;
			}
			else if(TrackHeight > TotalHeight)
			{
				TrackHeight = TotalHeight;
			}
			
			TotalPage = (pScrollBar->dwTotal-1)/pScrollBar->dwPageSize;
			CurrentPage = pScrollBar->dwPos/pScrollBar->dwPageSize;
		
			TopPos = (TotalHeight-TrackHeight)*CurrentPage/TotalPage;
			lpInfo->rcTrack.top = lpInfo->rcTrackBack.top+TopPos;
			lpInfo->rcTrack.bottom = lpInfo->rcTrack.top+TrackHeight;
			
			pScrollBar->dwFlags |= SBF_DRAWTRACK;
		}
	}
	else
	{
		lpInfo->rcCenter.left = rcScrollBar.left;
		lpInfo->rcCenter.right = rcScrollBar.right;
		lpInfo->rcCenter.top = rcScrollBar.top+pScrollBar->dwArrowHeight+2;
		lpInfo->rcCenter.bottom = rcScrollBar.bottom-pScrollBar->dwArrowHeight-2;

		lpInfo->rcTrackBack.left = rcScrollBar.left+3;
		lpInfo->rcTrackBack.right = rcScrollBar.right-3;
		lpInfo->rcTrackBack.top = lpInfo->rcCenter.top+3;
		lpInfo->rcTrackBack.bottom = lpInfo->rcCenter.bottom-3;
	
		lpInfo->rcTrack.left = lpInfo->rcTrackBack.left;
		lpInfo->rcTrack.right = lpInfo->rcTrackBack.right;
		if((pScrollBar->dwTotal == 0) || (pScrollBar->dwPageSize == 0) ||
			(pScrollBar->dwPageSize >= pScrollBar->dwTotal))
		{
			lpInfo->rcTrack.top = lpInfo->rcTrackBack.top;
			lpInfo->rcTrack.bottom = lpInfo->rcTrackBack.bottom;

			pScrollBar->dwFlags &= ~SBF_DRAWTRACK;
		}
		else
		{
			double pageNum;
			WORD TotalHeight;
			WORD TrackHeight;
			WORD TotalPage;
			WORD CurrentPage;
			WORD TopPos;

			pageNum = (double)pScrollBar->dwTotal/(double)pScrollBar->dwPageSize;
			TotalHeight = lpInfo->rcTrackBack.bottom-lpInfo->rcTrackBack.top;
			TrackHeight = (WORD)((double)TotalHeight/pageNum);
			if(TrackHeight < 4)
			{
				TrackHeight = 4;
			}
			else if(TrackHeight > TotalHeight)
			{
				TrackHeight = TotalHeight;
			}
			
			TotalPage = (pScrollBar->dwTotal-1)/pScrollBar->dwPageSize;
			CurrentPage = pScrollBar->dwPos/pScrollBar->dwPageSize;
		
			TopPos = (TotalHeight-TrackHeight)*CurrentPage/TotalPage;
			lpInfo->rcTrack.top = lpInfo->rcTrackBack.top+TopPos;
			lpInfo->rcTrack.bottom = lpInfo->rcTrack.top+TrackHeight;
			
			pScrollBar->dwFlags |= SBF_DRAWTRACK;
		}
	}
}

int UTFAPI UTFScrollBarSetScrollSize(HUIWND hScrollBar, DWORD dwTotal)
{
	LPUTFSCROLLBAR pScrollBar = (LPUTFSCROLLBAR)hScrollBar;
	
	if(UTFWndIsType(hScrollBar, WTP_SCROLLBAR) == FALSE)
		return 0;

	if(pScrollBar->dwTotal == dwTotal)
		return 0;
	
	pScrollBar->dwFlags |= UIF_REDRAW;
	pScrollBar->dwTotal = dwTotal;
	
	if(pScrollBar->dwTotal == 0)
	{
		pScrollBar->dwPos = 0;
	}
	else if(pScrollBar->dwPos >= pScrollBar->dwTotal)
	{
		pScrollBar->dwPos = pScrollBar->dwTotal-1;
	}
	
	return 1;
}

int UTFAPI UTFScrollBarSetScrollPos(HUIWND hScrollBar, DWORD dwPos)
{
	LPUTFSCROLLBAR pScrollBar = (LPUTFSCROLLBAR)hScrollBar;
	WORD oldPage,newPage;
	
	if(UTFWndIsType(hScrollBar, WTP_SCROLLBAR) == FALSE)
		return 0;

	if(pScrollBar->dwPos == dwPos)
		return 0;

	oldPage = pScrollBar->dwPos/pScrollBar->dwPageSize;
	
	pScrollBar->dwPos = dwPos;
	if(pScrollBar->dwTotal == 0)
	{
		pScrollBar->dwPos = 0;
	}
	else if(pScrollBar->dwPos >= pScrollBar->dwTotal)
	{
		pScrollBar->dwPos = pScrollBar->dwTotal-1;
	}
	newPage = pScrollBar->dwPos/pScrollBar->dwPageSize;

	if(oldPage != newPage)
	{
		pScrollBar->dwFlags |= UIF_REDRAW;
	}
	
	return 1;
}

