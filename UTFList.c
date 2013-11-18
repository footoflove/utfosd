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
#include "UTFZOrderManagerPriv.h"
#include "UTFDialog.h"
#include "UTFDialogPriv.h"
#include "UTFWndManage.h"
#include "UTFWndDefPriv.h"
#include "UTFMsgDefPriv.h"
#include "UTFDrawAPI.h"
#include "UTFDrawText.h"
#include "UTFFont.h"
#include "UTFList.h"
#include "UTFListPriv.h"
#include "UTFScrollBar.h"
#include "UTFScrollBarPriv.h"
#include "UTFMemory.h"

/******************************************************************/
static void UTFDrawListFace(HUIWND hList, UTFRECT rcRect, DWORD info)
{
	LPUILIST pList = (LPUILIST)hList;
	UTFRECT wndRect = rcRect;
	DWORD uFormat;
 
	/* Draw background */
	UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
	if(pList->dwFlags & UIF_FOCUS)
	{
		UTFSetTextColor(UTFRGB(255,255,255,255));
		UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,100,255), 0);
		UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
		
		UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,150,255), 0);
		UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.top+25);
	}
	else
	{
		UTFSetTextColor(UTFRGB(0,0,0,255));
		UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,100,255), 0);
		UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
		
		UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,200,255), 0);
		UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.top+25);
	}
	
	if(pList->textID >= 0)
	{
		UTFGetIDText(pList->textID, pList->wndText, TEXT_SIZE);
	}

	uFormat = DTUI_CENTER|DTUI_TOP|DTUI_SINGLELINE|DTUI_END_ELLIPSIS;
	UTFDrawText(pList->wndText, &wndRect, uFormat);
}

static void UTFListDrawRow(HUIWND hList, UTFRECT rcRect, DWORD info)
{
	LPLISTROWINFO lpInfo = (LPLISTROWINFO)info;
	LPUILIST pList = (LPUILIST)hList;
	UTFRECT wndRect = rcRect;
	DWORD uFormat;
	BYTE text[50];

	/* Draw background */	
	if(lpInfo->dwFlags & UIF_FOCUS)
	{
		UTFSetTextColor(UTFRGB(0,0,0,255));
		UTFSetBrush(BSUI_SOLID, UTFRGB(220,120,70,255), HSUI_HORIZONTAL);
	}
	else
	{
		UTFSetTextColor(UTFRGB(255,255,255,255));
		UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,100,255), HSUI_HORIZONTAL);
	}
	
	UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
	UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);

	if(lpInfo->dwFlags & LIF_VISIBLE)
	{
		sprintf((char *)text, "%d", pList->dwFirst+lpInfo->pos);
		uFormat = DTUI_CENTER|DTUI_VCENTER|DTUI_SINGLELINE|DTUI_END_ELLIPSIS;
		UTFDrawText(text, &wndRect, uFormat);
	}
}

static void UTFListInvalidate(HUIWND hList, UTFRECT rcRect, DWORD info)
{
	LPUILIST pList = (LPUILIST)hList;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pList->hParent;
	LISTROWINFO rowInfo;
	DWORD i,bDrawScrollBar = FALSE;
	UTFONDRAW pDrawRow;
	UTFRECT drawRect;

	if(pList->dwFlags & UIF_DRAWFACE)
	{
		if(pList->OnDraw)
		{
			pList->OnDraw(hList, rcRect, 0);
		}
		else
		{
			UTFDrawListFace(hList, rcRect, 0);
		}	

		pList->dwFlags &= ~UIF_DRAWFACE;
		bDrawScrollBar = TRUE;
	}

	if(pList->pItemInfo == NULL)
		return;

	if(pList->OnDrawRow)
	{
		pDrawRow = pList->OnDrawRow;
	}
	else
	{
		pDrawRow = UTFListDrawRow;
	}	

	for(i=0; i<pList->dwPageNum; i++)
	{
		if(pList->pItemInfo[i].dwFlags & UIF_REDRAW)
		{
			rowInfo.dwFlags = pList->pItemInfo[i].dwFlags & UIF_FOCUS;
			if(i < pList->dwTotal)
			{
				rowInfo.dwFlags |= LIF_VISIBLE;
			}
			else
			{
				rowInfo.dwFlags &= ~UIF_FOCUS;
			}
			rowInfo.colume = i/pList->dwSidePageNum;
			rowInfo.pos = i;

			drawRect = pList->pItemInfo[i].rect;
			drawRect.left += pDlg->rect.left;
			drawRect.right += pDlg->rect.left;
			drawRect.top += pDlg->rect.top;
			drawRect.bottom += pDlg->rect.top;
			pDrawRow(hList, drawRect, (DWORD)&rowInfo);
		
			pList->pItemInfo[i].dwFlags &= ~UIF_REDRAW;
		}
	}

	if(bDrawScrollBar == TRUE)
	{
		LPUTFDIALOGCTRL pCtrl = (LPUTFDIALOGCTRL)pList->hScrollBar;

		if(pCtrl != NULL)
		{
			pCtrl->dwFlags |= UIF_REDRAW|SBF_DRAWFRAME;
		}
	}
}

static int UTFListFocusForward(HUIWND hList)
{
	LPUILIST pList = (LPUILIST)hList;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pList->hParent;
	int bPaint = FALSE;
	DWORD oldFirst,oldFocus;
	DWORD i;

	if(pList->dwTotal > 1)
	{
		bPaint = TRUE;

		pList->dwFlags |= UIF_REDRAW;
		
		oldFirst = pList->dwFirst;
		oldFocus = pList->dwFocus;
		if(pList->dwFocus > 0)
			pList->dwFocus--;
		else
			pList->dwFocus = pList->dwTotal-1;
		
		if(pList->hScrollBar)
		{
			UTFScrollBarSetScrollPos(pList->hScrollBar, pList->dwFocus);
		}

		if(pList->dwTotal > pList->dwPageNum)
		{
			if(pList->dwFocus < pList->dwFirst)
			{
				if(pList->dwFirst > pList->dwPageNum)
				{
					pList->dwFirst -= pList->dwPageNum;
				}
				else
				{
					pList->dwFirst = 0;
				}
			}
			else if(pList->dwFocus >= (pList->dwFirst+pList->dwPageNum))
			{
				pList->dwFirst = pList->dwTotal-pList->dwPageNum;
			}

			// notify dialog that list need scroll page to update data
			if(oldFirst != pList->dwFirst)
			{
				for(i=0; i<pList->dwPageNum; i++)
				{
					pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
					pList->pItemInfo[i].dwFlags &= ~UIF_FOCUS;

					if((i+pList->dwFirst) == pList->dwFocus)
					{
						pList->pItemInfo[i].dwFlags |= UIF_FOCUS;
					}
				}
				
				if(pDlg->CallBack)
				{
					pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_PAGESCROLL, pList->wndID, pList->dwFirst);
				}
			}
			else
			{
				pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
				pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags &= ~UIF_FOCUS;
				pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
				pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_FOCUS;
			}
		}
		else
		{
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags &= ~UIF_FOCUS;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_FOCUS;
		}

		if(pDlg->CallBack)
		{
			pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_FOCUSMOVE, pList->wndID, pList->dwFocus);
		}
	}

	return bPaint;
}

static int UTFListFocusBack(HUIWND hList)
{
	LPUILIST pList = (LPUILIST)hList;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pList->hParent;
	int bPaint = FALSE;
	DWORD oldFirst,oldFocus;
	DWORD i;

	if(pList->dwTotal > 1)
	{
		bPaint = TRUE;
		
		pList->dwFlags |= UIF_REDRAW;

		oldFirst = pList->dwFirst;
		oldFocus = pList->dwFocus;
		if(++pList->dwFocus >= pList->dwTotal)
		{
			pList->dwFocus = 0;
		}
		
		if(pList->hScrollBar)
		{
			UTFScrollBarSetScrollPos(pList->hScrollBar, pList->dwFocus);
		}

		if(pList->dwTotal > pList->dwPageNum)
		{
			if(pList->dwFocus >= (pList->dwFirst+pList->dwPageNum))
			{
				if((pList->dwFocus+pList->dwPageNum) < pList->dwTotal)
				{
					pList->dwFirst += pList->dwPageNum;
				}
				else
				{
					pList->dwFirst = pList->dwTotal-pList->dwPageNum;
				}
			}
			else if(pList->dwFocus < pList->dwFirst)
			{
				pList->dwFirst = pList->dwFocus;
			}

			// notify dialog that list need scroll page to update data
			if(oldFirst != pList->dwFirst)
			{
				for(i=0; i<pList->dwPageNum; i++)
				{
					pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
					pList->pItemInfo[i].dwFlags &= ~UIF_FOCUS;

					if((i+pList->dwFirst) == pList->dwFocus)
					{
						pList->pItemInfo[i].dwFlags |= UIF_FOCUS;
					}
				}
				
				if(pDlg->CallBack)
				{
					pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_PAGESCROLL, pList->wndID, pList->dwFirst);
				}
			}
			else
			{
				pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
				pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags &= ~UIF_FOCUS;
				pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
				pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_FOCUS;
			}
		}
		else
		{
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags &= ~UIF_FOCUS;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_FOCUS;
		}

		if(pDlg->CallBack)
		{
			pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_FOCUSMOVE, pList->wndID, pList->dwFocus);
		}
	}

	return bPaint;
}

static int UTFListFocusPageUp(HUIWND hList)
{
	LPUILIST pList = (LPUILIST)hList;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pList->hParent;
	int bPaint = FALSE;
	DWORD oldFirst,oldFocus;
	DWORD i;

	if(pList->dwTotal > pList->dwPageNum)
	{
		bPaint = TRUE;

		pList->dwFlags |= UIF_REDRAW;
		
		oldFirst = pList->dwFirst;
		oldFocus = pList->dwFocus;

		if(pList->dwFocus >= pList->dwPageNum)
		{
			pList->dwFocus -= pList->dwPageNum;
		}
		else if(pList->dwFocus)
		{
			pList->dwFocus = 0;
		}
		else
		{
			pList->dwFocus = pList->dwTotal-1;
		}
		
		if(pList->hScrollBar)
		{
			UTFScrollBarSetScrollPos(pList->hScrollBar, pList->dwFocus);
		}

		if(pList->dwFirst >= pList->dwPageNum)
		{
			pList->dwFirst -= pList->dwPageNum;
		}
		else if(pList->dwFirst)
		{
			pList->dwFirst = 0;
		}
		else if(oldFocus == 0)
		{
			pList->dwFirst = pList->dwTotal-pList->dwPageNum;
		}

		// notify dialog that list need scroll page to update data
		if(oldFirst != pList->dwFirst)
		{
			for(i=0; i<pList->dwPageNum; i++)
			{
				pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
				pList->pItemInfo[i].dwFlags &= ~UIF_FOCUS;

				if((i+pList->dwFirst) == pList->dwFocus)
				{
					pList->pItemInfo[i].dwFlags |= UIF_FOCUS;
				}
			}
			
			if(pDlg->CallBack)
			{
				pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_PAGESCROLL, pList->wndID, pList->dwFirst);
			}
		}
		else if(oldFocus != pList->dwFocus)
		{
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags &= ~UIF_FOCUS;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_FOCUS;
		}

		if(oldFocus != pList->dwFocus)
		{
			if(pDlg->CallBack)
			{
				pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_FOCUSMOVE, pList->wndID, pList->dwFocus);
			}
		}
	}

	return bPaint;
}

static int UTFListFocusPageDn(HUIWND hList)
{
	LPUILIST pList = (LPUILIST)hList;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pList->hParent;
	int bPaint = FALSE;
	DWORD oldFirst,oldFocus;
	DWORD i;

	if(pList->dwTotal > pList->dwPageNum)
	{
		bPaint = TRUE;

		pList->dwFlags |= UIF_REDRAW;
		
		oldFirst = pList->dwFirst;
		oldFocus = pList->dwFocus;

		if(pList->dwFocus < (pList->dwTotal-1))
		{
			pList->dwFocus += pList->dwPageNum;
			if(pList->dwFocus >= pList->dwTotal)
			{
				pList->dwFocus = pList->dwTotal-1;
			}
		}
		else
		{
			pList->dwFocus = 0;
		}

		if(pList->hScrollBar)
		{
			UTFScrollBarSetScrollPos(pList->hScrollBar, pList->dwFocus);
		}

		if(pList->dwFocus < pList->dwFirst)
		{
			pList->dwFirst = pList->dwFocus;
			if((pList->dwFirst+pList->dwPageNum) > pList->dwTotal)
			{
				pList->dwFirst = pList->dwTotal-pList->dwPageNum;
			}
		}
		else
		{
			if((pList->dwFocus-oldFocus) == pList->dwPageNum)
			{
				pList->dwFirst += pList->dwPageNum;
				if((pList->dwFirst + pList->dwPageNum) > pList->dwTotal)
				{
					pList->dwFirst = pList->dwTotal-pList->dwPageNum;
				}
			}
			else if((pList->dwFocus-oldFocus) > 0)
			{
				pList->dwFirst = pList->dwTotal-pList->dwPageNum;
			}
		}

		// notify dialog that list need scroll page to update data
		if(oldFirst != pList->dwFirst)
		{
			for(i=0; i<pList->dwPageNum; i++)
			{
				pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
				pList->pItemInfo[i].dwFlags &= ~UIF_FOCUS;

				if((i+pList->dwFirst) == pList->dwFocus)
				{
					pList->pItemInfo[i].dwFlags |= UIF_FOCUS;
				}
			}
			
			if(pDlg->CallBack)
			{
				pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_PAGESCROLL, pList->wndID, pList->dwFirst);
			}
		}
		else if(oldFocus != pList->dwFocus)
		{
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags &= ~UIF_FOCUS;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_FOCUS;
		}

		if(oldFocus != pList->dwFocus)
		{
			if(pDlg->CallBack)
			{
				pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_FOCUSMOVE, pList->wndID, pList->dwFocus);
			}
		}
	}

	return bPaint;
}

static int UTFListFocusLeft(HUIWND hList)
{
	LPUILIST pList = (LPUILIST)hList;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pList->hParent;
	int bPaint = FALSE;
	DWORD oldFirst,oldFocus;
	DWORD i;

	if(pList->dwTotal > pList->dwSidePageNum)
	{
		bPaint = TRUE;

		pList->dwFlags |= UIF_REDRAW;
		
		oldFirst = pList->dwFirst;
		oldFocus = pList->dwFocus;

		if(pList->dwFocus >= pList->dwSidePageNum)
		{
			pList->dwFocus -= pList->dwSidePageNum;
		}
		else
		{
			pList->dwFocus = 0;
		}

		if(pList->hScrollBar)
		{
			UTFScrollBarSetScrollPos(pList->hScrollBar, pList->dwFocus);
		}

		if(pList->dwTotal > pList->dwPageNum)
		{
			if(pList->dwFocus < pList->dwFirst)
			{
				if(pList->dwFirst > pList->dwPageNum)
				{
					pList->dwFirst -= pList->dwPageNum;
				}
				else
				{
					pList->dwFirst = 0;
				}
			}
			else if(pList->dwFocus >= (pList->dwFirst+pList->dwPageNum))
			{
				pList->dwFirst = pList->dwTotal-pList->dwPageNum;
			}
		}

		// notify dialog that list need scroll page to update data
		if(oldFirst != pList->dwFirst)
		{
			for(i=0; i<pList->dwPageNum; i++)
			{
				pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
				pList->pItemInfo[i].dwFlags &= ~UIF_FOCUS;

				if((i+pList->dwFirst) == pList->dwFocus)
				{
					pList->pItemInfo[i].dwFlags |= UIF_FOCUS;
				}
			}
			
			if(pDlg->CallBack)
			{
				pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_PAGESCROLL, pList->wndID, pList->dwFirst);
			}
		}
		else if(oldFocus != pList->dwFocus)
		{
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags &= ~UIF_FOCUS;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_FOCUS;
		}

		if(oldFocus != pList->dwFocus)
		{
			if(pDlg->CallBack)
			{
				pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_FOCUSMOVE, pList->wndID, pList->dwFocus);
			}
		}
	}

	return bPaint;
}

static int UTFListFocusRight(HUIWND hList)
{
	LPUILIST pList = (LPUILIST)hList;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pList->hParent;
	int bPaint = FALSE;
	DWORD oldFirst,oldFocus;
	DWORD i;

	if(pList->dwTotal > pList->dwSidePageNum)
	{
		bPaint = TRUE;

		pList->dwFlags |= UIF_REDRAW;
		
		oldFirst = pList->dwFirst;
		oldFocus = pList->dwFocus;

		pList->dwFocus += pList->dwSidePageNum;
		if(pList->dwFocus >= pList->dwTotal)
		{
			pList->dwFocus = pList->dwTotal-1;
		}

		if(pList->hScrollBar)
		{
			UTFScrollBarSetScrollPos(pList->hScrollBar, pList->dwFocus);
		}

		if(pList->dwTotal > pList->dwPageNum)
		{
			if(pList->dwFocus >= (pList->dwFirst+pList->dwPageNum))
			{
				if((pList->dwFocus+pList->dwPageNum) < pList->dwTotal)
				{
					pList->dwFirst += pList->dwPageNum;
				}
				else
				{
					pList->dwFirst = pList->dwTotal-pList->dwPageNum;
				}
			}
			else if(pList->dwFocus < pList->dwFirst)
			{
				pList->dwFirst = pList->dwFocus;
			}
		}

		// notify dialog that list need scroll page to update data
		if(oldFirst != pList->dwFirst)
		{
			for(i=0; i<pList->dwPageNum; i++)
			{
				pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
				pList->pItemInfo[i].dwFlags &= ~UIF_FOCUS;

				if((i+pList->dwFirst) == pList->dwFocus)
				{
					pList->pItemInfo[i].dwFlags |= UIF_FOCUS;
				}
			}
			
			if(pDlg->CallBack)
			{
				pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_PAGESCROLL, pList->wndID, pList->dwFirst);
			}
		}
		else if(oldFocus != pList->dwFocus)
		{
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags &= ~UIF_FOCUS;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_FOCUS;
		}

		if(oldFocus != pList->dwFocus)
		{
			if(pDlg->CallBack)
			{
				pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_FOCUSMOVE, pList->wndID, pList->dwFocus);
			}
		}
	}

	return bPaint;
}

static int UTFListMsgProc(HUIWND hList, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUILIST pList = (LPUILIST)hList;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pList->hParent;
	int retCode = 1;
	DWORD i;

	switch(uMsg)
	{
	case WMUI_PUBLIC:
		switch(param1)
		{
		case WMUI_CREATE:
			pList->dwFlags |= UIF_DRAWFACE;
			pList->dwFlags |= UIF_REDRAW;
			for(i=0; i<pList->dwPageNum; i++)
			{
				pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
			}

			if(pList->dwTotal)
			{
				if(pDlg->CallBack)
				{
					pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_PAGESCROLL, pList->wndID, pList->dwFirst);
				}
			}
			break;

		case WMUI_DESTROY:
			break;

		case WMUI_SETFOCUS:
			if((pList->dwStyle & ( UIS_DISABLE | UIS_HIDE )) || !(pList->dwStyle & UIS_TABSTOP) || (pList->dwTotal == 0))
			{
				retCode = 0;
			}
			else if( !(pList->dwFlags & UIF_FOCUS) )
			{
				HUIWND hFocusCtrl = UTFDlgGetFocus(pList->hParent);

				pList->dwFlags |= UIF_FOCUS;
				pList->dwFlags |= UIF_REDRAW;
				for(i=0; i<pList->dwPageNum; i++)
				{
					pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
				}

				UTFSendPrivMessage(pList->hParent, WMUI_INTER, WMUI_NEWFOCUS, hList, hFocusCtrl);
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
		case KEYUI_SELECT:
			if(pList->dwTotal)
			{
				if(pDlg->CallBack)
				{
					if(pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_CLICKED, pList->wndID, pList->dwFocus))
					{
						pList->dwFlags |= UIF_REDRAW;
						pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
					}
				}
			}
			break;

		case KEYUI_UP:
			UTFListFocusForward(hList);
			break;

		case KEYUI_DOWN:
			UTFListFocusBack(hList);
			break;

		case KEYUI_LEFT:
			if(pList->dwPageNum > pList->dwSidePageNum)
			{
				UTFListFocusLeft(hList);
			}
			else
			{
				retCode = 0;
			}
			break;

		case KEYUI_RIGHT:
			if(pList->dwPageNum > pList->dwSidePageNum)
			{
				UTFListFocusRight(hList);
			}
			else
			{
				retCode = 0;
			}
			break;

		case KEYUI_PAGEUP:
			UTFListFocusPageUp(hList);
			break;

		case KEYUI_PAGEDN:
			UTFListFocusPageDn(hList);
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
			pList->dwFlags |= UIF_DRAWFACE;
			pList->dwFlags |= UIF_REDRAW;
			for(i=0; i<pList->dwPageNum; i++)
			{
				pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
			}
			if(pList->hScrollBar)
			{
				UTFSendPrivMessage(pList->hScrollBar, uMsg, param1, 0, 0);
			}
			break;

		case WMUI_KILLFOCUS:
			pList->dwFlags &= ~UIF_FOCUS;
			pList->dwFlags |= UIF_REDRAW;
			for(i=0; i<pList->dwPageNum; i++)
			{
				pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
			}
			break;

		case WMUI_ENABLE:
			if(pList->dwStyle & UIS_DISABLE)
			{
				pList->dwStyle &= ~UIS_DISABLE;
				pList->dwFlags |= UIF_REDRAW;
				for(i=0; i<pList->dwPageNum; i++)
				{
					pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
				}

				if(UTFDialogHaveFocus(pList->hParent) == FALSE)
				{
					UTFSendPrivMessage(hList, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}
			}
			break;

		case WMUI_DISABLE:
			if((pList->dwStyle & UIS_DISABLE) != UIS_DISABLE)
			{
				pList->dwStyle |= UIS_DISABLE;
				pList->dwFlags |= UIF_REDRAW;
				for(i=0; i<pList->dwPageNum; i++)
				{
					pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
				}

				if(pList->dwFlags & UIF_FOCUS)
				{
					UTFSendPrivMessage(pList->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
					pList->dwFlags &= ~UIF_FOCUS;
				}
			}
			break;

		case WMUI_SHOW:
			if(pList->dwStyle & UIS_HIDE)
			{
				pList->dwStyle &= ~UIS_HIDE;
				pList->dwFlags |= UIF_REDRAW;
				pList->dwFlags |= UIF_DRAWFACE;
				for(i=0; i<pList->dwPageNum; i++)
				{
					pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
				}

				UTFDialogDrawAllChildAboveCtrl(pList->hParent, hList);

				if(UTFDialogHaveFocus(pList->hParent) == FALSE)
				{
					UTFSendPrivMessage(hList, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}

				if(pList->hScrollBar)
				{
					UTFSendPrivMessage(pList->hScrollBar, uMsg, param1, param2, param3);
				}
			}
			break;

		case WMUI_HIDE:
			if((pList->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pList->dwStyle |= UIS_HIDE;
				pList->dwFlags |= UIF_REDRAW;
				pList->dwFlags |= DLGF_CLEANBACK;

				if(pList->dwFlags & UIF_FOCUS)
				{
					UTFSendPrivMessage(pList->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
					pList->dwFlags &= ~UIF_FOCUS;
				}

				UTFDialogRedrawCtrlInRect(pList->hParent, &pList->rect);

				if(pList->hScrollBar)
				{
					UTFSendPrivMessage(pList->hScrollBar, uMsg, param1, param2, param3);
				}
			}
			break;

		case WMUI_SETRECT:
			if(param2)
			{
				LPUTFRECT lpRect = (LPUTFRECT)param2;
				UTFRECT oldRect = pList->rect;
				int deltaX,deltaY;
				int Width,Height;

				Width = pList->rect.right-pList->rect.left;
				Height = pList->rect.bottom-pList->rect.top;
				if(((lpRect->left+Width) > (pDlg->rect.right-pDlg->rect.left)) || ((lpRect->top+Height) > (pDlg->rect.bottom-pDlg->rect.top)))
					return 0;
				
				deltaX = lpRect->left-pList->rect.left;
				deltaY = lpRect->top-pList->rect.top;

				pList->dwFlags |= UIF_REDRAW;
				pList->dwFlags |= UIF_DRAWFACE;
				for(i=0; i<pList->dwPageNum; i++)
				{
					pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
					
					pList->pItemInfo[i].rect.left += deltaX;
					pList->pItemInfo[i].rect.right += deltaX;
					pList->pItemInfo[i].rect.top += deltaY;
					pList->pItemInfo[i].rect.bottom += deltaY;
				}

				if(pList->hScrollBar)
				{
					LPUTFDIALOGCTRL pCtrl = (LPUTFDIALOGCTRL)pList->hScrollBar;
					UTFRECT rcRect = pCtrl->rect;

					UTFOffsetRect(&rcRect, deltaX, deltaY);

					pCtrl->dwExStyle &= ~SBS_ATTACH_TO_CHILD;
					UTFSendPrivMessage(pList->hScrollBar, WMUI_INTER, WMUI_SETRECT, (DWORD)&rcRect, 0);
					pCtrl->dwExStyle |= SBS_ATTACH_TO_CHILD;
				}
				
				UTFDialogBackClean(pList->hParent, hList);
				UTFDialogRedrawCtrlInRect(pList->hParent, &pList->rect);
			
				UTFOffsetRect(&pList->rect, deltaX, deltaY);
				UTFDialogDrawAllChildAboveCtrl(pList->hParent, hList);

				if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
				{
					UTFDialogResetCtrlXYOrder(pList->hParent, hList, oldRect);
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
						strcpy((char *)pList->wndText, (char *)param2);
					}
					else
					{
						memcpy((char *)pList->wndText, (char *)param2, TEXT_SIZE);
						pList->wndText[TEXT_SIZE-1] = 0;
					}
					pList->textID = -1;			
				}
				else
				{
					pList->textID = param3;
				}
				pList->dwFlags |= UIF_REDRAW;
				pList->dwFlags |= UIF_DRAWFACE;
				for(i=0; i<pList->dwPageNum; i++)
				{
					pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
				}
			}
			break;

		case WMUI_GETTEXT:
			if((param2 != NULL) && (param3 > 0))
			{
				if(pList->textID >= 0)
				{
					UTFGetIDText(pList->textID, pList->wndText, TEXT_SIZE);
				}
				if(strlen((char *)pList->wndText) < param3)
				{
					strcpy((char *)param2, (char *)pList->wndText);
				}
				else
				{
					char *pBuffer = (char *)param2;
					
					memcpy(pBuffer, (char *)pList->wndText, param3);
					*(pBuffer+param3-1) = 0;
				}
			}
			break;	

		case WMUI_DELETEWND:
			if(param2 == 0)
			{
				LPUTFDIALOGCTRL pCtrl;			

				if(pList->hPrevCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pList->hPrevCtrl;
					pCtrl->hNextCtrl = pList->hNextCtrl;
				}
				else
				{
					pDlg->pCtrl = (LPUTFDIALOGCTRL)pList->hNextCtrl;
				}

				if(pList->hNextCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pList->hNextCtrl;
					pCtrl->hPrevCtrl = pList->hPrevCtrl;
				}
				
				UTFDeleteWindow(pList->hScrollBar);
			}
			UTFFree((DWORD)pList->pItemInfo);
			UTFFree(hList);
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

void UTFListInitialize(void)
{
	UTFRegisterWndType(WTP_LIST, UTFListMsgProc, NULL);
}

HUIWND UTFAPI UTFCreateList(DWORD listID, int textID, char *text, DWORD dwStyle, DWORD dwExStyle,
						HUIWND hDlg, WORD left, WORD top, WORD right, WORD bottom,
						DWORD dataEx, UTFONDRAW OnDraw, UTFCALLBACK CallBack)
{
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hDlg;
	UTFRECT rcRect;

	if(hDlg == NULL)
	{
		return NULL;
	}

	rcRect.left	= left;
	rcRect.top	= top;
	rcRect.right = right;
	rcRect.bottom = bottom;
	UTFNormalizeRect(&rcRect);
	if(UTFIsRectEmpty(&rcRect))
		return NULL;
	
	if((rcRect.right > (pDlg->rect.right-pDlg->rect.left)) || (rcRect.bottom > (pDlg->rect.bottom-pDlg->rect.top)))
		return NULL;

	if(((rcRect.right-rcRect.left) < 20) || ((rcRect.bottom-rcRect.top) < 24))
		return NULL;
	
	if(UTFIsDialog(hDlg) == TRUE)
	{
		HUIWND hList = (HUIWND)UTFMalloc(sizeof(UILIST));
		
		if(hList != NULL)
		{
			LPUILIST pList = (LPUILIST)hList;

			memcpy(pList->type, WTP_LIST, 4);
			pList->wndID	= listID;
			
			pList->dwStyle	= dwStyle | UIS_CHILD | UIS_OWNERDRAW;
			pList->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST | UIS_NOFOCUS);
			pList->dwExStyle = dwExStyle;
			pList->dwFlags	= 0;
			
			pList->hParent	= hDlg;
			pList->hScrollBar = NULL;
			pList->dataEx	= dataEx;
			pList->rect 	= rcRect;

			pList->drawList = UTFListInvalidate;
			pList->CallBack	= CallBack;
			pList->OnDrawRow = NULL;
			if(dwStyle & UIS_OWNERDRAW)
			{
				pList->OnDraw = OnDraw;
				pList->dwExStyle |= LSTS_OWNERDRAW;
			}
			else
			{
				pList->OnDraw = NULL;
				pList->dwExStyle &= ~LSTS_OWNERDRAW;
			}

			pList->dwTotal = 0;
			pList->dwFirst = 0;
			pList->dwFocus = 0;
			pList->dwPageNum = 0;
			pList->dwSidePageNum = 0;
			pList->pItemInfo = NULL;

			if(text != NULL)
			{
				pList->textID = -1;
				if(strlen(text) >= TEXT_SIZE)
				{
					memcpy((char *)pList->wndText, text, TEXT_SIZE-1);
					pList->wndText[TEXT_SIZE-1] = 0;
				}
				else
				{
					strcpy((char *)pList->wndText, text);
				}
			}
			else
			{
				pList->textID = textID;
			}

			UTFAddCtrlToDlg(hDlg, hList);
		}

		return hList;
	}

	return NULL;
}

int UTFAPI UTFListSetInfo(HUIWND hList, WORD rowTop, WORD rowWidth, WORD rowHeight, UTFONDRAW OnDrawRow,
							 HIMAGE hScrollImage, WORD SCRLBarArrowHeight, WORD scrollBarWidth, UTFONDRAW OnDrawScrollBar)
{
	LPUILIST pList = (LPUILIST)hList;
	UTFRECT wndRect,originRect;
	WORD listHeight,listWidth;
	WORD i,totalColume,left;

	if(UTFWndIsType(hList, WTP_LIST) == FALSE)
		return 0;

	if(pList->dwExStyle & LSTS_NOSCROLL)
	{
		scrollBarWidth = 0;
	}
	else
	{
		if(scrollBarWidth < 12)
			return 0;
	}

	listWidth = pList->rect.right-pList->rect.left;
	listHeight = pList->rect.bottom-pList->rect.top;
	if((rowHeight < 10) || (rowWidth < 5) || ((rowTop+rowHeight) > listHeight) || ((rowWidth+scrollBarWidth+8) > listWidth))
		return 0;

	totalColume = (listWidth-8-scrollBarWidth)/rowWidth;
	if(totalColume == 0)
		return 0;

	pList->dwSidePageNum = (listHeight-rowTop)/rowHeight;
	if(pList->dwSidePageNum == 0)
		return 0;
	
	pList->dwPageNum = totalColume * pList->dwSidePageNum;
	pList->pItemInfo = (LPUILISTROW)UTFMalloc(sizeof(UILISTROW)*pList->dwPageNum);
	if(pList->pItemInfo == NULL)
		return 0;

	if(pList->dwExStyle & LSTS_OWNERDRAW)
		pList->OnDrawRow = OnDrawRow;
	else
		pList->OnDrawRow = NULL;
	
	if(pList->dwExStyle & LSTS_NOSCROLL)
	{
		left = 1;
		if(pList->hScrollBar)
		{
			UTFDeleteWindow(pList->hScrollBar);
			pList->hScrollBar = NULL;
		}
	}
	else
	{
		WORD ScrollLeft;
		
		if(pList->dwExStyle & LSTS_LEFTBAR)
		{
			left = scrollBarWidth+2;
			ScrollLeft = pList->rect.left+3;
		}
		else
		{
			left = 0;
			ScrollLeft = pList->rect.right-scrollBarWidth-3;
		}
		
		if(pList->hScrollBar == NULL)
		{
			DWORD dwStyle = 0;

			if(pList->dwExStyle & LSTS_OWNERDRAW)
			{
				dwStyle |= UIS_OWNERDRAW;
			}
			if(pList->dwStyle & UIS_HIDE)
			{
				dwStyle |= UIS_HIDE;
			}
			pList->hScrollBar = UTFCreateScrollBar(0xFFFFFFFF, dwStyle, SBS_ATTACH_TO_CHILD, pList->hParent, hScrollImage,
												ScrollLeft, pList->rect.top+rowTop,
												ScrollLeft+scrollBarWidth,
												pList->rect.top+rowTop+pList->dwSidePageNum*rowHeight,
												pList->dwTotal,
												pList->dwPageNum, 
												pList->dwFocus,
												SCRLBarArrowHeight,
												0, OnDrawScrollBar);
			if(pList->hScrollBar == NULL)
				return 0;
		}
	}

	wndRect.left = pList->rect.left+3+left;
	wndRect.top = pList->rect.top+rowTop;
	wndRect.right = wndRect.left+rowWidth;
	wndRect.bottom = wndRect.top+rowHeight;
	originRect = wndRect;
	
	for(i=0; i<pList->dwPageNum; i++)
	{
		pList->pItemInfo[i].dwFlags = UIF_REDRAW;
		pList->pItemInfo[i].rect = wndRect;

		if(pList->dwTotal)
		{
			if((i+pList->dwFirst) == pList->dwFocus)
			{
				pList->pItemInfo[i].dwFlags |= UIF_FOCUS;
			}
		}

		if((i+1) % pList->dwSidePageNum)
		{
			wndRect.top += rowHeight;
			wndRect.bottom += rowHeight;
		}
		else
		{
			wndRect.top = originRect.top;
			wndRect.bottom = originRect.bottom;
			wndRect.left += rowWidth;
			wndRect.right += rowWidth;
			
		}		
	}

	return 1;
}

int UTFAPI UTFListSetTotal(HUIWND hList, DWORD dwTotal)
{
	LPUILIST pList = (LPUILIST)hList;
	int bParentVisible;
	DWORD i;

	if(UTFWndIsType(hList, WTP_LIST) == FALSE)
		return 0;

	if(pList->pItemInfo == NULL)
		return 0;

	pList->dwTotal = dwTotal;
	pList->dwFocus = 0;
	pList->dwFirst = 0;
	if(pList->hScrollBar)
	{
		UTFScrollBarSetScrollSize(pList->hScrollBar, pList->dwTotal);
	}

	pList->dwFlags |= UIF_REDRAW;
	for(i=0; i<pList->dwPageNum; i++)
	{
		pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
		pList->pItemInfo[i].dwFlags &= ~UIF_FOCUS;

		if(pList->dwTotal)
		{
			if((i+pList->dwFirst) == pList->dwFocus)
			{
				pList->pItemInfo[i].dwFlags |= UIF_FOCUS;
			}
		}
	}

	// notify dialog that list need scroll page to update data
	bParentVisible = UTFIsWindowVisible(pList->hParent);
	if(pList->dwTotal && (bParentVisible != UTFWND_INVISIBLE))
	{
		LPUTFDIALOG pDlg = (LPUTFDIALOG)pList->hParent;
		
		if(pDlg->CallBack)
		{
			pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_PAGESCROLL, pList->wndID, pList->dwFirst);
		}
	}

	if(UTFDialogHaveFocus(pList->hParent) == FALSE)
	{
		UTFSendPrivMessage(hList, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
	}
	else if(pList->dwFlags & UIF_FOCUS)
	{
		if(UTFSendPrivMessage(hList, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0) == 0)
		{
			UTFSendPrivMessage(pList->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
			pList->dwFlags &= ~UIF_FOCUS;
		}
	}
	
	return 1;
}

int UTFAPI UTFListSetFocus(HUIWND hList, DWORD dwNewFocus)
{
	LPUILIST pList = (LPUILIST)hList;
	LPUTFDIALOG pDlg;
	DWORD oldFocus,oldFirst;
	int bParentVisible;
	DWORD i;

	if(UTFWndIsType(hList, WTP_LIST) == FALSE)
		return 0;

	if(pList->pItemInfo == NULL)
		return 0;

	// avoid error input param data
	if((dwNewFocus >= pList->dwTotal) || (dwNewFocus == pList->dwFocus))
		return 0;

	pList->dwFlags |= UIF_REDRAW;
	
	oldFocus = pList->dwFocus;
	oldFirst = pList->dwFirst;
	pList->dwFocus = dwNewFocus;
	if(pList->hScrollBar)
	{
		UTFScrollBarSetScrollPos(pList->hScrollBar, pList->dwFocus);
	}
	
	if(pList->dwTotal > pList->dwPageNum)
	{
		pList->dwFirst = (pList->dwFocus/pList->dwPageNum)*pList->dwPageNum;
		if((pList->dwFirst+pList->dwPageNum) > pList->dwTotal)
		{
			pList->dwFirst = pList->dwTotal-pList->dwPageNum;
		}
	}
	else
	{
		pList->dwFirst = 0;
	}

	bParentVisible = UTFIsWindowVisible(pList->hParent);

	// notify dialog that list need scroll page to update data
	if(oldFirst != pList->dwFirst)
	{
		LPUTFDIALOG pDlg = (LPUTFDIALOG)pList->hParent;
		
		for(i=0; i<pList->dwPageNum; i++)
		{
			pList->pItemInfo[i].dwFlags |= UIF_REDRAW;
			pList->pItemInfo[i].dwFlags &= ~UIF_FOCUS;

			if((i+pList->dwFirst) == pList->dwFocus)
			{
				pList->pItemInfo[i].dwFlags |= UIF_FOCUS;
			}
		}

		if(bParentVisible != UTFWND_INVISIBLE)
		{
			if(pDlg->CallBack)
			{
				pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_PAGESCROLL, pList->wndID, pList->dwFirst);
			}
		}
	}
	else
	{
		pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
		pList->pItemInfo[oldFocus-pList->dwFirst].dwFlags &= ~UIF_FOCUS;
		pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_REDRAW;
		pList->pItemInfo[pList->dwFocus-pList->dwFirst].dwFlags |= UIF_FOCUS;
	}

	if(bParentVisible != UTFWND_INVISIBLE)
	{
		pDlg = (LPUTFDIALOG)pList->hParent;
		if(pDlg->CallBack)
		{
			pDlg->CallBack(pList->hParent, WMUI_NOTIFY, LNUI_FOCUSMOVE, pList->wndID, pList->dwFocus);
		}
	}
	
	return 1;
}

DWORD UTFAPI UTFListGetFocusPos(HUIWND hList)
{
	LPUILIST pList = (LPUILIST)hList;

	if(UTFWndIsType(hList, WTP_LIST) == FALSE)
		return 0;

	return (pList->dwFocus);
}

DWORD UTFAPI UTFListGetTopPos(HUIWND hList)
{
	LPUILIST pList = (LPUILIST)hList;

	if(UTFWndIsType(hList, WTP_LIST) == FALSE)
		return 0;

	return (pList->dwFirst);
}

DWORD UTFAPI UTFListGetTotal(HUIWND hList)
{
	LPUILIST pList = (LPUILIST)hList;

	if(UTFWndIsType(hList, WTP_LIST) == FALSE)
		return 0;

	return (pList->dwTotal);
}

DWORD UTFAPI UTFListGetPageSize(HUIWND hList)
{
	LPUILIST pList = (LPUILIST)hList;

	if(UTFWndIsType(hList, WTP_LIST) == FALSE)
		return 0;

	return (pList->dwPageNum);
}

HUIWND UTFAPI UTFListGetScrollBarHandle(HUIWND hList)
{
	LPUILIST pList = (LPUILIST)hList;

	if(UTFWndIsType(hList, WTP_LIST) == FALSE)
		return 0;

	return (pList->hScrollBar);
}

int UTFAPI UTFListPaintRow(HUIWND hList, WORD bPosInOnePage)
{
	LPUILIST pList = (LPUILIST)hList;

	if(UTFWndIsType(hList, WTP_LIST) == FALSE)
		return 0;

	if(bPosInOnePage >= pList->dwPageNum)
		return 0;

	if(pList->pItemInfo == NULL)
		return 0;

	pList->dwFlags |= UIF_REDRAW;
	pList->pItemInfo[bPosInOnePage].dwFlags |= UIF_REDRAW;

	return 1;
}

BYTE UTFAPI UTFIsListCtrl(HUIWND hWnd)
{
	return UTFWndIsType(hWnd, WTP_LIST);
}

