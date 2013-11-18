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

#ifndef __UTF_WNDDEFPRIV_H__
#define __UTF_WNDDEFPRIV_H__

#include "UTFTypeDef.h"
#include "UTFRect.h"

#define WTP_MENU		"MENU"
#define WTP_MENUITEM	"MITM"
#define WTP_DIALOG		"DLOG"
#define WTP_WINDOW		"MSGW"
#define WTP_TEXT		"TEXT"
#define WTP_CHECKBOX	"CKBX"
#define WTP_BUTTON		"BUTN"
#define WTP_EDIT		"EDIT"
#define WTP_COMBOBOX	"COMB"
#define WTP_LIST		"LIST"
#define WTP_LRLIST		"LRLS"
#define WTP_IMAGE		"IMAG"
#define WTP_VIDEO		"VIDE"
#define WTP_SCROLLBAR	"SRLB"
#define WTP_PROGRESS	"PRGB"
#define WTP_IPEDIT	"IPED"
#define WTP_INPUT	"INPT"
/*****************************************************************/
// Window style define

#define UIS_NOFOCUS		(0x10000000)	// window never receive focus
#define UIS_CHILD		(0x20000000)	// Window are child type
#define UIS_MODAL		(0x40000000)	// Window can process message that come from IR or keypad

#ifdef __cplusplus
extern "C" {
#endif

HUIWND UTFGetMainParent(HUIWND hWnd);
int UTFSendPrivMessage(HUIWND hWnd, DWORD uMsg, DWORD p1, DWORD p2, DWORD p3);

#ifdef __cplusplus
}
#endif

#endif

