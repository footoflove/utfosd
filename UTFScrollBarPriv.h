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

#ifndef __UTFSCROLLBARPRIV_H__
#define __UTFSCROLLBARPRIV_H__

#include "UTFTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************/
/** define ctrl extend style **/

#define SBS_ATTACH_TO_CHILD		(0x10000000)	// scroll bar had been attached to any dialog ctrl

/***********************************************************************************/
/** define ctrl extend flag **/


/***********************************************************************************/

void UTFScrollBarInitialize(void);

/***********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

