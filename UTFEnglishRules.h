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

#ifndef __UTF_ENGRULES_H__
#define __UTF_ENGRULES_H__

#include "UTFTypeDef.h"
#include "UTFPalette.h"
#include "UTFFont.h"
#include "UTFDrawTextPriv.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************/
WORD ENGTextWidth(LPTEXT pText, int charInterval, LPUTFLOGFONT lpFont);
/*************************************************************************/
extern TEXTPROC ENGTextFunc;
/*************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

