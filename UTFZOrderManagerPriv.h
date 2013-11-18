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

#ifndef __UTF_ZORDERMANAGEPRIV_H__
#define __UTF_ZORDERMANAGEPRIV_H__

#include "UTFTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UTF_MSGBOX_WORKTYPE		0	// 0:If have modal on, cannot display all msgbox that not topmost
									// 1:If have modal on, only display msgbox that not topmost and free status

enum
{
	UTFWND_INVISIBLE,
	UTFWND_FREESHOW,
	UTFWND_PARTSHOW
};

int UTFIsWindowVisible(HUIWND hWnd);
void UTFSetOrderInfo(char bSetModeType);
int UTFEnableWndForMsg(HUIWND hWnd, BYTE bEnable);
void UTFSetOpenScreenFlag();
char UTFWndCanBeShowOrHide(HUIWND hWnd);
BYTE UTFWindowSaveOrPutBack(HUIWND hWnd, char bSaveMode);
void UTFCleanAllHideWindow(void);
void UTFSendMsgToDeskTop(DWORD uMsg, DWORD p1, DWORD p2, DWORD p3);
int UTFIsDeskTopCanGetMsg(void);
char UTFGetDefaultCloseWindowMode(void);

#ifdef __cplusplus
}
#endif

#endif

