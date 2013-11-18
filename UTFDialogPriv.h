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

#ifndef __UTFDIALOGPRIV_H__
#define __UTFDIALOGPRIV_H__

#include "UTFTypeDef.h"
#include "UTFRect.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************/
/** define dialog general style **/



/***********************************************************************************/
/** define dialog general flag **/

#define DLGF_CLEANBACK	(0x08000000)	// If dialog ctrl be hide and had this flag, must clean dialog back


// This function not be used in application
int UTFIsDialog(HUIWND hWnd);
void UTFAddCtrlToDlg(HUIWND hDlg, HUIWND hCtrl);
int UTFDialogHaveFocus(HUIWND hDlg);
void UTFDialogBackClean(HUIWND hDlg, HUIWND hCtrl);
void UTFDialogDrawAllChildAboveCtrl(HUIWND hDlg, HUIWND hCtrl);
void UTFDialogRedrawCtrlInRect(HUIWND hDlg, LPUTFRECT lpRect);
void UTFDialogResetCtrlXYOrder(HUIWND hDlg, HUIWND hCtrl, UTFRECT oldRect);
void UTFDialogInitialize(void);

#ifdef __cplusplus
}
#endif

#endif

