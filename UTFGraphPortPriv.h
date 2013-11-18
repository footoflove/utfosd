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

#ifndef __UTF_GRAPHPORTPRIV_H__
#define __UTF_GRAPHPORTPRIV_H__

#include "UTFTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif

int UTFRectFill(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color, DWORD bUsedLater);
int UTFRectDraw(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color, DWORD bUsedLater);
int UTFEllipseFill(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color, DWORD bUsedLater);
int UTFEllipseDraw(WORD left, WORD top, WORD right, WORD bottom, WORD penWidth, UTFOSDCOL color, DWORD bUsedLater);
int UTFRoundRectDraw(WORD left, WORD top, WORD right, WORD bottom, BYTE radiu, WORD penWidth, UTFOSDCOL color, DWORD bUsedLater);
int UTFRoundRectFill(WORD left, WORD top, WORD right, WORD bottom, BYTE radiu, UTFOSDCOL color, DWORD bUsedLater);
int UTFPolygonFill(LPUTFPOINT lpPoint, WORD nPoint, UTFOSDCOL color, DWORD bUsedLater);
int UTFPolygonDraw(LPUTFPOINT lpPoint, WORD nPoint, WORD penWidth, UTFOSDCOL color, DWORD bUsedLater);
int UTFLineDraw(WORD x1, WORD y1, WORD x2, WORD y2, WORD penWidth, UTFOSDCOL color, DWORD bUsedLater);
int UTFPixelDraw(WORD x, WORD y, UTFOSDCOL color, DWORD bUsedLater);
int UTFArc(WORD left, WORD top, WORD right, WORD bottom, WORD startAngle, WORD endAngle, UTFOSDCOL color, DWORD bUsedLater);

void UTFSetScreenDraw(int bNotMemoryDraw);
int UTFGetScreenDraw(void);

#ifdef __cplusplus
}
#endif

#endif


