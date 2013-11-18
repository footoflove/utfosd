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
#include "UTFTimer.h"
#include "UTFTimerPriv.h"
#include "UTFMsgDefPriv.h"
#include "UTFZOrderManage.h"
#include "UTFZOrderManagerPriv.h"

#define UTF_MAX_TIMER	64

/***************************************************************************/
typedef struct
{
	HUIWND hWnd;
	DWORD dwTimerTerm;
	DWORD dwCurTick;
	DWORD dwUsage;
	DWORD dwPause;		//vivian add 20080602 to pause or resume the utf timer  1: 暂停timer   0:  恢复timer
}UTFTIMERDATA, *LPUTFTIMERDATA;

static WORD g_wUTFTimerNumber;
static UTFTIMERDATA g_UTFTimer[UTF_MAX_TIMER];
/***************************************************************************/

void UTFTimerInitialize(void)
{
	int i;

	for(i=0; i<UTF_MAX_TIMER; i++)
	{
		g_UTFTimer[i].dwPause = 0;
		g_UTFTimer[i].dwUsage = 0;
		g_UTFTimer[i].dwCurTick = 0;
		g_UTFTimer[i].dwTimerTerm = 0;
		g_UTFTimer[i].hWnd = NULL;
	}

	g_wUTFTimerNumber = 0;
}

static void UTFTimerCalculate(void)
{
	WORD i,wTotal = 0;

	for(i=0; i<UTF_MAX_TIMER; i++)
	{
		if(g_UTFTimer[i].dwUsage)
		{
			wTotal++;
		}
	}

	g_wUTFTimerNumber = wTotal;
}

void UTFKillWindowTimer(HUIWND hWnd)
{
	int i;

	for(i=0; i<UTF_MAX_TIMER; i++)
	{
		if(g_UTFTimer[i].dwUsage)
		{
			if(hWnd == g_UTFTimer[i].hWnd)
			{
				g_UTFTimer[i].dwPause = 0;
				g_UTFTimer[i].dwUsage = 0;
				g_UTFTimer[i].dwCurTick = 0;
				g_UTFTimer[i].dwTimerTerm = 0;
				g_UTFTimer[i].hWnd = NULL;
			}
		}
	}

	UTFTimerCalculate();
}




/***************************************************************************/
int UTFAPI UTFCreateTimer(HUIWND hWnd, DWORD dwTimerTerm)
{
	int i;

	if(dwTimerTerm == 0)
		return (-1);

	for(i=0; i<UTF_MAX_TIMER; i++)
	{
		if(!g_UTFTimer[i].dwUsage)
		{
			g_UTFTimer[i].dwUsage = 1;
			g_UTFTimer[i].dwPause = 0;
			g_UTFTimer[i].hWnd = hWnd;
			g_UTFTimer[i].dwTimerTerm = dwTimerTerm;
			g_UTFTimer[i].dwCurTick = 0;

			UTFTimerCalculate();

			return i;
		}
	}

	return (-1);
}

int UTFAPI UTFKillTimer(int iTimerID)
{
	if((iTimerID < 0) || (iTimerID >= UTF_MAX_TIMER))
		return 0;

	g_UTFTimer[iTimerID].dwPause = 0;
	g_UTFTimer[iTimerID].dwUsage = 0;
	g_UTFTimer[iTimerID].dwCurTick = 0;
	g_UTFTimer[iTimerID].dwTimerTerm = 0;
	g_UTFTimer[iTimerID].hWnd = NULL;

	UTFTimerCalculate();

	return 1;
}

int UTFStopTimer(int iTimerID)
{
	if((iTimerID < 0) || (iTimerID >= UTF_MAX_TIMER))
		return 0;

	if(g_UTFTimer[iTimerID].dwUsage == 0)
	{
		return 0;
	}
	g_UTFTimer[iTimerID].dwPause = 1;
	return 1;
}
 
int UTFStartTimer(int iTimerID)
{
	if((iTimerID < 0) || (iTimerID >= UTF_MAX_TIMER))
		return 0;

	if(g_UTFTimer[iTimerID].dwUsage == 0)
	{
		return 0;
	}
	
	if(g_UTFTimer[iTimerID].dwPause == 0)
	{
		return 1;
	}
	g_UTFTimer[iTimerID].dwPause = 0;
	g_UTFTimer[iTimerID].dwCurTick = 0;
	return 1;
}

void UTFAPI UTFTimerTick(DWORD dwTickMicroSec)
{
	int i,bUpdate = FALSE;

	if(g_wUTFTimerNumber == 0)
		return;

	for(i=0; i<UTF_MAX_TIMER; i++)
	{
		//判断timer有效且并不是处于暂停状态，则计数tick
		if((g_UTFTimer[i].dwUsage ) && (g_UTFTimer[i].dwPause == 0))
		{
			if(g_UTFTimer[i].hWnd)
			{
				if(!UTFIsWndActiving(g_UTFTimer[i].hWnd))
				{
					g_UTFTimer[i].dwCurTick = 0;
					continue;
				}
			}
			else if(!UTFIsDeskTopCanGetMsg())
			{
				g_UTFTimer[i].dwCurTick = 0;
				continue;
			}
			g_UTFTimer[i].dwCurTick += dwTickMicroSec;
			if(g_UTFTimer[i].dwCurTick >= g_UTFTimer[i].dwTimerTerm)
			{
				g_UTFTimer[i].dwCurTick = 0;
				if(g_UTFTimer[i].hWnd)
				{
					LPUTFWND lpUIWnd = (LPUTFWND)g_UTFTimer[i].hWnd;

					if(lpUIWnd->CallBack)
					{
						bUpdate = TRUE;
						lpUIWnd->CallBack(g_UTFTimer[i].hWnd, WMUI_PUBLIC, WMUI_TIMER, i, 0);
					}
				}
				else
				{
					bUpdate = TRUE;
					UTFSendMsgToDeskTop(WMUI_PUBLIC, WMUI_TIMER, i, 0);
				}
			}
		}
	}

	if(bUpdate == TRUE)
	{
		UTFUpdateSurface();
	}
}

