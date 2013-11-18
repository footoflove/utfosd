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

#ifndef __UTFLISTPRIV_H__
#define __UTFLISTPRIV_H__

#include "UTFTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************/
/** define list control extend style **/

#define LSTS_OWNERDRAW	(0x00000010)	// Window will be draw by user

/***********************************************************************************/
/** define list control extend flag **/

#define UIF_DRAWFACE	(0x00000010)	// Window will be draw when paint face 

/***********************************************************************************/

void UTFListInitialize(void);

/***********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

