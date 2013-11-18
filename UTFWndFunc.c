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
#include "UTFMsgDefPriv.h"
#include "UTFWndDefPriv.h"
#include "UTFWndManage.h"
#include "UTFZOrderManage.h"
#include "UTFZOrderManagerPriv.h"
#include "UTFDrawAPI.h"

HUIWND UTFAPI UTFGetParent(HUIWND hWnd)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(hWnd != NULL)
	{
		return (lpUIWnd->hParent);
	}

	return NULL;
}

int UTFSetParent(HUIWND hWnd,HUIWND hParent)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(hParent == hWnd)
	{
		hParent = NULL;
	}
	
	if(hWnd != NULL)
	{
		lpUIWnd->hParent=hParent;
		return 1;
	}

	return 0;
}

HUIWND UTFGetMainParent(HUIWND hWnd)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(hWnd)
	{
		while(lpUIWnd->dwStyle & UIS_CHILD)
		{
			hWnd = lpUIWnd->hParent;
			if(hWnd == NULL)
				break;

			lpUIWnd = (LPUTFWND)hWnd;
		}

		return hWnd;
	}

	return NULL;
}

int UTFSendPrivMessage(HUIWND hWnd, DWORD uMsg, DWORD p1, DWORD p2, DWORD p3)
{
	return UTFDefMsgProc(hWnd, uMsg, p1, p2, p3);
}

int UTFAPI UTFDefProc(HUIWND hWnd, DWORD uMsg, DWORD p1, DWORD p2, DWORD p3)
{
	return UTFDefMsgProc(hWnd, uMsg, p1, p2, p3);
}

int UTFAPI UTFSendMessage(HUIWND hWnd, DWORD uMsg, DWORD p1, DWORD p2, DWORD p3)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(hWnd == NULL)
	{
		return 0;
	}

	if(lpUIWnd->CallBack != NULL)
	{
		int ret;

		if((uMsg == WMUI_PUBLIC) && (p1 == WMUI_CREATE))
		{
			lpUIWnd->CallBack(hWnd, uMsg, p1, p2, p3);

			ret = UTFDefMsgProc(hWnd, uMsg, p1, p2, p3);

			return ret;
		}
		else if((uMsg == WMUI_PUBLIC) && (p1 == WMUI_DESTROY))
		{
			ret = UTFDefMsgProc(hWnd, uMsg, p1, p2, p3);

			lpUIWnd->CallBack(hWnd, uMsg, p1, p2, p3);

			return ret;
		}
		else
		{
			ret = lpUIWnd->CallBack(hWnd, uMsg, p1, p2, p3);
			if(ret == 0)
			{
				return UTFDefMsgProc(hWnd, uMsg, p1, p2, p3);
			}
			else
			{
				return ret;
			}
		}
	}
	else
	{
		return UTFDefMsgProc(hWnd, uMsg, p1, p2, p3);
	}
}

int UTFAPI UTFEnableWindow(HUIWND hWnd, int bEnable)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(hWnd == NULL)
		return 0;

	if(lpUIWnd->dwStyle & UIS_CHILD)
	{
		if(bEnable == TRUE)
		{
			UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_ENABLE, 0, 0);
		}
		else
		{
			UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_DISABLE, 0, 0);
		}

		return 1;
	}

	return 0;
}

int UTFAPI UTFShowWindow(HUIWND hWnd, int bShow)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(hWnd == NULL)
		return 0;

	if(lpUIWnd->dwStyle & UIS_CHILD)
	{
		if(bShow == TRUE)
		{
			UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_SHOW, 0, 0);
		}
		else
		{
			UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_HIDE, 0, 0);
		}
	}
	else if(UTFWndCanBeShowOrHide(hWnd))
	{
		UTFRECT wndRect;
		BYTE bWindowMode;

		if(bShow == TRUE)
		{
			if(lpUIWnd->dwStyle & UIS_HIDE)
			{
				lpUIWnd->dwStyle &= ~UIS_HIDE;
				
				UTFSetOrderInfo(0);

				bWindowMode = UTFWindowSaveOrPutBack(hWnd, TRUE);
				if(bWindowMode == UTFWND_FREESHOW)
				{
					UTFInvalidateRect(hWnd, NULL, TRUE);
				}
				else if(bWindowMode == UTFWND_PARTSHOW)
				{
					UTFGetWindowRect(hWnd, &wndRect);
					UTFInvalidateRect(NULL, &wndRect, FALSE);
				}
			}
		}
		else
		{
			if(!(lpUIWnd->dwStyle & UIS_HIDE))
			{
				lpUIWnd->dwStyle |= UIS_HIDE;
				
				// Get window status before be hide
				bWindowMode = UTFWindowSaveOrPutBack(hWnd, FALSE);

				// Notice: this sentence cannot place after above sentence,
				// otherwise, window status will get error data
				UTFSetOrderInfo(0);
				
				if(bWindowMode != UTFWND_INVISIBLE)
				{
					UTFGetWindowRect(hWnd, &wndRect);
					UTFInvalidateRect(NULL, &wndRect, TRUE);
				}
			}
		}
	}

	return 1;
}

int UTFAPI UTFDeleteWindow(HUIWND hWnd)
{
	if(hWnd == NULL)
		return 0;

	UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_DELETEWND, 0, 0);

	return 1;
}

int UTFAPI UTFSetFocus(HUIWND hWnd)
{
	if(hWnd == NULL)
		return 0;

	UTFSendPrivMessage(hWnd, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);

	return 1;
}

int UTFAPI UTFWndIsType(HUIWND hWnd, char *pTypeStr)
{
	LPUTFWND lpWnd = (LPUTFWND)hWnd;
	DWORD wndType,menuType;

	if(hWnd == NULL)
		return FALSE;

	memcpy(&wndType, lpWnd->type, 4);
	memcpy(&menuType, pTypeStr, 4);
	if(wndType == menuType)
		return TRUE;

	return FALSE;
}

int UTFAPI UTFSetWindowText(HUIWND hWnd, const char *pBuffer)
{
	if((hWnd == NULL) || (pBuffer == NULL))
		return 0;

	UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_SETTEXT, (DWORD)pBuffer, 0);

	return 1;
}

int UTFAPI UTFSetWindowTextID(HUIWND hWnd, DWORD dwTextID)
{
	if(hWnd == NULL)
		return 0;

	UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_SETTEXT, NULL, dwTextID);

	return 1;
}

int UTFAPI UTFGetWindowText(HUIWND hWnd, char *pBuffer, DWORD nMaxCount)
{
	if((hWnd == NULL) || (pBuffer == NULL))
		return 0;

	UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_GETTEXT, (DWORD)pBuffer, nMaxCount);

	return 1;
}

int UTFAPI UTFGetWindowRect(HUIWND hWnd, LPUTFRECT lpRect)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if((hWnd == NULL) || (lpRect == NULL))
		return 0;

	memcpy(lpRect, &lpUIWnd->rect, sizeof(UTFRECT));

	return 1;
}

int UTFAPI UTFSetWindowRect(HUIWND hWnd, LPUTFRECT lpRect)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if((hWnd == NULL) || (lpRect == NULL))
		return 0;

	UTFNormalizeRect(lpRect);
	if(UTFIsRectEqual(&lpUIWnd->rect, lpRect))
		return 0;

	if(lpUIWnd->dwStyle & UIS_CHILD)
	{
		if(UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_SETRECT, (DWORD)lpRect, 0) == 0)
			return 0;
	}
	else
	{
		int OldVisibleFlag,NewVisibleFlag;
		UTFRECT rcScreenSize;
		UTFRECT rcRect,oldRect;

		if(lpUIWnd->dwStyle & UIS_SAVEBACK)
			return 0;

		UTFGetOSDRgnRect(&rcScreenSize);
		if(!UTFRectOver(&rcScreenSize, lpRect))
			return 0;

		OldVisibleFlag = UTFIsWindowVisible(hWnd);

		oldRect = lpUIWnd->rect;
		memcpy(&lpUIWnd->rect, lpRect, sizeof(UTFRECT));

		UTFSetOrderInfo(0);
		
		NewVisibleFlag = UTFIsWindowVisible(hWnd);

		if((OldVisibleFlag + NewVisibleFlag) != UTFWND_INVISIBLE)
		{
			if(OldVisibleFlag == UTFWND_INVISIBLE)
			{
				rcRect = lpUIWnd->rect;
			}
			else
			{
				if(NewVisibleFlag == UTFWND_INVISIBLE)
					rcRect = oldRect;
				else
					UTFUnionRect(&rcRect, &oldRect, lpRect);
			}

			if(lpUIWnd->dwStyle & UIS_MODAL)
			{
				UTFCleanAllHideWindow();
			}

			UTFInvalidateRect(NULL, &rcRect, TRUE);
		}
	}

	return 1;
}

DWORD UTFAPI UTFGetWindowID(HUIWND hWnd)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(hWnd == NULL)
		return 0;

	return (lpUIWnd->wndID);
}

int UTFAPI UTFSetWindowID(HUIWND hWnd, DWORD wndID)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;
	
	if(hWnd == NULL)
		return (0);

	lpUIWnd->wndID = wndID;

	return 1;
}

DWORD UTFAPI UTFGetWindowData(HUIWND hWnd)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(hWnd == NULL)
		return 0;

	return (lpUIWnd->dataEx);
}

int UTFAPI UTFSetWindowData(HUIWND hWnd, DWORD dwData)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(hWnd == NULL)
		return 0;

	lpUIWnd->dataEx = dwData;

	return 1;
}

DWORD UTFAPI UTFGetWindowExtendStyle(HUIWND hWnd)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(hWnd == NULL)
		return 0;

	return (lpUIWnd->dwExStyle);
}

int UTFAPI UTFSetWindowExtendStyle(HUIWND hWnd, DWORD dwExStyle)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(hWnd == NULL)
		return 0;

	lpUIWnd->dwExStyle = dwExStyle;

	return 1;
}

int UTFAPI UTFMoveWindow(HUIWND hWnd, WORD x, WORD y, WORD Width, WORD Height)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;
	UTFRECT newRect;

	if(hWnd == NULL)
		return 0;	
	
	newRect.left = x;
	newRect.top = y;
	newRect.right = newRect.left+Width;
	newRect.bottom = newRect.top+Height;

	if(UTFIsRectEqual(&lpUIWnd->rect, &newRect))
		return 0;

	if(lpUIWnd->dwStyle & UIS_CHILD)
	{
		if(UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_SETRECT, (DWORD)&newRect, 0) == 0)
			return 0;
	}
	else
	{
		int OldVisibleFlag,NewVisibleFlag;
		UTFRECT rcScreenSize;
		UTFRECT rcRect,oldRect;

		if(lpUIWnd->dwStyle & UIS_SAVEBACK)
			return 0;

		UTFGetOSDRgnRect(&rcScreenSize);
		if(!UTFRectOver(&rcScreenSize, &newRect))
			return 0;

		OldVisibleFlag = UTFIsWindowVisible(hWnd);

		oldRect = lpUIWnd->rect;
		lpUIWnd->rect = newRect;

		UTFSetOrderInfo(0);
		
		NewVisibleFlag = UTFIsWindowVisible(hWnd);
	
		if((OldVisibleFlag + NewVisibleFlag) != UTFWND_INVISIBLE)
		{
			if(OldVisibleFlag == UTFWND_INVISIBLE)
			{
				rcRect = newRect;
			}
			else
			{
				if(NewVisibleFlag == UTFWND_INVISIBLE)
					rcRect = oldRect;
				else
					UTFUnionRect(&rcRect, &oldRect, &newRect);
			}

			if(lpUIWnd->dwStyle & UIS_MODAL)
			{
				UTFCleanAllHideWindow();
			}

			UTFInvalidateRect(NULL, &rcRect, TRUE);
		}
	}

	return 1;
}

