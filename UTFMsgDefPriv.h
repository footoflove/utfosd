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

#ifndef __UTF_MSGDEFPRIV_H__
#define __UTF_MSGDEFPRIV_H__

#include "UTFMsgDef.h"

#define WMUI_INTER			(WMUI_BASE_TYPE + 0x00)	// window private message type

/*****************************************************************************/
/* these message will be send as WMUI_INTER param1 */
/* public notify message define **/

#define INTER_BASE			(0x80000000)	// ui notify message start define

#define WMUI_DELETEWND		(INTER_BASE + 0x00)	//Window memory will be free all
#define WMUI_SETTEXT		(INTER_BASE + 0x01)	//Window text will be changed
#define WMUI_GETTEXT		(INTER_BASE + 0x02)	//Window text will be got into buffer
#define WMUI_KILLFOCUS		(INTER_BASE + 0x03)	//dialog ctrl will lose focus
#define WMUI_NEWFOCUS		(INTER_BASE + 0x04)	//dialog focus had been changed to other ctrl
#define WMUI_ENABLE			(INTER_BASE + 0x05)	//Window enable to do key message
#define WMUI_DISABLE		(INTER_BASE + 0x06)	//Window cannot do key message
#define WMUI_SHOW			(INTER_BASE + 0x07)	//Window can be display on screen
#define WMUI_HIDE			(INTER_BASE + 0x08)	//Window cannot be display on screen
#define WMUI_PAINT			(INTER_BASE + 0x09)	//Window will be draw again
#define WMUI_UPDATE			(INTER_BASE + 0x0A)	//Window need finish draw
#define WMUI_SETPARENT		(INTER_BASE + 0x0B)	//Window's parent will be changed
#define WMUI_SETRECT		(INTER_BASE + 0x0C)	//Window's rect will be changed
#define WMUI_FOCUSGO		(INTER_BASE + 0x0D)	//dialog focus will be changed

#endif

