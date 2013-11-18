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
#include "UTFLRList.h"
#include "UTFLRListPriv.h"
#include "UTFMemory.h"

/******************************************************************/
static void UTFDrawLRListFace(HUIWND hLRList, UTFRECT rcRect, DWORD info)
{
	LPUTFLRLIST pCtrl = (LPUTFLRLIST)hLRList;
	UTFRECT wndRect = rcRect;
	BYTE buffer[TEXT_SIZE];
	UTFCOLOR color;
	int height;

	/* Draw background */
	if(pCtrl->dwFlags & LRF_DRAWSEL)
	{
		UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
		if(pCtrl->dwStyle & UIS_DISABLE)
		{
			color = UTFRGB(0,0,0,255);
			UTFSetBrush(BSUI_SOLID, UTFRGB(150,150,150,255), HSUI_HORIZONTAL);
			UTFSetTextColor(UTFRGB(0,0,0,255));
		}
		else if(pCtrl->dwFlags & UIF_FOCUS)
		{
			color = UTFRGB(0,0,0,255);
			UTFSetBrush(BSUI_SOLID, UTFRGB(220,120,70,255), HSUI_HORIZONTAL);
			UTFSetTextColor(UTFRGB(0,0,0,255));
		}
		else
		{
			color = UTFRGB(255,255,255,255);
			UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,150,255), HSUI_HORIZONTAL);
			UTFSetTextColor(UTFRGB(255,255,255,255));
		}
		UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);

		height = wndRect.bottom-wndRect.top-10;
		UTFSetPen(PSUI_SOLID, color, 1);
		UTFDrawArrow(wndRect.left+5, wndRect.top+5, height, height, ASUI_LEFT);
		UTFDrawArrow(wndRect.right-5-height, wndRect.top+5, height, height, ASUI_RIGHT);

		wndRect.left += height+10;
		wndRect.right -= height+10;
		if(pCtrl->lpSelInfo != NULL)
		{
			if(pCtrl->lpSelInfo[pCtrl->dwCurSel].textID >= 0)
			{
				UTFGetIDText(pCtrl->lpSelInfo[pCtrl->dwCurSel].textID, buffer, TEXT_SIZE);
				UTFDrawText(buffer, &wndRect, DTUI_SINGLELINE|DTUI_CENTER|DTUI_VCENTER);
			}
			else
			{
				UTFDrawText((LPTEXT)pCtrl->lpSelInfo[pCtrl->dwCurSel].text, &wndRect, DTUI_SINGLELINE|DTUI_CENTER|DTUI_VCENTER);
			}
		}
	}
}

static void UTFLRListInvalidate(HUIWND hLRList, UTFRECT rcRect, DWORD info)
{
	LPUTFLRLIST pCtrl = (LPUTFLRLIST)hLRList;

	if(pCtrl->OnDraw)
	{
		pCtrl->OnDraw(hLRList, rcRect, 0);
	}
	else
	{
		UTFDrawLRListFace(hLRList, rcRect, 0);
	}	

	pCtrl->dwFlags &= ~(LRF_DRAWSEL|LRF_DRAWFRAME);
}

static int UTFLRListMsgProc(HUIWND hLRList, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFLRLIST pCtrl = (LPUTFLRLIST)hLRList;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pCtrl->hParent;
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

		case WMUI_SETFOCUS:
			if((pCtrl->dwStyle & ( UIS_DISABLE | UIS_HIDE )) || !(pCtrl->dwStyle & UIS_TABSTOP) || (pCtrl->dwTotalSel == 0) || (pCtrl->lpSelInfo == NULL))
			{
				retCode = 0;
			}
			else if( !(pCtrl->dwFlags & UIF_FOCUS) )
			{
				HUIWND hFocusCtrl = UTFDlgGetFocus(pCtrl->hParent);

				pCtrl->dwFlags |= UIF_FOCUS;
				pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWFRAME|LRF_DRAWSEL;

				UTFSendPrivMessage(pCtrl->hParent, WMUI_INTER, WMUI_NEWFOCUS, hLRList, hFocusCtrl);
			}
			break;

		default:
			retCode = 0;
			break;
		}
		break;

	case WMUI_KEYPRESS:
		switch(param1)
		{
		case KEYUI_LEFT:
			if(pCtrl->dwTotalSel > 1)
			{
				if(pCtrl->dwCurSel > 0)
					pCtrl->dwCurSel--;
				else
					pCtrl->dwCurSel = pCtrl->dwTotalSel-1;
				
				pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWSEL;
				if(pDlg->CallBack)
				{
					pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, LRUI_SELCHANGE, pCtrl->wndID, pCtrl->dwCurSel);
				}
			}
			break;

		case KEYUI_RIGHT:
			if(pCtrl->dwTotalSel > 1)
			{
				if(++pCtrl->dwCurSel >= pCtrl->dwTotalSel)
				{
					pCtrl->dwCurSel = 0;
				}
				
				pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWSEL;
				if(pDlg->CallBack)
				{
					pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, LRUI_SELCHANGE, pCtrl->wndID, pCtrl->dwCurSel);
				}
			}
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
			pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWSEL|LRF_DRAWFRAME;
			break;

		case WMUI_KILLFOCUS:
			pCtrl->dwFlags &= ~UIF_FOCUS;
			pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWFRAME|LRF_DRAWSEL;
			break;

		case WMUI_ENABLE:
			if(pCtrl->dwStyle & UIS_DISABLE)
			{
				pCtrl->dwStyle &= ~UIS_DISABLE;
				pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWSEL|LRF_DRAWFRAME;

				if(UTFDialogHaveFocus(pCtrl->hParent) == FALSE)
				{
					UTFSendPrivMessage(hLRList, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}
			}
			break;

		case WMUI_DISABLE:
			if((pCtrl->dwStyle & UIS_DISABLE) != UIS_DISABLE)
			{
				pCtrl->dwStyle |= UIS_DISABLE;
				pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWSEL|LRF_DRAWFRAME;

				if(pCtrl->dwFlags & UIF_FOCUS)
				{
					UTFSendPrivMessage(pCtrl->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
					pCtrl->dwFlags &= ~UIF_FOCUS;
				}
			}
			break;

		case WMUI_SHOW:
			if(pCtrl->dwStyle & UIS_HIDE)
			{
				pCtrl->dwStyle &= ~UIS_HIDE;
				pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWSEL|LRF_DRAWFRAME;

				UTFDialogDrawAllChildAboveCtrl(pCtrl->hParent, hLRList);

				if(UTFDialogHaveFocus(pCtrl->hParent) == FALSE)
				{
					UTFSendPrivMessage(hLRList, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}
			}
			break;

		case WMUI_HIDE:
			if((pCtrl->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pCtrl->dwStyle |= UIS_HIDE;
				pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWSEL|LRF_DRAWFRAME;
				pCtrl->dwFlags |= DLGF_CLEANBACK;

				if(pCtrl->dwFlags & UIF_FOCUS)
				{
					UTFSendPrivMessage(pCtrl->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
					pCtrl->dwFlags &= ~UIF_FOCUS;
				}

				UTFDialogRedrawCtrlInRect(pCtrl->hParent, &pCtrl->rect);
			}
			break;

		case WMUI_SETRECT:
			if(param2)
			{
				LPUTFRECT lpRect = (LPUTFRECT)param2;
				UTFRECT oldRect = pCtrl->rect;

				if((lpRect->right > (pDlg->rect.right-pDlg->rect.left)) || (lpRect->bottom > (pDlg->rect.bottom-pDlg->rect.top)))
					return 0;

				pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWSEL|LRF_DRAWFRAME;
				
				UTFDialogBackClean(pCtrl->hParent, hLRList);
				UTFDialogRedrawCtrlInRect(pCtrl->hParent, &pCtrl->rect);
				
				memcpy(&pCtrl->rect, lpRect, sizeof(UTFRECT));
				UTFDialogDrawAllChildAboveCtrl(pCtrl->hParent, hLRList);

				if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
				{
					UTFDialogResetCtrlXYOrder(pCtrl->hParent, hLRList, oldRect);
				}
			}
			else
			{
				retCode = 0;
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
						strcpy((char *)pCtrl->wndText, (char *)param2);
					}
					else
					{
						memcpy((char *)pCtrl->wndText, (char *)param2, TEXT_SIZE);
						pCtrl->wndText[TEXT_SIZE-1] = 0;
					}
					pCtrl->textID = -1;
					pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWFRAME;
				}
				else if(pCtrl->textID != param3)
				{
					pCtrl->textID = param3;
					pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWFRAME;
				}				
			}
			break;

		case WMUI_GETTEXT:
			if((param2 != NULL) && (param3 > 0))
			{
				if(pCtrl->textID >= 0)
				{
					UTFGetIDText(pCtrl->textID, pCtrl->wndText, TEXT_SIZE);
				}
				if(strlen((char *)pCtrl->wndText) < param3)
				{
					strcpy((char *)param2, (char *)pCtrl->wndText);
				}
				else
				{
					char *pBuffer = (char *)param2;
					
					memcpy(pBuffer, (char *)pCtrl->wndText, param3);
					*(pBuffer+param3-1) = 0;
				}
			}
			break;	

		case WMUI_DELETEWND:
			if(param2 == 0)
			{
				LPUTFDIALOGCTRL pLRCtrl;			

				if(pCtrl->hPrevCtrl)
				{
					pLRCtrl = (LPUTFDIALOGCTRL)pCtrl->hPrevCtrl;
					pLRCtrl->hNextCtrl = pCtrl->hNextCtrl;
				}
				else
				{
					pDlg->pCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
				}

				if(pCtrl->hNextCtrl)
				{
					pLRCtrl = (LPUTFDIALOGCTRL)pCtrl->hNextCtrl;
					pLRCtrl->hPrevCtrl = pCtrl->hPrevCtrl;
				}			
			}
			UTFFree(hLRList);
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

void UTFLRListInitialize(void)
{
	UTFRegisterWndType(WTP_LRLIST, UTFLRListMsgProc, NULL);
}

HUIWND UTFAPI UTFCreateLRList(DWORD ctrlID, int textID, char *text, DWORD dwStyle, DWORD dwExStyle,
						HUIWND hDlg, WORD left, WORD top, WORD right, WORD bottom,
						DWORD ctrlData, UTFONDRAW OnDraw, UTFCALLBACK CallBack)
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
		HUIWND hCtrl = (HUIWND)UTFMalloc(sizeof(UTFLRLIST));
		
		if(hCtrl != NULL)
		{
			LPUTFLRLIST pCtrl = (LPUTFLRLIST)hCtrl;

			memcpy(pCtrl->type, WTP_LRLIST, 4);
			pCtrl->wndID	= ctrlID;
			
			pCtrl->dwStyle	= dwStyle | UIS_CHILD | UIS_OWNERDRAW;
			pCtrl->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST | UIS_NOFOCUS);
			pCtrl->dwExStyle = dwExStyle;
			pCtrl->dwFlags	= 0;
			
			pCtrl->hParent	= hDlg;
			pCtrl->dataEx	= ctrlData;
			pCtrl->rect 	= rcRect;
			pCtrl->DrawFace = UTFLRListInvalidate;
			pCtrl->CallBack	= CallBack;
			if(dwStyle & UIS_OWNERDRAW)
			{
				pCtrl->OnDraw = OnDraw;
			}
			else
			{
				pCtrl->OnDraw = NULL;
			}
			
			pCtrl->dwCurSel = 0;
			pCtrl->dwTotalSel = 0;
			pCtrl->lpSelInfo = NULL;

			if(text != NULL)
			{
				pCtrl->textID = -1;
				if(strlen(text) >= TEXT_SIZE)
				{
					memcpy((char *)pCtrl->wndText, text, TEXT_SIZE-1);
					pCtrl->wndText[TEXT_SIZE-1] = 0;
				}
				else
				{
					strcpy((char *)pCtrl->wndText, text);
				}
			}
			else
			{
				pCtrl->textID = textID;
			}

			UTFAddCtrlToDlg(hDlg, hCtrl);
		}

		return hCtrl;
	}

	return NULL;
}

int UTFAPI UTFLRListSetInfo(HUIWND hLRList, DWORD dwTotalSel, LPUTFSELINFO lpInfo)
{
	if(hLRList == NULL)
		return 0;

	if(UTFWndIsType(hLRList, WTP_LRLIST) == TRUE)
	{
		LPUTFLRLIST pCtrl = (LPUTFLRLIST)hLRList;

		pCtrl->dwTotalSel = dwTotalSel;
		if(pCtrl->dwTotalSel > 0)
		{
			pCtrl->lpSelInfo = lpInfo;
		}
		else
		{
			pCtrl->lpSelInfo = NULL;
		}
		
		if(pCtrl->dwCurSel >= pCtrl->dwTotalSel)
		{
			pCtrl->dwCurSel = 0;
		}
		pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWSEL;

		if(UTFDialogHaveFocus(pCtrl->hParent) == FALSE)
		{
			UTFSendPrivMessage(hLRList, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
		}
		else if(pCtrl->dwFlags & UIF_FOCUS)
		{
			if(UTFSendPrivMessage(hLRList, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0) == 0)
			{
				UTFSendPrivMessage(pCtrl->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
			}
		}
		
		return 1;
	}

	return 0;
}

int UTFAPI UTFLRListSetCurSel(HUIWND hLRList, DWORD dwCurSel)
{
	if(hLRList == NULL)
		return 0;

	if(UTFWndIsType(hLRList, WTP_LRLIST) == TRUE)
	{
		LPUTFLRLIST pCtrl = (LPUTFLRLIST)hLRList;

		if(dwCurSel >= pCtrl->dwTotalSel)
		{
			return 0;
		}

		if(pCtrl->dwCurSel != dwCurSel)
		{
			pCtrl->dwCurSel = dwCurSel;
			pCtrl->dwFlags |= UIF_REDRAW|LRF_DRAWSEL;
		}
		
		return 1;
	}

	return 0;
}

DWORD UTFAPI UTFLRListGetCurSel(HUIWND hLRList)
{
	if(hLRList == NULL)
		return 0;

	if(UTFWndIsType(hLRList, WTP_LRLIST) == TRUE)
	{
		LPUTFLRLIST pCtrl = (LPUTFLRLIST)hLRList;

		return pCtrl->dwCurSel;
	}

	return 0;
}

DWORD UTFAPI UTFLRListGetTotalSel(HUIWND hLRList)
{
	if(hLRList == NULL)
		return 0;

	if(UTFWndIsType(hLRList, WTP_LRLIST) == TRUE)
	{
		LPUTFLRLIST pCtrl = (LPUTFLRLIST)hLRList;

		return pCtrl->dwTotalSel;
	}

	return 0;
}

