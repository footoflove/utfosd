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
#include "UTFPalette.h"
#include "UTFGraphPort.h"
#include "UTFDrawTextPriv.h"
#include "UTFDrawText.h"
#include "UTFFont.h"
#include "UTFFontPriv.h"

static UI_IDTEXT g_GetTextFunc;
/*******************************************************************/
static UTFLOGFONT g_LogFont;
/*******************************************************************/

void UTFFontInit(void)
{
	g_LogFont.lfHeight = 24;
	g_LogFont.lfWidth = 24;
	g_LogFont.lfEscapement = 0;
	g_LogFont.lfOrientation = 0;
	g_LogFont.lfWeight = FWUI_NORMAL;
	g_LogFont.lfItalic = FALSE;
	g_LogFont.lfUnderline = FALSE;
	g_LogFont.lfStrikeOut = FALSE;
	g_LogFont.lfCharSet = ANSI_CHARSET_UI;
	g_LogFont.lfOutPrecision = OUTUI_STROKE_PRECIS;
	g_LogFont.lfClipPrecision = CLIPUI_STROKE_PRECIS;
	g_LogFont.lfQuality = DRAFT_QUALITY_UI;
	g_LogFont.lfPitchAndFamily = VARIABLE_PITCH_UI | FF_MODERN_UI;
	g_LogFont.lfFontExTop = 0;
	g_LogFont.lfAddr1 = NULL;
	g_LogFont.lfAddr2 = NULL;
	g_LogFont.lfLogFontEx = NULL;
	strcpy(g_LogFont.lfFaceName, "Arial");

	UTFSetTextColor(UTFRGB(255,255,255,255));
	g_GetTextFunc = NULL;

	UTFResetTextFunc(&g_LogFont, TRUE);
}

void UTFAPI UTFGetFont(LPUTFLOGFONT lpLogFont)
{
	if(lpLogFont != NULL)
	{
		memcpy(lpLogFont, &g_LogFont, sizeof(UTFLOGFONT));
	}
}

void UTFAPI UTFSetFontIndirect(LPUTFLOGFONT lpLogFont)
{
	if(lpLogFont != NULL)
	{
		memcpy(&g_LogFont, lpLogFont, sizeof(UTFLOGFONT));
		UTFResetTextFunc(&g_LogFont, FALSE);
	}
}

void UTFAPI UTFSetFont(long lfHeight, long lfWidth, long lfEscapement, long lfOrientation, 
			long lfWeight, BYTE lfItalic, BYTE lfUnderline, BYTE lfStrikeOut, BYTE lfCharSet,
			BYTE lfOutPrecision, BYTE lfClipPrecision, BYTE lfQuality, BYTE lfPitchAndFamily,
			long lfFontExTop, DWORD lfAddr1, DWORD lfAddr2, DWORD lfLogFontEx, LPTEXT lfFaceName)
{
	g_LogFont.lfHeight = lfHeight;
	g_LogFont.lfWidth = lfWidth;
	g_LogFont.lfEscapement = lfEscapement;
	g_LogFont.lfOrientation = lfOrientation;
	g_LogFont.lfWeight = lfWeight;
	g_LogFont.lfItalic = lfItalic;
	g_LogFont.lfUnderline = lfUnderline;
	g_LogFont.lfStrikeOut = lfStrikeOut;
	g_LogFont.lfCharSet = lfCharSet;
	g_LogFont.lfOutPrecision = lfOutPrecision;
	g_LogFont.lfClipPrecision = lfClipPrecision;
	g_LogFont.lfQuality = lfQuality;
	g_LogFont.lfPitchAndFamily = lfPitchAndFamily;
	g_LogFont.lfFontExTop = lfFontExTop;
	g_LogFont.lfAddr1 = lfAddr1;
	g_LogFont.lfAddr2 = lfAddr2;
	g_LogFont.lfLogFontEx = lfLogFontEx;
	if(strlen((char *)lfFaceName) >= FNT_NAME_LEN)
	{
		memcpy(g_LogFont.lfFaceName, lfFaceName, FNT_NAME_LEN-1);
		g_LogFont.lfFaceName[FNT_NAME_LEN-1] = 0;
	}
	else
	{
		strcpy(g_LogFont.lfFaceName, (char *)lfFaceName);
	}
	
	UTFResetTextFunc(&g_LogFont, FALSE);
}

/*********************************************************************/
int UTFGetIDText(DWORD textID, LPTEXT lpSTR, DWORD size)
{
	if(g_GetTextFunc != NULL)
	{
		return g_GetTextFunc(lpSTR, size, textID, g_LogFont.lfCharSet);
	}

	return -1;
}

void UTFAPI UTFRegisterTextIDProc(UI_IDTEXT lpIDTextFunc)
{
	g_GetTextFunc = lpIDTextFunc;
}

