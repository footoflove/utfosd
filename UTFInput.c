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
#include "UTFInput.h"
#include "UTFInputPriv.h"
#include "UTFDrawText.h"
#include "UTFMemory.h"

/******************************************************************/
#define IPT_PY_MODE 		0x0001
#define IPT_SYS_MODE 		0x0002
#define IPT_ENG_L_MODE 		0x0004
#define IPT_ENG_S_MODE 	0x0008
#define IPT_NUM_MODE 		0x0010


static int	InputInsertCharInText(LPUTFINPUT pInput,char cInputText[3])
{
	int i=0,j=0;
	while(pInput->textBuffer[i]!=0);
	
	for(j=0;j<3;j++)
	{	
		if(cInputText[j]!=0)
		{
			pInput->textBuffer[i]=cInputText[j];
			pInput->textBuffer[i+1]=0;
		}
		else
		{
			break;
		}
		i++;
		j++;
	}
	return 1;
}

static int InputNumicProc(HUIWND hInput,DWORD param2)
{
	LPUTFINPUT pInput = (LPUTFINPUT)hInput;
	char cInputTempText[3];
	if(pInput->bInputMode&IPT_NUM_MODE)
	{
		cInputTempText[0]=0x30+param2;
		cInputTempText[1]=0;
		InputInsertCharInText(pInput,cInputTempText);
		pInput->dwFlags |= UIF_REDRAW;
	}
	else if(pInput->bInputMode&IPT_SYS_MODE)
	{

	}
	else if(pInput->bInputMode&IPT_ENG_L_MODE)
	{

	}
	else if(pInput->bInputMode&IPT_ENG_S_MODE)
	{

	}
	else if(pInput->bInputMode&IPT_PY_MODE)
	{

	}
}

static int InputChangeMode(HUIWND hInput)
{
	LPUTFINPUT pInput = (LPUTFINPUT)hInput;
	if(pInput->bInputMode&IPT_NUM_MODE)
	{
		pInput->bInputMode=0;
		pInput->bInputMode|=IPT_SYS_MODE;
	}
	else if(pInput->bInputMode&IPT_SYS_MODE)
	{
		pInput->bInputMode=0;
		pInput->bInputMode|=IPT_ENG_L_MODE;
	}
	else if(pInput->bInputMode&IPT_ENG_L_MODE)
	{
		pInput->bInputMode=0;
		pInput->bInputMode|=IPT_ENG_S_MODE;
	}
	else if(pInput->bInputMode&IPT_ENG_S_MODE)
	{
		pInput->bInputMode=0;
		pInput->bInputMode|=IPT_PY_MODE;
	}
	else if(pInput->bInputMode&IPT_PY_MODE)
	{
		pInput->bInputMode=0;
		pInput->bInputMode|=IPT_NUM_MODE;
	}
	return 1;
}

static int UTFInputMsgProc(HUIWND hInput, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFINPUT pInput = (LPUTFINPUT)hInput;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)hInput->hParent;
	int retCode = 1;
	
	switch(uMsg)
	{
		case WMUI_KEYPRESS:
		{
			switch(param1)
			{
				case KEYUI_CHANINPUT:
					InputChangeMode(hInput);
					pInput->dwFlags |= UIF_REDRAW;
					break;
				case KEYUI_NUMERIC:
					InputNumicProc(hInput,param2);
					break;
				default:
					break;
			}
		}
			break;
		case WMUI_PUBLIC:
		{
			switch(param1)
			{
				case WMUI_CREATE:
					break;

				case WMUI_DESTROY:
					break;

				case WMUI_SETFOCUS:
					if((pInput->dwStyle & ( UIS_DISABLE | UIS_HIDE )) || !(pInput->dwStyle & UIS_TABSTOP))
					{
						retCode = 0;
					}
					else if( !(pInput->dwFlags & UIF_FOCUS) )
					{
						HUIWND hFocusCtrl = UTFDlgGetFocus(pInput->hParent);

						pInput->dwFlags |= UIF_FOCUS;
						pInput->dwFlags |= UIF_REDRAW;

						UTFSendPrivMessage(pInput->hParent, WMUI_INTER, WMUI_NEWFOCUS, hInput, hFocusCtrl);
					}
					break;

				default:
					retCode = 0;
					break;
			}		
		}
			break;
		case WMUI_INTER:
		{
			switch(param1)
			{
			case WMUI_PAINT:
				pInput->dwFlags |= UIF_REDRAW;
				break;

			case WMUI_KILLFOCUS:
				pInput->dwFlags &= ~UIF_FOCUS;
				pInput->dwFlags |= UIF_REDRAW;
				break;

			case WMUI_ENABLE:
				if(pInput->dwStyle & UIS_DISABLE)
				{
					pInput->dwStyle &= ~UIS_DISABLE;
					pInput->dwFlags |= UIF_REDRAW;

					if(UTFDialogHaveFocus(pInput->hParent) == FALSE)
					{
						UTFSendPrivMessage(hInput, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
					}
				}
				break;

			case WMUI_DISABLE:
				if((pInput->dwStyle & UIS_DISABLE) != UIS_DISABLE)
				{
					pInput->dwStyle |= UIS_DISABLE;
					pInput->dwFlags |= UIF_REDRAW;

					if(pInput->dwFlags & UIF_FOCUS)
					{
						UTFSendPrivMessage(pInput->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
						pInput->dwFlags &= ~UIF_FOCUS;
					}
				}
				break;

			case WMUI_SHOW:
				if(pInput->dwStyle & UIS_HIDE)
				{
					pInput->dwStyle &= ~UIS_HIDE;
					pInput->dwFlags |= UIF_REDRAW;
					
					UTFDialogDrawAllChildAboveCtrl(pInput->hParent, hInput);
					
					if(UTFDialogHaveFocus(pInput->hParent) == FALSE)
					{
						UTFSendPrivMessage(hInput, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
					}
				}
				break;

			case WMUI_HIDE:
				if((pInput->dwStyle & UIS_HIDE) != UIS_HIDE)
				{
					pInput->dwStyle |= UIS_HIDE;
					pInput->dwFlags |= UIF_REDRAW;
					pInput->dwFlags |= DLGF_CLEANBACK;
					
					if(pInput->dwFlags & UIF_FOCUS)
					{
						UTFSendPrivMessage(pInput->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
						pInput->dwFlags &= ~UIF_FOCUS;
					}
					
					UTFDialogRedrawCtrlInRect(pInput->hParent, &pInput->rect);
				}
				break;

			case WMUI_SETRECT:
				if(param2)
				{
					LPUTFRECT lpRect = (LPUTFRECT)param2;
					UTFRECT oldRect = pInput->rect;

					if((lpRect->right > (pDlg->rect.right-pDlg->rect.left)) || (lpRect->bottom > (pDlg->rect.bottom-pDlg->rect.top)))
						return 0;

					pInput->dwFlags |= UIF_REDRAW;
					
					UTFDialogBackClean(pInput->hParent, hInput);
					UTFDialogRedrawCtrlInRect(pInput->hParent, &pInput->rect);
					
					memcpy(&pInput->rect, lpRect, sizeof(UTFRECT));
					UTFDialogDrawAllChildAboveCtrl(pInput->hParent, hInput);

					if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
					{
						UTFDialogResetCtrlXYOrder(pInput->hParent, hInput, oldRect);
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
							strcpy((char *)pInput->wndText, (char *)param2);
						}
						else
						{
							memcpy((char *)pInput->wndText, (char *)param2, TEXT_SIZE);
							pInput->wndText[TEXT_SIZE-1] = 0;
						}
						pInput->textID = -1;	
						pInput->dwFlags |= UIF_REDRAW;
					}
					else if(pInput->textID != param3)
					{
						pInput->textID = param3;
						pInput->dwFlags |= UIF_REDRAW;
					}
				}
				break;

			case WMUI_GETTEXT:
				if((param2 != NULL) && (param3 > 0))
				{
					if(pInput->textID >= 0)
					{
						UTFGetIDText(pInput->textID, pInput->wndText, TEXT_SIZE);
					}
					if(strlen((char *)pInput->wndText) < param3)
					{
						strcpy((char *)param2, (char *)pInput->wndText);
					}
					else
					{
						char *pBuffer = (char *)param2;
						
						memcpy(pBuffer, (char *)pInput->wndText, param3);
						*(pBuffer+param3-1) = 0;
					}
				}
				break;	

			case WMUI_DELETEWND:
				if(param2 == 0)
				{
					LPUTFDIALOGCTRL pCtrl;			

					if(pInput->hPrevCtrl)
					{
						pCtrl = (LPUTFDIALOGCTRL)pInput->hPrevCtrl;
						pCtrl->hNextCtrl = pInput->hNextCtrl;
					}
					else
					{
						pDlg->pCtrl = (LPUTFDIALOGCTRL)pInput->hNextCtrl;
					}

					if(pInput->hNextCtrl)
					{
						pCtrl = (LPUTFDIALOGCTRL)pInput->hNextCtrl;
						pCtrl->hPrevCtrl = pInput->hPrevCtrl;
					}			
				}
				UTFFree(hInput);
				break;

			default:
				retCode = 0;
				break;
			}		
		}
			break;
		default:
			break;

	}

}

static void UTFDrawInputFace(HUIWND hInput, UTFRECT rcRect, DWORD info)
{
	LPUTFINPUT pInput = (LPUTFINPUT)hInput;
	UTFRECT wndRect = rcRect;
	DWORD uFormat;
	WORD wTextWidth=0;

	/* Draw background */
	UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
	if(pInput->dwStyle & UIS_DISABLE)
	{
		UTFSetBrush(BSUI_SOLID, UTFRGB(150,150,150,255), HSUI_HORIZONTAL);
		UTFSetTextColor(UTFRGB(0,0,0,255));
	}
	else if(pInput->dwFlags & UIF_FOCUS)
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
	
	if(pInput->textID >= 0)
	{
		UTFGetIDText(pInput->textID, pInput->wndText, TEXT_SIZE);
	}

	uFormat = DTUI_CENTER|DTUI_VCENTER|DTUI_SINGLELINE|DTUI_END_ELLIPSIS;
	
	wTextWidth=UTFGetCharWidth(pInput->wndText);
	wndRect.right=wndRect.left+wTextWidth;
	UTFDrawText(pInput->wndText, &wndRect, uFormat);
	if(0!=pInput->textBuffer[0])
	{
		wndRect.left=wndRect.right;
		wndRect.right=rcRect.right;
		UTFDrawText(pInput->textBuffer, &wndRect, uFormat);
	}
}


void UTFInputInitialize(void)
{
	UTFRegisterWndType(WTP_INPUT, UTFInputMsgProc, UTFDrawInputFace);
}

HUIWND UTFAPI UTFCreateInput(DWORD ctrlID, int textID, char *text, DWORD dwStyle, DWORD dwExStyle,
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
		HUIWND hCtrl = (HUIWND)UTFMalloc(sizeof(UTFINPUT));
		
		if(hCtrl != NULL)
		{
			LPUTFINPUT pCtrl = (LPUTFINPUT)hCtrl;

			memcpy(pCtrl->type, WTP_INPUT, 4);
			pCtrl->wndID	= ctrlID;
			
			pCtrl->dwStyle	= dwStyle | UIS_CHILD | UIS_OWNERDRAW;
			pCtrl->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST | UIS_NOFOCUS);
			pCtrl->dwExStyle = dwExStyle;
			pCtrl->dwFlags	= 0;
			
			pCtrl->hParent	= hDlg;
			pCtrl->dataEx	= ctrlData;
			pCtrl->rect 	= rcRect;
			
			pCtrl->CallBack	= CallBack;
			if(dwStyle & UIS_OWNERDRAW)
			{
				pCtrl->OnDraw = OnDraw;
			}
			else
			{
				pCtrl->OnDraw = NULL;
			}


			pCtrl->ptextcharbuf= NULL;
			pCtrl->bMaxSize = 0;
			pCtrl->bCaretPos = 0;
			pCtrl->bInputing=0;
			pCtrl->bInputMode=IPT_NUM_MODE;
			memset(pCtrl->textBuffer, 0, INPUTBUFLEN);

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
