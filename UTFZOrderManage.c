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
#include "UTFMemory.h"
#include "UTFMemoryPriv.h"
#include "UTFGraphPort.h"
#include "UTFGraphPortPriv.h"
#include "UTFMsgDefPriv.h"
#include "UTFPalettePriv.h"
#include "UTFFontPriv.h"
#include "UTFWndManage.h"
#include "UTFWndDef.h"
#include "UTFWndDefPriv.h"
#include "UTFDrawAPI.h"
#include "UTFZOrderManage.h"
#include "UTFZOrderManagerPriv.h"
#include "UTFMenuPriv.h"
#include "UTFDialogPriv.h"
#include "UTFMsgBox.h"
#include "UTFButtonPriv.h"
#include "UTFTextPriv.h"
#include "UTFProgressPriv.h"
#include "UTFImagePriv.h"
#include "UTFListPriv.h"
#include "UTFLRListPriv.h"
#include "UTFEditPriv.h"
#include "UTFVideoPriv.h"
#include "UTFScrollBarPriv.h"
#include "UTFCheckBoxPriv.h"
#include "UTFTimerPriv.h"

#define UTFZOF_MODAL		(0x00000001)	// window are modal type
#define UTFZOF_TOPMOST		(0x00000002)	// window will be show at the top of all modal type
#define UTFZOF_FREE			(0x00000004)	// window are not intersect with other window
#define UTFZOF_SHOW			(0x00000008)	// window are visible on screen
#define UTFZOF_MSGOPEN		(0x00000010)	// window are enable receive message
#define UTFZOF_NEEDCLOSE	(0x00000020)	// window need be close
#define UTFZOF_NEEDDRAW		(0x00000040)	// window need be redraw
#define UTFZOF_TAILWND		(0x00000080)	// window is the tail mode in list
#define UTFZOF_CANGETMSG	(0x00000100)	// window is visible and can receive message
#define UTFZOF_FLAGCHANGED	(0x00000200)	// window visible flag had been changed
extern int g_IsMemoryDraw;

typedef struct _zorder
{
	HUIWND hWnd;
	DWORD dwStyle;
	struct _zorder *prev;
	struct _zorder *next;
	UTFBITMAP bitmap; // Used for save window back
}ZORDERINFO, *LPZORDERINFO;

static LPZORDERINFO g_pZOrderHeader;
static LPZORDERINFO g_pZOrderTail;
static UTFCALLBACK g_DeskTopProc;
static char g_iNeedOpenScreen;
static char g_bMenuCloseWindow;

/**************************************************************************************/
static LPZORDERINFO UTFFindWindowOrder(HUIWND hWnd)
{
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;

	while(lpZOrderPos)
	{
		if(lpZOrderPos->hWnd == hWnd)
		{
			return lpZOrderPos;
		}

		lpZOrderPos = lpZOrderPos->next;
	}
	
	return NULL;
}

static BYTE UTFIsValidOrder(LPZORDERINFO lpZorderPosSrc)
{
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;

	if(lpZorderPosSrc == NULL)
		return FALSE;

	while(lpZOrderPos)
	{
		if(lpZOrderPos == lpZorderPosSrc)
		{
			return TRUE;
		}

		lpZOrderPos = lpZOrderPos->next;
	}
	
	return FALSE;
}

static char UTFWndVisible(HUIWND hWnd)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(lpUIWnd->dwStyle & UIS_HIDE)
	{
		return 0;
	}
	
	return 1;
}

static BYTE UTFNowHaveModalWnd()
{
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;

	while(lpZOrderPos)
	{
		if((lpZOrderPos->dwStyle & UTFZOF_MODAL) && UTFWndVisible(lpZOrderPos->hWnd))
		{
			return TRUE;
		}

		lpZOrderPos = lpZOrderPos->next;
	}

	return FALSE;
}

static BYTE UTFHaveModalTypeWnd()
{
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;
	DWORD dwFlag = UTFZOF_MODAL|UTFZOF_SHOW;

	while(lpZOrderPos)
	{
		if((lpZOrderPos->dwStyle & dwFlag) == dwFlag)
		{
			return TRUE;
		}

		lpZOrderPos = lpZOrderPos->next;
	}

	return FALSE;
}

char UTFWndCanBeShowOrHide(HUIWND hWnd)
{
	LPZORDERINFO lpZOrderPos;
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;

	if(lpUIWnd->dwStyle & UIS_SAVEBACK)
	{
		if(!(lpUIWnd->dwStyle & UIS_AUTOSAVE))
			return 0;
	}

	lpZOrderPos = UTFFindWindowOrder(hWnd);
	if(lpZOrderPos)
	{
		UTFRECT wndRect;
		UTFRECT rcRect;

		UTFGetWindowRect(hWnd, &wndRect);

		lpZOrderPos = lpZOrderPos->next;
		while(lpZOrderPos)
		{
			lpUIWnd = (LPUTFWND)lpZOrderPos->hWnd;

			// If any window above the window and need save background, 
			// and it is intersect with the window, the window cannot 
			// be show or hide
			if(lpUIWnd->dwStyle & UIS_SAVEBACK)
			{
				UTFGetWindowRect(lpZOrderPos->hWnd, &rcRect);
				if(UTFIntersectRect(NULL, &rcRect, &wndRect))
				{
					return 0;
				}
			}

			lpZOrderPos = lpZOrderPos->next;
		}
	}
	else
	{
		return 0;
	}

	return 1;
}

void UTFSetOpenScreenFlag()
{
	g_iNeedOpenScreen = TRUE;
}

void UTFSetOrderInfo(char bSetModeType)
{
	LPZORDERINFO lpZOrderPos;
	LPZORDERINFO lpZOrderPosTemp;
	UTFRECT rcRect,wndRect;
	HUIWND hParent;
	BYTE bStartSearch;
	DWORD dwFlag;

	if(bSetModeType == 0)
	{
	#if(UTF_MSGBOX_WORKTYPE == 0)
		DWORD dwMsgBoxFlag;
		BYTE bHaveModalWnd;

		dwMsgBoxFlag = UTFZOF_TOPMOST|UTFZOF_MODAL;
		bHaveModalWnd = UTFNowHaveModalWnd();
	#endif

		//set window visible flag
		lpZOrderPos = g_pZOrderHeader;
		while(lpZOrderPos)
		{
			dwFlag = (lpZOrderPos->dwStyle & UTFZOF_SHOW);

			if(UTFWndVisible(lpZOrderPos->hWnd))
			{
				UTFGetWindowRect(lpZOrderPos->hWnd, &wndRect);

				lpZOrderPos->dwStyle |= UTFZOF_FREE;
				lpZOrderPos->dwStyle |= UTFZOF_SHOW;

			#if(UTF_MSGBOX_WORKTYPE == 0)
				if((lpZOrderPos->dwStyle & dwMsgBoxFlag) == 0)
				{
					if(bHaveModalWnd == TRUE)
					{
						lpZOrderPos->dwStyle &= ~(UTFZOF_FREE | UTFZOF_SHOW);

						goto set_showflag_end;
					}
				}
			#endif

				lpZOrderPosTemp = lpZOrderPos->next;
				while(lpZOrderPosTemp)
				{
					if(UTFWndVisible(lpZOrderPosTemp->hWnd))
					{
						UTFGetWindowRect(lpZOrderPosTemp->hWnd, &rcRect);

						if(UTFRectOver(&rcRect, &wndRect))
						{
							lpZOrderPos->dwStyle &= ~(UTFZOF_FREE | UTFZOF_SHOW);
							break;
						}
						else if(UTFIntersectRect(NULL, &rcRect, &wndRect))
						{
							if(lpZOrderPos->dwStyle & UTFZOF_MODAL)
							{
								if(lpZOrderPosTemp->dwStyle & UTFZOF_MODAL)
								{
									lpZOrderPos->dwStyle &= ~UTFZOF_FREE;
								}
							}
							else
							{
								if(lpZOrderPosTemp->dwStyle & UTFZOF_MODAL)
								{
									lpZOrderPos->dwStyle &= ~(UTFZOF_FREE | UTFZOF_SHOW);
									break;
								}
								else
								{
									lpZOrderPos->dwStyle &= ~UTFZOF_FREE;
								}
							}
						}
					}

					lpZOrderPosTemp = lpZOrderPosTemp->next;
				}
			}
			else
			{
				lpZOrderPos->dwStyle &= ~(UTFZOF_FREE | UTFZOF_SHOW);
			}

set_showflag_end:

			//mark window visible status change flag
			if((lpZOrderPos->dwStyle & UTFZOF_SHOW) == dwFlag)
			{
				lpZOrderPos->dwStyle &= ~UTFZOF_FLAGCHANGED;
			}
			else
			{
				lpZOrderPos->dwStyle |= UTFZOF_FLAGCHANGED;
			}

			lpZOrderPos = lpZOrderPos->next;
		}

		//set window tail type flag
		lpZOrderPos = g_pZOrderHeader;
		while(lpZOrderPos)
		{
			lpZOrderPos->dwStyle |= UTFZOF_TAILWND;
			
			lpZOrderPosTemp = lpZOrderPos->next;
			while(lpZOrderPosTemp)
			{
				if(UTFGetParent(lpZOrderPosTemp->hWnd) == lpZOrderPos->hWnd)
				{
					lpZOrderPos->dwStyle &= ~UTFZOF_TAILWND;
					break;
				}
				
				lpZOrderPosTemp = lpZOrderPosTemp->next;
			}
			
			lpZOrderPos = lpZOrderPos->next;
		}
	}

	//set window can receive message or no
	dwFlag = UTFZOF_MSGOPEN|UTFZOF_SHOW;

	lpZOrderPos = g_pZOrderTail;
	while(lpZOrderPos)
	{
		//normally only tail window can receive message
		if(lpZOrderPos->dwStyle & UTFZOF_TAILWND)
		{
			hParent = UTFGetParent(lpZOrderPos->hWnd);

			if((lpZOrderPos->dwStyle & dwFlag) == dwFlag)
			{
				bStartSearch = FALSE;

				lpZOrderPos->dwStyle |= UTFZOF_CANGETMSG;
			}
			else
			{
				bStartSearch = TRUE;

				lpZOrderPos->dwStyle &= ~UTFZOF_CANGETMSG;
			}

			//search all parent window and set flag
			lpZOrderPosTemp = lpZOrderPos->prev;
			while(lpZOrderPosTemp)
			{
				if(hParent == lpZOrderPosTemp->hWnd)
				{
					hParent = UTFGetParent(lpZOrderPosTemp->hWnd);

					lpZOrderPosTemp->dwStyle &= ~UTFZOF_CANGETMSG;

					if(bStartSearch == TRUE)
					{
						if((lpZOrderPosTemp->dwStyle & dwFlag) == dwFlag)
						{
							bStartSearch = FALSE;

							lpZOrderPosTemp->dwStyle |= UTFZOF_CANGETMSG;
						}
					}
				}

				lpZOrderPosTemp = lpZOrderPosTemp->prev;
			}
		}

		lpZOrderPos = lpZOrderPos->prev;
	}
}

static int UTFZOrderInsert(HUIWND hWnd)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;
	LPZORDERINFO lpZOrderPosNew;

	if(hWnd == NULL)
		return 0;

	if(lpUIWnd->dwStyle & UIS_CHILD)
		return 0;

	// If window already in order list, not add again
	while(lpZOrderPos)
	{
		if(lpZOrderPos->hWnd == hWnd)
			return 0;

		lpZOrderPos = lpZOrderPos->next;
	}

	lpZOrderPosNew = (LPZORDERINFO)UTFMalloc(sizeof(ZORDERINFO));
	if(lpZOrderPosNew == NULL)
		return 0;

	lpZOrderPosNew->hWnd = hWnd;
	lpZOrderPosNew->dwStyle = UTFZOF_MSGOPEN;
	lpZOrderPosNew->next = NULL;
	lpZOrderPosNew->prev = NULL;
	lpZOrderPosNew->bitmap.bitPerPixel = 8;
	lpZOrderPosNew->bitmap.Width = 0;
	lpZOrderPosNew->bitmap.Height = 0;
	lpZOrderPosNew->bitmap.data = NULL;
	lpZOrderPosNew->bitmap.hDC = NULL;
	lpZOrderPosNew->bitmap.hBitmap = NULL;

	if(lpUIWnd->dwStyle & UIS_MODAL)
	{
		lpZOrderPosNew->dwStyle |= UTFZOF_MODAL;
	}
	else if(lpUIWnd->dwStyle & UIS_TOPMOST)
	{
		lpZOrderPosNew->dwStyle |= UTFZOF_TOPMOST;
	}

	if(g_pZOrderHeader == NULL)
	{
		g_pZOrderHeader = lpZOrderPosNew;
		g_pZOrderTail = lpZOrderPosNew;
	}
	else
	{
		if(lpZOrderPosNew->dwStyle & UTFZOF_MODAL)
		{
			lpZOrderPos = g_pZOrderTail;

			while(lpZOrderPos)
			{
				if(!(lpZOrderPos->dwStyle & UTFZOF_TOPMOST))
				{
					lpZOrderPosNew->prev = lpZOrderPos;
					lpZOrderPosNew->next = lpZOrderPos->next;
					lpZOrderPos->next = lpZOrderPosNew;
					if(lpZOrderPosNew->next != NULL)
					{
						lpZOrderPosNew->next->prev = lpZOrderPosNew;
					}

					break;
				}

				lpZOrderPos = lpZOrderPos->prev;
			}

			if(lpZOrderPosNew->prev == NULL)
			{
				lpZOrderPosNew->next = g_pZOrderHeader;				
				g_pZOrderHeader->prev = lpZOrderPosNew;
				g_pZOrderHeader = lpZOrderPosNew;
			}
			else if(lpZOrderPosNew->next == NULL)
			{
				g_pZOrderTail = lpZOrderPosNew;
			}
		}
		else if(lpZOrderPosNew->dwStyle & UTFZOF_TOPMOST)
		{
			lpZOrderPosNew->prev = g_pZOrderTail;
			g_pZOrderTail->next = lpZOrderPosNew;
			g_pZOrderTail = lpZOrderPosNew;
		}
		else
		{
			lpZOrderPos = g_pZOrderTail;

			while(lpZOrderPos)
			{
				if(!(lpZOrderPos->dwStyle & (UTFZOF_TOPMOST | UTFZOF_MODAL)))
				{
					lpZOrderPosNew->prev = lpZOrderPos;
					lpZOrderPosNew->next = lpZOrderPos->next;
					lpZOrderPos->next = lpZOrderPosNew;
					if(lpZOrderPosNew->next != NULL)
					{
						lpZOrderPosNew->next->prev = lpZOrderPosNew;
					}

					break;
				}

				lpZOrderPos = lpZOrderPos->prev;
			}

			if(lpZOrderPosNew->prev == NULL)
			{
				lpZOrderPosNew->next = g_pZOrderHeader;				
				g_pZOrderHeader->prev = lpZOrderPosNew;
				g_pZOrderHeader = lpZOrderPosNew;
			}
			else if(lpZOrderPosNew->next == NULL)
			{
				g_pZOrderTail = lpZOrderPosNew;
			}
		}
	}

	return 1;
}

static int UTFZOrderRemove(HUIWND hWnd)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;

	if(hWnd == NULL)
		return 0;

	if(lpUIWnd->dwStyle & UIS_CHILD)
		return 0;

	// If window already in order list, not add again
	while(lpZOrderPos)
	{
		if(lpZOrderPos->hWnd == hWnd)
		{
			if(lpZOrderPos->prev)
			{
				lpZOrderPos->prev->next = lpZOrderPos->next;
			}
			else
			{
				g_pZOrderHeader = lpZOrderPos->next;
			}

			if(lpZOrderPos->next)
			{
				lpZOrderPos->next->prev = lpZOrderPos->prev;
			}
			else
			{
				g_pZOrderTail = lpZOrderPos->prev;
			}

			UTFFree((DWORD)lpZOrderPos);

			return 1;
		}

		lpZOrderPos = lpZOrderPos->next;
	}

	return 0;
}

int UTFIsWindowVisible(HUIWND hWnd)
{
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;

	while(lpZOrderPos)
	{
		if(lpZOrderPos->hWnd == hWnd)
		{
			if(lpZOrderPos->dwStyle & UTFZOF_FREE)
			{
				return UTFWND_FREESHOW;
			}
			else if(lpZOrderPos->dwStyle & UTFZOF_SHOW)
			{
				return UTFWND_PARTSHOW;
			}

			break;
		}

		lpZOrderPos = lpZOrderPos->next;
	}

	return UTFWND_INVISIBLE;
}

int UTFAPI UTFInvalidateRect(HUIWND hWnd, LPUTFRECT lpRect, int bEarseBack)
{
	if(hWnd)
	{
		if(UTFIsWindowVisible(UTFGetMainParent(hWnd)) == UTFWND_FREESHOW)
		{
			UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_PAINT, (DWORD)lpRect, bEarseBack);
		}
	}
	else
	{
		LPZORDERINFO lpZOrderPos = g_pZOrderHeader;
		LPZORDERINFO lpZOrderPosTemp;
		int bWindowEnablePaint;		
		UTFRECT rcRectTemp;
		UTFRECT rcRect;

		if(bEarseBack == TRUE)
		{
			if(lpRect)
			{
				UTFCleanRect(lpRect->left, lpRect->top, lpRect->right, lpRect->bottom);
			}
			else
			{
				UTFCleanScreen(UTFRGB(0,0,0,0));
			}
		}

		while(lpZOrderPos)
		{
			if(lpZOrderPos->dwStyle & (UTFZOF_FREE | UTFZOF_SHOW))
			{
				bWindowEnablePaint = TRUE;

				if(lpRect)
				{
					if(lpZOrderPos->dwStyle & UTFZOF_NEEDDRAW)
					{
						//clear redraw flag
						lpZOrderPos->dwStyle &= ~UTFZOF_NEEDDRAW;

						//search and mark window intersect with it
						UTFGetWindowRect(lpZOrderPos->hWnd, &rcRect);
								
						lpZOrderPosTemp = lpZOrderPos->next;
						while(lpZOrderPosTemp)
						{
							if(lpZOrderPosTemp->dwStyle & (UTFZOF_FREE | UTFZOF_SHOW))
							{
								if(!(lpZOrderPosTemp->dwStyle & UTFZOF_NEEDDRAW))
								{
									UTFGetWindowRect(lpZOrderPosTemp->hWnd, &rcRectTemp);
									if(UTFIntersectRect(NULL, &rcRect, &rcRectTemp))
									{
										lpZOrderPosTemp->dwStyle |= UTFZOF_NEEDDRAW;
									}
								}
							}

							lpZOrderPosTemp = lpZOrderPosTemp->next;
						}
					}
					else
					{
						UTFGetWindowRect(lpZOrderPos->hWnd, &rcRect);
						if(UTFIntersectRect(&rcRect, &rcRect, lpRect))
						{
							lpZOrderPosTemp = lpZOrderPos->next;
							while(lpZOrderPosTemp)
							{
								if(lpZOrderPosTemp->dwStyle & (UTFZOF_FREE | UTFZOF_SHOW))
								{
									UTFGetWindowRect(lpZOrderPosTemp->hWnd, &rcRectTemp);
									if(UTFRectOver(&rcRectTemp, &rcRect))
									{
										bWindowEnablePaint = FALSE;
										break;
									}
								}

								lpZOrderPosTemp = lpZOrderPosTemp->next;
							}

							// redraw all window which is above this window
							if(bWindowEnablePaint == TRUE)
							{
								UTFGetWindowRect(lpZOrderPos->hWnd, &rcRect);
								
								lpZOrderPosTemp = lpZOrderPos->next;
								while(lpZOrderPosTemp)
								{
									if(lpZOrderPosTemp->dwStyle & (UTFZOF_FREE | UTFZOF_SHOW))
									{
										if(!(lpZOrderPosTemp->dwStyle & UTFZOF_NEEDDRAW))
										{
											UTFGetWindowRect(lpZOrderPosTemp->hWnd, &rcRectTemp);
											if(UTFIntersectRect(NULL, &rcRect, &rcRectTemp))
											{
												lpZOrderPosTemp->dwStyle |= UTFZOF_NEEDDRAW;
											}
										}
									}

									lpZOrderPosTemp = lpZOrderPosTemp->next;
								}
							}
						}
						else
						{
							bWindowEnablePaint = FALSE;
						}
					}
				}

				if(bWindowEnablePaint == TRUE)
				{
					UTFSendPrivMessage(lpZOrderPos->hWnd, WMUI_INTER, WMUI_PAINT, NULL, TRUE);
				}
			}

			lpZOrderPos = lpZOrderPos->next;
		}
	}

	return 1;
}

void UTFAPI UTFUpdateSurface(void)
{
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;

	while(lpZOrderPos)
	{
		if(lpZOrderPos->dwStyle & (UTFZOF_FREE | UTFZOF_SHOW))
		{
			UTFSendPrivMessage(lpZOrderPos->hWnd, WMUI_INTER, WMUI_UPDATE, 0, 0);
		}

		lpZOrderPos = lpZOrderPos->next;
	}

	UTFUpdateScreen();

	if(g_iNeedOpenScreen == TRUE)
	{
		g_DeskTopProc(NULL, WMUI_PUBLIC, WMUI_END_REDRAW, 0, 0);
		g_iNeedOpenScreen = FALSE;
	}
}

HUIWND UTFAPI UTFGetWindow(DWORD dwWndID)
{
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;
	LPUTFWND lpUIWnd;

	while(lpZOrderPos)
	{
		lpUIWnd = (LPUTFWND)lpZOrderPos->hWnd;
		if(lpUIWnd->wndID == dwWndID)
		{
			return lpZOrderPos->hWnd;
		}

		lpZOrderPos = lpZOrderPos->next;
	}
	
	return NULL;
}

ENUM_WND_RET UTFAPI UTFEnumWindowProc(ENUMWNDCALLBACK lpCallBack)
{
	LPZORDERINFO lpZOrderPos = g_pZOrderTail;
	ENUM_WND_RET bEnumRet = ENUM_FAILURE;
	WORD bRetCallBack;

	if(lpCallBack == NULL)
		return ENUM_FAILURE;

	while(lpZOrderPos)
	{
		bEnumRet = ENUM_SUCCESS;
		
		bRetCallBack = lpCallBack(lpZOrderPos->hWnd);
		if(bRetCallBack)
		{
			bEnumRet = ENUM_USERSTOP;
			break;
		}

		if(UTFIsValidOrder(lpZOrderPos) == TRUE)
		{
			lpZOrderPos = lpZOrderPos->prev;
		}
		else
		{
			lpZOrderPos = g_pZOrderTail;
		}
	}
	
	return bEnumRet;
}

int UTFEnableWndForMsg(HUIWND hWnd, BYTE bEnable)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;

	if(hWnd == NULL)
		return 0;

	if(lpUIWnd->dwStyle & UIS_CHILD)
		return 0;

	while(lpZOrderPos)
	{
		if(lpZOrderPos->hWnd == hWnd)
		{
			if(lpZOrderPos->dwStyle & UTFZOF_MODAL)
			{
				if(bEnable == TRUE)
				{
					lpZOrderPos->dwStyle |= UTFZOF_MSGOPEN;
				}
				else
				{
					lpZOrderPos->dwStyle &= ~UTFZOF_MSGOPEN;
				}

				UTFSetOrderInfo(1);
				
				return 1;
			}

			return 0;
		}

		lpZOrderPos = lpZOrderPos->next;
	}

	return 0;
}

BYTE UTFAPI UTFWindowSaveOrPutBack(HUIWND hWnd, char bSaveMode)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;
	BYTE bRetCode;

	bRetCode = UTFIsWindowVisible(hWnd);

	if((lpUIWnd->dwStyle & UIS_SAVEBACK) && (lpUIWnd->dwStyle & UIS_AUTOSAVE))
	{
		if(bRetCode == UTFWND_FREESHOW)
		{
			LPZORDERINFO lpZOrderPos;

			lpZOrderPos = UTFFindWindowOrder(hWnd);
			if(bSaveMode == TRUE)
			{
				UTFCreateBitmap(lpUIWnd->rect.right-lpUIWnd->rect.left+2, lpUIWnd->rect.bottom-lpUIWnd->rect.top+2, &lpZOrderPos->bitmap, 0);
				if(lpZOrderPos->bitmap.data)
				{
					UTFSaveScreen(lpUIWnd->rect.left, lpUIWnd->rect.top, &lpZOrderPos->bitmap, 0);
				}
			}
			else
			{
				if(lpZOrderPos->bitmap.data)
				{
					bRetCode = UTFWND_INVISIBLE;

					UTFPutScreen(lpUIWnd->rect.left, lpUIWnd->rect.top, &lpZOrderPos->bitmap, 0);
					//should not free bitmap memory before close window
				}
			}
		}
	}

	return bRetCode;
}

static void UTFDrawAllTopmostWindow(LPUTFRECT lpRect)
{
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;
	LPZORDERINFO lpZOrderPosTemp;
	int bWindowEnablePaint;		
	UTFRECT rcRectTemp;
	UTFRECT rcRect;

	//search for first topmost type window
	while(lpZOrderPos)
	{
		if(lpZOrderPos->dwStyle & UTFZOF_TOPMOST)
		{
			break;
		}

		lpZOrderPos = lpZOrderPos->next;
	}

	//redraw all topmost window intersect the rectangle area
	while(lpZOrderPos)
	{
		if(lpZOrderPos->dwStyle & (UTFZOF_FREE | UTFZOF_SHOW))
		{
			bWindowEnablePaint = TRUE;

			if(lpZOrderPos->dwStyle & UTFZOF_NEEDDRAW)
			{
				//clear redraw flag
				lpZOrderPos->dwStyle &= ~UTFZOF_NEEDDRAW;

				//search and mark window intersect with it
				UTFGetWindowRect(lpZOrderPos->hWnd, &rcRect);
						
				lpZOrderPosTemp = lpZOrderPos->next;
				while(lpZOrderPosTemp)
				{
					if(lpZOrderPosTemp->dwStyle & (UTFZOF_FREE | UTFZOF_SHOW))
					{
						if(!(lpZOrderPosTemp->dwStyle & UTFZOF_NEEDDRAW))
						{
							UTFGetWindowRect(lpZOrderPosTemp->hWnd, &rcRectTemp);
							if(UTFIntersectRect(NULL, &rcRect, &rcRectTemp))
							{
								lpZOrderPosTemp->dwStyle |= UTFZOF_NEEDDRAW;
							}
						}
					}

					lpZOrderPosTemp = lpZOrderPosTemp->next;
				}
			}
			else
			{
				UTFGetWindowRect(lpZOrderPos->hWnd, &rcRect);
				if(UTFIntersectRect(&rcRect, &rcRect, lpRect))
				{
					lpZOrderPosTemp = lpZOrderPos->next;
					while(lpZOrderPosTemp)
					{
						if(lpZOrderPosTemp->dwStyle & (UTFZOF_FREE | UTFZOF_SHOW))
						{
							UTFGetWindowRect(lpZOrderPosTemp->hWnd, &rcRectTemp);
							if(UTFRectOver(&rcRectTemp, &rcRect))
							{
								bWindowEnablePaint = FALSE;
								break;
							}
						}

						lpZOrderPosTemp = lpZOrderPosTemp->next;
					}

					// redraw all window which is above this window
					if(bWindowEnablePaint == TRUE)
					{
						UTFGetWindowRect(lpZOrderPos->hWnd, &rcRect);
						
						lpZOrderPosTemp = lpZOrderPos->next;
						while(lpZOrderPosTemp)
						{
							if(lpZOrderPosTemp->dwStyle & (UTFZOF_FREE | UTFZOF_SHOW))
							{
								if(!(lpZOrderPosTemp->dwStyle & UTFZOF_NEEDDRAW))
								{
									UTFGetWindowRect(lpZOrderPosTemp->hWnd, &rcRectTemp);
									if(UTFIntersectRect(NULL, &rcRect, &rcRectTemp))
									{
										lpZOrderPosTemp->dwStyle |= UTFZOF_NEEDDRAW;
									}
								}
							}

							lpZOrderPosTemp = lpZOrderPosTemp->next;
						}
					}
				}
				else
				{
					bWindowEnablePaint = FALSE;
				}
			}

			if(bWindowEnablePaint == TRUE)
			{
				UTFSendPrivMessage(lpZOrderPos->hWnd, WMUI_INTER, WMUI_PAINT, NULL, TRUE);
			}
		}

		lpZOrderPos = lpZOrderPos->next;
	}
}

void UTFCleanAllHideWindow(void)
{
#if(UTF_MSGBOX_WORKTYPE)
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;
	UTFRECT rcRect;
	DWORD dwFlag;

	while(lpZOrderPos)
	{
		if(lpZOrderPos->dwStyle & UTFZOF_FLAGCHANGED)
		{
			dwFlag = UTFZOF_MODAL|UTFZOF_TOPMOST|UTFZOF_SHOW;

			if((lpZOrderPos->dwStyle & dwFlag) == 0)
			{
				UTFGetWindowRect(lpZOrderPos->hWnd, &rcRect);
				UTFInvalidateRect(NULL, &rcRect, TRUE);
			}
		}

		lpZOrderPos = lpZOrderPos->next;
	}
#endif
}

int UTFAPI UTFWindowOpen(HUIWND hWnd, HUIWND hParent, DWORD param1, DWORD param2)
{
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;
	UTFRECT wndRect;
#if(UTF_MSGBOX_WORKTYPE == 0)
	BYTE bNowHaveModal;

	bNowHaveModal = UTFHaveModalTypeWnd();
#endif

	if(hParent == hWnd)
	{
		hParent = NULL;
	}

	if(UTFZOrderInsert(hWnd))
	{
		int ret;

		if(lpUIWnd->dwStyle & UIS_CENTER)
		{
			WORD Width,Height;
			UTFRECT rcScreenSize;

			UTFGetOSDRgnRect(&rcScreenSize);

			Width = lpUIWnd->rect.right - lpUIWnd->rect.left;
			Height = lpUIWnd->rect.bottom - lpUIWnd->rect.top;

			lpUIWnd->rect.left = (rcScreenSize.right-rcScreenSize.left-Width)/2;
			lpUIWnd->rect.top = (rcScreenSize.bottom-rcScreenSize.top-Height)/2;
			lpUIWnd->rect.right = lpUIWnd->rect.left+Width;
			lpUIWnd->rect.bottom = lpUIWnd->rect.top+Height;
		}

		UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_SETPARENT, hParent, 0);
		UTFSendMessage(hWnd, WMUI_PUBLIC, WMUI_CREATE, param1, param2);

		UTFSetOrderInfo(0);

		lpZOrderPos = UTFFindWindowOrder(hWnd);
		if(lpZOrderPos == NULL)
			return 0;

		ret = UTFIsWindowVisible(hWnd);
		//If window visible and be modal type, should clear all hide message box
		if(ret != UTFWND_INVISIBLE)
		{
			if(lpZOrderPos->dwStyle & UTFZOF_MODAL)
			{
				UTFCleanAllHideWindow();
			}		
		}

		if(ret == UTFWND_FREESHOW)
		{
		#if(UTF_MSGBOX_WORKTYPE == 0)
			if((bNowHaveModal == FALSE) && (lpUIWnd->dwStyle & UIS_MODAL))
			{
				UTFCleanScreen(UTFRGB(0,0,0,0));
			}
		#endif

//#if (defined(UTF_MOMERY_BITMAP) && (UTF_MOMERY_BITMAP == TRUE))
//#else
//#endif
			if(!g_IsMemoryDraw)
			{
				if((lpUIWnd->dwStyle & UIS_SAVEBACK) && (lpUIWnd->dwStyle & UIS_AUTOSAVE))
				{
					UTFCreateBitmap(lpUIWnd->rect.right-lpUIWnd->rect.left+2, lpUIWnd->rect.bottom-lpUIWnd->rect.top+2, &lpZOrderPos->bitmap, 1);
					if(lpZOrderPos->bitmap.data)
					{
						UTFSaveScreen(lpUIWnd->rect.left, lpUIWnd->rect.top, &lpZOrderPos->bitmap, 0);
					}
					else
					{
						lpUIWnd->dwStyle &= ~(UIS_SAVEBACK|UIS_AUTOSAVE);
					}
				}
			}
			
			UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_PAINT, NULL, TRUE);
		}
		else if(ret == UTFWND_PARTSHOW)
		{
			UTFSendPrivMessage(hWnd, WMUI_INTER, WMUI_PAINT, NULL, TRUE);
		}

		//If window visible and not be topmost type, should draw all topmost type window
		if(ret != UTFWND_INVISIBLE)
		{
			if(!(lpZOrderPos->dwStyle & UTFZOF_TOPMOST))
			{
				UTFGetWindowRect(hWnd, &wndRect);
				UTFDrawAllTopmostWindow(&wndRect);
			}
		}

		return 1;
	}

	return 0;
}

int UTFAPI UTFWindowEnd(HUIWND hWnd, DWORD EndCode)
{
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;
	LPZORDERINFO lpZOrderPosTemp;
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;
	LPUTFWND lpUIWndTemp;
	UTFRECT rcRect,wndRect;
	HUIWND hParent = hWnd;
	HUIWND hWndTemp;
	UTFBITMAP bitmap;
	int ret = UTFWND_INVISIBLE;
	int bWndVisible,bDrawBack;
	BYTE bNowHaveModal;

	if(hWnd == NULL)
		return 0;

	if(lpUIWnd->dwStyle & UIS_CHILD)
		return 0;

	// Search window position
	lpZOrderPos = UTFFindWindowOrder(hWnd);
	if(lpZOrderPos)
	{
		lpZOrderPos = lpZOrderPos->next;
	}
	else
	{
		return 0;
	}

	bNowHaveModal = UTFHaveModalTypeWnd();

	UTFGetWindowRect(hWnd, &rcRect);

	// Mark child window
	while(lpZOrderPos)
	{
		if(UTFGetParent(lpZOrderPos->hWnd) == hParent)
		{
			lpZOrderPos->dwStyle |= UTFZOF_NEEDCLOSE;
			hParent = lpZOrderPos->hWnd;
		}

		lpZOrderPos = lpZOrderPos->next;
	}

	// Close all child window
	lpZOrderPos = g_pZOrderTail;
	while(lpZOrderPos)
	{
		lpZOrderPosTemp = lpZOrderPos->prev;
		
		if(lpZOrderPos->dwStyle & UTFZOF_NEEDCLOSE)
		{
			bitmap = lpZOrderPos->bitmap;
			hWndTemp = lpZOrderPos->hWnd;

			bWndVisible = UTFIsWindowVisible(hWndTemp);
			if(UTFZOrderRemove(hWndTemp))
			{
				bDrawBack = TRUE;
				
				lpUIWndTemp = (LPUTFWND)hWndTemp;
				if(lpUIWndTemp->dwStyle & UIS_SAVEBACK)
				{
					if(lpUIWndTemp->dwStyle & UIS_AUTOSAVE)
					{
						if(bitmap.data != NULL)
						{
							bDrawBack = FALSE;
							UTFPutScreen(lpUIWndTemp->rect.left, lpUIWndTemp->rect.top, &bitmap, 0);
							UTFDeleteBitmap(&bitmap);
						}
					}
					else
					{
						bDrawBack = FALSE;
					}
				}
				
				if(bDrawBack == TRUE)
				{
					ret += bWndVisible;
					if(bWndVisible != UTFWND_INVISIBLE)
					{
						UTFGetWindowRect(hWndTemp, &wndRect);
						UTFUnionRect(&rcRect, &rcRect, &wndRect);
					}
				}

				UTFSendMessage(hWndTemp, WMUI_PUBLIC, WMUI_DESTROY, 0, 0);
			}
		}

		lpZOrderPos = lpZOrderPosTemp;
	}

	lpZOrderPos = UTFFindWindowOrder(hWnd);
	bitmap = lpZOrderPos->bitmap;
	
	bDrawBack = TRUE;
	if(lpUIWnd->dwStyle & UIS_SAVEBACK)
	{
		if(lpUIWnd->dwStyle & UIS_AUTOSAVE)
		{
			if(bitmap.data != NULL)
			{
				bDrawBack = FALSE;
				UTFPutScreen(lpUIWnd->rect.left, lpUIWnd->rect.top, &bitmap, 0);
				UTFDeleteBitmap(&bitmap);
			}
		}
		else
		{
			bDrawBack = FALSE;
		}
	}
	
	if(bDrawBack == TRUE)
	{
		ret += UTFIsWindowVisible(hWnd);
	}

	if(UTFZOrderRemove(hWnd))
	{
		DWORD wndID = lpUIWnd->wndID;

		hParent = lpUIWnd->hParent;

		UTFSendMessage(hWnd, WMUI_PUBLIC, WMUI_DESTROY, 0, 0);

		UTFSetOrderInfo(0);
		
		if(hParent)
		{
			UTFSendPrivMessage(hParent, WMUI_PUBLIC, WMUI_ACTIVE, wndID, EndCode);
		}

		if(g_pZOrderHeader == NULL)
		{
			UTFCleanScreen(UTFRGB(0,0,0,0));
		}
	#if(UTF_MSGBOX_WORKTYPE)
		else if((ret != UTFWND_INVISIBLE) || (bNowHaveModal != UTFHaveModalTypeWnd()))
		{
			UTFInvalidateRect(NULL, &rcRect, TRUE);
		}
	#else
		else
		{
			if((bNowHaveModal == TRUE) && (UTFHaveModalTypeWnd() == FALSE))
			{
				UTFInvalidateRect(NULL, NULL, TRUE);
			}
			else if(ret != UTFWND_INVISIBLE)
			{
				UTFInvalidateRect(NULL, &rcRect, TRUE);
			}
		}
	#endif

		return 1;
	}

	return 0;
}

int UTFAPI UTFWindowClose(HUIWND hWnd)
{
	LPZORDERINFO lpZOrderPos = g_pZOrderHeader;
	LPZORDERINFO lpZOrderPosTemp;
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;
	LPUTFWND lpUIWndTemp;
	UTFBITMAP bitmap;
	UTFRECT rcRect,wndRect;
	HUIWND hParent = hWnd;
	HUIWND hWndTemp;
	BYTE bNowHaveModal;
	int bWndVisible,bDrawBack;
	int ret = UTFWND_INVISIBLE;

	if(hWnd == NULL)
		return 0;

	if(lpUIWnd->dwStyle & UIS_CHILD)
		return 0;

	// Search window position
	lpZOrderPos = UTFFindWindowOrder(hWnd);
	if(lpZOrderPos)
	{
		lpZOrderPos = lpZOrderPos->next;
	}
	else
	{
		return 0;
	}

	bNowHaveModal = UTFHaveModalTypeWnd();

	UTFGetWindowRect(hWnd, &rcRect);

	// Mark child window
	while(lpZOrderPos)
	{
		if(UTFGetParent(lpZOrderPos->hWnd) == hParent)
		{
			lpZOrderPos->dwStyle |= UTFZOF_NEEDCLOSE;
			hParent = lpZOrderPos->hWnd;
		}

		lpZOrderPos = lpZOrderPos->next;
	}

	// Close all child window
	lpZOrderPos = g_pZOrderTail;
	while(lpZOrderPos)
	{
		lpZOrderPosTemp = lpZOrderPos->prev;
		
		if(lpZOrderPos->dwStyle & UTFZOF_NEEDCLOSE)
		{
			bitmap = lpZOrderPos->bitmap;			
			hWndTemp = lpZOrderPos->hWnd;

			bWndVisible = UTFIsWindowVisible(hWndTemp);
			if(UTFZOrderRemove(hWndTemp))
			{
				bDrawBack = TRUE;
				
				lpUIWndTemp = (LPUTFWND)hWndTemp;
				if(lpUIWndTemp->dwStyle & UIS_SAVEBACK)
				{
					if(lpUIWndTemp->dwStyle & UIS_AUTOSAVE)
					{
						if(bitmap.data != NULL)
						{
							bDrawBack = FALSE;
							UTFPutScreen(lpUIWndTemp->rect.left, lpUIWndTemp->rect.top, &bitmap, 0);
							UTFDeleteBitmap(&bitmap);
						}
					}
					else
					{
						bDrawBack = FALSE;
					}
				}
				
				if(bDrawBack == TRUE)
				{
					ret += bWndVisible;
					if(bWndVisible != UTFWND_INVISIBLE)
					{
						UTFGetWindowRect(hWndTemp, &wndRect);
						UTFUnionRect(&rcRect, &rcRect, &wndRect);
					}
				}

				UTFSendMessage(hWndTemp, WMUI_PUBLIC, WMUI_DESTROY, 0, 0);
			}
		}

		lpZOrderPos = lpZOrderPosTemp;
	}

	lpZOrderPos = UTFFindWindowOrder(hWnd);
	bitmap = lpZOrderPos->bitmap;
	
	bDrawBack = TRUE;
	if(lpUIWnd->dwStyle & UIS_SAVEBACK)
	{
		if(lpUIWnd->dwStyle & UIS_AUTOSAVE)
		{
			if(bitmap.data != NULL)
			{
				bDrawBack = FALSE;
				UTFPutScreen(lpUIWnd->rect.left, lpUIWnd->rect.top, &bitmap, 0);
				UTFDeleteBitmap(&bitmap);
			}
		}
		else
		{
			bDrawBack = FALSE;
		}
	}
	
	if(bDrawBack == TRUE)
	{
		ret += UTFIsWindowVisible(hWnd);
	}

	if(UTFZOrderRemove(hWnd))
	{
		hParent = UTFGetParent(hWnd);

		UTFSendMessage(hWnd, WMUI_PUBLIC, WMUI_DESTROY, 0, 0);
		
		while(hParent)
		{
			hWnd = hParent;
			hParent = UTFGetParent(hWnd);

			lpZOrderPos = UTFFindWindowOrder(hWnd);
			bitmap = lpZOrderPos->bitmap;
	
			bWndVisible = UTFIsWindowVisible(hWnd);
			if(UTFZOrderRemove(hWnd))
			{
				bDrawBack = TRUE;
				
				lpUIWndTemp = (LPUTFWND)hWnd;
				if(lpUIWndTemp->dwStyle & UIS_SAVEBACK)
				{
					if(lpUIWndTemp->dwStyle & UIS_AUTOSAVE)
					{
						if(bitmap.data != NULL)
						{
							bDrawBack = FALSE;
							UTFPutScreen(lpUIWndTemp->rect.left, lpUIWndTemp->rect.top, &bitmap, 0);
							UTFDeleteBitmap(&bitmap);
						}
					}
					else
					{
						bDrawBack = FALSE;
					}
				}
				
				if(bDrawBack == TRUE)
				{
					ret += bWndVisible;
					if(bWndVisible != UTFWND_INVISIBLE)
					{
						UTFGetWindowRect(hWnd, &wndRect);
						UTFUnionRect(&rcRect, &rcRect, &wndRect);
					}
				}

				UTFSendMessage(hWnd, WMUI_PUBLIC, WMUI_DESTROY, 0, 0);
			}
		}

		UTFSetOrderInfo(0);

		if(g_pZOrderHeader == NULL)
		{
			UTFCleanScreen(UTFRGB(0,0,0,0));
		}
	#if(UTF_MSGBOX_WORKTYPE)
		else if((ret != UTFWND_INVISIBLE) || (bNowHaveModal != UTFHaveModalTypeWnd()))
		{
			UTFInvalidateRect(NULL, &rcRect, TRUE);
		}
	#else
		else
		{
			if((bNowHaveModal == TRUE) && (UTFHaveModalTypeWnd() == FALSE))
			{
				UTFInvalidateRect(NULL, NULL, TRUE);
			}
			else if(ret != UTFWND_INVISIBLE)
			{
				UTFInvalidateRect(NULL, &rcRect, TRUE);
			}
		}
	#endif

		return 1;
	}

	return 0;
}
//周磊edited 08-07-02  增加返回值
void UTFAPI UTFProcessMessage(DWORD uMsg, DWORD p1, DWORD p2, DWORD p3)
{
	LPZORDERINFO lpZOrderPos = g_pZOrderTail;
	int bMsgSendToDeskTop = TRUE;
	int ret = 0;

	if(g_DeskTopProc == NULL)
		return;

	if(g_pZOrderHeader == NULL)
	{
		bMsgSendToDeskTop = TRUE;
	}
	else if((uMsg == WMUI_KEYPRESS) || (uMsg == WMUI_KEY_RELEASED))
	{
		DWORD dwFlag = UTFZOF_MODAL|UTFZOF_CANGETMSG;
		
		while(lpZOrderPos)
		{
			if((lpZOrderPos->dwStyle & dwFlag) == dwFlag)
			{
				bMsgSendToDeskTop = FALSE;
				ret = UTFSendMessage(lpZOrderPos->hWnd, uMsg, p1, p2, p3);
				break;
			}

			lpZOrderPos = lpZOrderPos->prev;
		}
	}
	else
	{
		int bUserMsg=0;
		int bTimerMsg=0;

		if((uMsg < WMUI_BASE_TYPE) || (uMsg > WMUI_LAST_TYPE))
		{
			bUserMsg = 1;
		}
		if((uMsg == WMUI_PUBLIC) && (p1 == WMUI_TIMER))
		{
			bTimerMsg = 1;
		}

		while(lpZOrderPos)
		{
			if(lpZOrderPos->dwStyle & UTFZOF_CANGETMSG)
			{
				if(bUserMsg)
				{
					if(lpZOrderPos->dwStyle & UTFZOF_MODAL)
					{
						ret = UTFSendMessage(lpZOrderPos->hWnd, uMsg, p1, p2, p3);
						bMsgSendToDeskTop = FALSE;
						break;
					}
				}
				else
				{
					ret = UTFSendMessage(lpZOrderPos->hWnd, uMsg, p1, p2, p3);
					bMsgSendToDeskTop = FALSE;

					if( !bTimerMsg ) break;
				}
			}

 			if(UTFIsValidOrder(lpZOrderPos) == TRUE)
  				lpZOrderPos = lpZOrderPos->prev;
			else
			{
				lpZOrderPos = g_pZOrderTail;
			}
		}
	}

	if(bMsgSendToDeskTop == TRUE)
	{
		ret = g_DeskTopProc(NULL, uMsg, p1, p2, p3);
	}

	UTFUpdateSurface();

//	return ret;

}

int UTFAPI UTFStartRun(UTFCALLBACK wndProc)
{
	UTFOSDInfo graphInfo;

	if(wndProc == NULL)
		return 0;

	g_bMenuCloseWindow = 0;
	g_iNeedOpenScreen = FALSE;
	g_DeskTopProc = wndProc;
	g_pZOrderHeader = NULL;
	g_pZOrderTail = NULL;

	UTFMemoryInit();
	UTFGraphInitialize();
	UTFInitPalette();
	UTFDrawAPIInit();

	UTFTypeManagerInit();
	UTFMenuInitialize();
	UTFDialogInitialize();
	UTFMsgBoxInitialize();
	
	UTFButtonInitialize();
	UTFTextInitialize();
	UTFProgressInitialize();
	UTFImageInitialize();
	UTFListInitialize();
	UTFLRListInitialize();
	UTFEditInitialize();
	UTFVideoInitialize();
	UTFScrollBarInitialize();
	UTFCheckBoxInitialize();
//	UTFIpEditInitialize();
	UTFTimerInitialize();

	UTFGetOSDInfo(&graphInfo);
	UTFSetGraphMode((UTFGRAPHMODE)graphInfo.bGraphMode);

//	UTFFontInit();

	g_DeskTopProc = wndProc;
	g_DeskTopProc(NULL, WMUI_PUBLIC, WMUI_INIT, 0, 0);

	UTFUpdateSurface();
	UTFCleanScreen(UTFRGB(0, 0, 0, 0));

	return 1;
}

int UTFAPI UTFEnterMemoryDrawInit(int flag)
{
	UTFDareIsMemoryDraw(flag);
	//UTFMemoryInit();
	UTFGraphInitialize();
	//UTFInitPalette();
	//UTFDrawAPIInit();

	//UTFTypeManagerInit();
	UTFMenuInitialize();
	UTFDialogInitialize();
	UTFMsgBoxInitialize();
	
	UTFButtonInitialize();
	UTFTextInitialize();
	UTFProgressInitialize();
	UTFImageInitialize();
	UTFListInitialize();
	UTFLRListInitialize();
	UTFEditInitialize();
	UTFVideoInitialize();
	UTFScrollBarInitialize();
	UTFCheckBoxInitialize();
//	UTFIpEditInitialize();
	//UTFTimerInitialize();

	//UTFGetOSDInfo(&graphInfo);
	//UTFSetGraphMode((UTFGRAPHMODE)graphInfo.bGraphMode);

//	UTFFontInit();
	//UTFUpdateSurface();
	//UTFCleanScreen(UTFRGB(0, 0, 0, 0));

	return 1;
}

WORD UTFAPI UTFGetVersionInfo(char *ptrBuffer)
{
	char version[100] = {"UTFTool Libary: 2005.8.17"};
	WORD wStrLen = strlen(version);

	strcpy(ptrBuffer, version);

	return wStrLen;
}

BYTE UTFAPI UTFModalWindowOpenning()
{
	return UTFHaveModalTypeWnd();
}

int UTFAPI UTFIsWndActiving(HUIWND hWnd)
{
	LPZORDERINFO lpZOrderPos;

	lpZOrderPos = UTFFindWindowOrder(hWnd);
	if(lpZOrderPos)
	{
		if(lpZOrderPos->dwStyle & UTFZOF_CANGETMSG)
		{
			return 1;
		}
	}

	return 0;
}

void UTFSendMsgToDeskTop(DWORD uMsg, DWORD p1, DWORD p2, DWORD p3)
{
	if(g_DeskTopProc)
	{
		g_DeskTopProc(NULL, uMsg, p1, p2, p3);
	}
}

int UTFIsDeskTopCanGetMsg(void)
{
	if(g_pZOrderHeader)
	{
		LPZORDERINFO lpZOrderPos = g_pZOrderTail;

		while(lpZOrderPos)
		{
			if(lpZOrderPos->dwStyle & UTFZOF_CANGETMSG)
			{
				return FALSE;
			}

			lpZOrderPos = lpZOrderPos->prev;
		}
	}

	return TRUE;
}

/*************************************************************************************/
void UTFAPI UTFSetDefaultCloseWindowMode(char bCloseMode)
{
	g_bMenuCloseWindow = bCloseMode;
}

char UTFGetDefaultCloseWindowMode(void)
{
	return g_bMenuCloseWindow;
}
/*************************************************************************************/
