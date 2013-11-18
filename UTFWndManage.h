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

#ifndef __UTF_WNDMANAGE_H__
#define __UTF_WNDMANAGE_H__

#include "UTFTypeDef.h"
#include "UTFWndDef.h"

#ifdef __cplusplus
extern "C" {
#endif

int UTFDefMsgProc(HUIWND hWnd, DWORD uMsg, DWORD param1, DWORD param2, DWORD param3);
int UTFRegisterWndType(const char *wndName, UTFCALLBACK CallBack, UTFONDRAW draw);

int UTFDefDrawProc(HUIWND hWnd, UTFRECT rcRect, DWORD info);
int UTFWndIsType(HUIWND hWnd, char *pTypeStr);
void UTFTypeManagerInit(void);

#ifdef __cplusplus
}
#endif

#endif

