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

#ifndef __UTFCHECKBOXPRIV_H__
#define __UTFCHECKBOXPRIV_H__

#include "UTFTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************/
/** define check box extend style **/

#define CKBF_CHECKED		(0x00000010)	// check box had been checked

/***********************************************************************************/
/** define check box extend flag **/

/***********************************************************************************/

void UTFCheckBoxInitialize(void);

/***********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

