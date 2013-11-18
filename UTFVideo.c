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
#include "UTFVideo.h"
#include "UTFVideoPriv.h"
#include "UTFMemory.h"

/******************************************************************/
static void UTFDrawVideoFace(HUIWND hVideo, UTFRECT rcRect, DWORD info)
{
	UTFRECT wndRect = rcRect;

	/* Draw background */	
	UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
	UTFSetBrush(BSUI_SOLID, UTFRGB(0,0,0,0), 0);
	UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
}

static int UTFVideoMsgProc(HUIWND hVideo, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFVIDEO pVideo = (LPUTFVIDEO)hVideo;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pVideo->hParent;
	int retCode = 1;

	switch(uMsg)
	{
	case WMUI_PUBLIC:
		switch(param1)
		{
		case WMUI_CREATE:
			if(pDlg->CallBack)
			{
				UTFRECT rcRect;

				rcRect = pVideo->rect;
				UTFOffsetRect(&rcRect, pDlg->rect.left, pDlg->rect.top);
				pDlg->CallBack(pVideo->hParent, WMUI_NOTIFY, VNUI_ZOOMVIDEO, pVideo->wndID, (DWORD)&rcRect);
			}
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
			pVideo->dwFlags |= UIF_REDRAW;
			break;

		case WMUI_ENABLE:
			if(pVideo->dwStyle & UIS_DISABLE)
			{
				pVideo->dwStyle &= ~UIS_DISABLE;
				pVideo->dwFlags |= UIF_REDRAW;
			}
			break;

		case WMUI_DISABLE:
			if((pVideo->dwStyle & UIS_DISABLE) != UIS_DISABLE)
			{
				pVideo->dwStyle |= UIS_DISABLE;
				pVideo->dwFlags |= UIF_REDRAW;
			}
			break;

		case WMUI_SHOW:
			if(pVideo->dwStyle & UIS_HIDE)
			{
				pVideo->dwStyle &= ~UIS_HIDE;
				pVideo->dwFlags |= UIF_REDRAW;

				UTFDialogDrawAllChildAboveCtrl(pVideo->hParent, hVideo);
			}
			break;

		case WMUI_HIDE:
			if((pVideo->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pVideo->dwStyle |= UIS_HIDE;
				pVideo->dwFlags |= UIF_REDRAW;
				pVideo->dwFlags |= DLGF_CLEANBACK;

				UTFDialogRedrawCtrlInRect(pVideo->hParent, &pVideo->rect);
			}
			break;

		case WMUI_SETRECT:
			if(param2)
			{
				LPUTFRECT lpRect = (LPUTFRECT)param2;
				UTFRECT oldRect = pVideo->rect;

				if((lpRect->right > (pDlg->rect.right-pDlg->rect.left)) || (lpRect->bottom > (pDlg->rect.bottom-pDlg->rect.top)))
					return 0;

				pVideo->dwFlags |= UIF_REDRAW;
				
				UTFDialogBackClean(pVideo->hParent, hVideo);
				UTFDialogRedrawCtrlInRect(pVideo->hParent, &pVideo->rect);
				
				memcpy(&pVideo->rect, lpRect, sizeof(UTFRECT));
				UTFDialogDrawAllChildAboveCtrl(pVideo->hParent, hVideo);

				if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
				{
					UTFDialogResetCtrlXYOrder(pVideo->hParent, hVideo, oldRect);
				}

				if(pDlg->CallBack)
				{
					UTFRECT rcRect;

					rcRect = pVideo->rect;
					UTFOffsetRect(&rcRect, pDlg->rect.left, pDlg->rect.top);
					pDlg->CallBack(pVideo->hParent, WMUI_NOTIFY, VNUI_ZOOMVIDEO, (DWORD)&rcRect, 0);
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

				if(pVideo->hPrevCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pVideo->hPrevCtrl;
					pCtrl->hNextCtrl = pVideo->hNextCtrl;
				}
				else
				{
					pDlg->pCtrl = (LPUTFDIALOGCTRL)pVideo->hNextCtrl;
				}

				if(pVideo->hNextCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pVideo->hNextCtrl;
					pCtrl->hPrevCtrl = pVideo->hPrevCtrl;
				}			
			}
			UTFFree(hVideo);
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

void UTFVideoInitialize(void)
{
	UTFRegisterWndType(WTP_VIDEO, UTFVideoMsgProc, UTFDrawVideoFace);
}

HUIWND UTFAPI UTFCreateVideo(DWORD ctrlID, DWORD dwStyle, DWORD dwExStyle, HUIWND hDlg,
						WORD left, WORD top, WORD right, WORD bottom,
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
		HUIWND hVideo = (HUIWND)UTFMalloc(sizeof(UTFVIDEO));
		
		if(hVideo != NULL)
		{
			LPUTFVIDEO pVideo = (LPUTFVIDEO)hVideo;

			memcpy(pVideo->type, WTP_VIDEO, 4);
			pVideo->wndID	= ctrlID;
			
			pVideo->dwStyle	= dwStyle | UIS_CHILD | UIS_NOFOCUS;
			pVideo->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST);
			pVideo->dwExStyle = dwExStyle;
			pVideo->dwFlags	= 0;
			
			pVideo->hParent	= hDlg;
			pVideo->dataEx	= ctrlData;
			pVideo->rect 	= rcRect;
			
			pVideo->OnDraw = OnDraw;
			pVideo->CallBack = NULL;

			UTFAddCtrlToDlg(hDlg, hVideo);
		}

		return hVideo;
	}

	return NULL;
}

