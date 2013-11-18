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

#ifndef __UTFMENUPRIV_H__
#define __UTFMENUPRIV_H__

#include "UTFTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************/
/** define menu general style **/

#define MIF_SUBOPEN	    (0x00000001)	// child menu of menu item need draw
#define MIF_CALLBACK	(0x00000002)	// application need callback function before enter submenu
#define MIF_WAITGOTOSUB	(0x00000004)	// menu is waiting for goto sub menu of focus item

/***********************************************************************************/
/** define menu general flag **/

int UTFIsMenu(HUIWND hWnd);

void UTFMenuInitialize(void);

#ifdef __cplusplus
}
#endif

#endif

