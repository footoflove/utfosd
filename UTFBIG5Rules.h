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

#ifndef __UTF_BIG5RULES_H__
#define __UTF_BIG5RULES_H__

#include "UTFTypeDef.h"
#include "UTFPalette.h"
#include "UTFFont.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BIG5_BYTE	0xA0

/*************************************************************************/

WORD BIG5TextWidth(LPTEXT pText, int charInterval, LPUTFLOGFONT lpFont);
void BIG5TextFunc(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont);

/*************************************************************************/



/*************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

