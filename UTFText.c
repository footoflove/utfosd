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
#include "UTFText.h"
#include "UTFTextPriv.h"
#include "UTFMemory.h"

/******************************************************************/
static void UTFDrawTextFace(HUIWND hText, UTFRECT rcRect, DWORD info)
{
	LPUTFTEXT pText = (LPUTFTEXT)hText;
	UTFRECT wndRect = rcRect;
	DWORD uFormat;

	/* Draw background */	
	UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
	UTFSetBrush(BSUI_SOLID, UTFGetDialogBkColor(pText->hParent), HSUI_HORIZONTAL);
	UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
	
	if(pText->textID >= 0)
	{
		UTFGetIDText(pText->textID, pText->wndText, TEXT_SIZE);
	}

	if(pText->dwStyle & UIS_DISABLE)
	{
		UTFSetTextColor(UTFRGB(100,100,100,255));
	}
	else
	{
		UTFSetTextColor(UTFRGB(0,0,0,255));
	}
	uFormat = DTUI_CENTER|DTUI_VCENTER|DTUI_END_ELLIPSIS;
	UTFDrawText(pText->wndText, &wndRect, uFormat);
}

static int UTFTextMsgProc(HUIWND hText, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFTEXT pText = (LPUTFTEXT)hText;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pText->hParent;
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
			pText->dwFlags |= UIF_REDRAW;
			break;

		case WMUI_ENABLE:
			if(pText->dwStyle & UIS_DISABLE)
			{
				pText->dwStyle &= ~UIS_DISABLE;
				pText->dwFlags |= UIF_REDRAW;
			}
			break;

		case WMUI_DISABLE:
			if((pText->dwStyle & UIS_DISABLE) != UIS_DISABLE)
			{
				pText->dwStyle |= UIS_DISABLE;
				pText->dwFlags |= UIF_REDRAW;
			}
			break;

		case WMUI_SHOW:
			if(pText->dwStyle & UIS_HIDE)
			{
				pText->dwStyle &= ~UIS_HIDE;
				pText->dwFlags |= UIF_REDRAW;

				UTFDialogDrawAllChildAboveCtrl(pText->hParent, hText);
			}
			break;

		case WMUI_HIDE:
			if((pText->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pText->dwStyle |= UIS_HIDE;
				pText->dwFlags |= UIF_REDRAW;
				pText->dwFlags |= DLGF_CLEANBACK;

				UTFDialogRedrawCtrlInRect(pText->hParent, &pText->rect);
			}
			break;

		case WMUI_SETRECT:
			if(param2)
			{
				LPUTFRECT lpRect = (LPUTFRECT)param2;
				UTFRECT oldRect = pText->rect;

				if((lpRect->right > (pDlg->rect.right-pDlg->rect.left)) || (lpRect->bottom > (pDlg->rect.bottom-pDlg->rect.top)))
					return 0;

				pText->dwFlags |= UIF_REDRAW;
				
				UTFDialogBackClean(pText->hParent, hText);
				UTFDialogRedrawCtrlInRect(pText->hParent, &pText->rect);
				
				memcpy(&pText->rect, lpRect, sizeof(UTFRECT));
				UTFDialogDrawAllChildAboveCtrl(pText->hParent, hText);

				if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
				{
					UTFDialogResetCtrlXYOrder(pText->hParent, hText, oldRect);
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
						strcpy((char *)pText->wndText, (char *)param2);
					}
					else
					{
						memcpy((char *)pText->wndText, (char *)param2, TEXT_SIZE);
						pText->wndText[TEXT_SIZE-1] = 0;
					}
					pText->textID = -1;
					pText->dwFlags |= UIF_REDRAW;
				}
				else if(pText->textID != param3)
				{
					pText->textID = param3;
					pText->dwFlags |= UIF_REDRAW;
				}				
			}
			break;

		case WMUI_GETTEXT:
			if((param2 != NULL) && (param3 > 0))
			{
				if(pText->textID >= 0)
				{
					UTFGetIDText(pText->textID, pText->wndText, TEXT_SIZE);
				}
				if(strlen((char *)pText->wndText) < param3)
				{
					strcpy((char *)param2, (char *)pText->wndText);
				}
				else
				{
					char *pBuffer = (char *)param2;
					
					memcpy(pBuffer, (char *)pText->wndText, param3);
					*(pBuffer+param3-1) = 0;
				}
			}
			break;	

		case WMUI_DELETEWND:
			if(param2 == 0)
			{
				LPUTFDIALOGCTRL pCtrl;			

				if(pText->hPrevCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pText->hPrevCtrl;
					pCtrl->hNextCtrl = pText->hNextCtrl;
				}
				else
				{
					pDlg->pCtrl = (LPUTFDIALOGCTRL)pText->hNextCtrl;
				}

				if(pText->hNextCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pText->hNextCtrl;
					pCtrl->hPrevCtrl = pText->hPrevCtrl;
				}
			}
			UTFFree(hText);
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

void UTFTextInitialize(void)
{
	UTFRegisterWndType(WTP_TEXT, UTFTextMsgProc, UTFDrawTextFace);
}

HUIWND UTFAPI UTFCreateText(DWORD ctrlID, int textID, char *text, DWORD dwStyle, DWORD dwExStyle,
						HUIWND hDlg, WORD left, WORD top, WORD right, WORD bottom,
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
		HUIWND hText = (HUIWND)UTFMalloc(sizeof(UTFTEXT));
		
		if(hText != NULL)
		{
			LPUTFTEXT pText = (LPUTFTEXT)hText;

			memcpy(pText->type, WTP_TEXT, 4);
			pText->wndID	= ctrlID;
			
			pText->dwStyle	= dwStyle | UIS_CHILD | UIS_NOFOCUS;
			pText->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST);
			pText->dwExStyle = dwExStyle;
			pText->dwFlags	= 0;
			
			pText->hParent	= hDlg;
			pText->dataEx	= ctrlData;
			pText->rect 	= rcRect;
			
			pText->OnDraw = OnDraw;
			pText->CallBack	= NULL;

			if(text != NULL)
			{
				pText->textID = -1;
				if(strlen(text) >= TEXT_SIZE)
				{
					memcpy((char *)pText->wndText, text, TEXT_SIZE-1);
					pText->wndText[TEXT_SIZE-1] = 0;
				}
				else
				{
					strcpy((char *)pText->wndText, text);
				}
			}
			else
			{
				pText->textID = textID;
			}

			UTFAddCtrlToDlg(hDlg, hText);
		}

		return hText;
	}

	return NULL;
}

