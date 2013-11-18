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
#include "UTFCheckBox.h"
#include "UTFCheckBoxPriv.h"
#include "UTFDrawText.h"
#include "UTFMemory.h"

/******************************************************************/
static void UTFDrawCheckBoxFace(HUIWND hCheckBox, UTFRECT rcRect, DWORD info)
{
	LPUTFCHECKBOX pCheckBox = (LPUTFCHECKBOX)hCheckBox;
	UTFRECT wndRect = rcRect;
	UTFCOLOR textColor;
	DWORD uFormat;

	/* Draw background */
	if(pCheckBox->dwStyle & UIS_DISABLE)
	{
		UTFSetBrush(BSUI_SOLID, UTFRGB(150,150,150,255), HSUI_HORIZONTAL);
		textColor = UTFRGB(0,0,0,255);
	}
	else if(pCheckBox->dwFlags & UIF_FOCUS)
	{
		UTFSetBrush(BSUI_SOLID, UTFRGB(220,120,70,255), HSUI_HORIZONTAL);
		textColor = UTFRGB(0,0,0,255);
	}
	else
	{
		UTFSetBrush(BSUI_SOLID, UTFRGB(50,50,150,255), HSUI_HORIZONTAL);
		textColor = UTFRGB(255,255,255,255);
	}
	
	if(pCheckBox->dwFlags & CKBF_DRAWFRAME)
	{
		UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 0);
		UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);

		wndRect.right -= 30;
		if(pCheckBox->textID >= 0)
		{
			UTFGetIDText(pCheckBox->textID, pCheckBox->wndText, TEXT_SIZE);
		}
		uFormat = DTUI_LEFT|DTUI_VCENTER|DTUI_SINGLELINE|DTUI_END_ELLIPSIS;
		UTFSetTextColor(textColor);
		UTFDrawText(pCheckBox->wndText, &wndRect, uFormat);
	}

	if(pCheckBox->dwFlags & CKBF_DRAWCHECK)
	{
		wndRect.top += 2;
		wndRect.bottom -= 2;
		wndRect.left = rcRect.right-30;
		wndRect.right = rcRect.right-2;
		UTFSetPen(PSUI_SOLID, textColor, 2);
		UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);

		if(pCheckBox->dwFlags & CKBF_CHECKFLAG)
		{
			UTFDrawline(wndRect.left+4, (wndRect.top+wndRect.bottom)>>1, (wndRect.left+wndRect.right)>>1, wndRect.bottom-3);
			UTFDrawline((wndRect.left+wndRect.right)>>1, wndRect.bottom-3, wndRect.right-4, wndRect.top+3);
		}
	}
}

static void UTFCheckBoxInvalidate(HUIWND hCheckBox, UTFRECT rcRect, DWORD info)
{
	LPUTFCHECKBOX pCheckBox = (LPUTFCHECKBOX)hCheckBox;

	if(pCheckBox->OnDraw)
	{
		pCheckBox->OnDraw(hCheckBox, rcRect, 0);
	}
	else
	{
		UTFDrawCheckBoxFace(hCheckBox, rcRect, 0);
	}	

	pCheckBox->dwFlags &= ~(CKBF_DRAWCHECK|CKBF_DRAWFRAME);
}

static int UTFCheckBoxMsgProc(HUIWND hCheckBox, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFCHECKBOX pCheckBox = (LPUTFCHECKBOX)hCheckBox;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pCheckBox->hParent;
	int retCode = 1;

	switch(uMsg)
	{
	case WMUI_PUBLIC:
		switch(param1)
		{
		case WMUI_CREATE:
			if(pCheckBox->dwFlags & CKBF_CHECKED)
			{
				pCheckBox->dwFlags |= CKBF_CHECKFLAG;
			}
			else
			{
				pCheckBox->dwFlags &= ~CKBF_CHECKFLAG;
			}
			break;

		case WMUI_DESTROY:
			break;

		case WMUI_SETFOCUS:
			if((pCheckBox->dwStyle & ( UIS_DISABLE | UIS_HIDE )) || !(pCheckBox->dwStyle & UIS_TABSTOP))
			{
				retCode = 0;
			}
			else if( !(pCheckBox->dwFlags & UIF_FOCUS) )
			{
				HUIWND hFocusCtrl = UTFDlgGetFocus(pCheckBox->hParent);

				pCheckBox->dwFlags |= UIF_FOCUS;
				pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWCHECK|CKBF_DRAWFRAME;

				UTFSendPrivMessage(pCheckBox->hParent, WMUI_INTER, WMUI_NEWFOCUS, hCheckBox, hFocusCtrl);
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
			{
				DWORD bCheck;
				
				pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWCHECK;
				if(pCheckBox->dwFlags & CKBF_CHECKED)
				{
					bCheck = 0;
					pCheckBox->dwFlags &= ~CKBF_CHECKED;
					pCheckBox->dwFlags &= ~CKBF_CHECKFLAG;
				}
				else
				{
					bCheck = 1;
					pCheckBox->dwFlags |= CKBF_CHECKED;
					pCheckBox->dwFlags |= CKBF_CHECKFLAG;
				}
				
				if(pDlg->CallBack)
				{
					pDlg->CallBack(pCheckBox->hParent, WMUI_NOTIFY, CKBN_CHECKED, pCheckBox->wndID, bCheck);
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
			pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWCHECK|CKBF_DRAWFRAME;
			break;

		case WMUI_KILLFOCUS:
			pCheckBox->dwFlags &= ~UIF_FOCUS;
			pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWCHECK|CKBF_DRAWFRAME;
			break;

		case WMUI_ENABLE:
			if(pCheckBox->dwStyle & UIS_DISABLE)
			{
				pCheckBox->dwStyle &= ~UIS_DISABLE;
				pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWCHECK|CKBF_DRAWFRAME;

				if(UTFDialogHaveFocus(pCheckBox->hParent) == FALSE)
				{
					UTFSendPrivMessage(hCheckBox, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}
			}
			break;

		case WMUI_DISABLE:
			if((pCheckBox->dwStyle & UIS_DISABLE) != UIS_DISABLE)
			{
				pCheckBox->dwStyle |= UIS_DISABLE;
				pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWCHECK|CKBF_DRAWFRAME;

				if(pCheckBox->dwFlags & UIF_FOCUS)
				{
					UTFSendPrivMessage(pCheckBox->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
					pCheckBox->dwFlags &= ~UIF_FOCUS;
				}
			}
			break;

		case WMUI_SHOW:
			if(pCheckBox->dwStyle & UIS_HIDE)
			{
				pCheckBox->dwStyle &= ~UIS_HIDE;
				pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWCHECK|CKBF_DRAWFRAME;
				
				UTFDialogDrawAllChildAboveCtrl(pCheckBox->hParent, hCheckBox);
				
				if(UTFDialogHaveFocus(pCheckBox->hParent) == FALSE)
				{
					UTFSendPrivMessage(hCheckBox, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}
			}
			break;

		case WMUI_HIDE:
			if((pCheckBox->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pCheckBox->dwStyle |= UIS_HIDE;
				pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWCHECK|CKBF_DRAWFRAME;
				pCheckBox->dwFlags |= DLGF_CLEANBACK;
				
				if(pCheckBox->dwFlags & UIF_FOCUS)
				{
					UTFSendPrivMessage(pCheckBox->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
					pCheckBox->dwFlags &= ~UIF_FOCUS;
				}
				
				UTFDialogRedrawCtrlInRect(pCheckBox->hParent, &pCheckBox->rect);
			}
			break;

		case WMUI_SETRECT:
			if(param2)
			{
				LPUTFRECT lpRect = (LPUTFRECT)param2;
				UTFRECT oldRect = pCheckBox->rect;

				if((lpRect->right > (pDlg->rect.right-pDlg->rect.left)) || (lpRect->bottom > (pDlg->rect.bottom-pDlg->rect.top)))
					return 0;

				pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWCHECK|CKBF_DRAWFRAME;
				
				UTFDialogBackClean(pCheckBox->hParent, hCheckBox);
				UTFDialogRedrawCtrlInRect(pCheckBox->hParent, &pCheckBox->rect);
				
				memcpy(&pCheckBox->rect, lpRect, sizeof(UTFRECT));
				UTFDialogDrawAllChildAboveCtrl(pCheckBox->hParent, hCheckBox);

				if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
				{
					UTFDialogResetCtrlXYOrder(pCheckBox->hParent, hCheckBox, oldRect);
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
						strcpy((char *)pCheckBox->wndText, (char *)param2);
					}
					else
					{
						memcpy((char *)pCheckBox->wndText, (char *)param2, TEXT_SIZE);
						pCheckBox->wndText[TEXT_SIZE-1] = 0;
					}
					pCheckBox->textID = -1;
					pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWFRAME;
				}
				else if(pCheckBox->textID != param3)
				{
					pCheckBox->textID = param3;
					pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWFRAME;
				}				
			}
			break;

		case WMUI_GETTEXT:
			if((param2 != NULL) && (param3 > 0))
			{
				if(pCheckBox->textID >= 0)
				{
					UTFGetIDText(pCheckBox->textID, pCheckBox->wndText, TEXT_SIZE);
				}
				if(strlen((char *)pCheckBox->wndText) < param3)
				{
					strcpy((char *)param2, (char *)pCheckBox->wndText);
				}
				else
				{
					char *pBuffer = (char *)param2;
					
					memcpy(pBuffer, (char *)pCheckBox->wndText, param3);
					*(pBuffer+param3-1) = 0;
				}
			}
			break;	

		case WMUI_DELETEWND:
			if(param2 == 0)
			{
				LPUTFDIALOGCTRL pCtrl;			

				if(pCheckBox->hPrevCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pCheckBox->hPrevCtrl;
					pCtrl->hNextCtrl = pCheckBox->hNextCtrl;
				}
				else
				{
					pDlg->pCtrl = (LPUTFDIALOGCTRL)pCheckBox->hNextCtrl;
				}

				if(pCheckBox->hNextCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pCheckBox->hNextCtrl;
					pCtrl->hPrevCtrl = pCheckBox->hPrevCtrl;
				}			
			}
			UTFFree(hCheckBox);
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

void UTFCheckBoxInitialize(void)
{
	UTFRegisterWndType(WTP_CHECKBOX, UTFCheckBoxMsgProc, NULL);
}

HUIWND UTFAPI UTFCreateCheckBox(DWORD ctrlID, int textID, char *text, DWORD dwStyle, DWORD dwExStyle,
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
		HUIWND hCheckBox = (HUIWND)UTFMalloc(sizeof(UTFCHECKBOX));

		if(hCheckBox != NULL)
		{
			LPUTFCHECKBOX pCheckBox = (LPUTFCHECKBOX)hCheckBox;

			memcpy(pCheckBox->type, WTP_CHECKBOX, 4);
			pCheckBox->wndID	= ctrlID;
			
			pCheckBox->dwStyle	= dwStyle | UIS_CHILD | UIS_OWNERDRAW;
			pCheckBox->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST | UIS_NOFOCUS);
			pCheckBox->dwExStyle = dwExStyle;
			pCheckBox->dwFlags	= 0;
			
			pCheckBox->hParent	= hDlg;
			pCheckBox->dataEx	= ctrlData;
			pCheckBox->rect		= rcRect;
			
			pCheckBox->CallBack	= CallBack;
			pCheckBox->DrawFace = UTFCheckBoxInvalidate;
			if(dwStyle & UIS_OWNERDRAW)
			{
				pCheckBox->OnDraw = OnDraw;
			}
			else
			{
				pCheckBox->OnDraw = NULL;
			}

			if(text != NULL)
			{
				pCheckBox->textID = -1;
				if(strlen(text) >= TEXT_SIZE)
				{
					memcpy((char *)pCheckBox->wndText, text, TEXT_SIZE-1);
					pCheckBox->wndText[TEXT_SIZE-1] = 0;
				}
				else
				{
					strcpy((char *)pCheckBox->wndText, text);
				}
			}
			else
			{
				pCheckBox->textID = textID;
			}

			UTFAddCtrlToDlg(hDlg, hCheckBox);
		}

		return hCheckBox;
	}

	return NULL;
}

int UTFAPI UTFCheckBoxSetCheck(HUIWND hCheckBox, char bCheck)
{
	if(UTFWndIsType(hCheckBox, WTP_CHECKBOX) == TRUE)
	{
		LPUTFCHECKBOX pCheckBox = (LPUTFCHECKBOX)hCheckBox;
		DWORD oldFlag = pCheckBox->dwFlags;

		if(bCheck)
		{
			pCheckBox->dwFlags |= CKBF_CHECKED;
			pCheckBox->dwFlags |= CKBF_CHECKFLAG;
		}
		else
		{
			pCheckBox->dwFlags &= ~CKBF_CHECKED;
			pCheckBox->dwFlags &= ~CKBF_CHECKFLAG;
		}
		
		if(oldFlag != pCheckBox->dwFlags)
		{
			pCheckBox->dwFlags |= UIF_REDRAW|CKBF_DRAWCHECK;
		}

		return 1;
	}

	return 0;
}

int UTFAPI UTFCheckBoxGetCheck(HUIWND hCheckBox)
{
	if(UTFWndIsType(hCheckBox, WTP_CHECKBOX) == TRUE)
	{
		LPUTFCHECKBOX pCheckBox = (LPUTFCHECKBOX)hCheckBox;
		int bCheck;
		
		if(pCheckBox->dwFlags & CKBF_CHECKED)
		{
			bCheck = 1;
		}
		else
		{
			bCheck = 0;
		}
		
		return bCheck;
	}

	return 0;
}

