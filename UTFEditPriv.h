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

#ifndef __UTFEDITPRIV_H__
#define __UTFEDITPRIV_H__

#include "UTFTypeDef.h"
#include "UTFWndDef.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************/
/** define edit control extend style **/

#define ETS_NUMERIC_HAVE	0x00000100		// EDIT control input buffer have numeric
#define ETS_NUMERIC_ONLY	0x00000200		// EDIT control input buffer only have numeric

#define ETF_NOMOVE			0x00000100		// EDIT control not permit to change focus

/***********************************************************************************/
/** define edit control extend flag **/

/***********************************************************************************/

void UTFEditInitialize(void);

/***********************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

