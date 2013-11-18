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

#ifndef __UTF_ARBRULES_H__
#define __UTF_ARBRULES_H__

#include "UTFTypeDef.h"
#include "UTFPalette.h"
#include "UTFFont.h"
#include "UTFDrawTextPriv.h"

#ifdef __cplusplus
extern "C" {
#endif

extern TEXTPROC ARBTextFunc;

/*************************************************************************/
int DVBArabicToWinArabic(BYTE cCode);
WORD ARBTextPreDone(LPTEXT pTextSrc, LPTEXT pTextBuffer, WORD bufferSize, LPUTFLOGFONT lpFont);
WORD ARBTextWidth(LPTEXT pText, int charInterval, LPUTFLOGFONT lpFont);
char ARBIsEnglishChar(BYTE code);
/*************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

