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
#include "UTFEdit.h"
#include "UTFEditPriv.h"
#include "UTFMemory.h"

/******************************************************************/
static void UTFDrawEditFace(HUIWND hEdit, UTFRECT rcRect, DWORD info)
{
	LPUTFEDIT pCtrl = (LPUTFEDIT)hEdit;
	UTFRECT wndRect = rcRect;
	UTFCOLOR color;

	/* Draw background */	
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

	if(pCtrl->dwFlags & ETF_DRAWEDIT)
	{
		UTFFillRect(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
		
		if(pCtrl->bMaxSize > 0)
		{
			if(pCtrl->dwFlags & ETF_EDITING)
			{
				BYTE text[EDITBUFLEN+1];

				if(pCtrl->dwExStyle & ETS_EXTEND_INPUT)
				{
					UTFRECT rcCurChar;
					BYTE buf[20] = {0};

					UTFDeflateRect(&wndRect, 5, 0, 5, 0);

					rcCurChar = wndRect;
					memcpy((char *)buf, (char *)pCtrl->textBuffer, pCtrl->bCaretPos);
					rcCurChar.left += UTFGetTextWidth(buf);

					buf[0] = pCtrl->textBuffer[pCtrl->bCaretPos];
					buf[1] = 0;
					rcCurChar.right = rcCurChar.left+UTFGetTextWidth(buf);

					UTFSetBrush(BSUI_SOLID, UTFRGB(200,200,200,255), 0);
					UTFFillRect(rcCurChar.left, rcCurChar.top, rcCurChar.right, rcCurChar.bottom);

					UTFDrawText(pCtrl->textBuffer, &wndRect, DTUI_SINGLELINE|DTUI_LEFT|DTUI_VCENTER);
				}
				else if(pCtrl->dwExStyle & ETS_NUMBER)
				{
					strcpy((char *)text, (char *)pCtrl->textBuffer);
					strcat((char *)text, "_");
					UTFDeflateRect(&wndRect, 5, 0, 5, 0);
					UTFDrawText(text, &wndRect, DTUI_SINGLELINE|DTUI_LEFT|DTUI_VCENTER);
				}
				else
				{
					UTFDeflateRect(&wndRect, 5, 0, 5, 0);
					UTFDrawText(pCtrl->textBuffer, &wndRect, DTUI_SINGLELINE|DTUI_LEFT|DTUI_VCENTER);
				}
			}
			else
			{
				UTFDrawText(pCtrl->textBuffer, &wndRect, DTUI_SINGLELINE|DTUI_CENTER|DTUI_VCENTER);
			}
		}
	}
}

static void UTFEditInvalidate(HUIWND hEdit, UTFRECT rcRect, DWORD info)
{
	LPUTFEDIT pCtrl = (LPUTFEDIT)hEdit;

	if(pCtrl->OnDraw)
	{
		pCtrl->OnDraw(hEdit, rcRect, 0);
	}
	else
	{
		UTFDrawEditFace(hEdit, rcRect, 0);
	}	

	pCtrl->dwFlags &= ~(ETF_DRAWEDIT|ETF_DRAWFRAME);
}

static char UTFEditInputJudge(HUIWND hEdit, DWORD dwNumber)
{
	LPUTFEDIT pCtrl = (LPUTFEDIT)hEdit;
	char bEnable = TRUE;
	int i;

	if(pCtrl->dwExStyle & ETS_TIME)
	{
		char bFound = FALSE;
		char bHaveNumeric = FALSE;
		char bFirstChar = TRUE;
		BYTE TopData[3] = {2, 3, 5};

		if(pCtrl->dwExStyle & ETS_TIME_CLOCK_12)
		{
			TopData[0] = 1;
			TopData[1] = 1;
			TopData[2] = 5;
		}

		for(i=0; i<pCtrl->bCaretPos; i++)
		{
			if((pCtrl->textBuffer[i] >= '0') && (pCtrl->textBuffer[i] <= '9'))
			{
				bFirstChar = FALSE;
				break;
			}
		}

		if(bFirstChar == TRUE)
		{
			if(dwNumber > TopData[0])
			{
				return FALSE;
			}
		}
		
		for(i=pCtrl->bCaretPos+1; i<pCtrl->bMaxSize; i++)
		{
			if(pCtrl->textBuffer[i] == ':')
			{
				if(pCtrl->textBuffer[pCtrl->bCaretPos-1] == ('0'+TopData[0]))
				{
					if(dwNumber > TopData[1])
					{
						bEnable = FALSE;
					}
				}
				bFound = TRUE;
				break;
			}
			else if((pCtrl->textBuffer[i] >= '0') && (pCtrl->textBuffer[i] <= '9'))
			{
				bHaveNumeric = TRUE;
				break;
			}
		}
		
		if((bFound == FALSE) && (bHaveNumeric == TRUE))
		{
			if(dwNumber > TopData[2])
			{
				bEnable = FALSE;
			}
		}
	}

	return bEnable;
}

static int UTFEditInputNumeric(HUIWND hEdit, DWORD dwNumber)
{
	LPUTFEDIT pCtrl = (LPUTFEDIT)hEdit;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pCtrl->hParent;
	int retCode = 1;

	if(pCtrl->bMaxSize)
	{
		pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
		if(pCtrl->dwFlags & ETF_EDITING)
		{
			if(pCtrl->dwExStyle & ETS_PASSWORD)
			{
				if(pCtrl->bCaretPos < pCtrl->bMaxSize)
				{
					pCtrl->textOldBuffer[pCtrl->bCaretPos] = '0'+dwNumber;
					pCtrl->textOldBuffer[pCtrl->bCaretPos+1] = 0;
					pCtrl->textBuffer[pCtrl->bCaretPos] = '*';
					pCtrl->textBuffer[pCtrl->bCaretPos+1] = 0;
				}

				if(++pCtrl->bCaretPos >= pCtrl->bMaxSize)
				{
					pCtrl->bCaretPos = 0;
					pCtrl->dwFlags &= ~(ETF_EDITING|ETF_NOMOVE);
					if(pDlg->CallBack)
					{
						pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_FINISH, pCtrl->wndID, 0);
					}
				}
			}
			else if(pCtrl->dwExStyle & ETS_NUMERIC_ONLY)
			{
				if(pCtrl->bCaretPos < pCtrl->bMaxSize)
				{
					pCtrl->textBuffer[pCtrl->bCaretPos] = '0'+dwNumber;
					pCtrl->textBuffer[pCtrl->bCaretPos+1] = 0;
				}

				if(++pCtrl->bCaretPos >= pCtrl->bMaxSize)
				{
					pCtrl->bCaretPos = 0;
					pCtrl->dwFlags &= ~ETF_EDITING;
					if(pDlg->CallBack)
					{
						pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_FINISH, pCtrl->wndID, 0);
					}
				}
				else
				{
					if(pDlg->CallBack)
					{
						pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_CHANGE, pCtrl->wndID, 0);
					}
				}
			}
			else
			{
				int i,pos = 0;
				char bEnable;

				bEnable = UTFEditInputJudge(hEdit, dwNumber);
				if(bEnable == TRUE)
				{
					pCtrl->textBuffer[pCtrl->bCaretPos] = '0'+dwNumber;
					for(i=pCtrl->bCaretPos+1; i<pCtrl->bMaxSize; i++)
					{
						if((pCtrl->textBuffer[i] >= '0') && (pCtrl->textBuffer[i] <= '9'))
						{
							pCtrl->bCaretPos = i;
							pos = 1;
							break;
						}
					}
					
					if(pos == 0)
					{
						pCtrl->bCaretPos = 0;
						pCtrl->dwFlags &= ~(ETF_EDITING|ETF_NOMOVE);
						if(pDlg->CallBack)
						{
							pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_FINISH, pCtrl->wndID, 0);
						}
					}
					else
					{
						if(pDlg->CallBack)
						{
							pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_CHANGE, pCtrl->wndID, 0);
						}
					}
				}
				else
				{
					pCtrl->dwFlags &= ~UIF_REDRAW;
				}
			}
		}
		else
		{
			if(pCtrl->dwExStyle & ETS_PASSWORD)
			{
				memset(pCtrl->textBuffer, 0, EDITBUFLEN);
				memset(pCtrl->textOldBuffer, 0, EDITBUFLEN);
				pCtrl->textBuffer[0] = '*';
				pCtrl->textBuffer[1] = 0;
				pCtrl->textOldBuffer[0] = '0'+dwNumber;
				pCtrl->textOldBuffer[1] = 0;
				if(pCtrl->bMaxSize > 1)
				{
					pCtrl->bCaretPos = 1;
					pCtrl->dwFlags |= ETF_NOMOVE|ETF_EDITING;
				}
				else
				{
					pCtrl->bCaretPos = 0;
					pCtrl->dwFlags &= ~(ETF_EDITING|ETF_NOMOVE);
					if(pDlg->CallBack)
					{
						pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_FINISH, pCtrl->wndID, 0);
					}
				}
			}
			else if(pCtrl->dwExStyle & ETS_NUMERIC_ONLY)
			{
				memcpy(pCtrl->textOldBuffer, pCtrl->textBuffer, EDITBUFLEN);
				memset(pCtrl->textBuffer, 0, EDITBUFLEN);
				pCtrl->textBuffer[0] = '0'+dwNumber;
				pCtrl->textBuffer[1] = 0;
				if(pCtrl->bMaxSize > 1)
				{
					pCtrl->bCaretPos = 1;
					pCtrl->dwFlags |= ETF_EDITING;
					if(pDlg->CallBack)
					{
						pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_CHANGE, pCtrl->wndID, 0);
					}
				}
				else
				{
					pCtrl->bCaretPos = 0;
					if(pDlg->CallBack)
					{
						pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_FINISH, pCtrl->wndID, 0);
					}
				}
				pCtrl->dwFlags &= ~ETF_NOMOVE;
			}
			else if(pCtrl->dwExStyle & ETS_NUMERIC_HAVE)
			{
				int i,pos = 0;
				char bEnable;

				bEnable = UTFEditInputJudge(hEdit, dwNumber);
				if(bEnable == TRUE)
				{
					memcpy(pCtrl->textOldBuffer, pCtrl->textBuffer, EDITBUFLEN);
					for(i=0; i<pCtrl->bMaxSize; i++)
					{
						if((pCtrl->textBuffer[i] >= '0') && (pCtrl->textBuffer[i] <= '9'))
						{
							if(pos == 0)
							{
								pCtrl->textBuffer[i] = '0'+dwNumber;
								pos = 1;
							}
							else
							{
								pos = 2;
								pCtrl->bCaretPos = i;
								break;
							}
						}
					}
					
					if(pos == 2)
					{
						pCtrl->dwFlags |= ETF_EDITING;
						if(pDlg->CallBack)
						{
							pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_CHANGE, pCtrl->wndID, 0);
						}
					}
					else
					{
						pCtrl->bCaretPos = 0;
						pCtrl->dwFlags &= ~(ETF_EDITING|ETF_NOMOVE);
						if(pDlg->CallBack)
						{
							pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_FINISH, pCtrl->wndID, 0);
						}
					}
				}
				else
				{
					pCtrl->dwFlags &= ~UIF_REDRAW;
				}
			}
			else
			{
				pCtrl->dwFlags &= ~UIF_REDRAW;
			}
		}
	}
	else
	{
		retCode = 0;
	}

	return retCode;
}

static int UTFEditMsgProc(HUIWND hEdit, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFEDIT pCtrl = (LPUTFEDIT)hEdit;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pCtrl->hParent;
	int retCode = 1;

	switch(uMsg)
	{
	case WMUI_PUBLIC:
		switch(param1)
		{
		case WMUI_CREATE:
			pCtrl->dwFlags &= ~(ETF_EDITING|ETF_NOMOVE);
			break;

		case WMUI_DESTROY:
			break;

		case WMUI_SETFOCUS:
			if((pCtrl->dwStyle & ( UIS_DISABLE | UIS_HIDE )) || !(pCtrl->dwStyle & UIS_TABSTOP))
			{
				retCode = 0;
			}
			else if( !(pCtrl->dwFlags & UIF_FOCUS) )
			{
				HUIWND hFocusCtrl = UTFDlgGetFocus(pCtrl->hParent);

				pCtrl->dwFlags |= UIF_FOCUS;
				pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT|ETF_DRAWFRAME;
				pCtrl->dwFlags &= ~(ETF_EDITING|ETF_NOMOVE);

				UTFSendPrivMessage(pCtrl->hParent, WMUI_INTER, WMUI_NEWFOCUS, hEdit, hFocusCtrl);
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
			pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT|ETF_DRAWFRAME;
			break;

		case WMUI_KILLFOCUS:
			pCtrl->dwFlags &= ~UIF_FOCUS;
			pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT|ETF_DRAWFRAME;
			if(pCtrl->dwFlags & ETF_EDITING)
			{
				pCtrl->dwFlags &= ~(ETF_EDITING|ETF_NOMOVE);
				if(pCtrl->dwExStyle & ETS_PASSWORD)
				{
					memset((char *)pCtrl->textBuffer, '*', pCtrl->bMaxSize);
					memset((char *)pCtrl->textOldBuffer, 0, pCtrl->bMaxSize+1);
					pCtrl->textBuffer[pCtrl->bMaxSize] = 0;
				}
				else
				{
					if(pCtrl->dwExStyle & ETS_NUMERIC_ONLY)
					{
						if(pCtrl->bCaretPos == 0)
						{
							memcpy(pCtrl->textBuffer, pCtrl->textOldBuffer, EDITBUFLEN);
						}
						if(pDlg->CallBack)
						{
							pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_FINISH, pCtrl->wndID, 0);
						}
					}
					else
					{
						memcpy(pCtrl->textBuffer, pCtrl->textOldBuffer, EDITBUFLEN);
					}
				}
				pCtrl->bCaretPos = 0;
			}
			break;

		case WMUI_ENABLE:
			if(pCtrl->dwStyle & UIS_DISABLE)
			{
				pCtrl->dwStyle &= ~UIS_DISABLE;
				pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT|ETF_DRAWFRAME;

				if(UTFDialogHaveFocus(pCtrl->hParent) == FALSE)
				{
					UTFSendPrivMessage(hEdit, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}
			}
			break;

		case WMUI_DISABLE:
			if((pCtrl->dwStyle & UIS_DISABLE) != UIS_DISABLE)
			{
				pCtrl->dwStyle |= UIS_DISABLE;
				pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT|ETF_DRAWFRAME;

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
				pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT|ETF_DRAWFRAME;

				UTFDialogDrawAllChildAboveCtrl(pCtrl->hParent, hEdit);

				if(UTFDialogHaveFocus(pCtrl->hParent) == FALSE)
				{
					UTFSendPrivMessage(hEdit, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}
			}
			break;

		case WMUI_HIDE:
			if((pCtrl->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pCtrl->dwStyle |= UIS_HIDE;
				pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT|ETF_DRAWFRAME;
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

				pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT|ETF_DRAWFRAME;
				
				UTFDialogBackClean(pCtrl->hParent, hEdit);
				UTFDialogRedrawCtrlInRect(pCtrl->hParent, &pCtrl->rect);
				
				memcpy(&pCtrl->rect, lpRect, sizeof(UTFRECT));
				UTFDialogDrawAllChildAboveCtrl(pCtrl->hParent, hEdit);

				if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
				{
					UTFDialogResetCtrlXYOrder(pCtrl->hParent, hEdit, oldRect);
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
					pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWFRAME;
				}
				else if(pCtrl->textID != param3)
				{
					pCtrl->textID = param3;
					pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWFRAME;
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
			UTFFree(hEdit);
			break;

		default:
			retCode = 0;
			break;
		}
		break;

	case WMUI_KEYPRESS:
		switch(param1)
		{
		case KEYUI_NUMERIC:
			if(pCtrl->dwExStyle & ETS_EXTEND_INPUT)
			{
				if(pCtrl->dwExStyle & ETS_PASSWORD)
				{
					retCode = UTFEditInputNumeric(hEdit, param2);
				}
				else
				{
					BYTE i,bValue;
					char bEnableInput;

					bEnableInput = UTFEditInputJudge(hEdit, param2);
					if(bEnableInput == FALSE)
					{
						retCode = 0;
					}
					else
					{
						if(pCtrl->dwFlags & ETF_EDITING)
						{
						}
						else
						{
							memcpy(pCtrl->textOldBuffer, pCtrl->textBuffer, EDITBUFLEN);
							
							pCtrl->bCaretPos = 0;
							for(i=0; i<pCtrl->bMaxSize; i++)
							{
								if((pCtrl->textBuffer[i] >= '0') && (pCtrl->textBuffer[i] <= '9'))
								{
									pCtrl->bCaretPos = i;
									break;
								}
							}
						}

						bValue = pCtrl->textBuffer[pCtrl->bCaretPos];
						if((bValue < '0') || (bValue > '9'))
						{
							retCode = 0;
						}
						else
						{
							BYTE bNewPos = pCtrl->bCaretPos;
							
							pCtrl->dwFlags |= ETF_EDITING;
							pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;						
							
							bValue = '0'+param2;
							pCtrl->textBuffer[pCtrl->bCaretPos] = bValue;

							for(i=0; i<pCtrl->bMaxSize; i++)
							{
								if(++bNewPos >= pCtrl->bMaxSize)
								{
									bNewPos = 0;
								}

								if(!((pCtrl->textBuffer[bNewPos] < '0') || (pCtrl->textBuffer[bNewPos] > '9')))
								{
									break;
								}
							}

							if(bNewPos != pCtrl->bCaretPos)
							{
								pCtrl->bCaretPos = bNewPos;
							}

							if(pDlg->CallBack)
							{
								pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_CHANGE, pCtrl->wndID, 0);
							}
						}
					}
				}
			}
			else
			{
				retCode = UTFEditInputNumeric(hEdit, param2);
			}
			break;

		case KEYUI_UP:
			if(pCtrl->dwExStyle & ETS_EXTEND_INPUT)
			{
				if(pCtrl->dwExStyle & ETS_PASSWORD)
				{
					retCode = UTFEditInputNumeric(hEdit, 0);
				}
				else if(pCtrl->dwFlags & ETF_EDITING)
				{
					BYTE bValue = pCtrl->textBuffer[pCtrl->bCaretPos];

					if((bValue < '0') || (bValue > '9'))
					{
						retCode = 0;
					}
					else
					{
						int i;
						
						for(i=0; i<10; i++)
						{
							if(++bValue > '9')
							{
								bValue = '0';
							}
							if(UTFEditInputJudge(hEdit, bValue-'0') == TRUE)
								break;
						}

						pCtrl->textBuffer[pCtrl->bCaretPos] = bValue;
						pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;

						if(pDlg->CallBack)
						{
							pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_CHANGE, pCtrl->wndID, 0);
						}
					}
				}
				else
				{
					retCode = 0;
				}
			}
			else
			{
				if(pCtrl->dwFlags & ETF_NOMOVE)
				{
				}
				else
				{
					retCode = 0;
				}
			}
			break;
			
		case KEYUI_DOWN:
			if(pCtrl->dwExStyle & ETS_EXTEND_INPUT)
			{
				if(pCtrl->dwExStyle & ETS_PASSWORD)
				{
					retCode = UTFEditInputNumeric(hEdit, 1);
				}
				else if(pCtrl->dwFlags & ETF_EDITING)
				{
					BYTE bValue = pCtrl->textBuffer[pCtrl->bCaretPos];

					if((bValue < '0') || (bValue > '9'))
					{
						retCode = 0;
					}
					else
					{
						int i;
						
						for(i=0; i<10; i++)
						{
							if(--bValue < '0')
							{
								bValue = '9';
							}
							if(UTFEditInputJudge(hEdit, bValue-'0') == TRUE)
								break;
						}
						
						pCtrl->textBuffer[pCtrl->bCaretPos] = bValue;
						pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;

						if(pDlg->CallBack)
						{
							pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_CHANGE, pCtrl->wndID, 0);
						}
					}
				}
				else
				{
					retCode = 0;
				}
			}
			else
			{
				if(pCtrl->dwFlags & ETF_NOMOVE)
				{
				}
				else
				{
					retCode = 0;
				}
			}
			break;

		case KEYUI_RIGHT:
			if((pCtrl->dwExStyle & ETS_TIME) && (pCtrl->dwExStyle & ETS_TIME_CLOCK_12))
			{
				if(pCtrl->dwFlags & ETF_TIME_PM)
				{
					pCtrl->dwFlags &= ~ETF_TIME_PM;
				}
				else
				{
					pCtrl->dwFlags |= ETF_TIME_PM;
				}
				pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
			}
			else if(pCtrl->dwExStyle & ETS_EXTEND_INPUT)
			{
				if(pCtrl->dwExStyle & ETS_PASSWORD)
				{
					retCode = UTFEditInputNumeric(hEdit, 3);
				}
				else if(pCtrl->dwFlags & ETF_EDITING)
				{
					BYTE bNewPos = pCtrl->bCaretPos;
					BYTE i;

					for(i=0; i<pCtrl->bMaxSize; i++)
					{
						if(++bNewPos >= pCtrl->bMaxSize)
						{
							bNewPos = 0;
						}

						if(!((pCtrl->textBuffer[bNewPos] < '0') || (pCtrl->textBuffer[bNewPos] > '9')))
						{
							break;
						}
					}

					if(bNewPos != pCtrl->bCaretPos)
					{
						pCtrl->bCaretPos = bNewPos;
						pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
					}
				}
				else
				{
					retCode = 0;
				}
			}
			else
			{
				if(pCtrl->dwFlags & ETF_EDITING)
				{
				}
				else if((pCtrl->wMaxSel > 1) && (pCtrl->lpSelInfo))
				{
					if(++pCtrl->wCurSel >= pCtrl->wMaxSel)
					{
						pCtrl->wCurSel = 0;
					}
					UTFEditSetData(hEdit, pCtrl->bMaxSize, pCtrl->lpSelInfo[pCtrl->wCurSel].text);
					
					pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
					if(pDlg->CallBack)
					{
						pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_SELCHANGE, pCtrl->wndID, pCtrl->wCurSel);
					}
				}
				else
				{
					retCode = 0;
				}
			}
			break;

		case KEYUI_LEFT:
			if((pCtrl->dwExStyle & ETS_TIME) && (pCtrl->dwExStyle & ETS_TIME_CLOCK_12))
			{
				if(pCtrl->dwFlags & ETF_TIME_PM)
				{
					pCtrl->dwFlags &= ~ETF_TIME_PM;
				}
				else
				{
					pCtrl->dwFlags |= ETF_TIME_PM;
				}
				pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
			}
			else if(pCtrl->dwExStyle & ETS_EXTEND_INPUT)
			{
				if(pCtrl->dwExStyle & ETS_PASSWORD)
				{
					retCode = UTFEditInputNumeric(hEdit, 2);
				}
				else if(pCtrl->dwFlags & ETF_EDITING)
				{
					BYTE bNewPos = pCtrl->bCaretPos;
					BYTE i;

					for(i=0; i<pCtrl->bMaxSize; i++)
					{
						if(bNewPos)
						{
							bNewPos--;
						}
						else
						{
							bNewPos = pCtrl->bMaxSize-1;
						}

						if(!((pCtrl->textBuffer[bNewPos] < '0') || (pCtrl->textBuffer[bNewPos] > '9')))
						{
							break;
						}
					}

					if(bNewPos != pCtrl->bCaretPos)
					{
						pCtrl->bCaretPos = bNewPos;
						pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
					}
				}
				else
				{
					retCode = 0;
				}
			}
			else
			{
				if(pCtrl->dwFlags & ETF_EDITING)
				{
					if(pCtrl->dwFlags & ETF_NOMOVE)
					{
					}
					else if(pCtrl->dwExStyle & ETS_NUMERIC_ONLY)
					{
						if(pCtrl->bCaretPos)
						{
							pCtrl->bCaretPos--;
							pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
							pCtrl->textBuffer[pCtrl->bCaretPos] = 0;
							if(pCtrl->bCaretPos)
							{
								if(pDlg->CallBack)
								{
									pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_CHANGE, pCtrl->wndID, 0);
								}
							}
						}
					}
				}
				else if((pCtrl->wMaxSel > 1) && (pCtrl->lpSelInfo))
				{
					if(pCtrl->wCurSel > 0)
						pCtrl->wCurSel--;
					else
						pCtrl->wCurSel = pCtrl->wMaxSel-1;
					UTFEditSetData(hEdit, pCtrl->bMaxSize, pCtrl->lpSelInfo[pCtrl->wCurSel].text);
					
					pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
					if(pDlg->CallBack)
					{
						pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_SELCHANGE, pCtrl->wndID, pCtrl->wCurSel);
					}				
				}
				else
				{
					retCode = 0;
				}
			}
			break;

		case KEYUI_EXIT:
		case KEYUI_MENU:
			if(pCtrl->dwFlags & ETF_EDITING)
			{
				if(pCtrl->dwFlags & ETF_NOMOVE)
				{
				}
				else
				{
					memcpy(pCtrl->textBuffer, pCtrl->textOldBuffer, EDITBUFLEN);
					pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
					pCtrl->bCaretPos = 0;
					pCtrl->dwFlags &= ~ETF_EDITING;
					if(pDlg->CallBack)
					{
						pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_FINISH, pCtrl->wndID, 0);
					}
				}
			}
			else
			{
				retCode = 0;
			}
			break;

		case KEYUI_SELECT:
			if(pCtrl->dwExStyle & ETS_EXTEND_INPUT)
			{
				if(pCtrl->dwExStyle & ETS_PASSWORD)
				{
					retCode = 0;
				}
				else
				{
					if(pCtrl->dwFlags & ETF_EDITING)
					{
						pCtrl->dwFlags &= ~ETF_EDITING;
						pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
						pCtrl->bCaretPos = 0;

						if(pDlg->CallBack)
						{
							pDlg->CallBack(pCtrl->hParent, WMUI_NOTIFY, ENUI_FINISH, pCtrl->wndID, 0);
						}
					}
					else if(pCtrl->dwExStyle & ETS_NUMERIC_HAVE)
					{
						BYTE i;
						
						pCtrl->dwFlags |= ETF_EDITING;
						pCtrl->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
						
						memcpy(pCtrl->textOldBuffer, pCtrl->textBuffer, EDITBUFLEN);

						pCtrl->bCaretPos = 0;
						for(i=0; i<pCtrl->bMaxSize; i++)
						{
							if(!((pCtrl->textBuffer[i] < '0') || (pCtrl->textBuffer[i] > '9')))
							{
								pCtrl->bCaretPos = i;
								break;
							}
						}
					}
				}
			}
			else
			{
				retCode = 0;
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

void UTFEditInitialize(void)
{
	UTFRegisterWndType(WTP_EDIT, UTFEditMsgProc, NULL);
}

HUIWND UTFAPI UTFCreateEdit(DWORD ctrlID, int textID, char *text, DWORD dwStyle, DWORD dwExStyle,
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
		HUIWND hCtrl = (HUIWND)UTFMalloc(sizeof(UTFEDIT));
		
		if(hCtrl != NULL)
		{
			LPUTFEDIT pCtrl = (LPUTFEDIT)hCtrl;

			memcpy(pCtrl->type, WTP_EDIT, 4);
			pCtrl->wndID	= ctrlID;
			
			pCtrl->dwStyle	= dwStyle | UIS_CHILD | UIS_OWNERDRAW;
			pCtrl->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST | UIS_NOFOCUS);
			pCtrl->dwExStyle = dwExStyle;
			pCtrl->dwExStyle &= ~ETS_NUMBER;
			pCtrl->dwFlags	= 0;
			
			pCtrl->hParent	= hDlg;
			pCtrl->dataEx	= ctrlData;
			pCtrl->rect 	= rcRect;
			
			pCtrl->DrawFace = UTFEditInvalidate;
			pCtrl->CallBack	= CallBack;
			if(dwStyle & UIS_OWNERDRAW)
			{
				pCtrl->OnDraw = OnDraw;
			}
			else
			{
				pCtrl->OnDraw = NULL;
			}

			pCtrl->wCurSel = 0;
			pCtrl->wMaxSel = 0;
			pCtrl->lpSelInfo = NULL;
			pCtrl->bMaxSize = 0;
			pCtrl->bCaretPos = 0;
			memset(pCtrl->textBuffer, 0, EDITBUFLEN);
			memset(pCtrl->textOldBuffer, 0, EDITBUFLEN);

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

int UTFAPI UTFEditSetData(HUIWND hEdit, WORD bMaxPos, char *lpTextInfo)
{
	LPUTFEDIT pEdit = (LPUTFEDIT)hEdit;
	BYTE textLen;

	if(UTFWndIsType(hEdit, WTP_EDIT) == FALSE)
		return 0;

	textLen = strlen(lpTextInfo);
	if((bMaxPos == 0) || (lpTextInfo == NULL) || (bMaxPos >= EDITBUFLEN))
		return 0;

	if(pEdit->dwFlags & ETF_EDITING)
		return 0;

	pEdit->bMaxSize = bMaxPos;
	pEdit->bCaretPos = 0;
	pEdit->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
	pEdit->dwFlags &= ~ETF_NOMOVE;

	if(pEdit->dwExStyle & ETS_PASSWORD)
	{
		memset((char *)pEdit->textBuffer, '*', pEdit->bMaxSize);
		memset((char *)pEdit->textOldBuffer, 0, pEdit->bMaxSize+1);
		pEdit->textBuffer[pEdit->bMaxSize] = 0;
	}
	else
	{
		BYTE bOnlyNumeric = TRUE;
		BYTE iNumericNum = 0;
		BYTE count=0;
		WORD i;

		strcpy((char *)pEdit->textBuffer, lpTextInfo);
		strcpy((char *)pEdit->textOldBuffer, lpTextInfo);
		for(i=0; i<pEdit->bMaxSize; i++)
		{
			if(pEdit->textBuffer[i] == 0)
				break;
			
			if((pEdit->textBuffer[i] < '0') || (pEdit->textBuffer[i] > '9'))
			{
				bOnlyNumeric = FALSE;
			}
			else
			{
				iNumericNum++;
			}
			count++;
		}

		if(count)
		{
			pEdit->dwExStyle &= ~(ETS_NUMERIC_HAVE|ETS_NUMERIC_ONLY|ETS_NUMBER);
			if(iNumericNum)
			{
				pEdit->dwExStyle |= ETS_NUMERIC_HAVE;
			}
			if(bOnlyNumeric == TRUE)
			{
				pEdit->dwExStyle |= ETS_NUMERIC_ONLY|ETS_NUMBER;
				pEdit->textBuffer[pEdit->bMaxSize] = 0;
				pEdit->textOldBuffer[pEdit->bMaxSize] = 0;
			}
			else
			{
				pEdit->bMaxSize = textLen;
			}
		}

		if(pEdit->dwExStyle & ETS_TIME)
		{
			BYTE bPre = TRUE;
			BYTE PreNum = 0;
			BYTE PostNum = 0;
			
			for(i=0; i<pEdit->bMaxSize; i++)
			{
				if(pEdit->textBuffer[i] == 0)
					break;
				
				if((pEdit->textBuffer[i] < '0') || (pEdit->textBuffer[i] > '9'))
				{
					if(pEdit->textBuffer[i] == ':')
					{
						bPre = FALSE;
					}
					else if(pEdit->textBuffer[i] != ' ')
					{
						PreNum = 0;
						PostNum = 0;
						break;
					}
				}
				else
				{
					if(bPre == TRUE)
					{
						PreNum++;
						if(PreNum == 1)
						{
							if((pEdit->textBuffer[i+1] < '0') || (pEdit->textBuffer[i+1] > '9'))
							{
								break;
							}
						}
					}
					else
					{
						PostNum++;
						if(PostNum == 1)
						{
							if((pEdit->textBuffer[i+1] < '0') || (pEdit->textBuffer[i+1] > '9'))
							{
								break;
							}
						}
					}
				}
			}

			if((PreNum != 2) || (PostNum != 2))
			{
				pEdit->dwExStyle &= ~ETS_TIME;
			}
		}
	}
	
	return 1;
}

int UTFAPI UTFEditGetData(HUIWND hEdit, char *lpBuffer, BYTE bBufferSize)
{
	LPUTFEDIT pEdit = (LPUTFEDIT)hEdit;
	BYTE textLen;

	if(UTFWndIsType(hEdit, WTP_EDIT) == FALSE)
		return 0;

	textLen = strlen((char *)pEdit->textBuffer);
	if((pEdit->bMaxSize == 0) || (lpBuffer == NULL) || (textLen == 0) || (textLen >= bBufferSize))
		return 0;

	if(pEdit->dwExStyle & ETS_PASSWORD)
	{
		if(pEdit->dwFlags & ETF_EDITING)
			return 0;

		strcpy(lpBuffer, (char *)pEdit->textOldBuffer);
	}
	else if(pEdit->dwExStyle & ETS_NUMERIC_ONLY)
	{
		if((pEdit->bCaretPos == 0) && (pEdit->dwFlags & ETF_EDITING))
			return 0;

		strcpy(lpBuffer, (char *)pEdit->textBuffer);
	}
	else
	{
		if(pEdit->dwFlags & ETF_EDITING)
			return 0;

		strcpy(lpBuffer, (char *)pEdit->textBuffer);
	}
	
	return 1;
}

int UTFAPI UTFEditSetInfo(HUIWND hEdit, WORD wMaxSel, LPUTFSELINFO lpInfo)
{
	if(hEdit == NULL)
		return 0;

	if(UTFWndIsType(hEdit, WTP_EDIT) == TRUE)
	{
		LPUTFEDIT pEdit = (LPUTFEDIT)hEdit;

		if((pEdit->dwExStyle & ETS_PASSWORD) || (pEdit->dwExStyle & ETS_TIME))
			return 0;

		pEdit->wMaxSel = wMaxSel;
		if(pEdit->wMaxSel > 0)
		{
			pEdit->lpSelInfo = lpInfo;
		}
		else
		{
			pEdit->lpSelInfo = NULL;
		}
		
		if(pEdit->wCurSel >= pEdit->wMaxSel)
		{
			pEdit->wCurSel = 0;
		}
		pEdit->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;

		return 1;
	}

	return 0;
}

int UTFAPI UTFEditSetCurSel(HUIWND hEdit, WORD wCurSel)
{
	if(hEdit == NULL)
		return 0;

	if(UTFWndIsType(hEdit, WTP_EDIT) == TRUE)
	{
		LPUTFEDIT pEdit = (LPUTFEDIT)hEdit;

		if((wCurSel >= pEdit->wMaxSel) || (pEdit->lpSelInfo == NULL))
		{
			return 0;
		}
		
		if((pEdit->dwExStyle & ETS_PASSWORD) || (pEdit->dwExStyle & ETS_TIME))
			return 0;

		if(pEdit->wCurSel != wCurSel)
		{
			pEdit->wCurSel = wCurSel;
			pEdit->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
			UTFEditSetData(hEdit, pEdit->bMaxSize, pEdit->lpSelInfo[pEdit->wCurSel].text);
		}
		
		return 1;
	}

	return 0;
}

WORD UTFAPI UTFEditGetCurSel(HUIWND hEdit)
{
	if(hEdit == NULL)
		return 0;

	if(UTFWndIsType(hEdit, WTP_EDIT) == TRUE)
	{
		LPUTFEDIT pEdit = (LPUTFEDIT)hEdit;

		if((pEdit->dwExStyle & ETS_PASSWORD) || (pEdit->dwExStyle & ETS_TIME))
			return 0;
		
		return pEdit->wCurSel;
	}

	return 0;
}

void UTFAPI UTFEditSetTimePM(HUIWND hEdit, char bPMMode)
{
	LPUTFEDIT pEdit = (LPUTFEDIT)hEdit;

	if(UTFWndIsType(hEdit, WTP_EDIT) == FALSE)
		return;

	if(pEdit->dwFlags & ETF_EDITING)
		return;

	if((pEdit->dwExStyle & ETS_TIME) && (pEdit->dwExStyle & ETS_TIME_CLOCK_12))
	{
		if(bPMMode == TRUE)
		{
			pEdit->dwFlags |= ETF_TIME_PM;
		}
		else
		{
			pEdit->dwFlags &= ~ETF_TIME_PM;
		}
		pEdit->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
	}
	else
	{
		pEdit->dwFlags &= ~ETF_TIME_PM;
		pEdit->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
	}
}

char UTFAPI UTFEditGetTimePM(HUIWND hEdit)
{
	LPUTFEDIT pEdit = (LPUTFEDIT)hEdit;

	if(UTFWndIsType(hEdit, WTP_EDIT) == FALSE)
		return FALSE;

	if((pEdit->dwExStyle & ETS_TIME) && (pEdit->dwExStyle & ETS_TIME_CLOCK_12))
	{
		if(pEdit->dwFlags & ETF_TIME_PM)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void UTFAPI UTFEditSetTimeType12H(HUIWND hEdit, char b12HType)
{
	LPUTFEDIT pEdit = (LPUTFEDIT)hEdit;

	if(UTFWndIsType(hEdit, WTP_EDIT) == FALSE)
		return;

	if(pEdit->dwFlags & ETF_EDITING)
		return;

	if(pEdit->dwExStyle & ETS_TIME)
	{
		if(b12HType == TRUE)
		{
			pEdit->dwExStyle |= ETS_TIME_CLOCK_12;
		}
		else
		{
			pEdit->dwExStyle &= ~ETS_TIME_CLOCK_12;
		}
		pEdit->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
	}
	else
	{
		pEdit->dwExStyle &= ~ETS_TIME_CLOCK_12;
		pEdit->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
	}
}

char UTFAPI UTFEditGetTimeType12H(HUIWND hEdit)
{
	LPUTFEDIT pEdit = (LPUTFEDIT)hEdit;

	if(UTFWndIsType(hEdit, WTP_EDIT) == FALSE)
		return FALSE;

	if(pEdit->dwExStyle & ETS_TIME)
	{
		if(pEdit->dwExStyle & ETS_TIME_CLOCK_12)
		{
			return TRUE;
		}
	}

	return FALSE;
}

BYTE UTFAPI UTFIsEditCtrl(HUIWND hWnd)
{
	return UTFWndIsType(hWnd, WTP_EDIT);
}

void UTFAPI UTFEditFinishEdit(HUIWND hEdit)
{
	LPUTFEDIT pEdit = (LPUTFEDIT)hEdit;

	if(UTFWndIsType(hEdit, WTP_EDIT) == FALSE)
		return;

	if(pEdit->dwFlags & ETF_EDITING)
	{
		LPUTFDIALOG pDlg = (LPUTFDIALOG)pEdit->hParent;

		pEdit->dwFlags &= ~(ETF_EDITING|ETF_NOMOVE);
		pEdit->dwFlags |= UIF_REDRAW|ETF_DRAWEDIT;
		if(pEdit->dwExStyle & ETS_PASSWORD)
		{
			memset((char *)pEdit->textBuffer, '*', pEdit->bMaxSize);
			memset((char *)pEdit->textOldBuffer, 0, pEdit->bMaxSize+1);
			pEdit->textBuffer[pEdit->bMaxSize] = 0;
		}
		else
		{
			if(pEdit->dwExStyle & ETS_NUMERIC_ONLY)
			{
				if(pEdit->bCaretPos == 0)
				{
					memcpy(pEdit->textBuffer, pEdit->textOldBuffer, EDITBUFLEN);
				}
				if(pDlg->CallBack)
				{
					pDlg->CallBack(pEdit->hParent, WMUI_NOTIFY, ENUI_FINISH, pEdit->wndID, 0);
				}
			}
			else
			{
				memcpy(pEdit->textBuffer, pEdit->textOldBuffer, EDITBUFLEN);
			}
		}
		pEdit->bCaretPos = 0;
	}
}

char UTFAPI UTFEditIsEditing(HUIWND hEdit)
{
	LPUTFEDIT pEdit = (LPUTFEDIT)hEdit;

	if(UTFWndIsType(hEdit, WTP_EDIT) == FALSE)
		return FALSE;

	if(pEdit->dwFlags & ETF_EDITING)
		return TRUE;

	return FALSE;
}

