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
#include "UTFGraphPort.h"
#include "UTFMenu.h"
#include "UTFMenuPriv.h"
#include "UTFWndManage.h"
#include "UTFWndDefPriv.h"
#include "UTFDrawAPI.h"
#include "UTFFont.h"
#include "UTFMsgDefPriv.h"
#include "UTFZOrderManage.h"
#include "UTFZOrderManagerPriv.h"
#include "UTFPalettePriv.h"
#include "UTFDrawText.h"
#include "UTFMemory.h"
#include "UTFTimer.h"

static UTFCOLOR g_MenuClearColor;
static char g_bEnableMenuUpdate;
/******************************************************************/
static void UTFMenuItemFace(HUIWND hItem, UTFRECT rcRect, DWORD info)
{
	LPUTFMENUITEM pItem = (LPUTFMENUITEM)hItem;
	LPUTFMenu pMenu = (LPUTFMenu)pItem->hParent;
	UTFRECT itemRect = rcRect;

	if(pItem->dwExStyle & MIS_SEPARATOR)
	{
		UTFSetPen(PSUI_SOLID, UTFRGB(0,0,0,255), 1);
		itemRect.top += (itemRect.bottom-itemRect.top)/2;
		UTFFillRect(itemRect.left, itemRect.top, itemRect.right, itemRect.top+2);
	}
	else
	{
		/* Draw background */
		UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
		if(pItem->dwStyle & UIS_DISABLE)
		{
			UTFSetBrush(BSUI_SOLID, UTFRGB(150,150,150,255), HSUI_HORIZONTAL);
			UTFSetTextColor(UTFRGB(0,0,0,255));
		}
		else if((pItem->dwFlags & UIF_FOCUS) && (pMenu->dwFlags & UIF_FOCUS))
		{
			UTFSetBrush(BSUI_SOLID, UTFRGB(220,120,70,255), HSUI_HORIZONTAL);
			UTFSetTextColor(UTFRGB(0,0,0,255));
		}
		else
		{
			UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,150,255), HSUI_HORIZONTAL);
			UTFSetTextColor(UTFRGB(255,255,255,255));
		}
		UTFFillRect(itemRect.left, itemRect.top, itemRect.right, itemRect.bottom);
		
		if(pItem->textID >= 0)
		{
			UTFGetIDText(pItem->textID, pItem->wndText, TEXT_SIZE);
		}

		UTFDrawText(pItem->wndText, &itemRect, DTUI_SINGLELINE|DTUI_END_ELLIPSIS|DTUI_CENTER|DTUI_VCENTER);

		if((pItem->hSubWnd) && !(pMenu->dwExStyle & MIS_HORITEM))
		{
			int left, top, height;

			height = itemRect.bottom-itemRect.top-10;
			left = itemRect.right-5-height;
			top = itemRect.top+5;
			
			UTFSetPen(PSUI_SOLID, UTFGetTextColor(), 1);
			UTFDrawArrow(left, top, height, height, ASUI_RIGHT);
		}
	}
}

static void UTFMenuDrawFace(HUIWND hMenu, UTFRECT rcRect, DWORD info)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	UTFRECT menuRect = rcRect;
	int borderDepth = 2;
	int titleHeight = 30;

	if(pMenu->dwFlags & UIF_REDRAW) // Draw all menu frame
	{
		UTFSetPen(PSUI_SOLID, UTFRGB(255,255,255,255), borderDepth);
		UTFSetBrush(BSUI_SOLID, UTFRGB(192,192,192,255), 0);
		UTFFillRect(menuRect.left, menuRect.top, menuRect.right, menuRect.bottom);
	}

	if(pMenu->dwFlags & UIF_DRAWTITLE) // draw menu title
	{
		menuRect.left += borderDepth;
		menuRect.top += borderDepth;
		menuRect.right -= borderDepth;
		menuRect.bottom = menuRect.top+titleHeight-borderDepth;
		
		UTFSetPen(PSUI_NULL, 0, 0);
		UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,100,255), 0);
		UTFFillRect(menuRect.left, menuRect.top, menuRect.right, menuRect.top+30);
		
		if(pMenu->textID >= 0)
		{
			UTFGetIDText(pMenu->textID, pMenu->wndText, TEXT_SIZE);
		}
		UTFSetTextColor(UTFRGB(255,255,255,255));
		UTFDrawText(pMenu->wndText, &menuRect, DTUI_SINGLELINE|DTUI_END_ELLIPSIS|DTUI_LEFT|DTUI_VCENTER);
	}
}

static LPUTFMENUITEM UTFMenuGetFocusItem(HUIWND hMenu)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;

	if(pMenu->total > 0)
	{
		LPUTFMENUITEM pItem = pMenu->pItem;
			
		while(pItem)
		{
			if(pItem->dwFlags & UIF_FOCUS)
				return pItem;
			
			pItem = (LPUTFMENUITEM)pItem->hNextItem;
		}
	}

	return NULL;
}

static void UTFMenuInvalidate(HUIWND hMenu, DWORD info)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMENUITEM pItem;
	LPUTFMENUITEM pFocusItem = NULL;
	int bShowChild = FALSE;
	UTFRECT drawRect;

	if(g_bEnableMenuUpdate == FALSE)
		return;

	if(pMenu->dwFlags & (UIF_REDRAW | UIF_DRAWTITLE))
	{
		if(pMenu->dwFlags & UIF_REDRAW)
		{
			UTFSetOpenScreenFlag();

			if(pMenu->CallBack)
			{
				pMenu->CallBack(hMenu, WMUI_PUBLIC, WMUI_QUERY_SHOW, 0, 0);
			}
		}
		
		if(pMenu->OnDraw != NULL)
		{
			if(pMenu->dwStyle & UIS_OWNERDRAW)
			{
				pMenu->OnDraw(hMenu, pMenu->rect, 0);
			}
			else
			{
				UTFMenuDrawFace(hMenu, pMenu->rect, 0);
			}
		}
		else
		{
			UTFMenuDrawFace(hMenu, pMenu->rect, 0);
		}
	}
	pMenu->dwFlags &= ~(UIF_REDRAW | UIF_DRAWTITLE);

	if(pMenu->pItem == NULL)
		return;

	// Start draw item
	pItem = pMenu->pItem;
	while(pItem)
	{
		if(pItem->dwFlags & UIF_REDRAW)
		{
			drawRect = pItem->rect;
			UTFOffsetRect(&drawRect, pMenu->rect.left, pMenu->rect.top);

			// Start draw each menu item which need be updated
			if(pItem->OnDraw != NULL)
			{
				if(pItem->dwStyle & UIS_OWNERDRAW)
				{
					pItem->OnDraw((HUIWND)pItem, drawRect, 0);
				}
				else
				{
					UTFMenuItemFace((HUIWND)pItem, drawRect, 0);
				}
			}
			else
			{
				UTFMenuItemFace((HUIWND)pItem, drawRect, 0);
			}

			// Clear status flag
			pItem->dwFlags &= ~UIF_REDRAW;

			// Decide submenu of focus item need draw or no
			if(pItem->dwFlags & UIF_FOCUS)
			{
				if(pItem->dwFlags & MIF_SUBOPEN)
				{
					pFocusItem = pItem;
					bShowChild = TRUE;
				}
			}
		}
		
		pItem = (LPUTFMENUITEM)pItem->hNextItem;
	}

	if((pMenu->dwExStyle & MIS_SHOWSUB) && (bShowChild == TRUE))
	{
		pFocusItem->dwFlags &= ~MIF_SUBOPEN;
		if(pFocusItem->hSubWnd)
		{
			UTFWindowOpen(pFocusItem->hSubWnd, hMenu, 0, 0);
			UTFEnableWndForMsg(pFocusItem->hSubWnd, FALSE);
		}
	}
}

// This function force all menu item will be updated when menu is painting
static void UTFMenuRedrawAllItem(HUIWND hMenu)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMENUITEM pItem;
	
	pItem = pMenu->pItem;
	while(pItem)
	{
		pItem->dwFlags |= UIF_REDRAW;
		
		pItem = (LPUTFMENUITEM)pItem->hNextItem;
	}
}

static void UTFMenuRedrawItemInRect(HUIWND hMenu, LPUTFRECT lpRect)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMENUITEM pItem;
	
	pItem = pMenu->pItem;
	while(pItem)
	{
		if(UTFIntersectRect(NULL, &pItem->rect, lpRect))
		{
			pItem->dwFlags |= UIF_REDRAW;
		}
		
		pItem = (LPUTFMENUITEM)pItem->hNextItem;
	}
}

static LPUTFMENUITEM UTFFindMenuItem(HUIWND hMenu, DWORD uItem)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMENUITEM pItem;

	pItem = pMenu->pItem;
	while(pItem)
	{
		if(pItem->wndID == uItem)
		{
			break;
		}

		pItem = (LPUTFMENUITEM)pItem->hNextItem;
	}

	return pItem;
}

static int UTFMenuSetFocus(HUIWND hItem, HUIWND hMenu)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMENUITEM pItem = (LPUTFMENUITEM)hItem;
	LPUTFMENUITEM pFocusItem;

	if((hItem == NULL) || (hMenu == NULL))
		return 0;

	pFocusItem = UTFMenuGetFocusItem(hMenu);
	if(pItem != pFocusItem)
	{
		pItem->dwFlags |= (UIF_FOCUS|UIF_REDRAW|MIF_SUBOPEN|MIF_CALLBACK);

		if(pFocusItem != NULL)
		{
			pFocusItem->dwFlags &= ~(UIF_FOCUS|MIF_SUBOPEN);
			pFocusItem->dwFlags |= UIF_REDRAW;

			if(pMenu->dwExStyle & MIS_SHOWSUB)
			{
				if(pFocusItem->hSubWnd)
				{
					UTFCOLOR oldColor = UTFSetCleanColor(g_MenuClearColor);

					UTFWindowEnd(pFocusItem->hSubWnd, 0);
					UTFSetCleanColor(oldColor);
				}
			}
		}
	}

	return 1;
}

// This function will move menu focus backward and set paint flag
static int UTFMenuFocusBack(HUIWND hMenu)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMENUITEM pItem;
	int bPaint = FALSE;

	if(pMenu->total > 1)
	{
		LPUTFMENUITEM pItemTemp;
		int i;
		
		pItem = UTFMenuGetFocusItem(hMenu);
		if(pItem == NULL)
		{
			return FALSE;
		}
		
		pItemTemp = pItem;			
		for(i=0; i<pMenu->total; i++)
		{				
			if(pItemTemp->hPrevItem)
			{
				pItemTemp = (LPUTFMENUITEM)pItemTemp->hPrevItem;
			}
			else
			{
				pItemTemp = pMenu->pItem;
				while(pItemTemp)
				{
					if(pItemTemp->hNextItem == NULL)
						break;
					
					pItemTemp = (LPUTFMENUITEM)pItemTemp->hNextItem;
				}
			}

			if(!((pItemTemp->dwStyle & UIS_DISABLE) || (pItemTemp->dwExStyle & MIS_SEPARATOR)))
				break;
		}

		if(pItemTemp != pItem)
		{
			pItem->dwFlags &= ~(UIF_FOCUS|MIF_SUBOPEN);
			pItem->dwFlags |= UIF_REDRAW;
			pItemTemp->dwFlags |= (UIF_FOCUS|UIF_REDRAW|MIF_SUBOPEN|MIF_CALLBACK);

			if(pMenu->dwExStyle & MIS_SHOWSUB)
			{
				if(pItem->hSubWnd)
				{
					UTFCOLOR oldColor = UTFSetCleanColor(g_MenuClearColor);

					UTFWindowEnd(pItem->hSubWnd, 0);
					UTFSetCleanColor(oldColor);
				}
			}

			bPaint = TRUE;
		}
	}

	return bPaint;
}

// This function will move menu focus forward and set paint flag
static int UTFMenuFocusForward(HUIWND hMenu)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMENUITEM pItem;
	int bPaint = FALSE;

	if(pMenu->total > 1)
	{
		LPUTFMENUITEM pItemTemp;
		int i;
		
		pItem = UTFMenuGetFocusItem(hMenu);
		if(pItem == NULL)
		{
			return FALSE;
		}
		
		pItemTemp = pItem;			
		for(i=0; i<pMenu->total; i++)
		{				
			if(pItemTemp->hNextItem)
			{
				pItemTemp = (LPUTFMENUITEM)pItemTemp->hNextItem;
			}
			else
			{
				pItemTemp = pMenu->pItem;
			}

			if(!((pItemTemp->dwStyle & UIS_DISABLE) || (pItemTemp->dwExStyle & MIS_SEPARATOR)))
				break;
		}

		if(pItemTemp != pItem)
		{
			pItem->dwFlags &= ~(UIF_FOCUS|MIF_SUBOPEN);
			pItem->dwFlags |= UIF_REDRAW;
			pItemTemp->dwFlags |= (UIF_FOCUS|UIF_REDRAW|MIF_SUBOPEN|MIF_CALLBACK);

			if(pMenu->dwExStyle & MIS_SHOWSUB)
			{
				if(pItem->hSubWnd)
				{
					UTFCOLOR oldColor = UTFSetCleanColor(g_MenuClearColor);

					UTFWindowEnd(pItem->hSubWnd, 0);
					UTFSetCleanColor(oldColor);
				}
			}

			bPaint = TRUE;
		}
	}

	return bPaint;
}

static int UIFMenuToSubMenu(HUIWND hMenu, int bForceToSub)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMENUITEM pItem;

	pItem = UTFMenuGetFocusItem(hMenu);
	if(pItem != NULL)
	{
		if(pItem->hSubWnd)
		{
			if(pMenu->dwExStyle & MIS_SHOWSUB)
			{
				LPUTFMenu pSubMenu = (LPUTFMenu)pItem->hSubWnd;

				if(pSubMenu->pItem == NULL)
					return 0;
				
				if(pMenu->CallBack)
				{
					if(pItem->dwFlags & MIF_CALLBACK)
					{
						if(pMenu->dwFlags & MIF_WAITGOTOSUB)
						{
							pMenu->dwFlags &= ~MIF_WAITGOTOSUB;
						}
						else
						{
							int ret;

							ret = pMenu->CallBack(hMenu, WMUI_NOTIFY, BNUI_CLICKED, pItem->wndID, (DWORD)pItem);
							if(ret == RETM_OPENWND)
							{
								pMenu->dwFlags |= MIF_WAITGOTOSUB;
								return 0;
							}
						}
					}
				}

				pMenu->dwFlags &= ~UIF_FOCUS;
				pItem->dwFlags |= UIF_REDRAW;
				pItem->dwFlags &= ~(MIF_SUBOPEN|MIF_CALLBACK);

				UTFEnableWndForMsg(pItem->hSubWnd, TRUE);
				pSubMenu->dwFlags |= UIF_FOCUS;				
				pItem = UTFMenuGetFocusItem(pItem->hSubWnd);
				if(pItem != NULL)
				{
					pItem->dwFlags |= (UIF_REDRAW|MIF_SUBOPEN|MIF_CALLBACK);
				}
			}
			else if(bForceToSub == TRUE)
			{
				if(pMenu->CallBack)
				{
					if(pMenu->dwFlags & MIF_WAITGOTOSUB)
					{
						pMenu->dwFlags &= ~MIF_WAITGOTOSUB;
					}
					else
					{
						int ret;

						ret = pMenu->CallBack(hMenu, WMUI_NOTIFY, BNUI_CLICKED, pItem->wndID, (DWORD)pItem);
						if(ret == RETM_OPENWND)
						{
							pMenu->dwFlags |= MIF_WAITGOTOSUB;
							return 0;
						}
					}
				}
				
				UTFWindowOpen(pItem->hSubWnd, hMenu, 0, 0);
			}
		}
		else if(bForceToSub == TRUE)
		{
			if(pMenu->CallBack)
			{
				pMenu->CallBack(hMenu, WMUI_NOTIFY, BNUI_CLICKED, pItem->wndID, (DWORD)pItem);
			}
		}
	}
						
	return 0;
}

static int UIFMenuToParentMenu(HUIWND hMenu)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMenu pParentMenu = (LPUTFMenu)pMenu->hParent;
	LPUTFMENUITEM pItem;

	if(pMenu->hParent)
	{
		if(pParentMenu->dwExStyle & MIS_SHOWSUB)
		{
			pItem = UTFMenuGetFocusItem(hMenu);
			if(pItem != NULL)
			{
				pMenu->dwFlags &= ~UIF_FOCUS;
				pItem->dwFlags |= UIF_REDRAW;
				pItem->dwFlags &= ~MIF_SUBOPEN;
				if((pItem->hSubWnd) && (pMenu->dwExStyle & MIS_SHOWSUB))
				{
					UTFCOLOR oldColor = UTFSetCleanColor(g_MenuClearColor);

					UTFWindowEnd(pItem->hSubWnd, 0);
					UTFSetCleanColor(oldColor);
				}
				UTFEnableWndForMsg(hMenu, FALSE);

				pParentMenu->dwFlags |= UIF_FOCUS;
				pItem = UTFMenuGetFocusItem(pMenu->hParent);
				if(pItem != NULL)
				{
					pItem->dwFlags |= UIF_REDRAW;
				}
			}
		}
	}

	return 0;
}

static int UTFMenuItemMsgProc(HUIWND hItem, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFMENUITEM pItem = (LPUTFMENUITEM)hItem;
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
			if((pItem->dwStyle & UIS_DISABLE) || (pItem->dwExStyle & MIS_SEPARATOR))
			{
				retCode = 0;
			}
			else
			{
				UTFMenuSetFocus(hItem, pItem->hParent);
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
			pItem->dwFlags |= UIF_REDRAW;
			break;

		case WMUI_ENABLE:
			if(pItem->dwStyle & UIS_DISABLE)
			{
				pItem->dwStyle &= ~UIS_DISABLE;
				pItem->dwFlags |= UIF_REDRAW;
			}
			break;

		case WMUI_DISABLE:
			if((pItem->dwStyle & UIS_DISABLE) != UIS_DISABLE)
			{
				pItem->dwStyle |= UIS_DISABLE;
				pItem->dwFlags |= UIF_REDRAW;
				if(pItem->dwFlags & UIF_FOCUS)
				{
					UTFMenuFocusForward(pItem->hParent);
				}
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
						strcpy((char *)pItem->wndText, (char *)param2);
					}
					else
					{
						memcpy((char *)pItem->wndText, (char *)param2, TEXT_SIZE);
						pItem->wndText[TEXT_SIZE-1] = 0;
					}
					pItem->textID = -1;
					pItem->dwFlags |= UIF_REDRAW;
				}
				else if(pItem->textID != param3)
				{
					pItem->textID = param3;
					pItem->dwFlags |= UIF_REDRAW;
				}
			}
			break;

		case WMUI_GETTEXT:
			if((param2 != NULL) && (param3 > 0))
			{
				if(pItem->textID >= 0)
				{
					UTFGetIDText(pItem->textID, pItem->wndText, TEXT_SIZE);
				}
				if(strlen((char *)pItem->wndText) < param3)
				{
					strcpy((char *)param2, (char *)pItem->wndText);
				}
				else
				{
					PBYTE pBuffer = (PBYTE)param2;
					
					memcpy(pBuffer, pItem->wndText, param3);
					*(pBuffer+param3-1) = 0;
				}
			}
			break;

		case WMUI_DELETEWND:
			{
				LPUTFMenu pMenu = (LPUTFMenu)pItem->hParent;
				LPUTFMENUITEM pItemTemp;			

				if(pItem->hPrevItem)
				{
					pItemTemp = (LPUTFMENUITEM)pItem->hPrevItem;
					pItemTemp->hNextItem = pItem->hNextItem;
				}
				else
				{
					pMenu->pItem = (LPUTFMENUITEM)pItem->hNextItem;
				}

				if(pItem->hNextItem)
				{
					pItemTemp = (LPUTFMENUITEM)pItem->hNextItem;
					pItemTemp->hPrevItem = pItem->hPrevItem;
				}
				UTFFree(hItem);
			}
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

static int UTFMenuMsgProc(HUIWND hMenu, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMENUITEM pItem;
	int retCode = 1;

	switch(uMsg)
	{
	case WMUI_PUBLIC:
		switch(param1)
		{
		case WMUI_CREATE:
			pMenu->dwStyle &= ~UIS_HIDE;
			pMenu->dwFlags &= ~MIF_WAITGOTOSUB;
			if(UTFIsMenu(pMenu->hParent) == FALSE)
			{
				pMenu->dwFlags |= UIF_FOCUS;
				pItem = UTFMenuGetFocusItem(hMenu);
				if(pItem != NULL)
				{
					pItem->dwFlags |= (MIF_CALLBACK|MIF_SUBOPEN);
				}
			}
			else
			{
				LPUTFMenu pParentMenu = (LPUTFMenu)pMenu->hParent;
				
				if(pParentMenu->dwExStyle & MIS_SHOWSUB)
				{
					pMenu->dwFlags &= ~UIF_FOCUS;
					pItem = UTFMenuGetFocusItem(hMenu);
					if(pItem != NULL)
					{
						pItem->dwFlags &= ~(MIF_CALLBACK|MIF_SUBOPEN);
					}
				}
				else
				{
					pMenu->dwFlags |= UIF_FOCUS;
					pItem = UTFMenuGetFocusItem(hMenu);
					if(pItem != NULL)
					{
						pItem->dwFlags |= (MIF_CALLBACK|MIF_SUBOPEN);
					}
				}
			}
			break;

		case WMUI_DESTROY:
			if(pMenu->dwExStyle & MIS_SHOWSUB)
			{
				pItem = UTFMenuGetFocusItem(hMenu);
				if(pItem != NULL)
				{
					if(pItem->hSubWnd)
					{
						UTFCOLOR oldColor = UTFSetCleanColor(g_MenuClearColor);

						UTFWindowEnd(pItem->hSubWnd, 0);
						UTFSetCleanColor(oldColor);
					}
				}
			}
			if(pMenu->dwFlags & UIF_FOCUS)
			{
				if(pMenu->hParent)
				{
					LPUTFMenu pParentMenu = (LPUTFMenu)pMenu->hParent;
					
					if(pParentMenu->dwExStyle & MIS_SHOWSUB)
					{
						pItem = UTFMenuGetFocusItem(hMenu);
						if(pItem != NULL)
						{
							pMenu->dwFlags &= ~UIF_FOCUS;
							pItem->dwFlags |= UIF_REDRAW;
							pItem->dwFlags &= ~MIF_SUBOPEN;
							UTFEnableWndForMsg(hMenu, FALSE);
							
							pParentMenu->dwFlags |= UIF_FOCUS;
							pItem = UTFMenuGetFocusItem(pMenu->hParent);
							if(pItem != NULL)
							{
								pItem->dwFlags |= (UIF_REDRAW|MIF_SUBOPEN);
							}
						}
					}
				}
			}
			pMenu->dwFlags &= ~(UIF_FOCUS | UIF_REDRAW);

			UTFKillWindowTimer(hMenu);
			break;

		case WMUI_ACTIVE:
			if(pMenu->CallBack)
			{
				int ret;
				
				ret = pMenu->CallBack(hMenu, uMsg, param1, param2, param3);
				if(pMenu->dwFlags & MIF_WAITGOTOSUB)
				{
					if(ret == RETM_ENTER)
					{
						UIFMenuToSubMenu(hMenu, TRUE);
					}
					else
					{
						pMenu->dwFlags &= ~MIF_WAITGOTOSUB;
					}
				}
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
		case KEYUI_UP:
			if(pMenu->dwExStyle & MIS_HORITEM)
			{
				UIFMenuToParentMenu(hMenu);
			}
			else
			{
				UTFMenuFocusBack(hMenu);
			}
			break;

		case KEYUI_DOWN:
			if(pMenu->dwExStyle & MIS_HORITEM)
			{
				if(pMenu->dwExStyle & MIS_SHOWSUB)
				{
					UIFMenuToSubMenu(hMenu, FALSE);
				}
			}
			else
			{
				UTFMenuFocusForward(hMenu);
			}
			break;

		case KEYUI_LEFT:
			if(pMenu->dwExStyle & MIS_HORITEM)
			{
				UTFMenuFocusBack(hMenu);
			}
			else
			{
				UIFMenuToParentMenu(hMenu);
			}
			break;

		case KEYUI_RIGHT:
			if(pMenu->dwExStyle & MIS_HORITEM)
			{
				UTFMenuFocusForward(hMenu);
			}
			else
			{
				char bCanToSub = FALSE;
				
				if(pMenu->dwExStyle & MIS_SHOWSUB)
				{
					pItem = UTFMenuGetFocusItem(hMenu);
					if(pItem != NULL)
					{
						if(pItem->hSubWnd)
						{
							bCanToSub = TRUE;
						}
					}
				}
				
				if(bCanToSub == TRUE)
				{
					UIFMenuToSubMenu(hMenu, FALSE);
				}
				else
				{
					UIFMenuToParentMenu(hMenu);
				}
			}
			break;

		case KEYUI_SELECT:
			UIFMenuToSubMenu(hMenu, TRUE);
			break;

		case KEYUI_MENU:
			if(UTFGetDefaultCloseWindowMode() == 0)
				UTFWindowEnd(hMenu, ID_CANCEL);
			else
				UTFWindowClose(hMenu);
			break;

		case KEYUI_EXIT:
			if(UTFGetDefaultCloseWindowMode() == 0)
				UTFWindowClose(hMenu);
			else
				UTFWindowEnd(hMenu, ID_CANCEL);
			break;

		default:
			pItem = UTFMenuGetFocusItem(hMenu);
			if(pItem != NULL)
			{
				retCode = UTFSendMessage((HUIWND)pItem, uMsg, param1, param2, param3);
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
		case WMUI_SETPARENT:
			pMenu->hParent = param2;
			break;

		case WMUI_UPDATE:
			UTFMenuInvalidate(hMenu, 0);
			break;
			
		case WMUI_PAINT:
			if(param3 == TRUE)
			{
				pMenu->dwFlags |= UIF_DRAWTITLE;

				if(param2 == NULL)
				{
					pMenu->dwFlags |= UIF_REDRAW;
					UTFMenuRedrawAllItem(hMenu);
				}
				else
				{
					UTFMenuRedrawItemInRect(hMenu, (LPUTFRECT)param2);
				}
			}
			else
			{
				pMenu->dwFlags |= UIF_DRAWTITLE;
				
				if(param2 == NULL)
				{
					UTFMenuRedrawAllItem(hMenu);
				}
				else
				{
					UTFMenuRedrawItemInRect(hMenu, (LPUTFRECT)param2);
				}
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
						strcpy((char *)pMenu->wndText, (char *)param2);
					}
					else
					{
						memcpy((char *)pMenu->wndText, (char *)param2, TEXT_SIZE);
						pMenu->wndText[TEXT_SIZE-1] = 0;
					}
					pMenu->textID = -1;
					pMenu->dwFlags |= UIF_DRAWTITLE;
				}
				else if(pMenu->textID != param3)
				{
					pMenu->textID = param3;
					pMenu->dwFlags |= UIF_DRAWTITLE;
				}				
			}
			break;

		case WMUI_GETTEXT:
			if((param2 != NULL) && (param3 > 0))
			{
				if(pMenu->textID >= 0)
				{
					UTFGetIDText(pMenu->textID, pMenu->wndText, TEXT_SIZE);
				}
				if(strlen((char *)pMenu->wndText) < param3)
				{
					strcpy((char *)param2, (char *)pMenu->wndText);
				}
				else
				{
					PBYTE pBuffer = (PBYTE)param2;
					
					memcpy(pBuffer, pMenu->wndText, param3);
					*(pBuffer+param3-1) = 0;
				}
			}
			break;

		case WMUI_SHOW:
			if(pMenu->dwStyle & UIS_HIDE)
			{
				pMenu->dwStyle &= ~UIS_HIDE;
			}
			break;

		case WMUI_HIDE:
			if((pMenu->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pMenu->dwStyle |= UIS_HIDE;
			}
			break;

		case WMUI_DELETEWND:
			{
				HUIWND hNextItem;
				
				pItem = pMenu->pItem;
				while(pItem)
				{
					hNextItem = pItem->hNextItem;
					
					UTFFree((DWORD)pItem);
					
					pItem = (LPUTFMENUITEM)hNextItem;
				}
				UTFFree(hMenu);
			}
			break;

		default:
			retCode = 0;
			break;
		}
		break;

	default:
		pItem = UTFMenuGetFocusItem(hMenu);
		if(pItem != NULL)
		{
			retCode = UTFSendMessage((HUIWND)pItem, uMsg, param1, param2, param3);
		}
		else
		{
			retCode = 0;
		}
		break;
	}

	return retCode;
}

// This function is private function for UI
void UTFMenuInitialize(void)
{
	g_MenuClearColor = UTFRGB(0,0,0,0);
	g_bEnableMenuUpdate = TRUE;

	// Register default proceed function for menu
	UTFRegisterWndType(WTP_MENU, UTFMenuMsgProc, UTFMenuDrawFace);

	// Register default proceed function for menu item
	UTFRegisterWndType(WTP_MENUITEM, UTFMenuItemMsgProc, UTFMenuItemFace);
}

/********************** start of menu api function ************************/
HUIWND UTFAPI UTFCreateMenu(DWORD menuID, int textID, char *text, DWORD dwStyle, DWORD dwExStyle,
						UTFRECT rcMenu, DWORD menuData, UTFONDRAW OnDraw, UTFCALLBACK CallBack)
{
	HUIWND hMenu = (HUIWND)UTFMalloc(sizeof(UTFMenu));

	if(hMenu != NULL)
	{
		LPUTFMenu pMenu = (LPUTFMenu)hMenu;

		memcpy(pMenu->type, WTP_MENU, 4);
		pMenu->wndID	= menuID;
		
		pMenu->dwStyle	= dwStyle | UIS_MODAL;
		pMenu->dwStyle	&= ~(UIS_CHILD | UIS_TOPMOST | UIS_TABSTOP);
		pMenu->dwExStyle = dwExStyle;
		pMenu->dwFlags	= 0;
		
		pMenu->hParent	= NULL;
		pMenu->dataEx	= menuData;
		
		pMenu->rect		= rcMenu;
		UTFNormalizeRect(&pMenu->rect);
		
		pMenu->OnDraw	= OnDraw;
		pMenu->CallBack	= CallBack;
		pMenu->pItem	= NULL;

		pMenu->total	= 0;

		if(text != NULL)
		{
			pMenu->textID = -1;
			if(strlen(text) >= TEXT_SIZE)
			{
				memcpy((char *)pMenu->wndText, text, TEXT_SIZE-1);
				pMenu->wndText[TEXT_SIZE-1] = 0;
			}
			else
			{
				strcpy((char *)pMenu->wndText, text);
			}
		}
		else
		{
			pMenu->textID = textID;
		}
	}

	return hMenu;
}

int UTFAPI UTFAddItemToMenu(HUIWND hMenu, DWORD itemID, DWORD dwStyle, DWORD dwExStyle, HIMAGE hBitmapChecked,
					HIMAGE hBitmapUnCheck, HUIWND hSubWnd, UTFRECT rcItem, UTFONDRAW OnDraw,
					UTFCALLBACK CallBack, DWORD itemData, int textID, char *text)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMENUITEM pItem;
	HUIWND hItem;

	if(UTFIsMenu(hMenu) == FALSE)
		return 0;

	if(hSubWnd != NULL)
	{
		if(UTFIsMenu(hSubWnd) == FALSE)
			return 0;
	}

	UTFNormalizeRect(&rcItem);
	if(UTFIsRectEmpty(&rcItem))
		return 0;
	if((rcItem.right > (pMenu->rect.right-pMenu->rect.left)) || (rcItem.bottom > (pMenu->rect.bottom-pMenu->rect.top)))
		return 0;

	hItem = (HUIWND)UTFMalloc(sizeof(UTFMENUITEM));
	if(hItem == NULL)
		return 0;
	
	pItem = (LPUTFMENUITEM)hItem;

	memcpy(pItem->type, WTP_MENUITEM, 4);
	pItem->wndID	= itemID;
	
	pItem->dwStyle	= dwStyle | UIS_CHILD;
	pItem->dwStyle	&= ~(UIS_TOPMOST | UIS_MODAL | UIS_HIDE | UIS_TABSTOP);
	pItem->dwExStyle = dwExStyle;
	pItem->dwFlags	= UIF_REDRAW;
	if(UTFMenuGetFocusItem(hMenu) == NULL)
	{
		if(!((pItem->dwStyle & UIS_DISABLE) || (pItem->dwExStyle & MIS_SEPARATOR)))
		{
			pItem->dwFlags |= UIF_FOCUS;
		}
	}
	
	pItem->hParent	= hMenu;
	pItem->hBitmapChecked = hBitmapChecked;
	pItem->hBitmapUnCheck = hBitmapUnCheck;
	pItem->hSubWnd	= hSubWnd;
	pItem->hNextItem = NULL;
	pItem->dataEx	= itemData;
	pItem->rect		= rcItem;
	pItem->OnDraw	= OnDraw;
	pItem->CallBack	= CallBack;

	pMenu->total++;
	
	if(text != NULL)
	{
		pItem->textID = -1;
		if(strlen(text) >= TEXT_SIZE)
		{
			memcpy((char *)pItem->wndText, text, TEXT_SIZE-1);
			pItem->wndText[TEXT_SIZE-1] = 0;
		}
		else
		{
			strcpy((char *)pItem->wndText, text);
		}
	}
	else
	{
		pItem->textID = textID;
	}

	if(pMenu->pItem == NULL)
	{		
		pItem->hPrevItem = NULL;
		pMenu->pItem = pItem;
	}
	else
	{
		LPUTFMENUITEM pMenuItem = pMenu->pItem;
		
		while(pMenuItem)
		{
			if(pMenuItem->hNextItem == NULL)
			{
				pMenuItem->hNextItem = hItem;
				pItem->hPrevItem = (HUIWND)pMenuItem;
				break;
			}
			
			pMenuItem = (LPUTFMENUITEM)pMenuItem->hNextItem;
		}
	}

	return 1;
}

int UTFIsMenu(HUIWND hWnd)
{
	return UTFWndIsType(hWnd, WTP_MENU);
}

int UTFAPI UTFGetMenuItemCount(HUIWND hMenu)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;

	if(hMenu == NULL)
		return 0;

	return (pMenu->total);
}

HUIWND UTFAPI UTFMenuGetFocus( HUIWND hMenu )
{
	if(UTFIsMenu(hMenu) == FALSE)
		return NULL;

	return (HUIWND)UTFMenuGetFocusItem(hMenu);
}

int UTFAPI UTFGetMenuString(HUIWND hMenu, int uIDItem, char *lpString, int nMaxCount)
{
	LPUTFMENUITEM pItem;

	pItem = UTFFindMenuItem(hMenu, uIDItem);
	
	return UTFGetWindowText((HUIWND)pItem, lpString, nMaxCount);
}

HUIWND UTFAPI UTFGetSubMenu(HUIWND hMenu, DWORD uItem)
{
	LPUTFMENUITEM pItem;

	pItem = UTFFindMenuItem(hMenu, uItem);
	
	if(pItem != NULL)
		return (pItem->hSubWnd);
	else
		return NULL;
}

HUIWND UTFAPI UTFGetMenuItem(HUIWND hMenu, DWORD uItem)
{
	return (HUIWND)UTFFindMenuItem(hMenu, uItem);
}

HUIWND UTFAPI UTFGetMenuIndexItem(HUIWND hMenu, DWORD uIndex)
{
	LPUTFMenu pMenu = (LPUTFMenu)hMenu;
	LPUTFMENUITEM pItem;
	DWORD dwItemIndex=0;

	pItem = pMenu->pItem;
	while(pItem)
	{
		if(dwItemIndex == uIndex)
		{
			return (HUIWND)pItem;
		}

		dwItemIndex++;

		pItem = (LPUTFMENUITEM)pItem->hNextItem;
	}

	return NULL;
}

int UTFAPI UTFGetMenuItemInfo(HUIWND hMenu, DWORD uItem, LPUTFMENUITEM lpMII)
{
	LPUTFMENUITEM pItem;

	pItem = UTFFindMenuItem(hMenu, uItem);
	if(pItem != NULL)
	{
		memcpy(lpMII, pItem, sizeof(UTFMENUITEM));
		return 1;
	}

	return 0;
}

void UTFAPI UTFEnableMenuUpdate(char bEnableUpdate)
{
	if(bEnableUpdate == FALSE)
		g_bEnableMenuUpdate = bEnableUpdate;
	else
		g_bEnableMenuUpdate = TRUE;
}

void UTFAPI UTFMenuSetClrColor(UTFCOLOR clrColor)
{
	g_MenuClearColor = clrColor;
}

