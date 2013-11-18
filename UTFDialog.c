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
#include "UTFWndManage.h"
#include "UTFWndDefPriv.h"
#include "UTFMsgDefPriv.h"
#include "UTFDrawAPI.h"
#include "UTFFont.h"
#include "UTFDialog.h"
#include "UTFDialogPriv.h"
#include "UTFZOrderManage.h"
#include "UTFZOrderManagerPriv.h"
#include "UTFDrawText.h"
#include "UTFMemory.h"
#include "UTFTimer.h"

static void UTFDialogDrawFace(HUIWND hDlg, UTFRECT rcRect, DWORD info)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	UTFRECT wndRect = rcRect;
	int borderDepth = 2;
	int titleHeight = 30;
	
	if(pDlg->dwFlags & UIF_REDRAW) // Draw all menu frame
	{
		UTFSetPen(PSUI_SOLID, UTFRGB(255,255,255,255), borderDepth);
		UTFSetBrush(BSUI_SOLID, pDlg->m_DlgBackColor, 0);
		UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
	}

	if(pDlg->dwFlags & UIF_DRAWTITLE) // draw menu title
	{
		wndRect.left += borderDepth;
		wndRect.top = pDlg->rect.top+borderDepth;
		wndRect.right -= borderDepth;
		wndRect.bottom = wndRect.top+titleHeight-borderDepth;
		
		UTFSetPen(PSUI_NULL, 0, 0);
		UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,100,255), 0);
		UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.top+30);
		
		if(pDlg->textID >= 0)
		{
			UTFGetIDText(pDlg->textID, pDlg->wndText, TEXT_SIZE);
		}
		UTFSetTextColor(UTFRGB(255,255,255,255));
		UTFDrawText(pDlg->wndText, &wndRect, DTUI_SINGLELINE|DTUI_END_ELLIPSIS|DTUI_LEFT|DTUI_VCENTER);
	}
}

static void UTFDialogRedrawAllCtrl(HUIWND hDlg)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	LPUTFDIALOGCTRL pCtrl;
	
	pCtrl = pDlg->pCtrl;
	while(pCtrl)
	{
		if((pCtrl->dwStyle & UIS_HIDE) != UIS_HIDE)
		{
			UTFSendPrivMessage((HUIWND)pCtrl, WMUI_INTER, WMUI_PAINT, 0, 0);
		
			pCtrl->dwFlags |= UIF_REDRAW;
		}
		
		pCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
	}
}

static void UTFDialogCleanRect(HUIWND hDlg, LPUTFRECT lpRect)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	UTFRECT rcRect;
	
	if((hDlg == NULL) || (lpRect == NULL))
		return;

	memcpy(&rcRect, lpRect, sizeof(UTFRECT));
	UTFOffsetRect(&rcRect, pDlg->rect.left, pDlg->rect.top);

	if(pDlg->CallBack)
	{
		if(pDlg->CallBack(hDlg, WMUI_PUBLIC, WMUI_DLG_CLEAN_RECT, (DWORD)&rcRect, -1) == 0)
		{
			UTFSetPen(PSUI_NULL, 0, 0);
			UTFSetBrush(BSUI_SOLID, pDlg->m_DlgBackColor, 0);
			UTFFillRect(rcRect.left, rcRect.top, rcRect.right, rcRect.bottom);
		}
	}
	else
	{
		UTFSetPen(PSUI_NULL, 0, 0);
		UTFSetBrush(BSUI_SOLID, pDlg->m_DlgBackColor, 0);
		UTFFillRect(rcRect.left, rcRect.top, rcRect.right, rcRect.bottom);
	}
}

void UTFDialogBackClean(HUIWND hDlg, HUIWND hCtrl)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	LPUTFDIALOGCTRL pCtrl = (LPUTFDIALOGCTRL)hCtrl;
	UTFRECT rcRect;
	
	if((hDlg == NULL) || (hCtrl == NULL))
		return;

	rcRect = pCtrl->rect;
	UTFOffsetRect(&rcRect, pDlg->rect.left, pDlg->rect.top);

	if(pDlg->CallBack)
	{
		if(pDlg->CallBack(hDlg, WMUI_PUBLIC, WMUI_DLG_CLEAN_RECT, (DWORD)&rcRect, pCtrl->wndID) == 0)
		{
			UTFSetPen(PSUI_NULL, 0, 0);
			UTFSetBrush(BSUI_SOLID, pDlg->m_DlgBackColor, 0);
			UTFFillRect(rcRect.left, rcRect.top, rcRect.right, rcRect.bottom);
		}
	}
	else
	{
		UTFSetPen(PSUI_NULL, 0, 0);
		UTFSetBrush(BSUI_SOLID, pDlg->m_DlgBackColor, 0);
		UTFFillRect(rcRect.left, rcRect.top, rcRect.right, rcRect.bottom);
	}
}

void UTFDialogRedrawCtrlInRect(HUIWND hDlg, LPUTFRECT lpRect)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	LPUTFDIALOGCTRL pCtrl;
	
	pCtrl = pDlg->pCtrl;
	while(pCtrl)
	{
		if((pCtrl->dwStyle & UIS_HIDE) != UIS_HIDE)
		{
			if(UTFIntersectRect(NULL, &pCtrl->rect, lpRect))
			{
				UTFSendPrivMessage((HUIWND)pCtrl, WMUI_INTER, WMUI_PAINT, 0, 0);
				
				pCtrl->dwFlags |= UIF_REDRAW;
			}
		}
		
		pCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
	}
}

void UTFDialogDrawAllChildAboveCtrl(HUIWND hDlg, HUIWND hCtrl)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	LPUTFDIALOGCTRL pCtrl;
	UTFRECT rcRect;
	int bFound = FALSE;

	pCtrl = pDlg->pCtrl;
	while(pCtrl)
	{
		if(hCtrl == (HUIWND)pCtrl)
		{
			rcRect = pCtrl->rect;
			bFound = TRUE;
		}
		
		pCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
		if(bFound == TRUE)
			break;
	}

	if(bFound == FALSE)
		return;
	
	while(pCtrl)
	{
		if((pCtrl->dwStyle & UIS_HIDE) != UIS_HIDE)
		{
			if(UTFIntersectRect(NULL, &pCtrl->rect, &rcRect))
			{
				UTFSendPrivMessage((HUIWND)pCtrl, WMUI_INTER, WMUI_PAINT, 0, 0);
				
				pCtrl->dwFlags |= UIF_REDRAW;
			}
		}
		
		pCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
	}
}

static LPUTFDIALOGCTRL UTFDialogGetFocusCtrl(HUIWND hDlg)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;

	if(pDlg->total > 0)
	{
		LPUTFDIALOGCTRL pCtrl = pDlg->pCtrl;
			
		while(pCtrl)
		{
			if(pCtrl->dwFlags & UIF_FOCUS)
				break;
			
			pCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
		}

		return pCtrl;
	}

	return NULL;
}

static void UTFDialogInvalidate(HUIWND hDlg, DWORD info)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	LPUTFDIALOGCTRL pCtrl;
	UTFRECT drawRect;

	if(pDlg->dwFlags & (UIF_REDRAW | UIF_DRAWTITLE))
	{
		if(pDlg->dwFlags & UIF_REDRAW)
		{
			UTFSetOpenScreenFlag();
			
			if(pDlg->CallBack)
			{
				pDlg->CallBack(hDlg, WMUI_PUBLIC, WMUI_QUERY_SHOW, 0, 0);
			}
		}
		
		if(pDlg->OnDraw != NULL)
		{
			if(pDlg->dwStyle & UIS_OWNERDRAW)
			{
				pDlg->OnDraw(hDlg, pDlg->rect, 0);
			}
			else
			{
				UTFDialogDrawFace(hDlg, pDlg->rect, 0);
			}
		}
		else
		{
			UTFDialogDrawFace(hDlg, pDlg->rect, 0);
		}
	}
	pDlg->dwFlags &= ~(UIF_REDRAW | UIF_DRAWTITLE);

	if(pDlg->pCtrl == NULL)
		return;

	//Clean all hide ctrl
	pCtrl = pDlg->pCtrl;
	while(pCtrl)
	{
		if(pCtrl->dwStyle & UIS_HIDE)
		{
			if(pCtrl->dwFlags & DLGF_CLEANBACK)
			{
				UTFDialogBackClean(hDlg, (HUIWND)pCtrl);
				
				pCtrl->dwFlags &= ~DLGF_CLEANBACK;
			}
			
			// Clear status flag
			pCtrl->dwFlags &= ~UIF_REDRAW;
		}
		
		pCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
	}

	// Start draw item
	pCtrl = pDlg->pCtrl;
	while(pCtrl)
	{
		if(pCtrl->dwFlags & UIF_REDRAW)
		{
			drawRect = pCtrl->rect;
			UTFOffsetRect(&drawRect, pDlg->rect.left, pDlg->rect.top);
		
			// Start draw each menu item which need be updated
			if(pCtrl->OnDraw != NULL)
			{
				if(pCtrl->dwStyle & UIS_OWNERDRAW)
				{
					pCtrl->OnDraw((HUIWND)pCtrl, drawRect, 0);
				}
				else
				{
					UTFDefDrawProc((HUIWND)pCtrl, drawRect, 0);
				}
			}
			else
			{
				UTFDefDrawProc((HUIWND)pCtrl, drawRect, 0);
			}

			// Clear status flag
			pCtrl->dwFlags &= ~UIF_REDRAW;
		}
		
		pCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
	}
}

static LPUTFDIALOGCTRL UTFDialogSetFocusCtrlByOrder(HUIWND hDlg, WORD xOrder, WORD yOrder)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	LPUTFDIALOGCTRL pCtrl = pDlg->pCtrl;

	while(pCtrl)
	{
		if((pCtrl->xOrder == xOrder) && (pCtrl->yOrder == yOrder))
		{
			if(!(pCtrl->dwStyle & (UIS_NOFOCUS|UIS_HIDE|UIS_DISABLE)))
			{
				if(UTFSendPrivMessage((HUIWND)pCtrl, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0))
				{
					return pCtrl;
				}
			}
		}

		pCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
	}

	return NULL;
}

// This function will move menu focus up and set paint flag
static int UTFDialogFocusUp(HUIWND hDlg)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;

	if(pDlg->total > 1)
	{
		LPUTFDIALOGCTRL pCtrlTemp;
		LPUTFDIALOGCTRL pCtrl;
		WORD xOrder,yOrder;
		int i,j;		
		
		pCtrl = UTFDialogGetFocusCtrl(hDlg);
		if(pCtrl == NULL)
		{
			return FALSE;
		}

		xOrder = pCtrl->xOrder;
		yOrder = pCtrl->yOrder;
		
		for(i=1; i<pDlg->yOrderMax; i++)
		{
			if(yOrder > 0)
			{
				yOrder--;
			}
			else
			{
				yOrder = pDlg->yOrderMax-1;
			}
			
			pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder, yOrder);
			if(pCtrlTemp != NULL)
			{
				return TRUE;
			}

			for(j=1; j<pDlg->xOrderMax; j++)
			{
				if(xOrder >= j)
				{
					pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder-j, yOrder);
					if(pCtrlTemp != NULL)
					{
						return TRUE;
					}
				}

				if((xOrder+j) < pDlg->xOrderMax)
				{
					pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder+j, yOrder);
					if(pCtrlTemp != NULL)
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

// This function will move menu focus down and set paint flag
static int UTFDialogFocusDown(HUIWND hDlg)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;

	if(pDlg->total > 1)
	{
		LPUTFDIALOGCTRL pCtrlTemp;
		LPUTFDIALOGCTRL pCtrl;
		WORD xOrder,yOrder;
		int i,j;		
		
		pCtrl = UTFDialogGetFocusCtrl(hDlg);
		if(pCtrl == NULL)
		{
			return FALSE;
		}

		xOrder = pCtrl->xOrder;
		yOrder = pCtrl->yOrder;
		
		for(i=1; i<pDlg->yOrderMax; i++)
		{
			if(++yOrder >= pDlg->yOrderMax)
			{
				yOrder = 0;
			}
			
			pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder, yOrder);
			if(pCtrlTemp != NULL)
			{
				return TRUE;
			}

			for(j=1; j<pDlg->xOrderMax; j++)
			{
				if(xOrder >= j)
				{
					pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder-j, yOrder);
					if(pCtrlTemp != NULL)
					{
						return TRUE;
					}
				}

				if((xOrder+j) < pDlg->xOrderMax)
				{
					pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder+j, yOrder);
					if(pCtrlTemp != NULL)
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

// This function will move menu focus left and set paint flag
static int UTFDialogFocusLeft(HUIWND hDlg)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;

	if(pDlg->total > 1)
	{
		LPUTFDIALOGCTRL pCtrlTemp;
		LPUTFDIALOGCTRL pCtrl;
		WORD xOrder,yOrder;
		int i,j;		
		
		pCtrl = UTFDialogGetFocusCtrl(hDlg);
		if(pCtrl == NULL)
		{
			return FALSE;
		}

		xOrder = pCtrl->xOrder;
		yOrder = pCtrl->yOrder;
		
		for(i=1; i<pDlg->xOrderMax; i++)
		{
			if(xOrder > 0)
			{
				xOrder--;
			}
			else
			{
				xOrder = pDlg->xOrderMax-1;
			}
			
			pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder, yOrder);
			if(pCtrlTemp != NULL)
			{
				return TRUE;
			}

			for(j=1; j<pDlg->yOrderMax; j++)
			{
				if(yOrder >= j)
				{
					pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder, yOrder-j);
					if(pCtrlTemp != NULL)
					{
						return TRUE;
					}
				}

				if((yOrder+j) < pDlg->yOrderMax)
				{
					pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder, yOrder+j);
					if(pCtrlTemp != NULL)
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

// This function will move menu focus right and set paint flag
static int UTFDialogFocusRight(HUIWND hDlg)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;

	if(pDlg->total > 1)
	{
		LPUTFDIALOGCTRL pCtrlTemp;
		LPUTFDIALOGCTRL pCtrl;
		WORD xOrder,yOrder;
		int i,j;		
		
		pCtrl = UTFDialogGetFocusCtrl(hDlg);
		if(pCtrl == NULL)
		{
			return FALSE;
		}

		xOrder = pCtrl->xOrder;
		yOrder = pCtrl->yOrder;
		
		for(i=1; i<pDlg->xOrderMax; i++)
		{
			if(++xOrder >= pDlg->xOrderMax)
			{
				xOrder = 0;
			}
			
			pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder, yOrder);
			if(pCtrlTemp != NULL)
			{
				return TRUE;
			}

			for(j=1; j<pDlg->yOrderMax; j++)
			{
				if(yOrder >= j)
				{
					pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder, yOrder-j);
					if(pCtrlTemp != NULL)
					{
						return TRUE;
					}
				}

				if((yOrder+j) < pDlg->yOrderMax)
				{
					pCtrlTemp = UTFDialogSetFocusCtrlByOrder(hDlg, xOrder, yOrder+j);
					if(pCtrlTemp != NULL)
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

static int UTFDialogMsgProc(HUIWND hDlg, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	LPUTFDIALOGCTRL pCtrl;
	int retCode = 1;

	switch(uMsg)
	{
	case WMUI_PUBLIC:
		switch(param1)
		{
		case WMUI_CREATE:
			pDlg->dwStyle &= ~UIS_HIDE;
			pDlg->dwFlags |= UIF_FOCUS;

			pCtrl = pDlg->pCtrl;
			while(pCtrl)
			{
				UTFSendPrivMessage((HUIWND)pCtrl, uMsg, param1, param2, param3);
				
				pCtrl->dwFlags |= UIF_REDRAW;

				pCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
			}

			pCtrl = UTFDialogGetFocusCtrl(hDlg);
			if(pCtrl != NULL)
			{
				if(pDlg->CallBack)
				{
					pDlg->CallBack(hDlg, WMUI_PUBLIC, WMUI_SETFOCUS, pCtrl->wndID, pCtrl->wndID);
				}
			}
			break;

		case WMUI_DESTROY:
			pDlg->dwFlags &= ~UIF_FOCUS;
			pDlg->dwFlags &= ~UIF_REDRAW;

			UTFKillWindowTimer(hDlg);
			break;

		case WMUI_ACTIVE:
			if(pDlg->CallBack)
			{
				pDlg->CallBack(hDlg, uMsg, param1, param2, param3);
			}
			break;

		case WMUI_SETFOCUS:
			break;

		default:
			retCode = 0;
			break;
		}
		break;

	case WMUI_KEYPRESS:
		switch(param1)
		{
		case KEYUI_UP:
			pCtrl = UTFDialogGetFocusCtrl(hDlg);
			if(UTFSendMessage((HUIWND)pCtrl, uMsg, param1, param2, param3) == 0)
			{
				UTFDialogFocusUp(hDlg);
			}
			break;

		case KEYUI_DOWN:
			pCtrl = UTFDialogGetFocusCtrl(hDlg);
			if(UTFSendMessage((HUIWND)pCtrl, uMsg, param1, param2, param3) == 0)
			{
				UTFDialogFocusDown(hDlg);
			}
			break;

		case KEYUI_LEFT:
			pCtrl = UTFDialogGetFocusCtrl(hDlg);
			if(UTFSendMessage((HUIWND)pCtrl, uMsg, param1, param2, param3) == 0)
			{
				UTFDialogFocusLeft(hDlg);
			}
			break;

		case KEYUI_RIGHT:
			pCtrl = UTFDialogGetFocusCtrl(hDlg);
			if(UTFSendMessage((HUIWND)pCtrl, uMsg, param1, param2, param3) == 0)
			{
				UTFDialogFocusRight(hDlg);
			}
			break;

		case KEYUI_SELECT:
			pCtrl = UTFDialogGetFocusCtrl(hDlg);
			if(pCtrl != NULL)
			{
				UTFSendMessage((HUIWND)pCtrl, uMsg, param1, param2, param3);
			}
			break;

		case KEYUI_MENU:
			pCtrl = UTFDialogGetFocusCtrl(hDlg);
			if(UTFSendMessage((HUIWND)pCtrl, uMsg, param1, param2, param3) == 0)
			{
				if(UTFGetDefaultCloseWindowMode() == 0)
					UTFWindowEnd(hDlg, ID_CANCEL);
				else
					UTFWindowClose(hDlg);
			}
			break;

		case KEYUI_EXIT:
			pCtrl = UTFDialogGetFocusCtrl(hDlg);
			if(UTFSendMessage((HUIWND)pCtrl, uMsg, param1, param2, param3) == 0)
			{
				if(UTFGetDefaultCloseWindowMode() == 0)
					UTFWindowClose(hDlg);
				else
					UTFWindowEnd(hDlg, ID_CANCEL);
			}
			break;

		default:
			pCtrl = UTFDialogGetFocusCtrl(hDlg);
			if(pCtrl != NULL)
			{
				retCode = UTFSendMessage((HUIWND)pCtrl, uMsg, param1, param2, param3);
			}
			else
			{
				retCode = 0;
			}
			break;
		}
		break;

	case WMUI_INTER:
		switch(param1)
		{
		case WMUI_UPDATE:
			UTFDialogInvalidate(hDlg, 0);
			break;
			
		case WMUI_PAINT:
			if(param3 == TRUE)
			{
				pDlg->dwFlags |= UIF_DRAWTITLE;
				
				if(param2 == NULL)
				{
					pDlg->dwFlags |= UIF_REDRAW;
					UTFDialogRedrawAllCtrl(hDlg);
				}
				else 
				{
					UTFDialogCleanRect(hDlg, (LPUTFRECT)param2);
					UTFDialogRedrawCtrlInRect(hDlg, (LPUTFRECT)param2);
				}
			}
			else
			{
				pDlg->dwFlags |= UIF_DRAWTITLE;
				
				if(param2 == NULL)
				{
					UTFDialogRedrawAllCtrl(hDlg);
				}
				else
				{
					UTFDialogRedrawCtrlInRect(hDlg, (LPUTFRECT)param2);
				}
			}
			break;

		case WMUI_NEWFOCUS:
			pCtrl = (LPUTFDIALOGCTRL)param3;
			if(pCtrl != NULL)
			{
				if(pCtrl != (LPUTFDIALOGCTRL)param2)
				{
					UTFSendPrivMessage((HUIWND)pCtrl, WMUI_INTER, WMUI_KILLFOCUS, 0, 0);
					if(pDlg->CallBack)
					{
						if(UTFIsWindowVisible(hDlg) != UTFWND_INVISIBLE)
						{
							pDlg->CallBack(hDlg, WMUI_PUBLIC, WMUI_SETFOCUS, UTFGetWindowID(param2), pCtrl->wndID);
						}
					}
				}
			}
			else if(pDlg->CallBack)
			{
				if(UTFIsWindowVisible(hDlg) != UTFWND_INVISIBLE)
				{
					pDlg->CallBack(hDlg, WMUI_PUBLIC, WMUI_SETFOCUS, UTFGetWindowID(param2), UTFGetWindowID(param2));
				}
			}
			break;

		case WMUI_FOCUSGO:
			if(UTFDialogFocusDown(hDlg) == FALSE)
			{
				UTFDialogFocusRight(hDlg);
			}
			break;

		case WMUI_SETTEXT:
			{
				int textLen;

				if(param2 != NULL)
				{
					textLen = strlen((char *)param2);
					if(textLen < TEXT_SIZE)
					{
						strcpy((char *)pDlg->wndText, (char *)param2);
					}
					else
					{
						memcpy((char *)pDlg->wndText, (char *)param2, TEXT_SIZE);
						pDlg->wndText[TEXT_SIZE-1] = 0;
					}
					pDlg->textID = -1;
					pDlg->dwFlags |= UIF_DRAWTITLE;
				}
				else if(pDlg->textID != param3)
				{
					pDlg->textID = param3;
					pDlg->dwFlags |= UIF_DRAWTITLE;
				}				
			}
			break;

		case WMUI_GETTEXT:
			if((param2 != NULL) && (param3 > 0))
			{
				if(pDlg->textID >= 0)
				{
					UTFGetIDText(pDlg->textID, pDlg->wndText, TEXT_SIZE);
				}
				if(strlen((char *)pDlg->wndText) < param3)
				{
					strcpy((char *)param2, (char *)pDlg->wndText);
				}
				else
				{
					char *pBuffer = (char *)param2;
					
					memcpy(pBuffer, (char *)pDlg->wndText, param3);
					*(pBuffer+param3-1) = 0;
				}
			}
			break;

		case WMUI_SHOW:
			if(pDlg->dwStyle & UIS_HIDE)
			{
				pDlg->dwStyle &= ~UIS_HIDE;
			}
			break;

		case WMUI_HIDE:
			if((pDlg->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pDlg->dwStyle |= UIS_HIDE;
			}
			break;

		case WMUI_SETPARENT:
			pDlg->hParent = param2;
			break;

		case WMUI_DELETEWND:
			{
				HUIWND hNextCtrl;
				
				pCtrl = pDlg->pCtrl;
				while(pCtrl)
				{
					hNextCtrl = pCtrl->hNextCtrl;
					
					UTFSendPrivMessage((HUIWND)pCtrl, WMUI_INTER, WMUI_DELETEWND, 1, 0);
					
					pCtrl = (LPUTFDIALOGCTRL)hNextCtrl;
				}
				UTFFree(hDlg);
			}
			break;

		case WMUI_ENABLE:
		case WMUI_DISABLE:
		case WMUI_KILLFOCUS:
			break;

		default:
			retCode = 0;
			break;
		}
		break;

	default:
		pCtrl = UTFDialogGetFocusCtrl(hDlg);
		if(pCtrl != NULL)
		{
			retCode = UTFSendMessage((HUIWND)pCtrl, uMsg, param1, param2, param3);
		}
		else
		{
			retCode = 0;
		}
		break;
	}

	return retCode;
}

void UTFDialogInitialize(void)
{
	UTFRegisterWndType(WTP_DIALOG, UTFDialogMsgProc, UTFDialogDrawFace);
}

int UTFDialogHaveFocus(HUIWND hDlg)
{
	LPUTFDIALOGCTRL pCtrl;

	if(hDlg == NULL)
		return FALSE;
	
	pCtrl = UTFDialogGetFocusCtrl(hDlg);
	if(pCtrl == NULL)
		return FALSE;
	else
		return TRUE;
}

static void UTFDialogSetCtrlXYOrder(HUIWND hDlg, HUIWND hCtrl)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	LPUTFDIALOGCTRL pCtrl = (LPUTFDIALOGCTRL)hCtrl;

	pCtrl->xOrder = 0;
	pCtrl->yOrder = 0;
	pDlg->xOrderMax = 0;
	pDlg->yOrderMax = 0;
	
	if(pDlg->pCtrl)
	{
		LPUTFDIALOGCTRL pDlgCtrl = pDlg->pCtrl;
		BYTE bHaveXSame = FALSE;
		BYTE bHaveYSame = FALSE;

		// Search for same position ctrl
		while(pDlgCtrl)
		{
			if(pCtrl != pDlgCtrl)
			{
				if(pDlgCtrl->rect.left == pCtrl->rect.left)
				{
					bHaveXSame = TRUE;
					pCtrl->xOrder = pDlgCtrl->xOrder;
				}

				if(pDlgCtrl->rect.top == pCtrl->rect.top)
				{
					bHaveYSame = TRUE;
					pCtrl->yOrder = pDlgCtrl->yOrder;
				}
			}
			
			pDlgCtrl = (LPUTFDIALOGCTRL)pDlgCtrl->hNextCtrl;
		}

		// Set new order of ctrl
		pDlgCtrl = pDlg->pCtrl;
		while(pDlgCtrl)
		{
			if(pCtrl != pDlgCtrl)
			{
				if(pDlgCtrl->rect.left < pCtrl->rect.left)
				{
					if(pCtrl->xOrder <= pDlgCtrl->xOrder)
					{
						pCtrl->xOrder = pDlgCtrl->xOrder+1;
					}
				}
				else if(pDlgCtrl->rect.left > pCtrl->rect.left)
				{
					if(bHaveXSame == FALSE)
					{
						pDlgCtrl->xOrder++;
					}
				}

				if(pDlgCtrl->rect.top < pCtrl->rect.top)
				{
					if(pCtrl->yOrder <= pDlgCtrl->yOrder)
					{
						pCtrl->yOrder = pDlgCtrl->yOrder+1;
					}
				}
				else if(pDlgCtrl->rect.top > pCtrl->rect.top)
				{
					if(bHaveYSame == FALSE)
					{
						pDlgCtrl->yOrder++;
					}
				}
			}
			
			pDlgCtrl = (LPUTFDIALOGCTRL)pDlgCtrl->hNextCtrl;
		}

		// Set max order to dialog
		pDlgCtrl = pDlg->pCtrl;
		while(pDlgCtrl)
		{
			if(pDlg->xOrderMax < pDlgCtrl->xOrder)
			{
				pDlg->xOrderMax = pDlgCtrl->xOrder;
			}

			if(pDlg->yOrderMax < pDlgCtrl->yOrder)
			{
				pDlg->yOrderMax = pDlgCtrl->yOrder;
			}
			
			pDlgCtrl = (LPUTFDIALOGCTRL)pDlgCtrl->hNextCtrl;
		}
	}
	pDlg->xOrderMax++;
	pDlg->yOrderMax++;
}

void UTFDialogResetCtrlXYOrder(HUIWND hDlg, HUIWND hCtrl, UTFRECT oldRect)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	LPUTFDIALOGCTRL pCtrl = (LPUTFDIALOGCTRL)hCtrl;
	LPUTFDIALOGCTRL pDlgCtrl = pDlg->pCtrl;
	BYTE bHaveXSame = FALSE;
	BYTE bHaveYSame = FALSE;

	if(pDlg->pCtrl)
	{
		// Find same position ctrl
		while(pDlgCtrl)
		{
			if(pCtrl != pDlgCtrl)
			{
				if(pDlgCtrl->rect.left == oldRect.left)
				{
					bHaveXSame = TRUE;
				}

				if(pDlgCtrl->rect.top == oldRect.top)
				{
					bHaveYSame = TRUE;
				}
			}
			
			pDlgCtrl = (LPUTFDIALOGCTRL)pDlgCtrl->hNextCtrl;
		}

		// Reset all ctrl order except for hCtrl
		pDlgCtrl = pDlg->pCtrl;
		while(pDlgCtrl)
		{
			if(pCtrl != pDlgCtrl)
			{
				if(bHaveXSame == FALSE)
				{
					if(pDlgCtrl->xOrder > pCtrl->xOrder)
					{
						pDlgCtrl->xOrder--;
					}
				}

				if(bHaveYSame == FALSE)
				{
					if(pDlgCtrl->yOrder > pCtrl->yOrder)
					{
						pDlgCtrl->yOrder--;
					}
				}
			}
			
			pDlgCtrl = (LPUTFDIALOGCTRL)pDlgCtrl->hNextCtrl;
		}

		UTFDialogSetCtrlXYOrder(hDlg, hCtrl);
	}
}

void UTFAddCtrlToDlg(HUIWND hDlg, HUIWND hCtrl)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	LPUTFDIALOGCTRL pCtrl = (LPUTFDIALOGCTRL)hCtrl;

	if((hDlg == NULL) || (hCtrl == NULL))
		return;

	pDlg->total++;
	if(UTFDialogGetFocusCtrl(hDlg) == NULL)
	{
		UTFSendPrivMessage(hCtrl, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
	}	

	pCtrl->hNextCtrl = NULL;
	if(pDlg->pCtrl == NULL)
	{		
		pCtrl->hPrevCtrl = NULL;
		pDlg->pCtrl = pCtrl;
	}
	else
	{
		LPUTFDIALOGCTRL pDlgCtrl = pDlg->pCtrl;
		
		while(pDlgCtrl)
		{
			if(pDlgCtrl->hNextCtrl == NULL)
			{
				pDlgCtrl->hNextCtrl = hCtrl;
				pCtrl->hPrevCtrl = (HUIWND)pDlgCtrl;
				break;
			}
			
			pDlgCtrl = (LPUTFDIALOGCTRL)pDlgCtrl->hNextCtrl;
		}
	}

	UTFDialogSetCtrlXYOrder(hDlg, hCtrl);
}

/**************************************************************************/
int UTFIsDialog(HUIWND hWnd)
{
	return UTFWndIsType(hWnd, WTP_DIALOG);
}

HUIWND UTFAPI UTFCreateDialog(DWORD dlgID, int textID, char *text, DWORD dwStyle, DWORD dwExStyle,
						WORD left, WORD top, WORD right, WORD bottom, DWORD dlgData,
						UTFONDRAW OnDraw, UTFCALLBACK CallBack)
{
	HUIWND hDlg = (HUIWND)UTFMalloc(sizeof(UTFDIALOG));

	if(hDlg != NULL)
	{
		LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;

		memcpy(pDlg->type, WTP_DIALOG, 4);
		pDlg->wndID		= dlgID;
		
		pDlg->dwStyle	= dwStyle | UIS_MODAL;
		pDlg->dwStyle	&= ~(UIS_CHILD | UIS_TOPMOST | UIS_TABSTOP);
		pDlg->dwExStyle = dwExStyle;
		pDlg->dwFlags	= 0;
		
		pDlg->hParent	= NULL;
		pDlg->dataEx 	= dlgData;		
		
		pDlg->OnDraw	= OnDraw;
		pDlg->CallBack	= CallBack;
		pDlg->pCtrl		= NULL;
		pDlg->m_DlgBackColor = UTFRGB(192,192,192,255);

		pDlg->rect.left		= left;
		pDlg->rect.top		= top;
		pDlg->rect.right	= right;
		pDlg->rect.bottom	= bottom;
		UTFNormalizeRect(&pDlg->rect);
		
		pDlg->total	= 0;
		pDlg->xOrderMax = 0;
		pDlg->yOrderMax = 0;
		
		if(text != NULL)
		{
			pDlg->textID = -1;
			if(strlen(text) >= TEXT_SIZE)
			{
				memcpy((char *)pDlg->wndText, text, TEXT_SIZE-1);
				pDlg->wndText[TEXT_SIZE-1] = 0;
			}
			else
			{
				strcpy((char *)pDlg->wndText, text);
			}
		}
		else
		{
			pDlg->textID = textID;
		}
	}

	return hDlg;
}

HUIWND UTFAPI UTFGetDlgItem( HUIWND hDlg, int nIDDlgItem )
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	LPUTFDIALOGCTRL pCtrl;

	if(UTFIsDialog(hDlg) == FALSE)
		return NULL;

	pCtrl = pDlg->pCtrl;
	while(pCtrl)
	{
		if(pCtrl->wndID == (DWORD)nIDDlgItem)
		{
			break;
		}

		pCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
	}

	return (HUIWND)pCtrl;
}

HUIWND UTFAPI UTFDlgGetFocus( HUIWND hDlg )
{
	if(UTFIsDialog(hDlg) == FALSE)
		return NULL;

	return (HUIWND)UTFDialogGetFocusCtrl(hDlg);
}

UTFCOLOR UTFAPI UTFGetDialogBkColor(HUIWND hDlg)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	
	if(UTFIsDialog(hDlg) == FALSE)
		return 0;
	
	return pDlg->m_DlgBackColor;
}

void UTFAPI UTFSetDialogBkColor(HUIWND hDlg, UTFCOLOR bkColor)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	
	if(UTFIsDialog(hDlg) == FALSE)
		return;
	
	pDlg->m_DlgBackColor = bkColor;
}

