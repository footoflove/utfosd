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

#ifndef __UTF_PALETTEPRIV_H__
#define __UTF_PALETTEPRIV_H__

#include "UTFPalette.h"

#ifdef __cplusplus
extern "C" {
#endif

UTFCOLOR UTFFillPalette(UTFCOLOR color, BYTE bColorIndex);
UTFCOLOR UTFSetIndexColor(UTFCOLOR color, BYTE ColorIndex);

UTFCOLOR UTFFillPaletteFunc(UTFCOLOR color, BYTE bColorIndex);
int UTFGetColorIndex(void);
void UTFPaletteSetColorMode(BYTE bitPerPixel);

void UTFInitPalette(void);

#ifdef __cplusplus
}
#endif

#endif

