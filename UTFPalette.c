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

#include "stdafx.h"
#include "UTFTypeDef.h"
#include "UTFPalettePriv.h"
#include "UTFGraphPort.h"
#include "dgbasetype.h"
#include "dgtrace.h"

typedef struct
{
	BYTE red;		// red base
	BYTE green;		// green base
	BYTE blue;		// blue base
	BYTE alpha;		// alpha base
}UTFPal, *LPUTFPal;

typedef union
{
	DWORD dataEx;
	UTFPal pal;
}UTFPALETTE, *LPUIPALETTE;

typedef struct
{
	DWORD dataEx;
	int Usage;		//palette usage information
}UTFPALINFO;

// Declare graph palette mode
enum
{
	UTFM_PALETTE,	// hardware work at palette mode
	UTFM_PIXEL		// hardware work at pixel map mode
};

#if(RUN_PC)
static BYTE g_bEnablePaletteTest;
static WORD g_wTestPaletteNum;
#endif

static WORD g_wPaletteNumber;
static WORD g_wGraphMode;
static WORD g_wMaxColorNum;
static GETREALCOLOR g_ColorFunc;
static UTFPALINFO g_PaletteMap[256];
/****************************************************************************/

UTFCOLOR UTFRGB(BYTE r, BYTE g, BYTE b, BYTE alpha)
{
	UTFPALETTE temp;

	temp.pal.red = r;
	temp.pal.green = g;
	temp.pal.blue = b;
	temp.pal.alpha = alpha;

	return (temp.dataEx);
}

UTFCOLOR UTFFillPalette(UTFCOLOR color, BYTE bColorIndex)
{
	UTFPALETTE data;
	DWORD i,j;
	DWORD pos = 256;
       int temp_data;
       

	data.dataEx = color;
	#if 1
	// Search for exist palette data 
	for(i=0; i<g_wPaletteNumber; i++)
	{
		// If have found, return palette pos and return
		if(g_PaletteMap[i].dataEx == data.dataEx)
		{
			// Update usage information
			for(j=0; j<g_wPaletteNumber; j++)
			{
				if(g_PaletteMap[j].Usage > g_PaletteMap[i].Usage)
				{
					g_PaletteMap[j].Usage--;
				}
			}

			g_PaletteMap[i].Usage = g_wPaletteNumber-1;

			return (UTFCOLOR)i;
		}
	}

	// If not found in old palette, install new palette at the end.
	if(g_wPaletteNumber < g_wMaxColorNum)
	{
		pos = g_wPaletteNumber++;
		g_PaletteMap[pos].Usage = pos;
	}
	else
	{
              temp_data = g_PaletteMap[0].Usage;
		for(i=0; i<g_wPaletteNumber; i++)
		{
                    if(g_PaletteMap[i].Usage < temp_data)
                    {
                        temp_data = g_PaletteMap[i].Usage;
                        pos = i;
                    }
                     g_PaletteMap[i].Usage--;
#if 0                     
			if(--g_PaletteMap[i].Usage < 0)
			{
				g_PaletteMap[i].Usage = g_wPaletteNumber-1;
				pos = i;
			}
#endif            
		}
              g_PaletteMap[pos].Usage = g_wPaletteNumber-1;
	}
#endif
	// Store new palette data and set hardware palette
	if(pos >= 0x100)
	{
            DG_trace(D_TRACE_ALL, "---the pos is %d\n", pos);
	}
	g_PaletteMap[pos].dataEx = data.dataEx;
	UTFInstallPalette((BYTE)pos, data.pal.red, data.pal.green, data.pal.blue, data.pal.alpha, 0);

	return (UTFCOLOR)pos;
}

void UTFEnumPaletteProc(ENUMPALETTECALLBACK lpCallBack)
{
	UTFPALETTE temp;
	WORD i;

	if(lpCallBack == NULL)
		return;

	for(i=0; i<g_wPaletteNumber; i++)
	{
		temp.dataEx = g_PaletteMap[i].dataEx;
		lpCallBack(i, temp.pal.red, temp.pal.green, temp.pal.blue, temp.pal.alpha);
	}
}

void UTFSetPaletteTopSize(WORD wTopSize)
{
	WORD i;

	if(g_wGraphMode == UTFM_PIXEL)
		return;
	
	if((wTopSize > 256) || (wTopSize == 0))
		return;

	if(g_wMaxColorNum == wTopSize)
		return;
	
	g_wMaxColorNum = wTopSize;
	g_wPaletteNumber = 0;

	for(i=0; i<256; i++)
	{
		g_PaletteMap[i].Usage = 0;
		g_PaletteMap[i].dataEx = 0;
		UTFInstallPalette(i, 0, 0, 0, 0, 0);
	}

#if(RUN_PC)
	g_wTestPaletteNum = 0;
#endif
}

WORD UTFGetPaletteTopSize(void)
{
	if(g_wGraphMode == UTFM_PIXEL)
		return 0;
	
	return (WORD)g_wMaxColorNum;
}

void UTFBeginTestPalette(BYTE bTestEnable)
{
#if(RUN_PC)
	g_bEnablePaletteTest = bTestEnable;
	g_wTestPaletteNum = 0;
#endif
}

UTFCOLOR UTFSetIndexColor(UTFCOLOR color, BYTE ColorIndex)
{
	UTFPALETTE temp;

	if(g_wGraphMode == UTFM_PIXEL)
	{
	#if(RUN_PC)
		return color;
	#else
		temp.dataEx = color;
		return g_ColorFunc(temp.pal.red, temp.pal.green, temp.pal.blue, temp.pal.alpha);
	#endif
	}

	temp.dataEx = color;
	UTFInstallPalette(ColorIndex, temp.pal.red, temp.pal.green, temp.pal.blue, temp.pal.alpha, 0);

	return (UTFCOLOR)ColorIndex;
}

void UTFRegisterColorFunction(GETREALCOLOR lpFunc)
{
	if(lpFunc)
	{
		g_ColorFunc = lpFunc;
	}
}

void UTFPaletteSetColorMode(BYTE bitPerPixel)
{
	WORD i,GM = UTFM_PALETTE;

	if(bitPerPixel == 16)
	{
		GM = UTFM_PIXEL;
	}

	if(GM != g_wGraphMode)
	{
		g_wGraphMode = GM;
		g_wPaletteNumber = 0;
		g_wMaxColorNum = 0;

		for(i=0; i<256; i++)
		{
			g_PaletteMap[i].Usage = 0;
			g_PaletteMap[i].dataEx = 0;
		}

		if(g_wGraphMode == UTFM_PALETTE)
		{
			g_wMaxColorNum = 1;
			for(i=0; i<bitPerPixel; i++)
			{
				g_wMaxColorNum *= 2;
			}
		}
	}
}

void UTFInitPalette(void)
{
	UTFOSDInfo graphInfo;
	WORD i;

	g_ColorFunc = UTFRGB;
	g_wGraphMode = UTFM_PIXEL;
	g_wPaletteNumber = 0;
	g_wMaxColorNum = 0;
	
	for(i=0; i<256; i++)
	{
		g_PaletteMap[i].Usage = 0;
		g_PaletteMap[i].dataEx = 0;
	}

	UTFGetOSDInfo(&graphInfo);

	if(graphInfo.bitPerPixel <= 8)
	{
		g_wGraphMode = UTFM_PALETTE;
		g_wMaxColorNum = 1;
		for(i=0; i<graphInfo.bitPerPixel; i++)
		{
			g_wMaxColorNum *= 2;
		}		
	}

#if(RUN_PC)
	g_bEnablePaletteTest = FALSE;
	g_wTestPaletteNum = 0;
	g_wMaxColorNum = 256;
#endif
}

