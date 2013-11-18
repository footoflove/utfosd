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
#include "UTFRect.h"
#include "UTFPalettePriv.h"
#include "UTFDrawAPI.h"
#include "UTFGifDrawPriv.h"
#include "UTFGraphPort.h"
#include "UTFGraphPortPriv.h"
#include "UTFMemoryPriv.h"
//#include "..\sdcapps\osdman\OSDUtil.H"
//#include "brazos.h"
#include "virgo.h"
#include "dgbasetype.h"
#include "dggdiapi.h"
#define OSDUTFDRAMIMGEX YES

#define MODIFY_COPY_RECT	0//vivian add 2008
//extern void UTFDrawBitmapWord(void * p4444Image, int srcX, int srcY, u_int32 uWidth, u_int32 uHeight, int screenX, int screenY);
/*******************************************************************/
typedef struct
{
	void ( *drawLine )			(WORD x1, WORD y1, WORD x2, WORD y2);
	void ( *drawRect )			(LPUTFRECT lpRect);
	void ( *fillRect )			(LPUTFRECT lpRect);
	void ( *drawEllipse )		(LPUTFRECT lpRect);
	void ( *fillEllipse )		(LPUTFRECT lpRect);
	void ( *drawPolygon )		(LPUTFPOINT lpPoint, WORD nPoint);
	void ( *fillPolygon )		(LPUTFPOINT lpPoint, WORD nPoint);
	void ( *drawRoundRect )		(LPUTFRECT lpRect, BYTE radiu);
	void ( *fillRoundRect )		(LPUTFRECT lpRect, BYTE radiu);
	void ( *drawImage )			(HIMAGE hImage, WORD left, WORD top, WORD right, WORD bottom);
#if defined(OSDUTFDRAMIMGEX) && (OSDUTFDRAMIMGEX == YES)
	void ( *drawImageEx )		(HIMAGE hImage, WORD left, WORD top, WORD right, WORD bottom, WORD xIMG, WORD yIMG, WORD wWidth, WORD wHeight);
#else
	void ( *drawImageEx )		(void * p4444Image, int srcX, int srcY, u_int32 uWidth, u_int32 uHeight, int screenX, int screenY);
#endif
}UIDRAWFUNC;

typedef UTFCOLOR ( *UIFILLPAL )(UTFCOLOR color, BYTE bColorIndex);
typedef struct
{
	DWORD r:8;
	DWORD g:8;
	DWORD b:8;
	DWORD a:8;
}BPP32COLORRGB;
typedef union
{
	BPP32COLORRGB color;
	DWORD wData;
}BPP32COLOR;

/*******************************************************************/
static UTFLOGPEN g_LogPen;
static UTFLOGBRUSH g_LogBrush;
static UIDRAWFUNC g_DrawFunc;
static UIFILLPAL g_FillPaletteFunc = UTFFillPalette;
static BPP32COLOR g_FillBrush;
/*******************************************************************/
static UTFRECT g_OSDRgnRect = {0, 0, 720, 576};
static UTFRECT g_FullScreenRect = {0, 0, 720, 576};
static BYTE g_BMPAlpha = 255;
static int g_iDrawColorIndex = -1;
static int g_bNotMemoryDraw;
static UTFCOLOR g_CleanColor;
static UTFCOLOR g_BMPPal[256];

static void UTFResetBrushFunc(void);
/***********************************************************************/
UTFCOLOR UTFAPI UTFSetCleanColor(UTFCOLOR clrColor)
{
	UTFCOLOR oldColor = g_CleanColor;

	g_CleanColor = clrColor;

	return oldColor;
}

void UTFAPI UTFCleanRect(WORD left, WORD top, WORD right, WORD bottom)
{
	UTFSetPen(PSUI_NULL, 0, 0);
	UTFSetBrush(BSUI_SOLID, g_CleanColor, 0);
	UTFFillRect(left, top, right, bottom);
}

void UTFAPI UTFCleanScreen(UTFCOLOR uColor)
{
	UTFRECT oldRgnRect = g_OSDRgnRect;
	
	g_OSDRgnRect = g_FullScreenRect;

	UTFSetPen(PSUI_NULL, 0, 0);
	UTFSetBrush(BSUI_SOLID, uColor, 0);
	UTFFillRect(0, 0, g_FullScreenRect.right-g_FullScreenRect.left, g_FullScreenRect.bottom-g_FullScreenRect.top);

	g_OSDRgnRect = oldRgnRect;
}

void UTFAPI UTFSetOSDRgnRect(WORD left, WORD top, WORD right, WORD bottom)
{
	g_OSDRgnRect.left = left;
	g_OSDRgnRect.top = top;
	g_OSDRgnRect.right = right;
	g_OSDRgnRect.bottom = bottom;
}

void UTFAPI UTFSetFullScreenRect(WORD left, WORD top, WORD right, WORD bottom)
{
	g_FullScreenRect.left = left;
	g_FullScreenRect.top = top;
	g_FullScreenRect.right = right;
	g_FullScreenRect.bottom = bottom;
}

void UTFAPI UTFDrawline(WORD x1, WORD y1, WORD x2, WORD y2)
{
	if(g_DrawFunc.drawLine != NULL)
	{
		g_DrawFunc.drawLine(x1, y1, x2, y2);
	}
}

void UTFAPI UTFFillRect(WORD left, WORD top, WORD right, WORD bottom)
{
	UTFRECT rect;

	rect.left	= left;
	rect.top	= top;
	rect.right	= right;
	rect.bottom	= bottom;

	UTFNormalizeRect(&rect);
	if(g_DrawFunc.fillRect != NULL)
	{
		g_DrawFunc.fillRect(&rect);
	}
	
	if(g_DrawFunc.drawRect != NULL)
	{
		g_DrawFunc.drawRect(&rect);
	}	
}

void UTFAPI UTFFillEllipse(WORD left, WORD top, WORD right, WORD bottom)
{
	UTFRECT rect;

	rect.left	= left;
	rect.top	= top;
	rect.right	= right;
	rect.bottom	= bottom;

	UTFNormalizeRect(&rect);
	if(g_DrawFunc.fillEllipse != NULL)
	{ 
		g_DrawFunc.fillEllipse(&rect);
	}
	
	if(g_DrawFunc.drawEllipse != NULL)
	{
		g_DrawFunc.drawEllipse(&rect);
	}	
}

void UTFAPI UTFFillRoundRect(WORD left, WORD top, WORD right, WORD bottom, BYTE radiu)
{
	UTFRECT rect;
	WORD width,height;

	rect.left	= left;
	rect.top	= top;
	rect.right	= right;
	rect.bottom	= bottom;

	UTFNormalizeRect(&rect);
	width = rect.right-rect.left;
	height = rect.bottom-rect.top;

	if(width > height)
	{
		if(height < 2*radiu)
		{
			radiu = height>>1;
		}
	}
	else if(width < 2*radiu)
	{
		radiu = width>>1;
	}

	if((g_LogPen.lopnWidth > 0) && (g_LogPen.lopnStyle == PSUI_SOLID) && (g_LogBrush.lbStyle == BSUI_SOLID))
	{
		UTFLOGBRUSH brush = g_LogBrush;

		g_LogBrush.lbColor = g_LogPen.lopnColor;
		g_LogBrush.lbStyle = BSUI_SOLID;
		UTFResetBrushFunc();
		if(g_DrawFunc.fillRoundRect != NULL)
		{ 
			g_DrawFunc.fillRoundRect(&rect, radiu);
		}

		if(radiu < g_LogPen.lopnWidth)
			radiu = 0;
		else
			radiu -= g_LogPen.lopnWidth;
		UTFDeflateRect(&rect, g_LogPen.lopnWidth+1, g_LogPen.lopnWidth, g_LogPen.lopnWidth, g_LogPen.lopnWidth);
		
		g_LogBrush = brush;
		UTFResetBrushFunc();
		if(g_DrawFunc.fillRoundRect != NULL)
		{ 
			g_DrawFunc.fillRoundRect(&rect, radiu);
		}
	}
	else
	{
		if(g_DrawFunc.fillRoundRect != NULL)
		{ 
			g_DrawFunc.fillRoundRect(&rect, radiu);
		}
		
		if(g_DrawFunc.drawRoundRect != NULL)
		{
			g_DrawFunc.drawRoundRect(&rect, radiu);
		}
	}
}

void UTFAPI UTFFillPolygon(LPUTFPOINT lpPoint, WORD nPoint)
{
	if(g_DrawFunc.fillPolygon != NULL)
	{ 
		g_DrawFunc.fillPolygon(lpPoint, nPoint);
	}
	
	if(g_DrawFunc.drawPolygon != NULL)
	{
		g_DrawFunc.drawPolygon(lpPoint, nPoint);
	}	
}

void UTFAPI UTFDrawPixel(WORD x, WORD y, UTFCOLOR Color)
{
	UTFOSDCOL color;

	if(g_bNotMemoryDraw)
	{
		x += g_OSDRgnRect.left;
		y += g_OSDRgnRect.top;
	}

	color.data = g_FillPaletteFunc(Color, g_iDrawColorIndex);
	
	UTFPixelDraw(x, y, color, 0);
}

static void UTFDrawLineSolid(WORD x1, WORD y1, WORD x2, WORD y2)
{
	UTFOSDCOL color;

	if(g_LogPen.lopnWidth)
	{
		if(g_bNotMemoryDraw)
		{
			x1 += g_OSDRgnRect.left;
			x2 += g_OSDRgnRect.left;
			y1 += g_OSDRgnRect.top;
			y2 += g_OSDRgnRect.top;
		}
		
		color.data = g_LogPen.lopnColor;
		UTFLineDraw(x1, y1, x2, y2, g_LogPen.lopnWidth, color, 0);
	}
}

static void UTFDrawRectSolid(LPUTFRECT lpRect)
{
	UTFOSDCOL color;
	UTFRECT rect;
	int i;

	memcpy(&rect, lpRect, sizeof(UTFRECT));
	if(g_bNotMemoryDraw)
	{
		rect.left += g_OSDRgnRect.left;
		rect.right += g_OSDRgnRect.left;
		rect.top += g_OSDRgnRect.top;
		rect.bottom += g_OSDRgnRect.top;
	}

	color.data = g_LogPen.lopnColor;
	for(i=0; i<g_LogPen.lopnWidth; i++)
	{
		UTFRectDraw(rect.left+i, rect.top+i, rect.right-i, rect.bottom-i, color, 0);
	}
}

static void UTFFillRectSolid(LPUTFRECT lpRect)
{
	UTFOSDCOL color;
	UTFRECT rect;

	memcpy(&rect, lpRect, sizeof(UTFRECT));
	if(g_bNotMemoryDraw)
	{
		rect.left += g_OSDRgnRect.left;
		rect.right += g_OSDRgnRect.left;
		rect.top += g_OSDRgnRect.top;
		rect.bottom += g_OSDRgnRect.top;
	}

	color.data = g_LogBrush.lbColor;
	UTFRectFill(rect.left, rect.top, rect.right, rect.bottom, color, 0);
}

static void UTFDrawEllipseSolid(LPUTFRECT lpRect)
{
	UTFOSDCOL color;
	UTFRECT rect;

	if(g_LogPen.lopnWidth)
	{
		WORD i;

		memcpy(&rect, lpRect, sizeof(UTFRECT));
		if(g_bNotMemoryDraw)
		{
			rect.left += g_OSDRgnRect.left;
			rect.right += g_OSDRgnRect.left;
			rect.top += g_OSDRgnRect.top;
			rect.bottom += g_OSDRgnRect.top;
		}

		color.data = g_LogPen.lopnColor;
		for(i=0; i<g_LogPen.lopnWidth; i++)
		{
			UTFEllipseDraw(rect.left+i, rect.top+i, rect.right-i, rect.bottom-i, 1, color, 0);
		}
	}
}

static void UTFFillEllipseSolid(LPUTFRECT lpRect)
{
	UTFOSDCOL color;
	UTFRECT rect;

	memcpy(&rect, lpRect, sizeof(UTFRECT));
	if(g_bNotMemoryDraw)
	{
		rect.left += g_OSDRgnRect.left;
		rect.right += g_OSDRgnRect.left;
		rect.top += g_OSDRgnRect.top;
		rect.bottom += g_OSDRgnRect.top;
	}
	
	color.data = g_LogBrush.lbColor;
	UTFEllipseFill(rect.left, rect.top, rect.right, rect.bottom, color, 0);
}

static void UTFDrawRoundRectSolid(LPUTFRECT lpRect, BYTE radiu)
{
	UTFOSDCOL color;
	UTFRECT rect;

	if(g_LogPen.lopnWidth)
	{
		WORD i;

		memcpy(&rect, lpRect, sizeof(UTFRECT));
		if(g_bNotMemoryDraw)
		{
			rect.left += g_OSDRgnRect.left;
			rect.right += g_OSDRgnRect.left;
			rect.top += g_OSDRgnRect.top;
			rect.bottom += g_OSDRgnRect.top;
		}

		color.data = g_LogPen.lopnColor;
		for(i=0; i<g_LogPen.lopnWidth; i++)
		{
			UTFRoundRectDraw(rect.left+i, rect.top+i, rect.right-i, rect.bottom-i, radiu-i, 1, color, 0);
		}
	}
}

static void UTFFillRoundRectSolid(LPUTFRECT lpRect, BYTE radiu)
{
	UTFOSDCOL color;
	UTFRECT rect;

	memcpy(&rect, lpRect, sizeof(UTFRECT));
	if(g_bNotMemoryDraw)
	{
		rect.left += g_OSDRgnRect.left;
		rect.right += g_OSDRgnRect.left;
		rect.top += g_OSDRgnRect.top;
		rect.bottom += g_OSDRgnRect.top;
	}
	
	color.data = g_LogBrush.lbColor;
	UTFRoundRectFill(rect.left, rect.top, rect.right, rect.bottom, radiu, color, 0);
}

static void UTFDrawPolygonSolid(LPUTFPOINT lpPoint, WORD nPoint)
{
	UTFOSDCOL color;
	int i;

	if(g_LogPen.lopnWidth)
	{
		if(g_bNotMemoryDraw)
		{
			for(i=0; i<nPoint; i++)
			{
				lpPoint[i].x += g_OSDRgnRect.left;
				lpPoint[i].y += g_OSDRgnRect.top;
			}
		}

		color.data = g_LogPen.lopnColor;
		UTFPolygonDraw((LPUTFPOINT)lpPoint, nPoint, g_LogPen.lopnWidth, color, 0);

		if(g_bNotMemoryDraw)
		{
			for(i=0; i<nPoint; i++)
			{
				lpPoint[i].x -= g_OSDRgnRect.left;
				lpPoint[i].y -= g_OSDRgnRect.top;
			}
		}
	}
}

static void UTFFillPolygonSolid(LPUTFPOINT lpPoint, WORD nPoint)
{
	UTFOSDCOL color;
	int i;

	if(g_bNotMemoryDraw)
	{
		for(i=0; i<nPoint; i++)
		{
			lpPoint[i].x += g_OSDRgnRect.left;
			lpPoint[i].y += g_OSDRgnRect.top;
		}
	}

	color.data = g_LogBrush.lbColor;
	UTFPolygonFill((LPUTFPOINT)lpPoint, nPoint, color, 0);

	if(g_bNotMemoryDraw)
	{
		for(i=0; i<nPoint; i++)
		{
			lpPoint[i].x -= g_OSDRgnRect.left;
			lpPoint[i].y -= g_OSDRgnRect.top;
		}
	}
}

void UTFAPI UTFDrawImage(HIMAGE hImage, WORD left, WORD top, WORD right, WORD bottom)
{
	g_DrawFunc.drawImage(hImage, left, top, right, bottom);
}
#if defined(OSDUTFDRAMIMGEX) && (OSDUTFDRAMIMGEX == YES)
void UTFAPI UTFDrawImageEx(HIMAGE hImage, WORD left, WORD top, WORD right, WORD bottom, WORD xIMG, WORD yIMG, WORD wWidth, WORD wHeight)
{
	g_DrawFunc.drawImageEx(hImage, left, top, right, bottom, xIMG, yIMG, wWidth, wHeight);
}
#else
void UTFAPI UTFDrawBitmap(void * p4444Image, int srcX, int srcY, u_int32 uWidth, u_int32 uHeight, int screenX, int screenY)
{
	g_DrawFunc.drawImageEx(p4444Image, srcX, srcY, uWidth, uHeight,  screenX,  screenY);
}

#endif

void UTFAPI UTFDrawArrow(WORD left, WORD top, WORD width, WORD height, ARROW_DIRECT direct)
{
	WORD index, length;
	double step, len = 0;
	UTFOSDCOL color;

	if((width < 2) || (height < 2))
	{
		return;
	}

	if(g_bNotMemoryDraw)
	{
		left += g_OSDRgnRect.left;
		top += g_OSDRgnRect.top;
	}
	
	color.data = g_LogPen.lopnColor;
	switch(direct)
	{
	case ASUI_LEFT:
		step = (double)height/(double)width;

		for(index = 0; index < width; index++)
		{
			length = (WORD)len;
			UTFLineDraw(left+index, top+(height-length)/2,
						left+index, top+(height+length)/2,
						1, color, 0);
			len += step;
		}
		break;

	case ASUI_RIGHT:
		step = (double)height/(double)width;

		for(index = 0; index < width; index++)
		{
			length = (WORD)len;
			UTFLineDraw(left+width-index, top+(height-length)/2,
						left+width-index, top+(height+length)/2,
						1, color, 0);
			len += step;
		}
		break;

	case ASUI_UP:
		step = (double)width/(double)height;

		for(index = 0; index < height; index++)
		{
			length = (WORD)len;
			UTFLineDraw(left+(width-length)/2, top+index, 
						left+(width+length)/2, top+index,
						1, color, 0);
			len += step;
		}
		break;

	case ASUI_DOWN:
		step = (double)width/(double)height;

		for(index = 0; index < height; index++)
		{
			length = (WORD)len;
			UTFLineDraw(left+(width-length)/2, top+height-index, 
						left+(width+length)/2, top+height-index,
						1, color, 0);
			len += step;
		}
		break;
	}
}

void UTFAPI UTFSetImageAlpha(BYTE alpha)
{
	g_BMPAlpha = alpha;
	
	UTFGIFSetAlpha(alpha);
}

void UTFAPI UTFSetColorIndex(int iStartIndex)
{
	if((iStartIndex > 255) || (iStartIndex < 0))
	{
		g_iDrawColorIndex = -1;
		g_FillPaletteFunc = UTFFillPalette;
	}
	else
	{
		g_iDrawColorIndex = iStartIndex;
		g_FillPaletteFunc = UTFSetIndexColor;
	}
}

void UTFAPI UTFGetImageInfo(HIMAGE hImage, LPUTFIMAGEINFO lptrWriteInfo)
{
//	BYTE *ptrRead = (BYTE *)hImage;
	WORD totalColor;
//	WORD width,height;
	DWORD width,height;
	BYTE *pBits;
//	PDRMPAL pPalette;

	if((hImage == NULL) || (lptrWriteInfo == NULL))
		return;

/*	UTFMemCpyWORD(&width, ptrRead, 2);
	ptrRead += 2;
	UTFMemCpyWORD(&height, ptrRead, 2);
	ptrRead += 2;
	UTFMemCpyWORD(&totalColor, ptrRead, 2);
	ptrRead += 2;
	*/
//	DG_AL_parse_img_header(hImage, &width, &height, &pBits, &pPalette);
	UTFParseImgHeader(hImage, &width, &height, &pBits);
	lptrWriteInfo->dwWidth = width;
	lptrWriteInfo->dwHeight = height;
	lptrWriteInfo->dwNumberOfColor =0;// (DWORD)pPalette;
}

/**************************************************************************/
#include "dgtrace.h"
void enum_palette (BYTE bIndex, BYTE red, BYTE green, BYTE blue, BYTE alpha)
{
	//DG_trace(D_TRACE_ALL,"%02d:   0x%02x,0x%02x,0x%02x\n,",bIndex,red,green,blue);
}

#if 0
static void UTFDrawImageByte(HIMAGE hImage, WORD left, WORD top, WORD right, WORD bottom)
{
	BYTE *ptrRead = (BYTE *)hImage;
	BYTE *ptrWrite;
	BYTE *ptrHeader;
	WORD totalColor;
	WORD wColorKey;
	WORD width,height;
	WORD outW,outH;
	WORD wPixelData;
	WORD dx=0,dy=0;
	DWORD dwSize;
	WORD i,j,bDrawMode=1;
	UTFOSDCOL color;
	UTFBITMAP bitmap;

	if(hImage == NULL)
		return;

	outW = right-left;
	outH = bottom-top;

	UTFMemCpyWORD(&width, ptrRead, 2);
	ptrRead += 2;
	UTFMemCpyWORD(&height, ptrRead, 2);
	ptrRead += 2;
	UTFMemCpyWORD(&totalColor, ptrRead, 2);	//该图片颜色总数
	ptrRead += 2;
	UTFMemCpyWORD(&wColorKey, ptrRead, 2);
	ptrRead += 2;

	if(outW > width)
	{
		outW = width;
	}
	if(outH > height)
	{
		outH = height;
	}

	UTFGetMemoryBitmap(&bitmap);
	if(bitmap.data)
	{
		bDrawMode = 0;
		dx = left;    
		dy = top;
		if((left+outW) > bitmap.Width) outW = bitmap.Width-left;
		if((top+outH) > bitmap.Height) outH = bitmap.Height-top;
	}
	else
	{
		dwSize = outW*outH;
		ptrHeader = (BYTE *)SOWMalloc(dwSize);
		if(ptrHeader == NULL)
			return;

		bitmap.data = ptrHeader;
		bitmap.Width = outW;
		bitmap.Height = outH;
		bitmap.bitPerPixel = 16;
		UTFSaveScreen(left, top, &bitmap, 0);
	}

	color.rgb.alpha = g_BMPAlpha;
	for(i=0; i<totalColor; i++)
	{
		color.rgb.red = *ptrRead++;
		color.rgb.green = *ptrRead++;
		color.rgb.blue = *ptrRead++;
		
		g_BMPPal[i] = g_FillPaletteFunc(color.data, i+g_iDrawColorIndex);
	}
	ptrWrite = (BYTE *)bitmap.data;
	ptrWrite = bitmap.data+dy*bitmap.Width+dx;
	for(i=0; i<outH; i++)
	{
		for(j=0; j<outW; j++)
		{
			wPixelData = *(ptrRead+j);
			if(wPixelData != wColorKey)
			{
				*(ptrWrite+j) = (BYTE)(g_BMPPal[wPixelData]);
			}
		}
		ptrWrite += bitmap.Width;
		ptrRead += Width;	//zhoulei  add 08-10-20
	}

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}
#else
static void UTFDrawImageByte(HIMAGE hImage, WORD left, WORD top, WORD right, WORD bottom)
{
	BYTE *ptrRead = (BYTE *)hImage;
	BYTE *ptrWrite;
	BYTE *ptrHeader;
	WORD totalColor;
	WORD wColorKey;
	WORD width,height;
	WORD outW,outH;
	WORD wPixelData;
	WORD dx=0,dy=0;
	DWORD dwSize;
	WORD i,j,bDrawMode=1;
	UTFOSDCOL color;
	UTFBITMAP bitmap;

	if(hImage == NULL)
		return;

	outW = right-left;
	outH = bottom-top;

	UTFMemCpyWORD(&width, ptrRead, 2);
	ptrRead += 2;
	UTFMemCpyWORD(&height, ptrRead, 2);
	ptrRead += 2;
	UTFMemCpyWORD(&totalColor, ptrRead, 2);	//该图片颜色总数
	ptrRead += 2;
	UTFMemCpyWORD(&wColorKey, ptrRead, 2);
	ptrRead += 2;

	if(outW > width)
	{
		outW = width;
	}
	if(outH > height)
	{
		outH = height;
	}

	UTFGetMemoryBitmap(&bitmap);
	if(bitmap.data)
	{
		bDrawMode = 0;
		dx = left;    
		dy = top;
		if((left+outW) > bitmap.Width) outW = bitmap.Width-left;
		if((top+outH) > bitmap.Height) outH = bitmap.Height-top;
	}
	else
	{
		dwSize = outW*outH;
		ptrHeader = (BYTE *)SOWMalloc(dwSize);
		if(ptrHeader == NULL)
			return;

		bitmap.data = ptrHeader;
		bitmap.Width = outW;
		bitmap.Height = outH;
		bitmap.bitPerPixel = 8;
		UTFSaveScreen(left, top, &bitmap, 0);
	}

	color.rgb.alpha = g_BMPAlpha;
	for(i=0; i<totalColor; i++)
	{
		color.rgb.red = *ptrRead++;
		color.rgb.green = *ptrRead++;
		color.rgb.blue = *ptrRead++;
		
		g_BMPPal[i] = g_FillPaletteFunc(color.data, i+g_iDrawColorIndex);
	}

	//UTFEnumPaletteProc(enum_palette);
	
	ptrWrite = bitmap.data+dy*bitmap.Width+dx;
	for(i=0; i<outH; i++)
	{
		for(j=0; j<outW; j++)
		{
			wPixelData = *(ptrRead+j);
			if(wPixelData != wColorKey)
			{
				*(ptrWrite+j) = (BYTE)(g_BMPPal[wPixelData]);				
			}
		}
		ptrRead += width;
		ptrWrite += bitmap.Width;
	}

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}

#endif

static void UTFDrawImageWORD(HIMAGE hImage, WORD left, WORD top, WORD right, WORD bottom)
{
	BYTE *ptrRead = (BYTE *)hImage;
	WORD *ptrWrite;
	BYTE *ptrHeader;
	DWORD ptrHeaderTemp;
	WORD *ptrReadTemp;
	WORD totalColor;
	WORD wColorKey;
//	WORD width,height;
	WORD outW,outH;
	WORD wPixelData;
	WORD dx=0,dy=0;
	DWORD dwSize;
	WORD i,j,bDrawMode=1;
	UTFOSDCOL color;
	UTFBITMAP bitmap;
	DWORD width = 0;
	DWORD height = 0;
	BYTE *pBits;
	PDRMPAL pPalette;
	if(hImage == NULL)
		return;
	
//	DG_AL_parse_img_header(hImage, &width, &height, &pBits, &pPalette);
	UTFParseImgHeader(hImage, &width, &height, &pBits);
//	DareDrawImageAdd(pBits,left,top,width, height);

	outW = right-left;
	outH = bottom-top;

	if(outW > width)
	{
		outW = width;
	}
	if(outH > height)
	{
		outH = height;
	}
	UTFGetMemoryBitmap(&bitmap);
	if(bitmap.data)
	{
		bDrawMode = 0;
		dx = left;
		dy = top;
		if((left+outW) > bitmap.Width) outW = bitmap.Width-left;
		if((top+outH) > bitmap.Height) outH = bitmap.Height-top;
/************************************************************/
		{
		#if MODIFY_COPY_RECT//vivian modified here 20080611
			WORD SrcStride, DstStride;
		#endif
			ptrWrite = (WORD *)bitmap.data;
			ptrReadTemp= (WORD *)pBits;
			ptrWrite += dy*bitmap.Width+dx;
		#if MODIFY_COPY_RECT//vivian modified 20080611 
			SrcStride = (width * bitmap.bitPerPixel)/8;
			DstStride = (bitmap.Width * bitmap.bitPerPixel)/8;
			GfxCopyRectMem(ptrWrite, ptrReadTemp, DstStride, SrcStride, outW, outH,
								0, 0, 0, 0, bitmap.bitPerPixel);
		#else
			for(i=0; i<outH; i++)
			{
				for(j=0; j<outW; j++)
				{
					*(ptrWrite+j)=*(ptrReadTemp+j);
				}
				ptrReadTemp += width;
				ptrWrite += bitmap.Width;
			}
				
		#endif
		}
		return ;
/************************************************************/
	}
	else
	{
	#if 0
		dwSize = outW*outH;
		ptrHeader = (BYTE *)SOWMalloc(dwSize*2+ADDWIDTH);
		if(ptrHeader == NULL)
			return;
		ptrHeaderTemp=(DWORD)ptrHeader;
		if(ptrHeaderTemp%ADDWIDTH)
			ptrHeaderTemp+=(ADDWIDTH-ptrHeaderTemp%ADDWIDTH);
		bitmap.data = (BYTE *)ptrHeaderTemp;//ptrHeader;//
		bitmap.Width = outW;
		bitmap.Height = outH;
		bitmap.bitPerPixel = 16;
		UTFSaveScreen(left, top, &bitmap, 0);
	#else
		bitmap.Width = width;
		bitmap.Height = height;
		bitmap.bitPerPixel = 16;
	#endif
	}
#if 0
	ptrWrite = (WORD *)bitmap.data;
	ptrReadTemp= (WORD *)pBits;

	ptrWrite += dy*bitmap.Width+dx;
	for(i=0; i<outH; i++)
	{
		for(j=0; j<outW; j++)
		{
			*(ptrWrite+j)=*(ptrReadTemp+j);
		}
		ptrReadTemp += width;
		ptrWrite += bitmap.Width;
	}
#else
	bitmap.data=pBits;
#endif
	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}

static void UTFDrawImageExByte(HIMAGE hImage, WORD left, WORD top, WORD right, WORD bottom, WORD xIMG, WORD yIMG, WORD wWidth, WORD wHeight)
{
	BYTE *ptrRead = (BYTE *)hImage;
	BYTE *ptrWrite;
	BYTE *ptrHeader;
	WORD totalColor;
	WORD wColorKey;
	WORD width,height;
	WORD wPixelData;
	WORD outW,outH;
	WORD dx=0,dy=0;
	WORD i,j,bDrawMode=1;
	DWORD dwSize;
	UTFOSDCOL color;
	UTFBITMAP bitmap;
	
	if(hImage == NULL)
		return;

	outW = right-left;
	outH = bottom-top;
	
	if(wWidth > outW)
	{
		wWidth = outW;
	}
	if(wHeight > outH)
	{
		wHeight = outH;
	}

	UTFMemCpyWORD(&width, ptrRead, 2);
	ptrRead += 2;
	UTFMemCpyWORD(&height, ptrRead, 2);
	ptrRead += 2;
	UTFMemCpyWORD(&totalColor, ptrRead, 2);
	ptrRead += 2;
	UTFMemCpyWORD(&wColorKey, ptrRead, 2);
	ptrRead += 2;

	if(xIMG >= width)
		return;
	else if(((xIMG+wWidth) > width))
		wWidth = width-xIMG;

	if(yIMG >= height)
		return;
	else if(((yIMG+wHeight) > height))
		wHeight = height-yIMG;

	if((wWidth == 0) || (wHeight == 0))
		return;

	UTFGetMemoryBitmap(&bitmap);
	if(bitmap.data)
	{
		bDrawMode = 0;
		dx = left;
		dy = top;
		if((left+wWidth) > bitmap.Width) wWidth = bitmap.Width-left;
		if((top+wHeight) > bitmap.Height) wHeight = bitmap.Height-top;
	}
	else
	{
		dwSize = wWidth*wHeight;
		ptrHeader = (BYTE *)SOWMalloc(dwSize);
		if(ptrHeader == NULL)
			return;

		bitmap.data = ptrHeader;
		bitmap.Width = wWidth;
		bitmap.Height = wHeight;
		bitmap.bitPerPixel = 8;
		UTFSaveScreen(left, top, &bitmap, 0);
	}

	color.rgb.alpha = g_BMPAlpha;
	for(i=0; i<totalColor; i++)
	{
		color.rgb.red = *ptrRead++;
		color.rgb.green = *ptrRead++;
		color.rgb.blue = *ptrRead++;
		
		g_BMPPal[i] = g_FillPaletteFunc(color.data, i+g_iDrawColorIndex);
	}

	ptrWrite = bitmap.data+dy*bitmap.Width+dx;
	ptrRead += (xIMG+yIMG*width);
	for(i=0; i<wHeight; i++)
	{
		for(j=0; j<wWidth; j++)
		{
			wPixelData = *(ptrRead+j);
			if(wPixelData != wColorKey)
			{
				*(ptrWrite+j) = (BYTE)(g_BMPPal[wPixelData]);
			}
		}
		ptrWrite += bitmap.Width;
		ptrRead += width;
	}

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}

static void UTFDrawImageExWORD(HIMAGE hImage, WORD left, WORD top, WORD right, WORD bottom, WORD xIMG, WORD yIMG, WORD wWidth, WORD wHeight)
{
	BYTE *ptrRead = (BYTE *)hImage;
	WORD *ptrWrite;
	BYTE *ptrHeader;
	WORD *ptrReadTemp;
	WORD totalColor;
	WORD wColorKey;
	WORD wPixelData;
	WORD outW,outH;
	WORD dx=0,dy=0;
	WORD i,j,bDrawMode=1;
	DWORD dwSize;
	UTFOSDCOL color;
	UTFBITMAP bitmap;
	
	DWORD width,height;
	BYTE *pBits;
	PDRMPAL pPalette;
#if MODIFY_COPY_RECT//vivian modified here 20080611
	WORD SrcStride, DstStride;
#endif

	if(hImage == NULL)
		return;
	
//	DG_AL_parse_img_header(hImage, &width, &height, &pBits, &pPalette);
	UTFParseImgHeader(hImage, &width, &height, &pBits);
	pBits = pBits + xIMG*2 + width*yIMG*2;

	outW = right-left;
	outH = bottom-top;
	
	if(wWidth > outW)
	{
		wWidth = outW;
	}
	if(wHeight > outH)
	{
		wHeight = outH;
	}
	
	if(xIMG >= width)
		return;
	else if(((xIMG+wWidth) > width))
		wWidth = width-xIMG;

	if(yIMG >= height)
		return;
	else if(((yIMG+wHeight) > height))
		wHeight = height-yIMG;

	if((wWidth == 0) || (wHeight == 0))
		return;

	UTFGetMemoryBitmap(&bitmap);
	if(bitmap.data)
	{
		bDrawMode = 0;
		dx = left;
		dy = top;
		if((left+wWidth) > bitmap.Width) wWidth = bitmap.Width-left;
		if((top+wHeight) > bitmap.Height) wHeight = bitmap.Height-top;
	}
	else
	{
		dwSize = wWidth*wHeight;
		ptrHeader = (BYTE *)SOWMalloc(dwSize<<1);
		if(ptrHeader == NULL)
			return;

		bitmap.data = ptrHeader;
		bitmap.Width = wWidth;
		bitmap.Height = wHeight;
		bitmap.bitPerPixel = 16;
		UTFSaveScreen(left, top, &bitmap, 0);
	}
	ptrReadTemp= (WORD *)pBits;
	ptrWrite = (WORD *)bitmap.data;
	ptrWrite += dy*bitmap.Width+dx;

#if MODIFY_COPY_RECT//vivian modified 20080611 
	SrcStride = (width * bitmap.bitPerPixel)/8;
	DstStride = (bitmap.Width * bitmap.bitPerPixel)/8;
	GfxCopyRectMem(ptrWrite, ptrReadTemp, DstStride, SrcStride, wWidth, wHeight,
						0, 0, 0, 0, bitmap.bitPerPixel);
#else
	for(i=0; i<wHeight; i++)
	{
		for(j=0; j<wWidth; j++)
		{
			*(ptrWrite+j)=*(ptrReadTemp+j);
		}
		ptrReadTemp += width;
		ptrWrite += bitmap.Width;
	}
#endif

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}
/**************************************************************************/

void UTFAPI UTFDrawGlueImage(HIMAGE hLImage, HIMAGE hCImage, HIMAGE hRImage, WORD xLeft, WORD yTop, WORD right, WORD bottom)
{
	WORD i, CBlockNum, CBlockWidth;
	WORD left = xLeft, width = right-xLeft;
	UTFIMAGEINFO LInfo,CInfo,RInfo;
	WORD Offset;
	if((hLImage == NULL) || (hCImage == NULL) || (hRImage == NULL))
		return;
	
	UTFGetImageInfo(hLImage, &LInfo);
	UTFGetImageInfo(hCImage, &CInfo);
	UTFGetImageInfo(hRImage, &RInfo);
	
#if 0
	if((LInfo.dwWidth+CInfo.dwWidth+RInfo.dwWidth) > width)
		return;

	UTFDrawImage(hLImage, left, yTop, left+LInfo.dwWidth, bottom);
	left += LInfo.dwWidth;

	CBlockWidth = width-LInfo.dwWidth-RInfo.dwWidth;
	CBlockNum = CBlockWidth/CInfo.dwWidth;

	// Draw center bitmap
	for(i=0; i<CBlockNum; i++)
	{
		UTFDrawImage(hCImage, left, yTop, left+CInfo.dwWidth, bottom);
		left += CInfo.dwWidth;
	}

	if(CBlockWidth%CInfo.dwWidth)
	{
		left = right-RInfo.dwWidth-CInfo.dwWidth;
		UTFDrawImage(hCImage, left, yTop, left+CInfo.dwWidth, bottom);
	}

	
	left = right - RInfo.dwWidth;
	UTFDrawImage(hRImage, left, yTop, right, bottom);
#else
	/*周磊08-06-02  让进度条显示更精确*/
	if (width==0) 
	{
		return;
	}
	if(width<LInfo.dwWidth)		//从左边开始画，有多少画多少
	{
	#if 1//vivian modified 20080612
		UTFDrawImageEx(hLImage, left, yTop,  left+width, bottom, 0, 0, width, LInfo.dwHeight);
	#else
		UTFDrawImage(hLImage, left, yTop, left+width, bottom);
	#endif
		return;
	}
	else
	{
		UTFDrawImage(hLImage, left, yTop, left+LInfo.dwWidth, bottom);
	}
	left += LInfo.dwWidth;

	if(width-LInfo.dwWidth<RInfo.dwWidth)	//不足够画右边时就只画中间
		CBlockWidth = width-LInfo.dwWidth;
	else
		CBlockWidth = width-LInfo.dwWidth-RInfo.dwWidth;
	CBlockNum = CBlockWidth/CInfo.dwWidth;

	// Draw center bitmap
	for(i=0; i<CBlockNum; i++)
	{
		UTFDrawImage(hCImage, left, yTop, left+CInfo.dwWidth, bottom);
		left += CInfo.dwWidth;
	}

	Offset = CBlockWidth % CInfo.dwWidth;
	if(Offset)		//对于中间也是有多少画多少
	{
	#if 1//vivian modified 20080612
		UTFDrawImageEx(hCImage, left, yTop,left+Offset, bottom, 0, 0, Offset, CInfo.dwHeight);
	#else
		UTFDrawImage(hCImage, left, yTop, left+CBlockWidth%CInfo.dwWidth, bottom);
	#endif
	}

	if(width-LInfo.dwWidth>=RInfo.dwWidth)
	{
		left = right - RInfo.dwWidth;
		UTFDrawImage(hRImage, left, yTop, right, bottom);
	}
#endif
}

void UTFAPI UTFDrawGlueImageV(HIMAGE hUpImage, HIMAGE hCImage, HIMAGE hDnImage, WORD xLeft, WORD yTop, WORD right, WORD bottom)
{
	WORD i, CBlockNum, CBlockHeight;
	WORD top = yTop, height = bottom-yTop+1;
	UTFIMAGEINFO UInfo,CInfo,DInfo;

	if((hUpImage == NULL) || (hCImage == NULL) || (hDnImage == NULL))
		return;
	
	UTFGetImageInfo(hUpImage, &UInfo);
	UTFGetImageInfo(hCImage, &CInfo);
	UTFGetImageInfo(hDnImage, &DInfo);

	if((UInfo.dwHeight+CInfo.dwHeight+DInfo.dwHeight) > height)
		return;

	UTFDrawImage(hUpImage, xLeft, top, right, top+UInfo.dwHeight);
	top += UInfo.dwHeight;

	CBlockHeight = height-UInfo.dwHeight-DInfo.dwHeight;
	CBlockNum = CBlockHeight/CInfo.dwHeight;

	// Draw center bitmap
	for(i=0; i<CBlockNum; i++)
	{
		UTFDrawImage(hCImage, xLeft, top, right, top+CInfo.dwHeight);
		top += CInfo.dwHeight;
	}

	if(CBlockHeight%CInfo.dwHeight)
	{
		top = bottom-DInfo.dwHeight-CInfo.dwHeight;
		UTFDrawImage(hCImage, xLeft, top, right, top+CInfo.dwHeight);
	}

	top = bottom-DInfo.dwHeight;
	UTFDrawImage(hDnImage, xLeft, top, right, bottom);
}

void UTFAPI UTFTileImage(HIMAGE hImage, WORD xLeft, WORD yTop, WORD right, WORD bottom)
{
	UTFIMAGEINFO Info;
	WORD width = right-xLeft+1;
	WORD i, blockNum;
	WORD left = xLeft;

	if(hImage == NULL)
		return;

	UTFGetImageInfo(hImage, &Info);
	if(Info.dwWidth > width)
	{
		UTFDrawImage(hImage, xLeft, yTop, right, bottom);
		return;
	}

	if(Info.dwWidth == 0)
		return;

	blockNum = width/Info.dwWidth;

	// Draw bitmap
	for(i=0; i<blockNum; i++)
	{
		UTFDrawImage(hImage, left, yTop, left+Info.dwWidth, bottom);
		left += Info.dwWidth;
	}
	
	if(width%Info.dwWidth)
	{
		left = right-Info.dwWidth;
		UTFDrawImage(hImage, left, yTop, right, bottom);
	}
}

void UTFAPI UTFTileImageV(HIMAGE hImage, WORD xLeft, WORD yTop, WORD right, WORD bottom)
{
	UTFIMAGEINFO Info;
	WORD height = bottom-yTop+1;
	WORD i, blockNum;
	WORD top = yTop;

	if(hImage == NULL)
		return;

	UTFGetImageInfo(hImage, &Info);
	if(Info.dwHeight > height)
	{
		UTFDrawImage(hImage, xLeft, yTop, right, bottom);
		return;
	}

	if(Info.dwHeight == 0)
		return;

	blockNum = height/Info.dwHeight;

	// Draw bitmap
	for(i=0; i<blockNum; i++)
	{
		UTFDrawImage(hImage, xLeft, top, right, top+Info.dwHeight);
		top += Info.dwHeight;
	}
	
	if(height%Info.dwHeight)
	{
		top = bottom-Info.dwHeight;
		UTFDrawImage(hImage, xLeft, top, right, bottom);
	}
}

void UTFAPI UTFDrawImageProgBar(HIMAGE hImage, DWORD curPos, DWORD maxPos, BYTE bSpace, UTFCOLOR backColor, WORD xLeft, WORD yTop, WORD right, WORD bottom, char bVertical )
{
	UTFIMAGEINFO Info;
	WORD oneStepWidth, progWidth;
	WORD left, top;
	WORD blockNum, i;

	if((hImage == NULL) || (maxPos == 0))
		return;

	if(curPos > maxPos)
	{
		curPos = maxPos;
	}

	UTFGetImageInfo(hImage, &Info);

	UTFSetPen(PSUI_NULL, 0, 1);
	UTFSetBrush(BSUI_SOLID, backColor, 0);
	
	if(bVertical == FALSE)
	{
		progWidth = curPos*(right-xLeft)/maxPos;
		oneStepWidth = Info.dwWidth+bSpace;
		blockNum = progWidth/oneStepWidth;
		if(Info.dwHeight < (bottom-yTop))
		{
			bottom = yTop+Info.dwHeight;
		}

		left = xLeft;
		for(i=0; i<blockNum; i++)
		{
			UTFDrawImage(hImage, left, yTop, left+Info.dwWidth, bottom);
			UTFFillRect(left+Info.dwWidth, yTop, left+oneStepWidth, bottom);
			left += oneStepWidth;
		}

		if(bSpace == 0)
		{
			left = xLeft+progWidth;
			if(progWidth%oneStepWidth)
			{
				UTFDrawImage(hImage, left-Info.dwWidth, yTop, left, bottom);
			}				
		}
		else
		{
			if((left+Info.dwWidth) <= (xLeft+progWidth))
			{
				UTFDrawImage(hImage, left, yTop, left+Info.dwWidth, bottom);
				left += Info.dwWidth;
			}
			else
			{
				left -= bSpace;
			}
		}

		UTFFillRect(left, yTop, right, bottom);
	}
	else
	{
		progWidth = curPos*(bottom-yTop)/maxPos;
		oneStepWidth = Info.dwHeight+bSpace;
		blockNum = progWidth/oneStepWidth;
		if(Info.dwWidth < (right-xLeft))
		{
			right = xLeft+Info.dwWidth;
		}

		top = bottom+bSpace;
		for(i=0; i<blockNum; i++)
		{
			top -= oneStepWidth;
			UTFDrawImage(hImage, xLeft, top, right, top+Info.dwHeight);
			UTFFillRect(xLeft, top-bSpace, right, top);
		}

		if(bSpace == 0)
		{
			top = bottom-progWidth;
			if(progWidth%oneStepWidth)
			{
				UTFDrawImage(hImage, xLeft, top, right, top+Info.dwHeight);
			}				
		}
		else
		{
			if((top-oneStepWidth) >= (bottom-progWidth))
			{
				top -= oneStepWidth;
				UTFDrawImage(hImage, xLeft, top, right, top+Info.dwHeight);
			}
		}

		UTFFillRect(xLeft, yTop, right, top);
	}
}

static void UTFSetPenFunc(void)
{
	switch(g_LogPen.lopnStyle)
	{
	case PSUI_SOLID:
		g_DrawFunc.drawPolygon = UTFDrawPolygonSolid;
		g_DrawFunc.drawRoundRect = UTFDrawRoundRectSolid;
		g_DrawFunc.drawRect = UTFDrawRectSolid;
		g_DrawFunc.drawEllipse = UTFDrawEllipseSolid;
		g_DrawFunc.drawLine = UTFDrawLineSolid;
		break;

	case PSUI_DASH:
		g_DrawFunc.drawPolygon = NULL;
		g_DrawFunc.drawRoundRect = NULL;
		g_DrawFunc.drawRect = NULL;
		g_DrawFunc.drawEllipse = NULL;
		g_DrawFunc.drawLine = NULL;
		break;

	case PSUI_DOT:
		g_DrawFunc.drawPolygon = NULL;
		g_DrawFunc.drawRoundRect = NULL;
		g_DrawFunc.drawRect = NULL;
		g_DrawFunc.drawEllipse = NULL;
		g_DrawFunc.drawLine = NULL;
		break;

	case PSUI_DASHDOT:
		g_DrawFunc.drawPolygon = NULL;
		g_DrawFunc.drawRoundRect = NULL;
		g_DrawFunc.drawRect = NULL;
		g_DrawFunc.drawEllipse = NULL;
		g_DrawFunc.drawLine = NULL;
		break;

	case PSUI_DASHDOTDOT:
		g_DrawFunc.drawPolygon = NULL;
		g_DrawFunc.drawRoundRect = NULL;
		g_DrawFunc.drawRect = NULL;
		g_DrawFunc.drawEllipse = NULL;
		g_DrawFunc.drawLine = NULL;
		break;

	case PSUI_NULL:
		g_DrawFunc.drawPolygon = NULL;
		g_DrawFunc.drawRoundRect = NULL;
		g_DrawFunc.drawRect = NULL;
		g_DrawFunc.drawEllipse = NULL;
		g_DrawFunc.drawLine = NULL;
		break;

	default:
		g_DrawFunc.drawPolygon = NULL;
		g_DrawFunc.drawRoundRect = NULL;
		g_DrawFunc.drawRect = NULL;
		g_DrawFunc.drawEllipse = NULL;
		g_DrawFunc.drawLine = NULL;
		break;
	}
}

static void UTFResetBrushFunc(void)
{
	switch(g_LogBrush.lbStyle)
	{
	case BSUI_SOLID:
		g_DrawFunc.fillRect = UTFFillRectSolid;
		g_DrawFunc.fillEllipse = UTFFillEllipseSolid;
		g_DrawFunc.fillRoundRect = UTFFillRoundRectSolid;
		g_DrawFunc.fillPolygon = UTFFillPolygonSolid;
		break;

	case BSUI_PATTERN:
		g_DrawFunc.fillRect = NULL;
		g_DrawFunc.fillEllipse = NULL;
		g_DrawFunc.fillRoundRect = NULL;
		g_DrawFunc.fillPolygon = NULL;
		break;

	case BSUI_NULL:
		g_DrawFunc.fillRect = NULL;
		g_DrawFunc.fillEllipse = NULL;
		g_DrawFunc.fillRoundRect = NULL;
		g_DrawFunc.fillPolygon = NULL;
		break;

	case BSUI_HATCHED:
		switch(g_LogBrush.lbHatch)
		{
		case HSUI_BDIAGONAL:
			g_DrawFunc.fillRect = NULL;
			g_DrawFunc.fillEllipse = NULL;
			g_DrawFunc.fillRoundRect = NULL;
			g_DrawFunc.fillPolygon = NULL;
			break;

		case HSUI_CROSS:
			g_DrawFunc.fillRect = NULL;
			g_DrawFunc.fillEllipse = NULL;
			g_DrawFunc.fillRoundRect = NULL;
			g_DrawFunc.fillPolygon = NULL;
			break;

		case HSUI_DIAGCROSS:
			g_DrawFunc.fillRect = NULL;
			g_DrawFunc.fillEllipse = NULL;
			g_DrawFunc.fillRoundRect = NULL;
			g_DrawFunc.fillPolygon = NULL;
			break;

		case HSUI_FDIAGONAL:
			g_DrawFunc.fillRect = NULL;
			g_DrawFunc.fillEllipse = NULL;
			g_DrawFunc.fillRoundRect = NULL;
			g_DrawFunc.fillPolygon = NULL;
			break;

		case HSUI_HORIZONTAL:
			g_DrawFunc.fillRect = NULL;
			g_DrawFunc.fillEllipse = NULL;
			g_DrawFunc.fillRoundRect = NULL;
			g_DrawFunc.fillPolygon = NULL;
			break;

		case HSUI_VERTICAL:
			g_DrawFunc.fillRect = NULL;
			g_DrawFunc.fillEllipse = NULL;
			g_DrawFunc.fillRoundRect = NULL;
			g_DrawFunc.fillPolygon = NULL;
			break;

		default:
			g_DrawFunc.fillRect = NULL;
			g_DrawFunc.fillEllipse = NULL;
			g_DrawFunc.fillRoundRect = NULL;
			g_DrawFunc.fillPolygon = NULL;
			break;
		}
		break;

	default:
		g_DrawFunc.fillRect = NULL;
		g_DrawFunc.fillEllipse = NULL;
		g_DrawFunc.fillRoundRect = NULL;
		g_DrawFunc.fillPolygon = NULL;
		break;
	}
}

void UTFAPI UTFSetPen(DWORD lopnStyle, UTFCOLOR lopnColor, WORD lopnWidth)
{
	g_LogPen.lopnStyle = lopnStyle;

	if(g_LogPen.lopnStyle != PSUI_NULL)
	{
		g_LogPen.lopnColor = g_FillPaletteFunc(lopnColor, g_iDrawColorIndex);
		g_LogPen.lopnWidth = lopnWidth;
	}

	UTFSetPenFunc();
}

void UTFAPI UTFSetBrush(DWORD lbStyle, UTFCOLOR lbColor, DWORD lbHatch)
{
	g_LogBrush.lbStyle = lbStyle;
	g_LogBrush.lbHatch = lbHatch;
	
	if(lbStyle != BSUI_NULL)
	{
		g_LogBrush.lbColor = g_FillPaletteFunc(lbColor, g_iDrawColorIndex);
	}	

	UTFResetBrushFunc();
}

void UTFAPI UTFGetOSDRgnRect(LPUTFRECT pRect)
{
	if(pRect != NULL)
	{
		memcpy(pRect, &g_OSDRgnRect, sizeof(UTFRECT));
	}
}

void UTFAPI UTFGetFullScreenRect(LPUTFRECT pRect)
{
	if(pRect != NULL)
	{
		memcpy(pRect, &g_FullScreenRect, sizeof(UTFRECT));
	}
}

int UTFGetColorIndex(void)
{
	return g_iDrawColorIndex;
}

UTFCOLOR UTFFillPaletteFunc(UTFCOLOR color, BYTE bColorIndex)
{
	return g_FillPaletteFunc(color, bColorIndex);
}

void UTFSetScreenDraw(int bNotMemoryDraw)
{
	g_bNotMemoryDraw = bNotMemoryDraw;
}

int UTFGetScreenDraw(void)
{
	return g_bNotMemoryDraw;
}

void UTFDrawAPISetColorMode(BYTE bitPerPixel)
{
	if(bitPerPixel == 8)
	{
		g_DrawFunc.drawImage = UTFDrawImageByte;
#if defined(OSDUTFDRAMIMGEX) && (OSDUTFDRAMIMGEX == YES)
		g_DrawFunc.drawImageEx = UTFDrawImageExByte;
#endif
	}
	if(bitPerPixel == 16)
	{
		g_DrawFunc.drawImage = UTFDrawImageWORD;
#if defined(OSDUTFDRAMIMGEX) && (OSDUTFDRAMIMGEX == YES)
		g_DrawFunc.drawImageEx = UTFDrawImageExWORD;
#else
		g_DrawFunc.drawImageEx = NULL;//UTFDrawBitmapWord;
#endif
	}
}

void UTFDrawAPIInit(void)
{
	g_CleanColor = UTFRGB(0,0,0,0);

	g_LogBrush.lbColor = UTFFillPalette(UTFRGB(0,0,0,0), 0);
	g_LogBrush.lbStyle = BSUI_SOLID;
	g_LogBrush.lbHatch = HSUI_CROSS;

	UTFResetBrushFunc();

	g_LogPen.lopnColor = UTFFillPalette(UTFRGB(0,0,0,255), 0);
	g_LogPen.lopnStyle = PSUI_SOLID;
	g_LogPen.lopnWidth = 1;
	
	UTFSetPenFunc();

	if(g_iDrawColorIndex < 0)
		g_FillPaletteFunc = UTFFillPalette;
	else
		g_FillPaletteFunc = UTFSetIndexColor;
}


/****************  modify drawing picture********************************/
void SetFillRectColor(BYTE r, BYTE g, BYTE b, BYTE alpha)
{
	g_FillBrush.color.a=alpha;
	g_FillBrush.color.r=r;
	g_FillBrush.color.g=g;
	g_FillBrush.color.b=b;
}

DWORD GetFillRectColor()
{
	return g_FillBrush.wData;
}

int  UTFParseImgHeader(DWORD hImg, DWORD *pWidth,  DWORD *pHeight, BYTE **ppBits)
{
	LPUTFIMAGEFILE pImage=(LPUTFIMAGEFILE)hImg;
	WORD	nHeight, nWidth;
//-----------8 bit bmp----zhoulei 08-10-22
	BYTE *img = (BYTE*)hImg;

	*pWidth =  (img[1]<<8) | (img[0]);
	*pHeight = (img[3]<<8) | (img[2]);
	
	return 0;
//----------------	
	#if 1			//zhoulei changed
	
	
	if (pImage)
	{
		
		nHeight = pImage->dwHeight;
		nWidth  = pImage->dwWidth;
		*pHeight = nHeight;
		*pWidth  = nWidth;
		*ppBits  = (BYTE*)(&(pImage->dwPalette[pImage->dwNumPalEntries]));
	}
	#else
	//-----------------------//zhoulei changed--------------------------
	BYTE *pImage = (BYTE*)hImg;
	WORD	nHeight, nWidth;
	DWORD dwPalEntries;

	if (pImage)
	{
		nWidth  = pImage[11]<<8;
		nWidth |= pImage[10];
		nHeight = pImage[13]<<8;
		nHeight |=pImage[12];		
		*pHeight = nHeight;
		*pWidth  = nWidth;
		dwPalEntries = pImage[19]<<24;
		dwPalEntries |= pImage[18]<<16;
		dwPalEntries |= pImage[17]<<8;
		dwPalEntries |= pImage[16];
		*ppBits  = (BYTE*)(&(pImage[20+4*dwPalEntries]));
	}
	//----------------------------------------------------
	#endif
	
	else
	{
		return -1;
	}
	return 0;
}

void UTFAPI UTFDrawBmp1555(BYTE  *pBits, DWORD width,DWORD height, WORD left, WORD top, WORD right, WORD bottom)
{
	WORD *ptrWrite;
	BYTE *ptrHeader;
	DWORD ptrHeaderTemp;
	WORD *ptrReadTemp;
	WORD totalColor;
	WORD wColorKey;
//	WORD width,height;
	WORD outW,outH;
	WORD wPixelData;
	WORD dx=0,dy=0;
	DWORD dwSize;
	WORD i,j,bDrawMode=1;
	UTFOSDCOL color;
	UTFBITMAP bitmap;
#if MODIFY_COPY_RECT//vivian modified here 20080611
	WORD SrcStride, DstStride;
#endif


	outW = right-left;
	outH = bottom-top;

	if(outW > width)
	{
		outW = width;
	}
	if(outH > height)
	{
		outH = height;
	}

	UTFGetMemoryBitmap(&bitmap);
	if(bitmap.data)
	{
		bDrawMode = 0;
		dx = left;
		dy = top;
		if((left+outW) > bitmap.Width) outW = bitmap.Width-left;
		if((top+outH) > bitmap.Height) outH = bitmap.Height-top;
	}
	else
	{
		dwSize = outW*outH;
		ptrHeader = (BYTE *)SOWMalloc(dwSize*2+ADDWIDTH);
		if(ptrHeader == NULL)
			return;
		ptrHeaderTemp=(DWORD)ptrHeader;
		if(ptrHeaderTemp%ADDWIDTH)
			ptrHeaderTemp+=(ADDWIDTH-ptrHeaderTemp%ADDWIDTH);
		bitmap.data = (BYTE *)ptrHeaderTemp;//ptrHeader;//
		bitmap.Width = outW;
		bitmap.Height = outH;
		bitmap.bitPerPixel = 16;
		UTFSaveScreen(left, top, &bitmap, 0);
	}

	ptrWrite = (WORD *)bitmap.data;
	ptrReadTemp= (WORD *)pBits;

	ptrWrite += dy*bitmap.Width+dx;

#if MODIFY_COPY_RECT//vivian modified 20080611 
	SrcStride = (width * bitmap.bitPerPixel)/8;
	DstStride = (bitmap.Width * bitmap.bitPerPixel)/8;
	GfxCopyRectMem(ptrWrite, ptrReadTemp, DstStride, SrcStride, outW, outH,
						0, 0, 0, 0, bitmap.bitPerPixel);
#else
	for(i=0; i<outH; i++)
	{
		for(j=0; j<outW; j++)
		{
			*(ptrWrite+j)=*(ptrReadTemp+j);
		}
		ptrReadTemp += width;
		ptrWrite += bitmap.Width;
	}
#endif
	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}


void UTFDrawImageJpeg(void *image,unsigned long imagesize,int left,int top,int right,int bottom)
{
	void *cur_bmp;

	if(0==UTF_jpeg_decode(image,imagesize,0,&cur_bmp))
		return ;
	change24bitmapto16bitmap(cur_bmp,left,top);
	if(cur_bmp)
		mem_free(cur_bmp);
	return ;
}

