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

#ifndef __UTF_DRAWTEXTPRIV_H__
#define __UTF_DRAWTEXTPRIV_H__

#include "UTFTypeDef.h"
#include "UTFFont.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************/
typedef void ( *TEXTPROC ) (LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont);
typedef WORD ( *TEXTWIDTH ) (LPTEXT pText, int charInterval, LPUTFLOGFONT lpFont);
typedef WORD ( *TEXTPREDONE ) (LPTEXT pTextSrc, LPTEXT pTextBuffer, WORD bufferSize, LPUTFLOGFONT lpFont);
typedef void ( *DRAWTEXT ) (LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat);
typedef WORD ( *DECODETEXT ) (LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat);
typedef void ( *ADDENDLLIPSE ) (LPTEXT lpText, WORD width, BYTE bFlag);
typedef void ( *COPYDATAFORPREDONE )(LPTEXT lpStart, LPTEXT lpCurPtr);
/*************************************************************************/

void UTFResetTextFunc(LPUTFLOGFONT lpFont, BYTE bForceSetFunc);
BYTE *UTFGetTextTempPtr(WORD *lpTempSize);

/*************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

