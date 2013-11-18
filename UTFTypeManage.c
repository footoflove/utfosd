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
#include "UTFWndDef.h"
#include "UTFWndManage.h"
#include "UTFMsgDefPriv.h"

#define UIWNDTYPE	100

typedef struct
{
	DWORD type;
	UTFCALLBACK CallBack;
	UTFONDRAW draw;
}UTFWndDefProc, *LPUTFWndDefProc;

/****************************************************************************/
static UTFWndDefProc g_WndDefProc[UIWNDTYPE];
static int g_iTotalType;
/****************************************************************************/

static int UTFFindType(DWORD wndType)
{
	int low=0,high,middle;

	high = g_iTotalType;
	middle = (low+high)/2;

	while(low != high)
	{
		if(g_WndDefProc[middle].type < wndType)
		{
			low = middle+1;
		}
		else if(g_WndDefProc[middle].type > wndType)
		{
			high = middle;
		}
		else
		{
			return middle;
		}

		middle = (low+high)/2;
	}

	return (-1);
}

int UTFDefDrawProc(HUIWND hWnd, UTFRECT rcRect, DWORD info)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;
	int position;
	DWORD value;

	if(hWnd == NULL)
	{
		return (-1);
	}

	memcpy(&value, lpUIWnd->type, 4);

	position = UTFFindType(value);
	if(position >= 0)
	{
		if(g_WndDefProc[position].draw != NULL)
		{
			g_WndDefProc[position].draw(hWnd, rcRect, info);
			return 0;
		}
	}

	return (-1);
}

int UTFDefMsgProc(HUIWND hWnd, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3)
{
	LPUTFWND lpUIWnd = (LPUTFWND)hWnd;
	int position;
	DWORD value;

	if(hWnd == NULL)
	{
		return 0;
	}

	if((uMsg < WMUI_BASE_TYPE) || (uMsg > WMUI_LAST_TYPE))
	{
		return 0;
	}

	memcpy(&value, lpUIWnd->type, 4);

	position = UTFFindType(value);
	if(position >= 0)
	{
		if(g_WndDefProc[position].CallBack != NULL)
		{
			return g_WndDefProc[position].CallBack(hWnd, uMsg, param1, param2, param3);
		}
	}

	return 0;
}

int UTFAPI UTFRegisterWndType(const char *wndName, UTFCALLBACK CallBack, UTFONDRAW draw)
{
	int position;
	char buffer[4];
	DWORD value;

	if((g_iTotalType >= UIWNDTYPE) || (strlen(wndName) < 4))
		return 0;

	memcpy(buffer, wndName, 4);
	memcpy(&value, buffer, 4);

	position = UTFFindType(value);
	if(position < 0)
	{
		int i,j=0;

		for(i=0; i<g_iTotalType; i++)
		{
			if(g_WndDefProc[i].type < value)
			{
				j = i+1;
			}
			else
			{
				break;
			}
		}

		for(i=0; i<(g_iTotalType-j); i++)
		{
			memcpy(&g_WndDefProc[g_iTotalType-i], &g_WndDefProc[g_iTotalType-i-1], sizeof(UTFWndDefProc));
		}

		g_WndDefProc[j].type = value;
		g_WndDefProc[j].CallBack = CallBack;
		g_WndDefProc[j].draw = draw;
		g_iTotalType++;
	}
	else
	{
		return 0;
	}

	return 1;
}

void UTFTypeManagerInit(void)
{
	g_iTotalType = 0;
	memset(g_WndDefProc, 0, sizeof(g_WndDefProc));
}

