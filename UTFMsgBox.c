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
#include "UTFMsgbox.h"
#include "UTFZOrderManage.h"
#include "UTFZOrderManagerPriv.h"
#include "UTFDrawText.h"
#include "UTFMemory.h"
#include "UTFTimer.h"

static void UTFMsgBoxDrawFace(HUIWND hMsgBox, UTFRECT rcRect, DWORD info)
{
	LPUTFMSGBOX pMsgBox = (LPUTFMSGBOX)hMsgBox;
	UTFRECT wndRect = rcRect;
	int borderDepth = 2;
	
	UTFSetPen(PSUI_SOLID, UTFRGB(255,255,255,255), borderDepth);
	UTFSetBrush(BSUI_SOLID, UTFRGB(30,55,95,255), 0);
	UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);

	UTFDeflateRect(&wndRect, 5, 5, 5, 5);
	if(pMsgBox->textID >= 0)
	{
		UTFGetIDText(pMsgBox->textID, pMsgBox->wndText, TEXT_SIZE);
	}
	
	UTFSetTextColor(UTFRGB(255,255,255,255));
	UTFDrawText(pMsgBox->wndText, &wndRect, DTUI_END_ELLIPSIS|DTUI_CENTER|DTUI_VCENTER);
}

static int UTFMsgBoxMsgProc(HUIWND hMsgBox, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFMSGBOX pMsgBox = (LPUTFMSGBOX)hMsgBox;
	int retCode = 1;

	switch(uMsg)
	{
	case WMUI_PUBLIC:
		switch(param1)
		{
		case WMUI_CREATE:
			pMsgBox->dwStyle &= ~UIS_HIDE;
			pMsgBox->dwFlags |= UIF_FOCUS;
			pMsgBox->dwFlags |= UIF_REDRAW;
			break;

		case WMUI_DESTROY:
			pMsgBox->dwFlags &= ~UIF_FOCUS;
			pMsgBox->dwFlags &= ~UIF_REDRAW;

			UTFKillWindowTimer(hMsgBox);
			break;

		case WMUI_ACTIVE:
			if(pMsgBox->CallBack)
			{
				pMsgBox->CallBack(hMsgBox, uMsg, param1, param2, param3);
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
		case WMUI_UPDATE:
			if(pMsgBox->dwFlags & UIF_REDRAW)
			{
				UTFSetOpenScreenFlag();

				if(pMsgBox->CallBack)
				{
					pMsgBox->CallBack(hMsgBox, WMUI_PUBLIC, WMUI_QUERY_SHOW, 0, 0);
				}				
				
				if(pMsgBox->OnDraw)
				{
					if(pMsgBox->dwStyle & UIS_OWNERDRAW)
					{
						pMsgBox->OnDraw(hMsgBox, pMsgBox->rect, 0);
					}
					else
					{
						UTFMsgBoxDrawFace(hMsgBox, pMsgBox->rect, 0);
					}
				}
				else
				{
					UTFMsgBoxDrawFace(hMsgBox, pMsgBox->rect, 0);
				}
				pMsgBox->dwFlags &= ~UIF_REDRAW;
			}
			break;
			
		case WMUI_PAINT:
			pMsgBox->dwFlags |= UIF_REDRAW;
			break;

		case WMUI_SETTEXT:
			{
				int textLen;

				if(param2 != NULL)
				{
					textLen = strlen((char *)param2);
					if(textLen < TEXT_SIZE)
					{
						strcpy((char *)pMsgBox->wndText, (char *)param2);
					}
					else
					{
						memcpy((char *)pMsgBox->wndText, (char *)param2, TEXT_SIZE);
						pMsgBox->wndText[TEXT_SIZE-1] = 0;
					}
					pMsgBox->textID = -1;
					pMsgBox->dwFlags |= UIF_REDRAW;
				}
				else if(pMsgBox->textID != param3)
				{
					pMsgBox->textID = param3;
					pMsgBox->dwFlags |= UIF_REDRAW;
				}				
			}
			break;

		case WMUI_GETTEXT:
			if((param2 != NULL) && (param3 > 0))
			{
				if(pMsgBox->textID >= 0)
				{
					UTFGetIDText(pMsgBox->textID, pMsgBox->wndText, TEXT_SIZE);
				}
				if(strlen((char *)pMsgBox->wndText) < param3)
				{
					strcpy((char *)param2, (char *)pMsgBox->wndText);
				}
				else
				{
					char *pBuffer = (char *)param2;
					
					memcpy(pBuffer, (char *)pMsgBox->wndText, param3);
					*(pBuffer+param3-1) = 0;
				}
			}
			break;

		case WMUI_DELETEWND:
			UTFFree(hMsgBox);
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

void UTFMsgBoxInitialize(void)
{
	UTFRegisterWndType(WTP_WINDOW, UTFMsgBoxMsgProc, NULL);
}

/**************************************************************************/
int UTFAPI UTFIsMsgBox(HUIWND hWnd)
{
	return UTFWndIsType(hWnd, WTP_WINDOW);
}

HUIWND UTFAPI UTFCreateMsgBox(DWORD msgBoxID, int textID, char *text, DWORD dwStyle, DWORD dwExStyle,
						WORD left, WORD top, WORD right, WORD bottom, DWORD wndData,
						UTFONDRAW OnDraw, UTFCALLBACK CallBack, DWORD data1, DWORD data2)
{
	HUIWND hMsgBox = (HUIWND)UTFMalloc(sizeof(UTFMSGBOX));

	if(hMsgBox != NULL)
	{
		LPUTFMSGBOX pMsgBox = (LPUTFMSGBOX)hMsgBox;

		memcpy(pMsgBox->type, WTP_WINDOW, 4);
		pMsgBox->wndID		= msgBoxID;
		
		pMsgBox->dwStyle	= dwStyle;
		pMsgBox->dwStyle	&= ~(UIS_CHILD | UIS_MODAL | UIS_TABSTOP);
		pMsgBox->dwExStyle = dwExStyle;
		pMsgBox->dwFlags	= 0;
		
		pMsgBox->hParent	= NULL;
		pMsgBox->dataEx 	= wndData;
		
		pMsgBox->OnDraw	= OnDraw;
		pMsgBox->CallBack	= CallBack;

		pMsgBox->rect.left		= left;
		pMsgBox->rect.top		= top;
		pMsgBox->rect.right		= right;
		pMsgBox->rect.bottom	= bottom;
		UTFNormalizeRect(&pMsgBox->rect);
		
		pMsgBox->data1	= data1;
		pMsgBox->data2 	= data2;
		
		if(text != NULL)
		{
			pMsgBox->textID = -1;
			if(strlen(text) >= TEXT_SIZE)
			{
				memcpy((char *)pMsgBox->wndText, text, TEXT_SIZE-1);
				pMsgBox->wndText[TEXT_SIZE-1] = 0;
			}
			else
			{
				strcpy((char *)pMsgBox->wndText, text);
			}
		}
		else
		{
			pMsgBox->textID = textID;
		}
	}

	return hMsgBox;
}

void UTFAPI UTFMsgBoxSetData(HUIWND hMsgBox, DWORD data1, DWORD data2)
{
	if(UTFIsMsgBox(hMsgBox) == TRUE)
	{
		LPUTFMSGBOX pMsgBox = (LPUTFMSGBOX)hMsgBox;

		if((pMsgBox->data1 != data1) || (pMsgBox->data2 != data2))
		{
			pMsgBox->dwFlags |= UIF_REDRAW;
		}
		
		pMsgBox->data1 = data1;
		pMsgBox->data2 = data2;
	}
}

