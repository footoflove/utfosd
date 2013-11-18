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
#include "UTFMsgDefPriv.h"
#include "UTFWndDef.h"
#include "UTFWndDefPriv.h"
#include "UTFDrawAPI.h"
#include "UTFFont.h"
#include "UTFButton.h"
#include "UTFButtonPriv.h"
#include "UTFDrawText.h"
#include "UTFMemory.h"

/******************************************************************/
static void UTFDrawButtonFace(HUIWND hButton, UTFRECT rcRect, DWORD info)
{
	LPUTFBUTTON pButton = (LPUTFBUTTON)hButton;
	UTFRECT wndRect = rcRect;
	DWORD uFormat;

	/* Draw background */
	UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
	if(pButton->dwStyle & UIS_DISABLE)
	{
		UTFSetBrush(BSUI_SOLID, UTFRGB(150,150,150,255), HSUI_HORIZONTAL);
		UTFSetTextColor(UTFRGB(0,0,0,255));
	}
	else if(pButton->dwFlags & UIF_FOCUS)
	{
		UTFSetBrush(BSUI_SOLID, UTFRGB(220,120,70,255), HSUI_HORIZONTAL);
		UTFSetTextColor(UTFRGB(0,0,0,255));
	}
	else
	{
		UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,150,255), HSUI_HORIZONTAL);
		UTFSetTextColor(UTFRGB(255,255,255,255));
	}
	UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
	
	if(pButton->textID >= 0)
	{
		UTFGetIDText(pButton->textID, pButton->wndText, TEXT_SIZE);
	}

	uFormat = DTUI_CENTER|DTUI_VCENTER|DTUI_SINGLELINE|DTUI_END_ELLIPSIS;
	UTFDrawText(pButton->wndText, &wndRect, uFormat);
}

static int UTFButtonMsgProc(HUIWND hButton, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFBUTTON pButton = (LPUTFBUTTON)hButton;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pButton->hParent;
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
			if((pButton->dwStyle & ( UIS_DISABLE | UIS_HIDE )) || !(pButton->dwStyle & UIS_TABSTOP))
			{
				retCode = 0;
			}
			else if( !(pButton->dwFlags & UIF_FOCUS) )
			{
				HUIWND hFocusCtrl = UTFDlgGetFocus(pButton->hParent);

				pButton->dwFlags |= UIF_FOCUS;
				pButton->dwFlags |= UIF_REDRAW;

				UTFSendPrivMessage(pButton->hParent, WMUI_INTER, WMUI_NEWFOCUS, hButton, hFocusCtrl);
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
			if(pDlg->CallBack)
			{
				pDlg->CallBack(pButton->hParent, WMUI_NOTIFY, BNUI_CLICKED, pButton->wndID, 0);
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
			pButton->dwFlags |= UIF_REDRAW;
			break;

		case WMUI_KILLFOCUS:
			pButton->dwFlags &= ~UIF_FOCUS;
			pButton->dwFlags |= UIF_REDRAW;
			break;

		case WMUI_ENABLE:
			if(pButton->dwStyle & UIS_DISABLE)
			{
				pButton->dwStyle &= ~UIS_DISABLE;
				pButton->dwFlags |= UIF_REDRAW;

				if(UTFDialogHaveFocus(pButton->hParent) == FALSE)
				{
					UTFSendPrivMessage(hButton, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}
			}
			break;

		case WMUI_DISABLE:
			if((pButton->dwStyle & UIS_DISABLE) != UIS_DISABLE)
			{
				pButton->dwStyle |= UIS_DISABLE;
				pButton->dwFlags |= UIF_REDRAW;

				if(pButton->dwFlags & UIF_FOCUS)
				{
					UTFSendPrivMessage(pButton->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
					pButton->dwFlags &= ~UIF_FOCUS;
				}
			}
			break;

		case WMUI_SHOW:
			if(pButton->dwStyle & UIS_HIDE)
			{
				pButton->dwStyle &= ~UIS_HIDE;
				pButton->dwFlags |= UIF_REDRAW;
				
				UTFDialogDrawAllChildAboveCtrl(pButton->hParent, hButton);
				
				if(UTFDialogHaveFocus(pButton->hParent) == FALSE)
				{
					UTFSendPrivMessage(hButton, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}
			}
			break;

		case WMUI_HIDE:
			if((pButton->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pButton->dwStyle |= UIS_HIDE;
				pButton->dwFlags |= UIF_REDRAW;
				pButton->dwFlags |= DLGF_CLEANBACK;
				
				if(pButton->dwFlags & UIF_FOCUS)
				{
					UTFSendPrivMessage(pButton->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
					pButton->dwFlags &= ~UIF_FOCUS;
				}
				
				UTFDialogRedrawCtrlInRect(pButton->hParent, &pButton->rect);
			}
			break;

		case WMUI_SETRECT:
			if(param2)
			{
				LPUTFRECT lpRect = (LPUTFRECT)param2;
				UTFRECT oldRect = pButton->rect;

				if((lpRect->right > (pDlg->rect.right-pDlg->rect.left)) || (lpRect->bottom > (pDlg->rect.bottom-pDlg->rect.top)))
					return 0;

				pButton->dwFlags |= UIF_REDRAW;
				
				UTFDialogBackClean(pButton->hParent, hButton);
				UTFDialogRedrawCtrlInRect(pButton->hParent, &pButton->rect);
				
				memcpy(&pButton->rect, lpRect, sizeof(UTFRECT));
				UTFDialogDrawAllChildAboveCtrl(pButton->hParent, hButton);

				if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
				{
					UTFDialogResetCtrlXYOrder(pButton->hParent, hButton, oldRect);
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
						strcpy((char *)pButton->wndText, (char *)param2);
					}
					else
					{
						memcpy((char *)pButton->wndText, (char *)param2, TEXT_SIZE);
						pButton->wndText[TEXT_SIZE-1] = 0;
					}
					pButton->textID = -1;	
					pButton->dwFlags |= UIF_REDRAW;
				}
				else if(pButton->textID != param3)
				{
					pButton->textID = param3;
					pButton->dwFlags |= UIF_REDRAW;
				}
			}
			break;

		case WMUI_GETTEXT:
			if((param2 != NULL) && (param3 > 0))
			{
				if(pButton->textID >= 0)
				{
					UTFGetIDText(pButton->textID, pButton->wndText, TEXT_SIZE);
				}
				if(strlen((char *)pButton->wndText) < param3)
				{
					strcpy((char *)param2, (char *)pButton->wndText);
				}
				else
				{
					char *pBuffer = (char *)param2;
					
					memcpy(pBuffer, (char *)pButton->wndText, param3);
					*(pBuffer+param3-1) = 0;
				}
			}
			break;	

		case WMUI_DELETEWND:
			if(param2 == 0)
			{
				LPUTFDIALOGCTRL pCtrl;			

				if(pButton->hPrevCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pButton->hPrevCtrl;
					pCtrl->hNextCtrl = pButton->hNextCtrl;
				}
				else
				{
					pDlg->pCtrl = (LPUTFDIALOGCTRL)pButton->hNextCtrl;
				}

				if(pButton->hNextCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pButton->hNextCtrl;
					pCtrl->hPrevCtrl = pButton->hPrevCtrl;
				}			
			}
			UTFFree(hButton);
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

void UTFButtonInitialize(void)
{
	UTFRegisterWndType(WTP_BUTTON, UTFButtonMsgProc, UTFDrawButtonFace);
}

HUIWND UTFAPI UTFCreateButton(DWORD ctrlID, int textID, char *text, DWORD dwStyle, DWORD dwExStyle,
						HUIWND hDlg, WORD left, WORD top, WORD right, WORD bottom, DWORD ctrlData,
						UTFONDRAW OnDraw, UTFCALLBACK CallBack)
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
		HUIWND hButton = (HUIWND)UTFMalloc(sizeof(UTFBUTTON));

		if(hButton != NULL)
		{
			LPUTFBUTTON pButton = (LPUTFBUTTON)hButton;

			memcpy(pButton->type, WTP_BUTTON, 4);
			pButton->wndID		= ctrlID;
			
			pButton->dwStyle	= dwStyle | UIS_CHILD;
			pButton->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST | UIS_NOFOCUS);
			pButton->dwExStyle 	= dwExStyle;
			pButton->dwFlags	= 0;
			
			pButton->hParent	= hDlg;
			pButton->dataEx		= ctrlData;
			pButton->rect		= rcRect;
			
			pButton->OnDraw		= OnDraw;
			pButton->CallBack	= CallBack;

			if(text != NULL)
			{
				pButton->textID = -1;
				if(strlen(text) >= TEXT_SIZE)
				{
					memcpy((char *)pButton->wndText, text, TEXT_SIZE-1);
					pButton->wndText[TEXT_SIZE-1] = 0;
				}
				else
				{
					strcpy((char *)pButton->wndText, text);
				}
			}
			else
			{
				pButton->textID = textID;
			}

			UTFAddCtrlToDlg(hDlg, hButton);
		}

		return hButton;
	}

	return NULL;
}

