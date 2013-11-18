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
#include "UTFIpEdit.h"
#include "UTFIpEditPriv.h"
#include "UTFDrawText.h"
#include "UTFMemory.h"

/******************************************************************/
static void UTFDrawIpEditFace(HUIWND hIpEdit, UTFRECT rcRect, DWORD info)
{
	LPUTFIPEDIT pIpEdit = (LPUTFIPEDIT)hIpEdit;
	UTFRECT wndRect = rcRect;
	DWORD uFormat;
	BYTE text[20],texttemp[2];
	int i,j,m,n;

	/* Draw background */
	UTFSetPen(PSUI_NULL, UTFRGB(0,0,0,255), 1);
	if(pIpEdit->dwStyle & UIS_DISABLE)
	{
		UTFSetBrush(BSUI_SOLID, UTFRGB(150,150,150,255), HSUI_HORIZONTAL);
		UTFSetTextColor(UTFRGB(0,0,0,255));
	}
	else if(pIpEdit->dwFlags & UIF_FOCUS)
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
	
	if(pIpEdit->textID >= 0)
	{
		UTFGetIDText(pIpEdit->textID, pIpEdit->wndText, TEXT_SIZE);
	}
	m=0;
	for(i=0;i<4;i++)
	{
		for(j=0;j<3;j++)
		{
			text[m]=pIpEdit->cIptext[i][j];
			m++;
		}
		if(i<3)
		{
			text[m]='.';
			m++;
		}
	}
	text[m]='\0';
	trace("\n text : %s  -- 0x%x     sizeof(BYTE) : %d \n",text,pIpEdit->dwFlags,sizeof(BYTE));
	uFormat = DTUI_CENTER|DTUI_VCENTER|DTUI_SINGLELINE|DTUI_END_ELLIPSIS;
//	UTFDrawText(text, &wndRect, uFormat);
	n=0;
	for(i=0;i<15;i++)
	{
		texttemp[0]=text[i];
		texttemp[1]='\0';
		wndRect.left+=10;
		wndRect.right=wndRect.left+10;
		if(text[i]!='.')
			n++;
		UTFDrawText(texttemp, &wndRect, uFormat);
		if(n==pIpEdit->bCaretPos&&text[i]!='.')
		{
			if(n>11)
			{

			}
			else if(text[i+1]!='.')
			{
				UTFSetPen(PSUI_SOLID, UTFRGB(255,255,255,255), 5);
				UTFDrawline(wndRect.left+10+2, wndRect.bottom-5, wndRect.right+10-2, wndRect.bottom-5);
			}
			else if(text[i+1]=='.')
			{
				UTFSetPen(PSUI_SOLID, UTFRGB(255,255,255,255), 5);
				UTFDrawline(wndRect.left+10*2+2, wndRect.bottom-5, wndRect.right+10*2-2, wndRect.bottom-5);
			}
		}
		else if(0==pIpEdit->bCaretPos&&0==i)
		{
			UTFSetPen(PSUI_SOLID, UTFRGB(255,255,255,255), 5);
			UTFDrawline(wndRect.left+2, wndRect.bottom-5, wndRect.right-2, wndRect.bottom-5);
		}
	}

	

}

static int UTFJudgePos(BYTE P[4][3],int m,int n)
{
	int i=0,j=0;
	for(i=m;i>=0;i--)
	{
		for(j=n;j>=0;j--)
		{
			if(0xff!=P[i][j])
			{
				break;
			}
			n--;
		}
		m--;
	}
	
	return (m*4+n);
}

static int UTFIpEditMsgProc(HUIWND hButton, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFIPEDIT pIpEdit = (LPUTFIPEDIT)hButton;
	LPUTFDIALOG pDlg = (LPUTFDIALOG)pIpEdit->hParent;
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
			if((pIpEdit->dwStyle & ( UIS_DISABLE | UIS_HIDE )) || !(pIpEdit->dwStyle & UIS_TABSTOP))
			{
				retCode = 0;
			}
			else if( !(pIpEdit->dwFlags & UIF_FOCUS) )
			{
				HUIWND hFocusCtrl = UTFDlgGetFocus(pIpEdit->hParent);

				pIpEdit->dwFlags |= UIF_FOCUS;
				pIpEdit->dwFlags |= UIF_REDRAW;

				UTFSendPrivMessage(pIpEdit->hParent, WMUI_INTER, WMUI_NEWFOCUS, hButton, hFocusCtrl);
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
		case KEYUI_RIGHT:
			{
				if(pIpEdit->bCaretPos<12)
				{
					int curpos=pIpEdit->bCaretPos;
					pIpEdit->dwFlags |= UIF_REDRAW;
					if(pIpEdit->cIptext[curpos/3][curpos%3]==0xff)
					{
						if(curpos>=9)
						{
						}
						else
						pIpEdit->bCaretPos=(curpos/3+1)*3;
					}
					else
					{
						if(!(pIpEdit->bCaretPos>=11))
							pIpEdit->bCaretPos++;//=pIpEdit->bCaretPos/3;
					}
					
				}
				

			}
			break;
		case KEYUI_LEFT:
			{
				int i=0,j=0,curpos=pIpEdit->bCaretPos,flag_ok=0;
				if(pIpEdit->bCaretPos>0)
				{
					pIpEdit->dwFlags |= UIF_REDRAW;
					trace("w 00: pIpEdit->bCaretPos : %d  \n",pIpEdit->bCaretPos);
					for(i=0;i<4;i++)
					{
						for(j=0;j<3;j++)
						{
							if(pIpEdit->cIptext[(curpos-1)/3][(curpos-1)%3]!=0xff)
							{
								flag_ok=1;
								pIpEdit->bCaretPos=curpos;
								break;
							}
							if(pIpEdit->bCaretPos>2&&(pIpEdit->bCaretPos/3-1)*3==curpos)
							{
								trace("w 11: curpos : %d  \n",curpos);
								if(curpos%3)
								{
									pIpEdit->bCaretPos=curpos;
								}
								else
									pIpEdit->bCaretPos=curpos+1;
								flag_ok=1;
								break;
							}
							if(curpos==1)
							{
								pIpEdit->bCaretPos=1;
								flag_ok=1;
								break;
							}
							trace("w 11: curpos : %d  ,pIpEdit->cIptext[(curpos-1)/3][(curpos-1)%3] : 0x%x \n",curpos,pIpEdit->cIptext[(curpos-1)/3][(curpos-1)%3]);

							curpos--;
						}
						if(flag_ok)
							break;
					}
					trace("w 11: pIpEdit->bCaretPos : %d  \n",pIpEdit->bCaretPos);

					pIpEdit->bCaretPos--;//=UTFJudgePos(pIpEdit->cIptext,i,j);
					trace("w 22: pIpEdit->bCaretPos: %d \n",pIpEdit->bCaretPos);

				//	pIpEdit->cIptext[i][j]=0xff;
					
				}

			}
			break;
		case KEYUI_BACK:
	/*	{
			int curpos=pIpEdit->bCaretPos,i=0,j=0,is_ok=0;
			if(curpos>0)
			{
				pIpEdit->dwFlags |= UIF_REDRAW;
				if(pIpEdit->cIptext[curpos/3][curpos%3]==0xff)
				{
					for(i=0;i<4;i++)
					{
						for(j=0;j<3;j++)
						{
							if(pIpEdit->cIptext[(curpos-1)/3][(curpos-1)%3]!=0xff)
							{
								pIpEdit->cIptext[(curpos-1)/3][(curpos-1)%3]=0xff;
								pIpEdit->bCaretPos=curpos-1;
								is_ok=1;
								break;
							}
							curpos--;
							if(curpos<0)
							{
								pIpEdit->bCaretPos=0;
								is_ok=1;
								break;
							}
						}
						if(is_ok==1)
							break;
					}
				}
				else
				{
					pIpEdit->cIptext[curpos/3][curpos%3]=0xff;
				}
			}
		}*/
			break;
		case  KEYUI_NUMERIC:
			{
				int w=0,v=0;
				pIpEdit->dwFlags |= UIF_REDRAW;
				w=(pIpEdit->bCaretPos)/3;
				v=(pIpEdit->bCaretPos)%3;
				trace("w : %d v : %d  pIpEdit->bCaretPos : %d \n",w,v,pIpEdit->bCaretPos);
				pIpEdit->cIptext[w][v]='0'+param2;
				pIpEdit->bCaretPos++;
				if(pIpEdit->bCaretPos>11)
				{
					pIpEdit->bCaretPos=0;
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
			pIpEdit->dwFlags |= UIF_REDRAW;
			break;

		case WMUI_KILLFOCUS:
			pIpEdit->dwFlags &= ~UIF_FOCUS;
			pIpEdit->dwFlags |= UIF_REDRAW;
			pIpEdit->bCaretPos=0;

			break;

		case WMUI_ENABLE:
			if(pIpEdit->dwStyle & UIS_DISABLE)
			{
				pIpEdit->dwStyle &= ~UIS_DISABLE;
				pIpEdit->dwFlags |= UIF_REDRAW;

				if(UTFDialogHaveFocus(pIpEdit->hParent) == FALSE)
				{
					UTFSendPrivMessage(hButton, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}
			}
			break;

		case WMUI_DISABLE:
			if((pIpEdit->dwStyle & UIS_DISABLE) != UIS_DISABLE)
			{
				pIpEdit->dwStyle |= UIS_DISABLE;
				pIpEdit->dwFlags |= UIF_REDRAW;

				if(pIpEdit->dwFlags & UIF_FOCUS)
				{
					UTFSendPrivMessage(pIpEdit->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
					pIpEdit->dwFlags &= ~UIF_FOCUS;
				}
			}
			break;

		case WMUI_SHOW:
			if(pIpEdit->dwStyle & UIS_HIDE)
			{
				pIpEdit->dwStyle &= ~UIS_HIDE;
				pIpEdit->dwFlags |= UIF_REDRAW;
				
				UTFDialogDrawAllChildAboveCtrl(pIpEdit->hParent, hButton);
				
				if(UTFDialogHaveFocus(pIpEdit->hParent) == FALSE)
				{
					UTFSendPrivMessage(hButton, WMUI_PUBLIC, WMUI_SETFOCUS, 0, 0);
				}
			}
			break;

		case WMUI_HIDE:
			if((pIpEdit->dwStyle & UIS_HIDE) != UIS_HIDE)
			{
				pIpEdit->dwStyle |= UIS_HIDE;
				pIpEdit->dwFlags |= UIF_REDRAW;
				pIpEdit->dwFlags |= DLGF_CLEANBACK;
				
				if(pIpEdit->dwFlags & UIF_FOCUS)
				{
					UTFSendPrivMessage(pIpEdit->hParent, WMUI_INTER, WMUI_FOCUSGO, 0, 0);
					pIpEdit->dwFlags &= ~UIF_FOCUS;
				}
				
				UTFDialogRedrawCtrlInRect(pIpEdit->hParent, &pIpEdit->rect);
			}
			break;

		case WMUI_SETRECT:
			if(param2)
			{
				LPUTFRECT lpRect = (LPUTFRECT)param2;
				UTFRECT oldRect = pIpEdit->rect;

				if((lpRect->right > (pDlg->rect.right-pDlg->rect.left)) || (lpRect->bottom > (pDlg->rect.bottom-pDlg->rect.top)))
					return 0;

				pIpEdit->dwFlags |= UIF_REDRAW;
				
				UTFDialogBackClean(pIpEdit->hParent, hButton);
				UTFDialogRedrawCtrlInRect(pIpEdit->hParent, &pIpEdit->rect);
				
				memcpy(&pIpEdit->rect, lpRect, sizeof(UTFRECT));
				UTFDialogDrawAllChildAboveCtrl(pIpEdit->hParent, hButton);

				if((lpRect->left != oldRect.left) || (lpRect->top != oldRect.top))
				{
					UTFDialogResetCtrlXYOrder(pIpEdit->hParent, hButton, oldRect);
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
						strcpy((char *)pIpEdit->wndText, (char *)param2);
					}
					else
					{
						memcpy((char *)pIpEdit->wndText, (char *)param2, TEXT_SIZE);
						pIpEdit->wndText[TEXT_SIZE-1] = 0;
					}
					pIpEdit->textID = -1;	
					pIpEdit->dwFlags |= UIF_REDRAW;
				}
				else if(pIpEdit->textID != param3)
				{
					pIpEdit->textID = param3;
					pIpEdit->dwFlags |= UIF_REDRAW;
				}
			}
			break;

		case WMUI_GETTEXT:
			if((param2 != NULL) && (param3 > 0))
			{
				if(pIpEdit->textID >= 0)
				{
					UTFGetIDText(pIpEdit->textID, pIpEdit->wndText, TEXT_SIZE);
				}
				if(strlen((char *)pIpEdit->wndText) < param3)
				{
					strcpy((char *)param2, (char *)pIpEdit->wndText);
				}
				else
				{
					char *pBuffer = (char *)param2;
					
					memcpy(pBuffer, (char *)pIpEdit->wndText, param3);
					*(pBuffer+param3-1) = 0;
				}
			}
			break;	

		case WMUI_DELETEWND:
			if(param2 == 0)
			{
				LPUTFDIALOGCTRL pCtrl;			

				if(pIpEdit->hPrevCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pIpEdit->hPrevCtrl;
					pCtrl->hNextCtrl = pIpEdit->hNextCtrl;
				}
				else
				{
					pDlg->pCtrl = (LPUTFDIALOGCTRL)pIpEdit->hNextCtrl;
				}

				if(pIpEdit->hNextCtrl)
				{
					pCtrl = (LPUTFDIALOGCTRL)pIpEdit->hNextCtrl;
					pCtrl->hPrevCtrl = pIpEdit->hPrevCtrl;
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

void UTFIpEditInitialize(void)
{
	UTFRegisterWndType(WTP_IPEDIT, UTFIpEditMsgProc, UTFDrawIpEditFace);
}

HUIWND UTFAPI UTFCreateIpEdit(DWORD ctrlID, int textID, char *text, DWORD dwStyle, DWORD dwExStyle,
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
		HUIWND hButton = (HUIWND)UTFMalloc(sizeof(UTFIPEDIT));

		if(hButton != NULL)
		{
			LPUTFIPEDIT pIpEdit = (LPUTFIPEDIT)hButton;

			memcpy(pIpEdit->type, WTP_IPEDIT, 4);
			pIpEdit->wndID		= ctrlID;
			
			pIpEdit->dwStyle	= dwStyle | UIS_CHILD;
			pIpEdit->dwStyle	&= ~(UIS_MODAL | UIS_TOPMOST | UIS_NOFOCUS);
			pIpEdit->dwExStyle 	= dwExStyle;
			pIpEdit->dwFlags	= 0;
			
			pIpEdit->hParent	= hDlg;
			pIpEdit->dataEx		= ctrlData;
			pIpEdit->rect		= rcRect;
			
			pIpEdit->OnDraw		= OnDraw;
			pIpEdit->CallBack	= CallBack;
			memset(pIpEdit->cIptext,0xFF,sizeof(pIpEdit->cIptext));
			pIpEdit->bCaretPos=0;

			if(text != NULL)
			{
				pIpEdit->textID = -1;
				if(strlen(text) >= TEXT_SIZE)
				{
					memcpy((char *)pIpEdit->wndText, text, TEXT_SIZE-1);
					pIpEdit->wndText[TEXT_SIZE-1] = 0;
				}
				else
				{
					strcpy((char *)pIpEdit->wndText, text);
				}
			}
			else
			{
				pIpEdit->textID = textID;
			}

			UTFAddCtrlToDlg(hDlg, hButton);
		}

		return hButton;
	}

	return NULL;
}

