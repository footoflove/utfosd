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
#include "UTFGraphPort.h"
#include "UTFGraphPortPriv.h"
#include "UTFDrawAPI.h"
#include "UTFMath.h"
#include "UTFMemoryPriv.h"

/*
#include "basetype.h"
#include "gfxtypes.h"
#include "osdlibc.h"
#include "retcodes.h"
#include "iofuncs.h"
*/
#include "string.h"
#include "libra.h"
#include "gfxtypes.h"
#include "iofuncs.h"
#include "dgbasetype.h"
#include "dgos.h"
#include "dggdiapi.h"
//extern OSDHANDLE mwom_hscreen;
//extern GFX_BITMAP    mwom_screen_bitmap;

/******************************************************************/
typedef struct
{ 
	int x1;
	int y1;
	int x2;
	int y2;
}RECT;

typedef struct
{
	void ( *drawPixel )			(WORD x, WORD y, UTFOSDCOL color);
	void ( *drawLine )			(WORD x1, WORD y1, WORD x2, WORD y2, WORD penWidth, UTFOSDCOL color);
	void ( *drawRect )			(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color);
	void ( *fillRect )			(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color);
	void ( *drawEllipse )		(WORD xx1, WORD yy1, WORD xx2, WORD yy2, WORD penWidth, UTFOSDCOL color);
	void ( *fillEllipse )		(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color);
	void ( *fillPolygon)		(LPUTFPOINT lpPoint, WORD nPoint, UTFOSDCOL color);
	void ( *drawRoundRect )		(WORD left, WORD top, WORD right, WORD bottom, BYTE radiu, UTFOSDCOL color);
	void ( *fillRoundRect )		(WORD left, WORD top, WORD right, WORD bottom, BYTE radiu, UTFOSDCOL color);
	int	 ( *bitblt )			(LPUTFBITMAP lpSrcBmp, WORD wX, WORD wY, WORD wWidth, WORD wHeight, LPUTFBITMAP lpDstBmp, WORD wLeft, WORD wTop);
}UTFGRAPHFUNC;

static UTFGRAPHFUNC g_GraphProc;
static BYTE g_bUpdateScreenFlag;
static UTFOSDInfo g_OSDInfo;
static UTFBITMAP g_GlobalBitmap;
static UTFBITMAP g_GlobalBitmapBackup;
static DWORD tran_col=0xFFE848D8;//0xff000000;
int g_IsMemoryDraw = 0;
#define RC_OK 0

/******************************************************************/
void *SOWMallocBitmap(DWORD size);

void UTFSetTranColor(DWORD drtran_col)
{
	tran_col=drtran_col;
}


DWORD Change5551DataTo8888Data(WORD wData)
{
	UTFOSDCOL color;
	color.rgb.alpha = ((wData>>15)==0)?0:255;
	color.rgb.red = (wData&0x001f)<<3;
	color.rgb.green = ((wData>>5)&0x001f)<<3;
	color.rgb.blue =  ((wData>>10)&0x001f)<<3;

	return color.data;
}
void UTFSetPalette(BYTE id, DWORD color)
{
	DG_AL_DRMPAL pal;

	pal.dwVal = color;
	DG_AL_set_osd_rgn_palentry(DG_AL_get_osd_hscreen(), (D_UINT32)id, pal);

//	SetOSDRgnPalEntry(mwom_hscreen, id, pal);
}

void mwom_set_osd_alpha(BYTE alpha)
{
	
}

void copy_memory_bitmap_to_screen()
{
//#if (defined(UTF_MOMERY_BITMAP) && (UTF_MOMERY_BITMAP == TRUE))
if(g_IsMemoryDraw)
	UTFPutScreen(0 ,0, &g_GlobalBitmapBackup, 0);
//#endif
}
/*
void mwom_pixel(int x,int y,DWORD color)
{
	GFX_OP gfxOp;
	GFX_XY xy[2];
	DWORD oldColor;

	mwom_get_foreground_color(&oldColor);
	mwom_set_foreground_color(color);

	xy[0].X=x;
	xy[0].Y=y;
	xy[1].X=x;
	xy[1].Y=y;

	gfxOp.ROP        = GFX_ROP_COPY;
	gfxOp.BltControl = GFX_OP_COPY;
	gfxOp.AlphaUse   = 0;

	GfxPolyLine(&mwom_screen_bitmap,xy,2,&gfxOp);

	mwom_set_foreground_color(oldColor); 
}
void mwom_polygon(PGFX_XY lpPoints, DWORD nPoints, DWORD color)
{
	GFX_OP gfxOp;
	DWORD oldColor;

	mwom_get_foreground_color(&oldColor);
	mwom_set_foreground_color(color);

	gfxOp.ROP        = GFX_ROP_COPY;
	gfxOp.BltControl = GFX_OP_COPY;
	gfxOp.AlphaUse   = 0;

	GfxPolyLine(&mwom_screen_bitmap,lpPoints,nPoints,&gfxOp);

	mwom_set_foreground_color(oldColor); 
}


void mwom_rect_draw(int x1,int y1,int x2,int y2,DWORD color)
{
	GFX_OP gfxOp;
	GFX_XY xy[5];
	DWORD oldColor;

	if((x1==x2) || (y1==y2))
		return;

	mwom_get_foreground_color(&oldColor);
	mwom_set_foreground_color(color);

	xy[0].X=x1;
	xy[0].Y=y1;
	xy[1].X=x2;
	xy[1].Y=y1;
	xy[2].X=x2;
	xy[2].Y=y2;
	xy[3].X=x1;
	xy[3].Y=y2;
	xy[4].X=x1;
	xy[4].Y=y1;
		
	gfxOp.ROP        = GFX_ROP_COPY;
	gfxOp.BltControl = GFX_OP_COPY;
	gfxOp.AlphaUse   = 0;

	GfxPolyLine(&mwom_screen_bitmap,xy,5,&gfxOp);

	mwom_set_foreground_color(oldColor); 
}
*/

void blt_image_to_osd(BYTE *pdata, WORD srcX, WORD srcY, WORD uWidth, WORD uHeight, WORD screenX, WORD screenY)
{
	DG_AL_GFX_BITMAP 	gfxPhoto;
	DG_AL_GFX_BITMAP 	gfxDest;
	DG_AL_GFX_OP     	gfxOp;
	DG_AL_GFX_RECT   	gfxRect;
	DG_AL_GFX_XY     	gfxXY;
//	DWORD 	dwAddr;
	DWORD oldcolor;//,color;
	int i;

	if(pdata == NULL)
		return;

//	dwAddr = (DWORD)pdata;
//	if(dwAddr%8) dwAddr += 8-dwAddr%8;
//	pdata = (BYTE *)dwAddr;
#if 0
	gfxPhoto.Version  = 0;
	gfxPhoto.VerSize  = 0;
	gfxPhoto.Type     = GFX_ARGB8;
	gfxPhoto.Bpp      = 8;
	gfxPhoto.Height   = uHeight;
	gfxPhoto.Width    = uWidth;
	gfxPhoto.Stride   = uWidth;
	gfxPhoto.dwRef    = 0;
	gfxPhoto.pBits    = pdata;
	gfxPhoto.pPalette = NULL; 

	printf("\n");
	printf("\n");
	printf("\n");
//	gfxDest=mwom_screen_bitmap;
	for(i=0;i<20;i++)
	{
		printf("0x%02x\n",pdata[i]);
	}
	printf("\n");
	printf("\n");
	printf("\n");
#endif

	memcpy(&gfxPhoto,DG_AL_get_osd_screen_bitmap(),sizeof(DG_AL_GFX_BITMAP));		
	gfxPhoto.pBits = pdata;
	gfxPhoto.Height = uHeight;
	gfxPhoto.Width = uWidth;
	gfxPhoto.Stride    = uWidth;
	memcpy(&gfxDest,DG_AL_get_osd_screen_bitmap(),sizeof(DG_AL_GFX_BITMAP));	
	gfxDest.Height    = 576;
	gfxDest.Width     = 720;
//	gfxDest.Stride    = uWidth;
//	gfxDest.dwRef     = 0;

	gfxRect.Left    = 0;
	gfxRect.Top     = 0;
	gfxRect.Right   = uWidth;
	gfxRect.Bottom  = uHeight;
	
	gfxXY.X         = screenX;
	gfxXY.Y         = screenY;

	gfxOp.AlphaUse  = 0;//0;
	gfxOp.Alpha     = 0;
	gfxOp.ROP       =GFX_ROP_COPY;//X_ROP_OR;
	gfxOp.BltControl =GFX_OP_COPY;//X_OP_COPY|GFX_OP_PRESERVE_ALPHA;//GFX_OP_COPY;//
//		gfxOp.AlphaUse  = 0x40;
//		gfxOp.Alpha       = 255;    // Hacked to prevent alpha from affecting anything
//		gfxOp.ROP         = GFX_ROP_OR;
//		gfxOp.BltControl = /*GFX_OP_TRANS*/GFX_OP_COPY|GFX_OP_PRESERVE_ALPHA;
//
	/*

	gfxOp.AlphaUse  = 0x40;
	gfxOp.Alpha       = 0;    // Hacked to prevent alpha from affecting anything
	gfxOp.ROP         = GFX_ROP_COPY;
	gfxOp.BltControl = GFX_OP_TRANS;//GFX_OP_COPY|GFX_OP_PRESERVE_ALPHA;
	cnxt_iofuncs_get_background_color(&oldcolor); 
	color.sColor.rgb = 0xff0000c8;//需要过滤的颜色
	cnxt_iofuncs_set_background_color(&color);
*/
	//cnxt_iofuncs_get_background_color(&oldcolor); 
//--	oldcolor=DG_AL_get_background_color();
	//color.sColor.rgb = tran_col;//0xff000000;//需要过滤的颜色
	//cnxt_iofuncs_set_background_color(&color);
//--	DG_AL_set_background_color((D_UINT32)tran_col);

//	GfxCopyBlt(&gfxPhoto, &gfxRect, &gfxDest, &gfxXY, &gfxOp);
	DG_AL_gfx_copy_blt(&gfxPhoto, &gfxRect, &gfxDest, &gfxXY, &gfxOp);
	while(DG_AL_gfx_isGXA_cmd_done(100) != RC_OK);
//--	DG_AL_gfx_isGXA_cmd_done(100);//while(DG_AL_gfx_isGXA_cmd_done(100) != RC_OK);
//--	DG_AL_set_background_color(oldcolor);
	//cnxt_iofuncs_set_background_color(&oldcolor);
}

void blt_osd_to_memory(BYTE *pdata, DWORD x, DWORD y, DWORD w, DWORD h)
{
	DG_AL_GFX_OP		gfxOp;
	DG_AL_GFX_RECT	gfxRect;
	DG_AL_GFX_XY		gfxXY;
	DG_AL_GFX_BITMAP mem_bitmap;
//	DWORD 	dwAddr;

	//	dwAddr = (DWORD)pdata;
	//	if(dwAddr%8) dwAddr += 8-dwAddr%8;
	//	pdata = (BYTE *)dwAddr;

	gfxRect.Left    = x;
	gfxRect.Top     = y;
	gfxRect.Right   = x+w;
	gfxRect.Bottom  = y+h;

	gfxXY.X         = 0;
	gfxXY.Y         = 0;

	//		gfxOp.AlphaUse  = 0x40;
	//		gfxOp.Alpha       = 255;    // Hacked to prevent alpha from affecting anything
	//		gfxOp.ROP         = GFX_ROP_OR;
	//		gfxOp.BltControl = /*GFX_OP_TRANS*/GFX_OP_COPY|GFX_OP_PRESERVE_ALPHA;
	gfxOp.AlphaUse  = 0;//0;
	gfxOp.Alpha     = 255;
	gfxOp.ROP       =GFX_ROP_COPY;//X_ROP_OR;
	gfxOp.BltControl =GFX_OP_COPY|GFX_OP_PRESERVE_ALPHA;//GFX_OP_COPY;//GFX_OP_TRANS;//

	//	mem_bitmap = mwom_screen_bitmap;
	memcpy(&mem_bitmap,DG_AL_get_osd_screen_bitmap(),sizeof(DG_AL_GFX_BITMAP));
	mem_bitmap.Height  = h;
	mem_bitmap.Width   = w;
	mem_bitmap.Stride  = w;
	mem_bitmap.dwRef   = 0;
	//mem_bitmap.pPalette = NULL;
	mem_bitmap.pBits    = pdata;

	DG_AL_gfx_copy_blt(DG_AL_get_osd_screen_bitmap(), &gfxRect, &mem_bitmap, &gfxXY, &gfxOp);
	while(DG_AL_gfx_isGXA_cmd_done(100) != RC_OK);
}
/*
int UTFGraphInitialize(void)
{
	//Initialize OSD information
	g_OSDInfo.bitPerPixel = 8;
	g_OSDInfo.bGraphMode = UTF_DEPTH_8;
	g_OSDInfo.MaxBitNum = 8;
	g_OSDInfo.dwStyle = 0;
	
	g_bUpdateScreenFlag = 0;

	//Initialize global bitmap data
	g_GlobalBitmap.Width = 0;
	g_GlobalBitmap.Height = 0;
	g_GlobalBitmap.bitPerPixel = g_OSDInfo.bitPerPixel;
	g_GlobalBitmap.data = NULL;
	g_GlobalBitmap.hDC = NULL;
	g_GlobalBitmap.hBitmap = NULL;

	g_GlobalBitmapBackup = g_GlobalBitmap;

	UTFSetScreenDraw(1);

	return 1;
}

*/
/////////////////////////////////////////////////////////////////////////////
//Start define memory draw function
static void UTFMemoryPixel8(WORD x, WORD y, UTFOSDCOL color)
{
	BYTE *pWrite;
		
	pWrite = g_GlobalBitmap.data+y*g_GlobalBitmap.Width+x;
	*pWrite = color.data;
}

static void UTFMemoryPixel16(WORD x, WORD y, UTFOSDCOL color)
{
	WORD *pWrite;

	pWrite = (WORD *)g_GlobalBitmap.data;
	pWrite += y*g_GlobalBitmap.Width+x;
	*pWrite = color.data;
}

static void UTFMemoryLine8(WORD x1, WORD y1, WORD x2, WORD y2, WORD penWidth, UTFOSDCOL color)
{
	int i,p,n,x,y,tn;
	BYTE *ptr;

	if(y1 == y2)	//horiziontal line
	{
		if(x1 > x2) { x=x2; x2=x1; x1=x; }
		if(x1 >= g_GlobalBitmap.Width)
			return;
		if(x2 >= g_GlobalBitmap.Width)
			x2 = g_GlobalBitmap.Width-1;
		
		ptr = g_GlobalBitmap.data+y1*g_GlobalBitmap.Width;
		memset(ptr+x1, color.data, x2-x1);

		return;
	}
	
	if(x1 == x2)	//vertical line
	{
		if(y1 > y2) { y=y2; y2=y1; y1=y; }
		if(y1 >= g_GlobalBitmap.Height)
			return;
		if(y2 >= g_GlobalBitmap.Height)
			y2 = g_GlobalBitmap.Height-1;

		ptr = g_GlobalBitmap.data+y1*g_GlobalBitmap.Width+x1;
		for(i=0;i<(y2-y1);i++)
		{
			*ptr = color.data;
			ptr += g_GlobalBitmap.Width;
		}
		
		return;
	}
	
	if(UTFAbs(y2-y1) <= UTFAbs(x2-x1))
	{
		if(((y2<y1) && (x2<x1)) || ((y1<=y2) && (x1>x2)))
		{
			x=x2; y=y2; x2=x1; y2=y1; x1=x; y1=y;
		}
		if((y2>=y1) && (x2>=x1))
		{
			x=x2-x1; y=y2-y1;
			p=2*y; n=2*x-2*y; tn=x;

			ptr = g_GlobalBitmap.data+y1*g_GlobalBitmap.Width;
			while(x1<=x2)
			{
				if(tn >= 0) tn-=p;
				else { tn+=n; y1++; ptr+=g_GlobalBitmap.Width; }
				*(ptr+x1++) = color.data;
			}
		}
		else
		{
			x=x2-x1; y=y2-y1;
			p=-2*y; n=2*x+2*y; tn=x;

			ptr = g_GlobalBitmap.data+y1*g_GlobalBitmap.Width;
			while(x1<=x2)
			{
				if(tn>=0) tn-=p;
				else { tn+=n; y1--; ptr-=g_GlobalBitmap.Width; }
				*(ptr+x1++) = color.data;
			}
		}
	}
	else
	{
		x=x1; x1=y2; y2=x; y=y1; y1=x2; x2=y;
		if(((y2<y1) && (x2<x1)) || ((y1<=y2) && (x1>x2)))
		{
			x=x2; y=y2; x2=x1; y2=y1; x1=x; y1=y;
		}
		if((y2>=y1) && (x2>=x1))
		{
			x=x2-x1; y=y2-y1; p=2*y; n=2*x-2*y; tn=x;
			ptr = g_GlobalBitmap.data+x1*g_GlobalBitmap.Width;
			while(x1<=x2)
			{
				if(tn>=0) tn-=p;
	 			else { tn+=n; y1++; }
				*(ptr+y1) = color.data;
				ptr += g_GlobalBitmap.Width;
				x1++;
			}
		}
		else
		{
			x=x2-x1; y=y2-y1; p=-2*y; n=2*x+2*y; tn=x;
			ptr = g_GlobalBitmap.data+x1*g_GlobalBitmap.Width;
			while(x1<=x2)
			{
				if(tn>=0) tn-=p;
	 			else { tn+=n; y1--; }
				*(ptr+y1) = color.data;
				ptr += g_GlobalBitmap.Width;
				x1++;
			}
		}
	}
}

static void UTFMemoryLine16(WORD x1, WORD y1, WORD x2, WORD y2, WORD penWidth, UTFOSDCOL color)
{
	int i,p,n,x,y,tn;
	WORD *ptr;

	if(y1 == y2)	//horiziontal line
	{
		if(x1 > x2) { x=x2; x2=x1; x1=x; }
		if(x1 >= g_GlobalBitmap.Width)
			return;
		if(x2 >= g_GlobalBitmap.Width)
			x2 = g_GlobalBitmap.Width-1;

		ptr = (WORD *)g_GlobalBitmap.data;
		ptr += y1*g_GlobalBitmap.Width+x1;

		for(i=x1; i<x2; i++) *ptr++ = color.data;

		return;
	}
	
	if(x1 == x2)	//vertical line
	{
		if(y1 > y2) { y=y2; y2=y1; y1=y; }
		if(y1 >= g_GlobalBitmap.Height)
			return;
		if(y2 >= g_GlobalBitmap.Height)
			y2 = g_GlobalBitmap.Height-1;

		ptr = (WORD *)g_GlobalBitmap.data;
		ptr += y1*g_GlobalBitmap.Width+x1;
		for(i=0;i<(y2-y1);i++)
		{
			*ptr = color.data;
			ptr += g_GlobalBitmap.Width;
		}
		
		return;
	}
	
	if(UTFAbs(y2-y1) <= UTFAbs(x2-x1))
	{
		if(((y2<y1) && (x2<x1)) || ((y1<=y2) && (x1>x2)))
		{
			x=x2; y=y2; x2=x1; y2=y1; x1=x; y1=y;
		}
		if((y2>=y1) && (x2>=x1))
		{
			x=x2-x1; y=y2-y1;
			p=2*y; n=2*x-2*y; tn=x;
			ptr = (WORD *)g_GlobalBitmap.data;
			ptr += y1*g_GlobalBitmap.Width;
			while(x1<=x2)
			{
				if(tn >= 0) tn-=p;
				else { tn+=n; y1++; ptr+=g_GlobalBitmap.Width; }
				*(ptr+x1++) = color.data;
			}
		}
		else
		{
			x=x2-x1; y=y2-y1;
			p=-2*y; n=2*x+2*y; tn=x;
			ptr = (WORD *)g_GlobalBitmap.data;
			ptr += y1*g_GlobalBitmap.Width;
			while(x1<=x2)
			{
				if(tn>=0) tn-=p;
				else { tn+=n; y1--; ptr-=g_GlobalBitmap.Width; }
				*(ptr+x1++) = color.data;
			}
		}
	}
	else
	{
		x=x1; x1=y2; y2=x; y=y1; y1=x2; x2=y;
		if(((y2<y1) && (x2<x1)) || ((y1<=y2) && (x1>x2)))
		{
			x=x2; y=y2; x2=x1; y2=y1; x1=x; y1=y;
		}
		if((y2>=y1) && (x2>=x1))
		{
			x=x2-x1; y=y2-y1; p=2*y; n=2*x-2*y; tn=x;
			ptr = (WORD *)g_GlobalBitmap.data;
			ptr += x1*g_GlobalBitmap.Width;
			while(x1<=x2)
			{
				if(tn>=0) tn-=p;
	 			else { tn+=n; y1++; }
				*(ptr+y1) = color.data;
				ptr += g_GlobalBitmap.Width;
				x1++;
			}
		}
		else
		{
			x=x2-x1; y=y2-y1; p=-2*y; n=2*x+2*y; tn=x;
			ptr = (WORD *)g_GlobalBitmap.data;
			ptr += x1*g_GlobalBitmap.Width;
			while(x1<=x2)
			{
				if(tn>=0) tn-=p;
	 			else { tn+=n; y1--; }
				*(ptr+y1) = color.data;
				ptr += g_GlobalBitmap.Width;
				x1++;
			}
		}
	}
}

static void UTFMemoryDrawRect8(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color)
{
	WORD wWidth = right-left;
	WORD wHeight = bottom-top;
	BYTE *pWrite;

	if(left >= g_GlobalBitmap.Width)
		return;
	if(top >= g_GlobalBitmap.Height)
		return;

	//draw top line
	pWrite = g_GlobalBitmap.data+top*g_GlobalBitmap.Width+left;
	memset(pWrite, color.data, wWidth);

	//draw bottom line
	if(wHeight > 1)
	{
		pWrite += (wHeight-1)*g_GlobalBitmap.Width;
		memset(pWrite, color.data, wWidth);
	}

	if(wHeight > 2)
	{
		WORD i;

		//notice:must do this step
		wHeight -= 2;

		//start write vertical line
		pWrite = g_GlobalBitmap.data+(top+1)*g_GlobalBitmap.Width+left;
		for(i=0; i<wHeight; i++)
		{
			*pWrite = color.data;
			*(pWrite+wWidth-1) = color.data;

			pWrite += g_GlobalBitmap.Width;
		}
	}
}

static void UTFMemoryDrawRect16(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color)
{
	WORD wWidth = right-left;
	WORD i,wHeight = bottom-top;
	WORD *pWrite;

	if(left >= g_GlobalBitmap.Width)
		return;
	if(top >= g_GlobalBitmap.Height)
		return;

	//draw top line
	pWrite = (WORD *)g_GlobalBitmap.data;
	pWrite += top*g_GlobalBitmap.Width+left;
	for(i=0; i<wWidth; i++) *(pWrite+i) = color.data;

	//draw bottom line
	if(wHeight > 1)
	{
		pWrite += (wHeight-1)*g_GlobalBitmap.Width;
		for(i=0; i<wWidth; i++) *(pWrite+i) = color.data;
	}

	if(wHeight > 2)
	{
		//notice:must do this step
		wHeight -= 2;

		//start write vertical line
		pWrite = (WORD *)g_GlobalBitmap.data;
		pWrite += (top+1)*g_GlobalBitmap.Width+left;
		for(i=0; i<wHeight; i++)
		{
			*pWrite = color.data;
			*(pWrite+wWidth-1) = color.data;

			pWrite += g_GlobalBitmap.Width;
		}
	}
}

static void UTFMemoryFillRect8(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color)
{
	WORD i,wWidth = right-left;
	WORD wHeight = bottom-top;
	BYTE *pWrite;

	if(left >= g_GlobalBitmap.Width)
		return;
	if(top >= g_GlobalBitmap.Height)
		return;
	if((left+wWidth) > g_GlobalBitmap.Width)
		wWidth = g_GlobalBitmap.Width-left;
	if((top+wHeight) > g_GlobalBitmap.Height)
		wHeight = g_GlobalBitmap.Height-top;

	//draw top line
	pWrite = g_GlobalBitmap.data+top*g_GlobalBitmap.Width+left;
	for(i=0; i<wHeight; i++)
	{
		memset(pWrite, color.data, wWidth);
		pWrite += g_GlobalBitmap.Width;
	}
}

static void UTFMemoryFillRect16(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color)
{
	WORD i,j,wWidth = right-left;
	WORD wHeight = bottom-top;
	WORD *pWrite;

	if(left >= g_GlobalBitmap.Width)
		return;
	if(top >= g_GlobalBitmap.Height)
		return;
	if((left+wWidth) > g_GlobalBitmap.Width)
		wWidth = g_GlobalBitmap.Width-left;
	if((top+wHeight) > g_GlobalBitmap.Height)
		wHeight = g_GlobalBitmap.Height-top;

	pWrite = (WORD *)g_GlobalBitmap.data;
	pWrite += top*g_GlobalBitmap.Width+left;
	for(i=0; i<wHeight; i++)
	{
		for(j=0; j<wWidth; j++) *(pWrite+j) = color.data;
		pWrite += g_GlobalBitmap.Width;
	}
}

#define INDEX_FORWORD(index)	index = (index+1)%nPoint;
#define INDEX_BACKWORD(index)	index = (index-1+nPoint)%nPoint;
static void UTFMemoryPolygonFill8(LPUTFPOINT lpPoint, WORD nPoint, UTFOSDCOL color)
{
	int lx1,ly1,lx2,ly2,rx1,ry1,rx2,ry2,ldx,ldy,rdx,rdy;
	int minY,maxY,lindex,rindex,lxinc,rxinc,lxadd,rxadd,lxsgn,rxsgn,lxsum,rxsum;
	int i,x1,x2;
	long width = g_GlobalBitmap.Width-1;
	long height = g_GlobalBitmap.Height-1;
	long pitch = g_GlobalBitmap.Width;
	BYTE *pt,*p=g_GlobalBitmap.data;

	rindex = lindex = 0;
	maxY = minY = lpPoint[0].y;
	for(i=0;i<nPoint;i++)
	{	
		if(lpPoint[i].y<minY)
			minY = lpPoint[lindex = i].y;
		else if(lpPoint[i].y > maxY)
			maxY = lpPoint[i].y;
	}
	if(minY == maxY)
		return;
	rindex = lindex;
	while(lpPoint[lindex].y == minY)
		INDEX_FORWORD(lindex);
	INDEX_BACKWORD(lindex);
	while(lpPoint[rindex].y == minY)
		INDEX_BACKWORD(rindex);
	INDEX_FORWORD(rindex);
	lx1 = lpPoint[lindex].x;
	ly1 = lpPoint[lindex].y;
	INDEX_FORWORD(lindex);
	lx2 = lpPoint[lindex].x;
	ly2 = lpPoint[lindex].y;
	ldx = lx2 - lx1;
	ldy = ly2 - ly1;
	lxsgn = (ldx >= 0)?1:-1;
	if(ldy>0)
	{	
		lxadd = ldx/ldy;
		lxinc = (ldx%ldy)*lxsgn;
	}
	rx1 = lpPoint[rindex].x;
	ry1 = lpPoint[rindex].y;
	INDEX_BACKWORD(rindex);
	rx2 = lpPoint[rindex].x;
	ry2 = lpPoint[rindex].y;
	rdx = rx2 - rx1;
	rdy = ry2 - ry1;
	rxsgn = (rdx >= 0)?1:-1;
	if(rdy>0)
	{
		rxadd = rdx/rdy;
		rxinc = (rdx%rdy)*rxsgn;
	}
	lxsum = 0;
	rxsum = 0;
	minY++;
	p += pitch*minY;
	while(minY <= maxY)
	{	
		lxsum+=lxinc;
		lx1+=lxadd;
		if(lxsum >= ldy)
		{
			lx1+=lxsgn;
			lxsum -= ldy;
		}
		rxsum+=rxinc;
		rx1+=rxadd;
		if(rxsum >= rdy)
		{	
			rx1+=rxsgn;
			rxsum -= rdy;
		}
		x1 = lx1,x2 = rx1 - 1;
		if(minY>=0 && minY<=height && x1<=width && x2>=0 && x2>=x1)
		{	
			if(x1<0)
				x1=0;
			if(x2>width)
				x2=width;
			pt = p + x1;	
			*pt = color.data;
			while(x1<x2)
			{
				pt++;
				*pt = color.data;
				x1++;
			}
		}
		if(minY == ly2)
		{	
			do
			{
				lx1 = lpPoint[lindex].x;
				ly1 = lpPoint[lindex].y;
				INDEX_FORWORD(lindex);
				lx2 = lpPoint[lindex].x;
				ly2 = lpPoint[lindex].y;
			}while(ly2 == ly1);
			ldx = lx2 - lx1;
			ldy = ly2 - ly1;
			lxsgn = (ldx >= 0)?1:-1;
			if(ldy>0)
			{
				lxadd = ldx/ldy;
				lxinc = (ldx%ldy)*lxsgn;
			}
		}
		if(minY == ry2)
		{	
			do
			{
				rx1 = lpPoint[rindex].x;
				ry1 = lpPoint[rindex].y;
				INDEX_BACKWORD(rindex);
				rx2 = lpPoint[rindex].x;
				ry2 = lpPoint[rindex].y;
			}while(ry2 == ry1);
			rdx = rx2 - rx1;
			rdy = ry2 - ry1;
			rxsgn = (rdx >= 0)?1:-1;
			if(rdy>0);
			{	
				rxadd = rdx/rdy;
				rxinc = (rdx%rdy)*rxsgn;
			}
		}
		minY++;
		p += pitch;
	}
}

static void UTFMemoryPolygonFill16(LPUTFPOINT lpPoint, WORD nPoint, UTFOSDCOL color)
{
	int lx1,ly1,lx2,ly2,rx1,ry1,rx2,ry2,ldx,ldy,rdx,rdy;
	int minY,maxY,lindex,rindex,lxinc,rxinc,lxadd,rxadd,lxsgn,rxsgn,lxsum,rxsum;
	int i,x1,x2;
	long width = g_GlobalBitmap.Width-1;
	long height = g_GlobalBitmap.Height-1;
	long pitch = g_GlobalBitmap.Width;
	WORD *pt,*p=(WORD *)g_GlobalBitmap.data;

	rindex = lindex = 0;
	maxY = minY = lpPoint[0].y;
	for(i=0;i<nPoint;i++)
	{	
		if(lpPoint[i].y<minY)
			minY = lpPoint[lindex = i].y;
		else if(lpPoint[i].y > maxY)
			maxY = lpPoint[i].y;
	}
	if(minY == maxY)
		return;
	rindex = lindex;
	while(lpPoint[lindex].y == minY)
		INDEX_FORWORD(lindex);
	INDEX_BACKWORD(lindex);
	while(lpPoint[rindex].y == minY)
		INDEX_BACKWORD(rindex);
	INDEX_FORWORD(rindex);
	lx1 = lpPoint[lindex].x;
	ly1 = lpPoint[lindex].y;
	INDEX_FORWORD(lindex);
	lx2 = lpPoint[lindex].x;
	ly2 = lpPoint[lindex].y;
	ldx = lx2 - lx1;
	ldy = ly2 - ly1;
	lxsgn = (ldx >= 0)?1:-1;
	if(ldy>0)
	{	
		lxadd = ldx/ldy;
		lxinc = (ldx%ldy)*lxsgn;
	}
	rx1 = lpPoint[rindex].x;
	ry1 = lpPoint[rindex].y;
	INDEX_BACKWORD(rindex);
	rx2 = lpPoint[rindex].x;
	ry2 = lpPoint[rindex].y;
	rdx = rx2 - rx1;
	rdy = ry2 - ry1;
	rxsgn = (rdx >= 0)?1:-1;
	if(rdy>0)
	{
		rxadd = rdx/rdy;
		rxinc = (rdx%rdy)*rxsgn;
	}
	lxsum = 0;
	rxsum = 0;
	minY++;
	p += pitch*minY;
	while(minY <= maxY)
	{	
		lxsum+=lxinc;
		lx1+=lxadd;
		if(lxsum >= ldy)
		{
			lx1+=lxsgn;
			lxsum -= ldy;
		}
		rxsum+=rxinc;
		rx1+=rxadd;
		if(rxsum >= rdy)
		{	
			rx1+=rxsgn;
			rxsum -= rdy;
		}
		x1 = lx1,x2 = rx1 - 1;
		if(minY>=0 && minY<=height && x1<=width && x2>=0 && x2>=x1)
		{	
			if(x1<0)
				x1=0;
			if(x2>width)
				x2=width;
			pt = p + x1;	
			*pt = color.data;
			while(x1<x2)
			{
				pt++;
				*pt = color.data;
				x1++;
			}
		}
		if(minY == ly2)
		{	
			do
			{
				lx1 = lpPoint[lindex].x;
				ly1 = lpPoint[lindex].y;
				INDEX_FORWORD(lindex);
				lx2 = lpPoint[lindex].x;
				ly2 = lpPoint[lindex].y;
			}while(ly2 == ly1);
			ldx = lx2 - lx1;
			ldy = ly2 - ly1;
			lxsgn = (ldx >= 0)?1:-1;
			if(ldy>0)
			{
				lxadd = ldx/ldy;
				lxinc = (ldx%ldy)*lxsgn;
			}
		}
		if(minY == ry2)
		{	
			do
			{
				rx1 = lpPoint[rindex].x;
				ry1 = lpPoint[rindex].y;
				INDEX_BACKWORD(rindex);
				rx2 = lpPoint[rindex].x;
				ry2 = lpPoint[rindex].y;
			}while(ry2 == ry1);
			rdx = rx2 - rx1;
			rdy = ry2 - ry1;
			rxsgn = (rdx >= 0)?1:-1;
			if(rdy>0);
			{	
				rxadd = rdx/rdy;
				rxinc = (rdx%rdy)*rxsgn;
			}
		}
		minY++;
		p += pitch;
	}
}

static void UTFMemoryEllipseDraw8(WORD xx1, WORD yy1, WORD xx2, WORD yy2, WORD penWidth, UTFOSDCOL color)
{
	int xx,yy,x1,x2,x3,x4,y1,y2,y3,y4,pk;
	int xc,yc,rx,ry,rx2,ry2,tworx2,twory2,px,py;
	UTFBITMAP bitmap;
	BYTE *ptrWrite;

	bitmap = g_GlobalBitmap;
	ptrWrite = bitmap.data;

	xc = (xx1+xx2)>>1;
	yc = (yy1+yy2)>>1;
	
	if(xx1 > xx2)
		rx = (xx1-xx2)>>1;
	else
		rx = (xx2-xx1)>>1;
	if(yy1 > yy2)
		ry = (yy1-yy2)>>1;
	else
		ry = (yy2-yy1)>>1;
	rx2 = rx*rx;
	ry2 = ry*ry;
	tworx2 = rx2<<1;
	twory2 = ry2<<1;
	xx = 0; 	yy = ry; px = 0;
	py = tworx2*yy;
	pk = ry2-rx2*ry+rx2>>2; 
	x1 = xc; y1 = yc+ry;
	x2 = xc; y2 = yc+ry;
	x3 = xc; y3 = yc-ry;
	x4 = xc; y4 = yc-ry;

	*(ptrWrite+y1*bitmap.Width+x1) = color.data;
	*(ptrWrite+y2*bitmap.Width+x2) = color.data;
	*(ptrWrite+y3*bitmap.Width+x3) = color.data;
	*(ptrWrite+y4*bitmap.Width+x4) = color.data;
	while(px < py)
	{
		xx++;
		px += twory2;
		x1++,x2--,x3++,x4--;
		if(pk < 0)
			pk += ry2+px;
		else
		{	
			yy--;
			y1--,y2--,y3++,y4++;
			py -= tworx2;
			pk += ry2+px-py;
		}
		*(ptrWrite+y1*bitmap.Width+x1) = color.data;
		*(ptrWrite+y2*bitmap.Width+x2) = color.data;
		*(ptrWrite+y3*bitmap.Width+x3) = color.data;
		*(ptrWrite+y4*bitmap.Width+x4) = color.data;
	}
	pk = (int)UTFSqrt(((ry2*(xx*2+1)*(xx*2+1))>>2)+rx2*(yy-1)*(yy-1)-rx2*ry2);
	while(yy>0)
	{	
		yy--;
		y1--,y2--,y3++,y4++;
		py -= tworx2;
		if(pk>0)
		{
			pk += rx2-py;
		}
		else
		{	
			xx++;
			x1++,x2--,x3++,x4--;
			px += twory2;
			pk += rx2-py+px;
		}
		*(ptrWrite+y1*bitmap.Width+x1) = color.data;
		*(ptrWrite+y2*bitmap.Width+x2) = color.data;
		*(ptrWrite+y3*bitmap.Width+x3) = color.data;
		*(ptrWrite+y4*bitmap.Width+x4) = color.data;
	}
}

static void UTFMemoryEllipseDraw16(WORD xx1, WORD yy1, WORD xx2, WORD yy2, WORD penWidth, UTFOSDCOL color)
{
	int xx,yy,x1,x2,x3,x4,y1,y2,y3,y4,pk;
	int xc,yc,rx,ry,rx2,ry2,tworx2,twory2,px,py;
	UTFBITMAP bitmap;
	WORD *ptrWrite;

	bitmap = g_GlobalBitmap;
	ptrWrite = (WORD *)bitmap.data;

	xc = (xx1+xx2)>>1;
	yc = (yy1+yy2)>>1;
	
	if(xx1 > xx2)
		rx = (xx1-xx2)>>1;
	else
		rx = (xx2-xx1)>>1;
	if(yy1 > yy2)
		ry = (yy1-yy2)>>1;
	else
		ry = (yy2-yy1)>>1;
	rx2 = rx*rx;
	ry2 = ry*ry;
	tworx2 = rx2<<1;
	twory2 = ry2<<1;
	xx = 0; 	yy = ry; px = 0;
	py = tworx2*yy;
	pk = ry2-rx2*ry+rx2>>2; 
	x1 = xc; y1 = yc+ry;
	x2 = xc; y2 = yc+ry;
	x3 = xc; y3 = yc-ry;
	x4 = xc; y4 = yc-ry;

	*(ptrWrite+y1*bitmap.Width+x1) = color.data;
	*(ptrWrite+y2*bitmap.Width+x2) = color.data;
	*(ptrWrite+y3*bitmap.Width+x3) = color.data;
	*(ptrWrite+y4*bitmap.Width+x4) = color.data;
	while(px < py)
	{
		xx++;
		px += twory2;
		x1++,x2--,x3++,x4--;
		if(pk < 0)
			pk += ry2+px;
		else
		{	
			yy--;
			y1--,y2--,y3++,y4++;
			py -= tworx2;
			pk += ry2+px-py;
		}
		*(ptrWrite+y1*bitmap.Width+x1) = color.data;
		*(ptrWrite+y2*bitmap.Width+x2) = color.data;
		*(ptrWrite+y3*bitmap.Width+x3) = color.data;
		*(ptrWrite+y4*bitmap.Width+x4) = color.data;
	}
	pk = (int)UTFSqrt(((ry2*(xx*2+1)*(xx*2+1))>>2)+rx2*(yy-1)*(yy-1)-rx2*ry2);
	while(yy>0)
	{	
		yy--;
		y1--,y2--,y3++,y4++;
		py -= tworx2;
		if(pk>0)
		{
			pk += rx2-py;
		}
		else
		{	
			xx++;
			x1++,x2--,x3++,x4--;
			px += twory2;
			pk += rx2-py+px;
		}
		*(ptrWrite+y1*bitmap.Width+x1) = color.data;
		*(ptrWrite+y2*bitmap.Width+x2) = color.data;
		*(ptrWrite+y3*bitmap.Width+x3) = color.data;
		*(ptrWrite+y4*bitmap.Width+x4) = color.data;
	}
}

static void UTFMemoryEllipseFill8(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color)
{
	int a,b,a2,b2,x,delta;
	int loop,threshold;
	DWORD xCheck,yCheck;
	DWORD yFactor,i;
	UTFBITMAP bitmap;
	BYTE *ptrWrite;
	BYTE *ptrStart1,*ptrStart2;

	bitmap = g_GlobalBitmap;
	ptrWrite = bitmap.data+top*bitmap.Width+left;

	a = (right-left)>>1;
	b = (bottom-top)>>1;
	if(b>0) b--;

	a2 = a*a;
	b2 = b*b;

	x = 0;
	delta = 8*b2 + a2*(1-2*b);
	loop = 6*b2;
	threshold = 4*a2*(1-b);
	yFactor = b;

	xCheck = b2;
	yCheck = b*a2;

	*(ptrWrite+a) = color.data;
	*(ptrWrite+(b+b)*bitmap.Width+a) = color.data;
	while(xCheck <= yCheck)
	{
		if(delta >= 0)
		{
			delta += threshold;
			threshold += 4*a2;
			yCheck -= a2;
			yFactor--;
		}		

		loop += 4*b2;
		delta += loop;
		xCheck += b2;
		x++;

		ptrStart1 = ptrWrite+(b-yFactor)*bitmap.Width+a-x;
		ptrStart2 = ptrWrite+(b+yFactor)*bitmap.Width+a-x;
		for(i=0; i<2*x; i++)
		{
			*(ptrStart1+i) = (BYTE)color.data;
			*(ptrStart2+i) = (BYTE)color.data;
		}
	}

	x = a;
	delta = 8*a2 + b2*(1-2*a);
	loop = 6*a2;
	threshold = 4*b2*(1-a);
	yFactor = 1;
	xCheck = a*b2;
	yCheck = a2;

	ptrStart1 = ptrWrite+b*bitmap.Width+a-x;
	for(i=0; i<2*x; i++)
	{
		*(ptrStart1+i) = (BYTE)color.data;
	}
	while(yCheck <= xCheck)
	{
		ptrStart1 = ptrWrite+(b-yFactor)*bitmap.Width+a-x;
		ptrStart2 = ptrWrite+(b+yFactor)*bitmap.Width+a-x;
		for(i=0; i<2*x; i++)
		{
			*(ptrStart1+i) = (BYTE)color.data;
			*(ptrStart2+i) = (BYTE)color.data;
		}
		
		if(delta >= 0)
		{
			x--;
			delta += threshold;
			threshold += 4*b2;
			xCheck -= b2;
		}

		loop += 4*a2;
		delta += loop;
		yCheck += a2;
		yFactor++;
	}
}

static void UTFMemoryEllipseFill16(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color)
{
	int a,b,a2,b2,x,delta;
	int loop,threshold;
	DWORD xCheck,yCheck;
	DWORD yFactor,i;
	UTFBITMAP bitmap;
	WORD *ptrWrite;
	WORD *ptrStart1,*ptrStart2;

	bitmap = g_GlobalBitmap;
	ptrWrite = (WORD *)bitmap.data;
	ptrWrite += top*bitmap.Width+left;

	a = (right-left)>>1;
	b = (bottom-top)>>1;
	if(b>0) b--;

	a2 = a*a;
	b2 = b*b;

	x = 0;
	delta = 8*b2 + a2*(1-2*b);
	loop = 6*b2;
	threshold = 4*a2*(1-b);
	yFactor = b;

	xCheck = b2;
	yCheck = b*a2;

	*(ptrWrite+a) = color.data;
	*(ptrWrite+(b+b)*bitmap.Width+a) = color.data;
	while(xCheck <= yCheck)
	{
		if(delta >= 0)
		{
			delta += threshold;
			threshold += 4*a2;
			yCheck -= a2;
			yFactor--;
		}		

		loop += 4*b2;
		delta += loop;
		xCheck += b2;
		x++;

		ptrStart1 = ptrWrite+(b-yFactor)*bitmap.Width+a-x;
		ptrStart2 = ptrWrite+(b+yFactor)*bitmap.Width+a-x;
		for(i=0; i<2*x; i++)
		{
			*(ptrStart1+i) = color.data;
			*(ptrStart2+i) = color.data;
		}
	}

	x = a;
	delta = 8*a2 + b2*(1-2*a);
	loop = 6*a2;
	threshold = 4*b2*(1-a);
	yFactor = 1;
	xCheck = a*b2;
	yCheck = a2;

	ptrStart1 = ptrWrite+b*bitmap.Width+a-x;
	for(i=0; i<2*x; i++)
	{
		*(ptrStart1+i) = color.data;
	}
	while(yCheck <= xCheck)
	{
		ptrStart1 = ptrWrite+(b-yFactor)*bitmap.Width+a-x;
		ptrStart2 = ptrWrite+(b+yFactor)*bitmap.Width+a-x;
		for(i=0; i<2*x; i++)
		{
			*(ptrStart1+i) = color.data;
			*(ptrStart2+i) = color.data;
		}
		
		if(delta >= 0)
		{
			x--;
			delta += threshold;
			threshold += 4*b2;
			xCheck -= b2;
		}

		loop += 4*a2;
		delta += loop;
		yCheck += a2;
		yFactor++;
	}
}

static void UTFDrawMemoryCircleArcLT8(WORD x, WORD y, WORD r, UTFOSDCOL color)
{
	int xx,yy,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,y3,y4,y5,y6,y7,y8,pk;
	UTFBITMAP bitmap;
	BYTE *ptrWrite;

	bitmap = g_GlobalBitmap;
	ptrWrite = bitmap.data;

	xx = 0;
	yy = r;
	x1 = x, y1 = y + r;
	x2 = x, y2 = y + r;
	x3 = x, y3 = y - r;
	x4 = x, y4 = y - r;
	x5 = x + r, y5 = y;
	x6 = x - r, y6 = y;
	x7 = x + r, y7 = y;
	x8 = x - r, y8 = y;
	pk = 1 - r;

	*(ptrWrite+y4*bitmap.Width+x4) = color.data;
	*(ptrWrite+y8*bitmap.Width+x8) = color.data;
	while(xx < yy)
	{	
		xx++;
		x1++, x2--, x3++, x4--;
		y5++, y6++, y7--, y8--;
		if(pk < 0)
			pk += 2*xx+1;
		else
		{
			yy--;
			y1--, y2--, y3++, y4++;
			x5--, x6++, x7--, x8++;
			pk += 2*(xx-yy)+1;
		}
		*(ptrWrite+y4*bitmap.Width+x4) = color.data;
		*(ptrWrite+y8*bitmap.Width+x8) = color.data;
	}
}

static void UTFDrawMemoryCircleArcLT16(WORD x, WORD y, WORD r, UTFOSDCOL color)
{
	int xx,yy,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,y3,y4,y5,y6,y7,y8,pk;
	UTFBITMAP bitmap;
	WORD *ptrWrite;

	bitmap = g_GlobalBitmap;
	ptrWrite = (WORD *)bitmap.data;

	xx = 0;
	yy = r;
	x1 = x, y1 = y + r;
	x2 = x, y2 = y + r;
	x3 = x, y3 = y - r;
	x4 = x, y4 = y - r;
	x5 = x + r, y5 = y;
	x6 = x - r, y6 = y;
	x7 = x + r, y7 = y;
	x8 = x - r, y8 = y;
	pk = 1 - r;

	*(ptrWrite+y4*bitmap.Width+x4) = color.data;
	*(ptrWrite+y8*bitmap.Width+x8) = color.data;
	while(xx < yy)
	{	
		xx++;
		x1++, x2--, x3++, x4--;
		y5++, y6++, y7--, y8--;
		if(pk < 0)
			pk += 2*xx+1;
		else
		{
			yy--;
			y1--, y2--, y3++, y4++;
			x5--, x6++, x7--, x8++;
			pk += 2*(xx-yy)+1;
		}
		*(ptrWrite+y4*bitmap.Width+x4) = color.data;
		*(ptrWrite+y8*bitmap.Width+x8) = color.data;
	}
}

static void UTFDrawMemoryCircleArcRT8(WORD x, WORD y, WORD r, UTFOSDCOL color)
{
	int xx,yy,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,y3,y4,y5,y6,y7,y8,pk;
	UTFBITMAP bitmap;
	BYTE *ptrWrite;

	bitmap = g_GlobalBitmap;
	ptrWrite = bitmap.data;

	xx = 0;
	yy = r;
	x1 = x, y1 = y + r;
	x2 = x, y2 = y + r;
	x3 = x, y3 = y - r;
	x4 = x, y4 = y - r;
	x5 = x + r, y5 = y;
	x6 = x - r, y6 = y;
	x7 = x + r, y7 = y;
	x8 = x - r, y8 = y;
	pk = 1 - r;

	*(ptrWrite+y3*bitmap.Width+x3) = color.data;
	*(ptrWrite+y7*bitmap.Width+x7) = color.data;
	while(xx < yy)
	{	
		xx++;
		x1++, x2--, x3++, x4--;
		y5++, y6++, y7--, y8--;
		if(pk < 0)
			pk += 2*xx+1;
		else
		{
			yy--;
			y1--, y2--, y3++, y4++;
			x5--, x6++, x7--, x8++;
			pk += 2*(xx-yy)+1;
		}
		*(ptrWrite+y3*bitmap.Width+x3) = color.data;
		*(ptrWrite+y7*bitmap.Width+x7) = color.data;
	}
}

static void UTFDrawMemoryCircleArcRT16(WORD x, WORD y, WORD r, UTFOSDCOL color)
{
	int xx,yy,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,y3,y4,y5,y6,y7,y8,pk;
	UTFBITMAP bitmap;
	WORD *ptrWrite;

	bitmap = g_GlobalBitmap;
	ptrWrite = (WORD *)bitmap.data;

	xx = 0;
	yy = r;
	x1 = x, y1 = y + r;
	x2 = x, y2 = y + r;
	x3 = x, y3 = y - r;
	x4 = x, y4 = y - r;
	x5 = x + r, y5 = y;
	x6 = x - r, y6 = y;
	x7 = x + r, y7 = y;
	x8 = x - r, y8 = y;
	pk = 1 - r;

	*(ptrWrite+y3*bitmap.Width+x3) = color.data;
	*(ptrWrite+y7*bitmap.Width+x7) = color.data;
	while(xx < yy)
	{	
		xx++;
		x1++, x2--, x3++, x4--;
		y5++, y6++, y7--, y8--;
		if(pk < 0)
			pk += 2*xx+1;
		else
		{
			yy--;
			y1--, y2--, y3++, y4++;
			x5--, x6++, x7--, x8++;
			pk += 2*(xx-yy)+1;
		}
		*(ptrWrite+y3*bitmap.Width+x3) = color.data;
		*(ptrWrite+y7*bitmap.Width+x7) = color.data;
	}
}

static void UTFDrawMemoryCircleArcLB8(WORD x, WORD y, WORD r, UTFOSDCOL color)
{
	int xx,yy,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,y3,y4,y5,y6,y7,y8,pk;
	UTFBITMAP bitmap;
	BYTE *ptrWrite;

	bitmap = g_GlobalBitmap;
	ptrWrite = bitmap.data;

	xx = 0;
	yy = r;
	x1 = x, y1 = y + r;
	x2 = x, y2 = y + r;
	x3 = x, y3 = y - r;
	x4 = x, y4 = y - r;
	x5 = x + r, y5 = y;
	x6 = x - r, y6 = y;
	x7 = x + r, y7 = y;
	x8 = x - r, y8 = y;
	pk = 1 - r;

	*(ptrWrite+y2*bitmap.Width+x2) = color.data;
	*(ptrWrite+y6*bitmap.Width+x6) = color.data;
	while(xx < yy)
	{	
		xx++;
		x1++, x2--, x3++, x4--;
		y5++, y6++, y7--, y8--;
		if(pk < 0)
			pk += 2*xx+1;
		else
		{
			yy--;
			y1--, y2--, y3++, y4++;
			x5--, x6++, x7--, x8++;
			pk += 2*(xx-yy)+1;
		}
		*(ptrWrite+y2*bitmap.Width+x2) = color.data;
		*(ptrWrite+y6*bitmap.Width+x6) = color.data;
	}
}

static void UTFDrawMemoryCircleArcLB16(WORD x, WORD y, WORD r, UTFOSDCOL color)
{
	int xx,yy,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,y3,y4,y5,y6,y7,y8,pk;
	UTFBITMAP bitmap;
	WORD *ptrWrite;

	bitmap = g_GlobalBitmap;
	ptrWrite = (WORD *)bitmap.data;

	xx = 0;
	yy = r;
	x1 = x, y1 = y + r;
	x2 = x, y2 = y + r;
	x3 = x, y3 = y - r;
	x4 = x, y4 = y - r;
	x5 = x + r, y5 = y;
	x6 = x - r, y6 = y;
	x7 = x + r, y7 = y;
	x8 = x - r, y8 = y;
	pk = 1 - r;

	*(ptrWrite+y2*bitmap.Width+x2) = color.data;
	*(ptrWrite+y6*bitmap.Width+x6) = color.data;
	while(xx < yy)
	{	
		xx++;
		x1++, x2--, x3++, x4--;
		y5++, y6++, y7--, y8--;
		if(pk < 0)
			pk += 2*xx+1;
		else
		{
			yy--;
			y1--, y2--, y3++, y4++;
			x5--, x6++, x7--, x8++;
			pk += 2*(xx-yy)+1;
		}
		*(ptrWrite+y2*bitmap.Width+x2) = color.data;
		*(ptrWrite+y6*bitmap.Width+x6) = color.data;
	}
}

static void UTFDrawMemoryCircleArcRB8(WORD x, WORD y, WORD r, UTFOSDCOL color)
{
	int xx,yy,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,y3,y4,y5,y6,y7,y8,pk;
	UTFBITMAP bitmap;
	BYTE *ptrWrite;

	bitmap = g_GlobalBitmap;
	ptrWrite = bitmap.data;

	xx = 0;
	yy = r;
	x1 = x, y1 = y + r;
	x2 = x, y2 = y + r;
	x3 = x, y3 = y - r;
	x4 = x, y4 = y - r;
	x5 = x + r, y5 = y;
	x6 = x - r, y6 = y;
	x7 = x + r, y7 = y;
	x8 = x - r, y8 = y;
	pk = 1 - r;

	*(ptrWrite+y1*bitmap.Width+x1) = color.data;
	*(ptrWrite+y5*bitmap.Width+x5) = color.data;
	while(xx < yy)
	{	
		xx++;
		x1++, x2--, x3++, x4--;
		y5++, y6++, y7--, y8--;
		if(pk < 0)
			pk += 2*xx+1;
		else
		{
			yy--;
			y1--, y2--, y3++, y4++;
			x5--, x6++, x7--, x8++;
			pk += 2*(xx-yy)+1;
		}
		*(ptrWrite+y1*bitmap.Width+x1) = color.data;
		*(ptrWrite+y5*bitmap.Width+x5) = color.data;
	}
}

static void UTFDrawMemoryCircleArcRB16(WORD x, WORD y, WORD r, UTFOSDCOL color)
{
	int xx,yy,x1,x2,x3,x4,x5,x6,x7,x8,y1,y2,y3,y4,y5,y6,y7,y8,pk;
	UTFBITMAP bitmap;
	WORD *ptrWrite;

	bitmap = g_GlobalBitmap;
	ptrWrite = (WORD *)bitmap.data;

	xx = 0;
	yy = r;
	x1 = x, y1 = y + r;
	x2 = x, y2 = y + r;
	x3 = x, y3 = y - r;
	x4 = x, y4 = y - r;
	x5 = x + r, y5 = y;
	x6 = x - r, y6 = y;
	x7 = x + r, y7 = y;
	x8 = x - r, y8 = y;
	pk = 1 - r;

	*(ptrWrite+y1*bitmap.Width+x1) = color.data;
	*(ptrWrite+y5*bitmap.Width+x5) = color.data;
	while(xx < yy)
	{	
		xx++;
		x1++, x2--, x3++, x4--;
		y5++, y6++, y7--, y8--;
		if(pk < 0)
			pk += 2*xx+1;
		else
		{
			yy--;
			y1--, y2--, y3++, y4++;
			x5--, x6++, x7--, x8++;
			pk += 2*(xx-yy)+1;
		}
		*(ptrWrite+y1*bitmap.Width+x1) = color.data;
		*(ptrWrite+y5*bitmap.Width+x5) = color.data;
	}
}

static void UTFMemoryDrawRoundRect8(WORD left, WORD top, WORD right, WORD bottom, BYTE radiu, UTFOSDCOL color)
{
	WORD wHeight = bottom-top;
	WORD wWidth = right-left;
	WORD wTemp;

	wTemp = wWidth>>1;
	if(radiu > wTemp)
	{
		radiu = wTemp;
	}

	wTemp = wHeight>>1;
	if(radiu > wTemp)
	{
		radiu = wTemp;
	}
	if(radiu < 2)
	{
		g_GraphProc.drawRect(left, top, right, bottom, color);
		return;
	}

	wTemp = radiu<<1;
	if(wTemp < wWidth)
	{
		g_GraphProc.drawLine(left+radiu, top, right-radiu, top, 1, color);
		g_GraphProc.drawLine(left+radiu, bottom-1, right-radiu, bottom-1, 1, color);
	}
	if(wTemp < wHeight)
	{
		g_GraphProc.drawLine(left, top+radiu, left, bottom-radiu, 1, color);
		g_GraphProc.drawLine(right-1, top+radiu, right-1, bottom-radiu, 1, color);
	}
	UTFDrawMemoryCircleArcLT8(left+radiu, top+radiu, radiu, color);
	UTFDrawMemoryCircleArcRT8(right-radiu-1, top+radiu, radiu, color);
	UTFDrawMemoryCircleArcLB8(left+radiu, bottom-radiu-1, radiu, color);
	UTFDrawMemoryCircleArcRB8(right-radiu-1, bottom-radiu-1, radiu, color);
}

static void UTFMemoryDrawRoundRect16(WORD left, WORD top, WORD right, WORD bottom, BYTE radiu, UTFOSDCOL color)
{
	WORD wHeight = bottom-top;
	WORD wWidth = right-left;
	WORD wTemp;

	wTemp = wWidth>>1;
	if(radiu > wTemp)
	{
		radiu = wTemp;
	}

	wTemp = wHeight>>1;
	if(radiu > wTemp)
	{
		radiu = wTemp;
	}
	if(radiu < 2)
	{
		g_GraphProc.drawRect(left, top, right, bottom, color);
		return;
	}

	wTemp = radiu<<1;
	if(wTemp < wWidth)
	{
		g_GraphProc.drawLine(left+radiu, top, right-radiu, top, 1, color);
		g_GraphProc.drawLine(left+radiu, bottom-1, right-radiu, bottom-1, 1, color);
	}
	if(wTemp < wHeight)
	{
		g_GraphProc.drawLine(left, top+radiu, left, bottom-radiu, 1, color);
		g_GraphProc.drawLine(right-1, top+radiu, right-1, bottom-radiu, 1, color);
	}
	UTFDrawMemoryCircleArcLT16(left+radiu, top+radiu, radiu, color);
	UTFDrawMemoryCircleArcRT16(right-radiu-1, top+radiu, radiu, color);
	UTFDrawMemoryCircleArcLB16(left+radiu, bottom-radiu-1, radiu, color);
	UTFDrawMemoryCircleArcRB16(right-radiu-1, bottom-radiu-1, radiu, color);
}

static void UTFMemoryFillCircleLT8(WORD xc, WORD yc, WORD r, UTFOSDCOL color)
{
	int a,b,a2,b2,x,delta;
	int loop,threshold;
	DWORD xCheck,yCheck;
	DWORD yFactor,i;
	UTFBITMAP bitmap;
	BYTE *ptrWrite;

	bitmap = g_GlobalBitmap;

	a = r;
	b = r;
	a2 = a*a;
	b2 = b*b;

	x = 0;
	delta = 8*b2 + a2*(1-2*b);
	loop = 6*b2;
	threshold = 4*a2*(1-b);
	yFactor = b;

	xCheck = b2;
	yCheck = b*a2;
	while(xCheck <= yCheck)
	{
		if(delta >= 0)
		{
			delta += threshold;
			threshold += 4*a2;
			yCheck -= a2;
			yFactor--;
		}		

		loop += 4*b2;
		delta += loop;
		xCheck += b2;
		x++;

		ptrWrite = bitmap.data+(yc-yFactor)*bitmap.Width+xc-x;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
	}

	x = a;
	delta = 8*a2 + b2*(1-2*a);
	loop = 6*a2;
	threshold = 4*b2*(1-a);
	yFactor = 1;
	xCheck = a*b2;
	yCheck = a2;

	ptrWrite = bitmap.data+yc*bitmap.Width+xc-x;
	for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }

	while(yCheck <= xCheck)
	{
		ptrWrite = bitmap.data+(yc-yFactor)*bitmap.Width+xc-x;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
		
		if(delta >= 0)
		{
			x--;
			delta += threshold;
			threshold += 4*b2;
			xCheck -= b2;
		}

		loop += 4*a2;
		delta += loop;
		yCheck += a2;
		yFactor++;
	}
}

static void UTFMemoryFillCircleLT16(WORD xc, WORD yc, WORD r, UTFOSDCOL color)
{
	int a,b,a2,b2,x,delta;
	int loop,threshold;
	DWORD xCheck,yCheck;
	DWORD yFactor,i;
	UTFBITMAP bitmap;
	WORD *ptrWrite;

	bitmap = g_GlobalBitmap;

	a = r;
	b = r;
	a2 = a*a;
	b2 = b*b;

	x = 0;
	delta = 8*b2 + a2*(1-2*b);
	loop = 6*b2;
	threshold = 4*a2*(1-b);
	yFactor = b;

	xCheck = b2;
	yCheck = b*a2;
	while(xCheck <= yCheck)
	{
		if(delta >= 0)
		{
			delta += threshold;
			threshold += 4*a2;
			yCheck -= a2;
			yFactor--;
		}		

		loop += 4*b2;
		delta += loop;
		xCheck += b2;
		x++;

		ptrWrite = (WORD *)bitmap.data;
		ptrWrite += (yc-yFactor)*bitmap.Width+xc-x;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
	}

	x = a;
	delta = 8*a2 + b2*(1-2*a);
	loop = 6*a2;
	threshold = 4*b2*(1-a);
	yFactor = 1;
	xCheck = a*b2;
	yCheck = a2;

	ptrWrite = (WORD *)bitmap.data;
	ptrWrite += yc*bitmap.Width+xc-x;
	for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }

	while(yCheck <= xCheck)
	{
		ptrWrite = (WORD *)bitmap.data;
		ptrWrite += (yc-yFactor)*bitmap.Width+xc-x;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
		
		if(delta >= 0)
		{
			x--;
			delta += threshold;
			threshold += 4*b2;
			xCheck -= b2;
		}

		loop += 4*a2;
		delta += loop;
		yCheck += a2;
		yFactor++;
	}
}

static void UTFMemoryFillCircleRT8(WORD xc, WORD yc, WORD r, UTFOSDCOL color)
{
	int a,b,a2,b2,x,delta;
	int loop,threshold;
	DWORD xCheck,yCheck;
	DWORD yFactor,i;
	UTFBITMAP bitmap;
	BYTE *ptrWrite;

	bitmap = g_GlobalBitmap;

	a = r;
	b = r;
	a2 = a*a;
	b2 = b*b;

	x = 0;
	delta = 8*b2 + a2*(1-2*b);
	loop = 6*b2;
	threshold = 4*a2*(1-b);
	yFactor = b;

	xCheck = b2;
	yCheck = b*a2;

	*(bitmap.data+(yc-b)*bitmap.Width+xc) = color.data;
	while(xCheck <= yCheck)
	{
		if(delta >= 0)
		{
			delta += threshold;
			threshold += 4*a2;
			yCheck -= a2;
			yFactor--;
		}		

		loop += 4*b2;
		delta += loop;
		xCheck += b2;
		x++;

		ptrWrite = bitmap.data+(yc-yFactor)*bitmap.Width+xc;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
	}

	x = a;
	delta = 8*a2 + b2*(1-2*a);
	loop = 6*a2;
	threshold = 4*b2*(1-a);
	yFactor = 1;
	xCheck = a*b2;
	yCheck = a2;

	ptrWrite = bitmap.data+yc*bitmap.Width+xc;
	for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }

	while(yCheck <= xCheck)
	{
		ptrWrite = bitmap.data+(yc-yFactor)*bitmap.Width+xc;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
		
		if(delta >= 0)
		{
			x--;
			delta += threshold;
			threshold += 4*b2;
			xCheck -= b2;
		}

		loop += 4*a2;
		delta += loop;
		yCheck += a2;
		yFactor++;
	}
}

static void UTFMemoryFillCircleRT16(WORD xc, WORD yc, WORD r, UTFOSDCOL color)
{
	int a,b,a2,b2,x,delta;
	int loop,threshold;
	DWORD xCheck,yCheck;
	DWORD yFactor,i;
	UTFBITMAP bitmap;
	WORD *ptrWrite;

	bitmap = g_GlobalBitmap;

	a = r;
	b = r;
	a2 = a*a;
	b2 = b*b;

	x = 0;
	delta = 8*b2 + a2*(1-2*b);
	loop = 6*b2;
	threshold = 4*a2*(1-b);
	yFactor = b;

	xCheck = b2;
	yCheck = b*a2;

	ptrWrite = (WORD *)bitmap.data;
	ptrWrite += (yc-b)*bitmap.Width+xc;
	*ptrWrite = color.data;
	while(xCheck <= yCheck)
	{
		if(delta >= 0)
		{
			delta += threshold;
			threshold += 4*a2;
			yCheck -= a2;
			yFactor--;
		}		

		loop += 4*b2;
		delta += loop;
		xCheck += b2;
		x++;

		ptrWrite = (WORD *)bitmap.data;
		ptrWrite += (yc-yFactor)*bitmap.Width+xc;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
	}

	x = a;
	delta = 8*a2 + b2*(1-2*a);
	loop = 6*a2;
	threshold = 4*b2*(1-a);
	yFactor = 1;
	xCheck = a*b2;
	yCheck = a2;

	ptrWrite = (WORD *)bitmap.data;
	ptrWrite += yc*bitmap.Width+xc;
	for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }

	while(yCheck <= xCheck)
	{
		ptrWrite = (WORD *)bitmap.data;
		ptrWrite += (yc-yFactor)*bitmap.Width+xc;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
		
		if(delta >= 0)
		{
			x--;
			delta += threshold;
			threshold += 4*b2;
			xCheck -= b2;
		}

		loop += 4*a2;
		delta += loop;
		yCheck += a2;
		yFactor++;
	}
}

static void UTFMemoryFillCircleLB8(WORD xc, WORD yc, WORD r, UTFOSDCOL color)
{
	int a,b,a2,b2,x,delta;
	int loop,threshold;
	DWORD xCheck,yCheck;
	DWORD yFactor,i;
	UTFBITMAP bitmap;
	BYTE *ptrWrite;

	bitmap = g_GlobalBitmap;

	a = r;
	b = r;
	a2 = a*a;
	b2 = b*b;

	x = 0;
	delta = 8*b2 + a2*(1-2*b);
	loop = 6*b2;
	threshold = 4*a2*(1-b);
	yFactor = b;

	xCheck = b2;
	yCheck = b*a2;
	while(xCheck <= yCheck)
	{
		if(delta >= 0)
		{
			delta += threshold;
			threshold += 4*a2;
			yCheck -= a2;
			yFactor--;
		}		

		loop += 4*b2;
		delta += loop;
		xCheck += b2;
		x++;

		ptrWrite = bitmap.data+(yc+yFactor)*bitmap.Width+xc-x;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
	}

	x = a;
	delta = 8*a2 + b2*(1-2*a);
	loop = 6*a2;
	threshold = 4*b2*(1-a);
	yFactor = 1;
	xCheck = a*b2;
	yCheck = a2;

	ptrWrite = bitmap.data+yc*bitmap.Width+xc-x;
	for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }

	while(yCheck <= xCheck)
	{
		ptrWrite = bitmap.data+(yc+yFactor)*bitmap.Width+xc-x;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
		
		if(delta >= 0)
		{
			x--;
			delta += threshold;
			threshold += 4*b2;
			xCheck -= b2;
		}

		loop += 4*a2;
		delta += loop;
		yCheck += a2;
		yFactor++;
	}
}

static void UTFMemoryFillCircleLB16(WORD xc, WORD yc, WORD r, UTFOSDCOL color)
{
	int a,b,a2,b2,x,delta;
	int loop,threshold;
	DWORD xCheck,yCheck;
	DWORD yFactor,i;
	UTFBITMAP bitmap;
	WORD *ptrWrite;

	bitmap = g_GlobalBitmap;

	a = r;
	b = r;
	a2 = a*a;
	b2 = b*b;

	x = 0;
	delta = 8*b2 + a2*(1-2*b);
	loop = 6*b2;
	threshold = 4*a2*(1-b);
	yFactor = b;

	xCheck = b2;
	yCheck = b*a2;
	while(xCheck <= yCheck)
	{
		if(delta >= 0)
		{
			delta += threshold;
			threshold += 4*a2;
			yCheck -= a2;
			yFactor--;
		}		

		loop += 4*b2;
		delta += loop;
		xCheck += b2;
		x++;

		ptrWrite = (WORD *)bitmap.data;
		ptrWrite += (yc+yFactor)*bitmap.Width+xc-x;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
	}

	x = a;
	delta = 8*a2 + b2*(1-2*a);
	loop = 6*a2;
	threshold = 4*b2*(1-a);
	yFactor = 1;
	xCheck = a*b2;
	yCheck = a2;

	ptrWrite = (WORD *)bitmap.data;
	ptrWrite += yc*bitmap.Width+xc-x;
	for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }

	while(yCheck <= xCheck)
	{
		ptrWrite = (WORD *)bitmap.data;
		ptrWrite += (yc+yFactor)*bitmap.Width+xc-x;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
		
		if(delta >= 0)
		{
			x--;
			delta += threshold;
			threshold += 4*b2;
			xCheck -= b2;
		}

		loop += 4*a2;
		delta += loop;
		yCheck += a2;
		yFactor++;
	}
}

static void UTFMemoryFillCircleRB8(WORD xc, WORD yc, WORD r, UTFOSDCOL color)
{
	int a,b,a2,b2,x,delta;
	int loop,threshold;
	DWORD xCheck,yCheck;
	DWORD yFactor,i;
	UTFBITMAP bitmap;
	BYTE *ptrWrite;

	bitmap = g_GlobalBitmap;

	a = r;
	b = r;
	a2 = a*a;
	b2 = b*b;

	x = 0;
	delta = 8*b2 + a2*(1-2*b);
	loop = 6*b2;
	threshold = 4*a2*(1-b);
	yFactor = b;

	xCheck = b2;
	yCheck = b*a2;

	*(bitmap.data+(yc+b)*bitmap.Width+xc) = color.data;
	while(xCheck <= yCheck)
	{
		if(delta >= 0)
		{
			delta += threshold;
			threshold += 4*a2;
			yCheck -= a2;
			yFactor--;
		}		

		loop += 4*b2;
		delta += loop;
		xCheck += b2;
		x++;

		ptrWrite = bitmap.data+(yc+yFactor)*bitmap.Width+xc;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
	}

	x = a;
	delta = 8*a2 + b2*(1-2*a);
	loop = 6*a2;
	threshold = 4*b2*(1-a);
	yFactor = 1;
	xCheck = a*b2;
	yCheck = a2;

	ptrWrite = bitmap.data+yc*bitmap.Width+xc;
	for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }

	while(yCheck <= xCheck)
	{
		ptrWrite = bitmap.data+(yc+yFactor)*bitmap.Width+xc;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
		
		if(delta >= 0)
		{
			x--;
			delta += threshold;
			threshold += 4*b2;
			xCheck -= b2;
		}

		loop += 4*a2;
		delta += loop;
		yCheck += a2;
		yFactor++;
	}
}

static void UTFMemoryFillCircleRB16(WORD xc, WORD yc, WORD r, UTFOSDCOL color)
{
	int a,b,a2,b2,x,delta;
	int loop,threshold;
	DWORD xCheck,yCheck;
	DWORD yFactor,i;
	UTFBITMAP bitmap;
	WORD *ptrWrite;

	bitmap = g_GlobalBitmap;

	a = r;
	b = r;
	a2 = a*a;
	b2 = b*b;

	x = 0;
	delta = 8*b2 + a2*(1-2*b);
	loop = 6*b2;
	threshold = 4*a2*(1-b);
	yFactor = b;

	xCheck = b2;
	yCheck = b*a2;

	ptrWrite = (WORD *)bitmap.data;
	ptrWrite += (yc+b)*bitmap.Width+xc;
	*ptrWrite = color.data;
	while(xCheck <= yCheck)
	{
		if(delta >= 0)
		{
			delta += threshold;
			threshold += 4*a2;
			yCheck -= a2;
			yFactor--;
		}		

		loop += 4*b2;
		delta += loop;
		xCheck += b2;
		x++;

		ptrWrite = (WORD *)bitmap.data;
		ptrWrite += (yc+yFactor)*bitmap.Width+xc;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
	}

	x = a;
	delta = 8*a2 + b2*(1-2*a);
	loop = 6*a2;
	threshold = 4*b2*(1-a);
	yFactor = 1;
	xCheck = a*b2;
	yCheck = a2;

	ptrWrite = (WORD *)bitmap.data;
	ptrWrite += yc*bitmap.Width+xc;
	for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }

	while(yCheck <= xCheck)
	{
		ptrWrite = (WORD *)bitmap.data;
		ptrWrite += (yc+yFactor)*bitmap.Width+xc;
		for(i=0; i<x; i++) { *(ptrWrite+i) = color.data; }
		
		if(delta >= 0)
		{
			x--;
			delta += threshold;
			threshold += 4*b2;
			xCheck -= b2;
		}

		loop += 4*a2;
		delta += loop;
		yCheck += a2;
		yFactor++;
	}
}

static void UTFMemoryFillRoundRect8(WORD left, WORD top, WORD right, WORD bottom, BYTE radiu, UTFOSDCOL color)
{
	WORD wHeight = bottom-top;
	WORD wWidth = right-left;
	WORD wTemp;

	wTemp = wWidth>>1;
	if(radiu > wTemp)
	{
		radiu = wTemp;
	}

	wTemp = wHeight>>1;
	if(radiu > wTemp)
	{
		radiu = wTemp;
	}
	if(radiu < 2)
	{
		g_GraphProc.fillRect(left, top, right, bottom, color);
		return;
	}

	wTemp = radiu<<1;
	if(wTemp < wWidth)
	{
		g_GraphProc.fillRect(left+radiu, top, right-radiu, bottom, color);
	}
	if(wTemp < wHeight)
	{
		g_GraphProc.fillRect(left, top+radiu, right, bottom-radiu, color);
	}
	UTFMemoryFillCircleLT8(left+radiu, top+radiu, radiu, color);
	UTFMemoryFillCircleRT8(right-radiu, top+radiu, radiu, color);
	UTFMemoryFillCircleLB8(left+radiu, bottom-radiu-1, radiu, color);
	UTFMemoryFillCircleRB8(right-radiu, bottom-radiu-1, radiu, color);
}

static void UTFMemoryFillRoundRect16(WORD left, WORD top, WORD right, WORD bottom, BYTE radiu, UTFOSDCOL color)
{
	WORD wHeight = bottom-top;
	WORD wWidth = right-left;
	WORD wTemp;

	wTemp = wWidth>>1;
	if(radiu > wTemp)
	{
		radiu = wTemp;
	}

	wTemp = wHeight>>1;
	if(radiu > wTemp)
	{
		radiu = wTemp;
	}
	if(radiu < 2)
	{
		g_GraphProc.fillRect(left, top, right, bottom, color);
		return;
	}

	wTemp = radiu<<1;
	if(wTemp < wWidth)
	{
		g_GraphProc.fillRect(left+radiu, top, right-radiu, bottom, color);
	}
	if(wTemp < wHeight)
	{
		g_GraphProc.fillRect(left, top+radiu, right, bottom-radiu, color);
	}
	UTFMemoryFillCircleLT16(left+radiu, top+radiu, radiu, color);
	UTFMemoryFillCircleRT16(right-radiu, top+radiu, radiu, color);
	UTFMemoryFillCircleLB16(left+radiu, bottom-radiu-1, radiu, color);
	UTFMemoryFillCircleRB16(right-radiu, bottom-radiu-1, radiu, color);
}

int UTFBitBlt8(LPUTFBITMAP lpSrcBmp, WORD wX, WORD wY, WORD wWidth, WORD wHeight, LPUTFBITMAP lpDstBmp, WORD wLeft, WORD wTop)
{
	DWORD hDstDrawDC = 0;
	
	g_bUpdateScreenFlag = 1;

	if(lpSrcBmp == NULL)
		return 0;
	if(lpSrcBmp->data == NULL)
		return 0;

	if(wX >= lpSrcBmp->Width)
		return 0;
	if(wY >= lpSrcBmp->Height)
		return 0;

	if((wX+wWidth) > lpSrcBmp->Width)
		wWidth = lpSrcBmp->Width-wX;
	if((wY+wHeight) > lpSrcBmp->Height)
		wHeight = lpSrcBmp->Height-wY;

	if(lpDstBmp)
	{
		if(lpDstBmp->data)
		{
			hDstDrawDC = 1;
		}
	}
	
	if(hDstDrawDC)	//copy source bitmap to target bitmap
	{
		BYTE *pRead,*pWrite;
		WORD i;
	#if 1//vivian modified here 20080605
		WORD SrcStride, DstStride;
	#endif

		if(wLeft >= lpDstBmp->Width)
			return 0;
		if(wTop >= lpDstBmp->Height)
			return 0;

		if((wLeft+wWidth) > lpDstBmp->Width)
			wWidth = lpDstBmp->Width-wLeft;
		if((wTop+wHeight) > lpDstBmp->Height)
			wHeight = lpDstBmp->Height-wTop;

		pRead = lpSrcBmp->data+wY*lpSrcBmp->Width+wX;
		pWrite = lpDstBmp->data+wTop*lpDstBmp->Width+wLeft;

	#if 1//vivian modified here 20080605
		SrcStride = (lpSrcBmp->Width * lpSrcBmp->bitPerPixel)/8;
		DstStride = (lpDstBmp->Width * lpDstBmp->bitPerPixel)/8;
			GfxCopyRectMem(pWrite, pRead, DstStride, SrcStride, wWidth, wHeight,
								0, 0, 0, 0, lpDstBmp->bitPerPixel);
	#else
		for(i=0; i<wHeight; i++)
		{
			memcpy(pWrite, pRead, wWidth);

			pRead += lpSrcBmp->Width;
			pWrite += lpDstBmp->Width;
		}
	#endif
	}
	else	//copy bitmap to screen
	{
		UTFRECT rcScreen;
		UTFRECT rcFullScreen;

		UTFGetOSDRgnRect(&rcScreen);
		UTFGetFullScreenRect(&rcFullScreen);

		wLeft += rcScreen.left;
		wTop += rcScreen.top;

		if(wLeft >= rcFullScreen.right)
			return 0;
		if(wTop >= rcFullScreen.bottom)
			return 0;

		if((wLeft+wWidth) > rcFullScreen.right)
			wWidth = rcFullScreen.right-wLeft;
		if((wTop+wHeight) > rcFullScreen.bottom)
			wHeight = rcFullScreen.bottom-wTop;

		blt_image_to_osd(lpSrcBmp->data, wX, wY, wWidth, wHeight, wLeft, wTop);
	}

	return 1;
}

int UTFBitBlt16(LPUTFBITMAP lpSrcBmp, WORD wX, WORD wY, WORD wWidth, WORD wHeight, LPUTFBITMAP lpDstBmp, WORD wLeft, WORD wTop)
{
	DWORD hDstDrawDC = 0;
	
	g_bUpdateScreenFlag = 1;

	if(lpSrcBmp == NULL)
		return 0;
	if(lpSrcBmp->data == NULL)
		return 0;

	if(wX >= lpSrcBmp->Width)
		return 0;
	if(wY >= lpSrcBmp->Height)
		return 0;

	if((wX+wWidth) > lpSrcBmp->Width)
		wWidth = lpSrcBmp->Width-wX;
	if((wY+wHeight) > lpSrcBmp->Height)
		wHeight = lpSrcBmp->Height-wY;

	if(lpDstBmp)
	{
		if(lpDstBmp->data)
		{
			hDstDrawDC = 1;
		}
	}
	
	if(hDstDrawDC)	//copy source bitmap to target bitmap
	{
		WORD *pRead,*pWrite;
		WORD i;
	#if 1//vivian modified here 20080605
		WORD SrcStride, DstStride;
	#endif
		if(wLeft >= lpDstBmp->Width)
			return 0;
		if(wTop >= lpDstBmp->Height)
			return 0;

		if((wLeft+wWidth) > lpDstBmp->Width)
			wWidth = lpDstBmp->Width-wLeft;
		if((wTop+wHeight) > lpDstBmp->Height)
			wHeight = lpDstBmp->Height-wTop;

		pRead = (WORD *)lpSrcBmp->data;
		pRead += wY*lpSrcBmp->Width+wX;
		pWrite = (WORD *)lpDstBmp->data;
		pWrite += wTop*lpDstBmp->Width+wLeft;
	#if 1//vivian modified here 20080605
		//trace(" UTFBitBlt16   start copy   100 ++++ \n");
		SrcStride = (lpSrcBmp->Width * lpSrcBmp->bitPerPixel)/8;
		DstStride = (lpDstBmp->Width * lpDstBmp->bitPerPixel)/8;
	//	for(i=0; i < 20; i++)
		//{
			GfxCopyRectMem(pWrite, pRead, DstStride, SrcStride, wWidth, wHeight,
								0, 0, 0, 0, lpDstBmp->bitPerPixel);
	//	}
	//	trace(" UTFBitBlt16   end copy   ++++ \n");
	#else
				for(i=0; i<wHeight; i++)
				{
					memcpy(pWrite, pRead, wWidth<<1);

					pRead += lpSrcBmp->Width;
					pWrite += lpDstBmp->Width;
				}
	#endif
	}
	else	//copy bitmap to screen
	{
		UTFRECT rcScreen;
		UTFRECT rcFullScreen;

		UTFGetOSDRgnRect(&rcScreen);
		UTFGetFullScreenRect(&rcFullScreen);

		wLeft += rcScreen.left;
		wTop += rcScreen.top;

		if(wLeft >= rcFullScreen.right)
			return 0;
		if(wTop >= rcFullScreen.bottom)
			return 0;

		if((wLeft+wWidth) > rcFullScreen.right)
			wWidth = rcFullScreen.right-wLeft;
		if((wTop+wHeight) > rcFullScreen.bottom)
			wHeight = rcFullScreen.bottom-wTop;

		blt_image_to_osd(lpSrcBmp->data, wX, wY, wWidth, wHeight, wLeft, wTop);
	}

	return 1;
}
//End of define memory draw function
/////////////////////////////////////////////////////////////////////////////

int UTFLineDraw(WORD x1, WORD y1, WORD x2, WORD y2, WORD penWidth, UTFOSDCOL color, DWORD bUsedLater)
{
	if(g_GlobalBitmap.data)	//draw to memory
	{
		g_GraphProc.drawLine(x1, y1, x2, y2, 1, color);
	}
	else	//draw to screen
	{
		DG_AL_draw_line(x1, y1, x2, y2, color.data);//Change5551DataTo8888Data((WORD)color.data));//color.data);
	}

	g_bUpdateScreenFlag = 1;

	return 0;
}

int UTFRectDraw(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color, DWORD bUsedLater)
{
	if(g_GlobalBitmap.data)	//draw to memory
	{
		g_GraphProc.drawRect(left, top, right, bottom, color);
	}
	else	//draw to screen
	{
		//mwom_rect_draw(left, top, right, bottom, Change5551DataTo8888Data((WORD)color.data));//color.data);color.data);
		DG_AL_draw_rect(left, top, right, bottom, color.data);//Change5551DataTo8888Data((WORD)color.data));
	}

	g_bUpdateScreenFlag = 1;

	return 0;
}

int UTFRectFill(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color, DWORD bUsedLater)
{
	if(g_GlobalBitmap.data)	//draw to memory
	{
		g_GraphProc.fillRect(left, top, right, bottom, color);
	}
	else	//draw to screen
	{
		DG_AL_fill_rect(left, top, right, bottom, color.data);//Change5551DataTo8888Data((WORD)color.data));//color.data);color.data);
	}

	g_bUpdateScreenFlag = 1;

	return 0;
}

int UTFEllipseDraw(WORD left, WORD top, WORD right, WORD bottom, WORD penWidth, UTFOSDCOL color, DWORD bUsedLater)
{
	if(g_GlobalBitmap.data)	//draw to memory
	{
		g_GraphProc.drawEllipse(left, top, right, bottom, 1, color);
	}
	else	//draw to screen
	{
		int x,y;
		WORD a,b;

		x = (left+right)>>1;
		y = (top+bottom)>>1;
		a = (right-left)>>1;
		b = (bottom-top)>>1;
		DG_AL_draw_ellipse(x, y, a, b, 1,color.data);//Change5551DataTo8888Data((WORD)color.data));//color.data); color.data);
	}
	
	g_bUpdateScreenFlag = 1;

	return 1;
}

int UTFEllipseFill(WORD left, WORD top, WORD right, WORD bottom, UTFOSDCOL color, DWORD bUsedLater)
{
	UTFBITMAP bitmap;
	UTFRECT rcScreen;
	BYTE BytesPerPixel;

	g_bUpdateScreenFlag = 1;

	if(g_GlobalBitmap.data)	//draw to memory
	{
		g_GraphProc.fillEllipse(left, top, right, bottom, color);
		return 1;
	}

	BytesPerPixel = (g_OSDInfo.bitPerPixel>>3);

	//start initialize bitmap data
	bitmap.Width = right-left;
	bitmap.Height = bottom-top;
	bitmap.bitPerPixel = g_OSDInfo.bitPerPixel;
	bitmap.data = (BYTE *)SOWMalloc(bitmap.Width*bitmap.Height*BytesPerPixel);
	if(bitmap.data == NULL)
		return 0;

	UTFGetOSDRgnRect(&rcScreen);
	left -= rcScreen.left;
	top -= rcScreen.top;

	UTFSaveScreen(left, top, &bitmap, 0);

	UTFSetMemoryBitmap(&bitmap);
	g_GraphProc.fillEllipse(0, 0, bitmap.Width, bitmap.Height, color);
	UTFSetMemoryBitmap(NULL);

	UTFPutScreen(left, top, &bitmap, 0);
	SOWFree((void *)bitmap.data);

	return 1;
}

int UTFRoundRectDraw(WORD left, WORD top, WORD right, WORD bottom, BYTE radiu, WORD penWidth, UTFOSDCOL color, DWORD bUsedLater)
{
	if(g_GlobalBitmap.data)	//draw to memory
	{
		g_GraphProc.drawRoundRect(left, top, right, bottom, radiu, color);
	}
	else	//draw to screen
	{
		RECT rect;
		WORD i;

		rect.x1 = left;
		rect.y1 = top;
		rect.x2 = right;
		rect.y2 = bottom;
		for(i=0; i<penWidth; i++)
		{
			DG_AL_draw_round_rect((DG_AL_RECT*)&rect, 1, radiu-i, 0,color.data);//Change5551DataTo8888Data((WORD)color.data));//color.data); color.data);

			rect.x1++;
			rect.y1++;
			rect.x2--;
			rect.y2--;
		}
	}

	g_bUpdateScreenFlag = 1;
	
	return 0;
}

int UTFRoundRectFill(WORD left, WORD top, WORD right, WORD bottom, BYTE radiu, UTFOSDCOL color, DWORD bUsedLater)
{
	if(g_GlobalBitmap.data)	//draw to memory
	{
		g_GraphProc.fillRoundRect(left, top, right, bottom, radiu, color);
	}
	else	//draw to screen
	{
		RECT rect;

		rect.x1 = left;
		rect.y1 = top;
		rect.x2 = right;
		rect.y2 = bottom;
		DG_AL_fill_round_rect((DG_AL_RECT*)&rect, radiu, 0, color.data);//Change5551DataTo8888Data((WORD)color.data));//color.data);color.data);
	}

	g_bUpdateScreenFlag = 1;
	
	return 0;
}

int UTFArc(WORD left, WORD top, WORD right, WORD bottom, WORD startAngle, WORD endAngle, UTFOSDCOL color, DWORD bUsedLater)
{
	g_bUpdateScreenFlag = 1;
	
	return 0;
}

int UTFPolygonDraw(LPUTFPOINT lpPoint, WORD nPoint, WORD penWidth, UTFOSDCOL color, DWORD bUsedLater)
{
	if(nPoint < 3)
		return 0;

	if(g_GlobalBitmap.data)	//draw to memory
	{
		WORD i;

		for(i=1; i<nPoint; i++)
		{
			g_GraphProc.drawLine(lpPoint[i-1].x, lpPoint[i-1].y, lpPoint[i].x, lpPoint[i].y, 1, color);
		}
		g_GraphProc.drawLine(lpPoint[0].x, lpPoint[0].y, lpPoint[nPoint-1].x, lpPoint[nPoint-1].y, 1, color);
	}
	else	//draw to screen
	{
		//mwom_polygon((PGFX_XY)lpPoint, nPoint, Change5551DataTo8888Data((WORD)color.data));//color.data);color.data);
		DG_AL_draw_polygon((PDG_AL_GFX_XY)lpPoint, nPoint, color.data);//Change5551DataTo8888Data((WORD)color.data));
	}

	g_bUpdateScreenFlag = 1;

	return 0;
}

int UTFPolygonFill(LPUTFPOINT lpPoint, WORD nPoint, UTFOSDCOL color, DWORD bUsedLater)
{
	if(nPoint < 3)
		return 0;

	if(g_GlobalBitmap.data)	//draw to memory
	{
		g_GraphProc.fillPolygon(lpPoint, nPoint, color);
	}
	else	//draw to screen
	{
	//	mwom_polygon((PGFX_XY)lpPoint, nPoint, Change5551DataTo8888Data((WORD)color.data));//color.data);color.data);
		DG_AL_draw_polygon((PDG_AL_GFX_XY)lpPoint, nPoint, color.data);//Change5551DataTo8888Data((WORD)color.data));
	}

	g_bUpdateScreenFlag = 1;
	
	return 0;
}

int UTFPixelDraw(WORD x, WORD y, UTFOSDCOL color, DWORD bUsedLater)
{
	if(g_GlobalBitmap.data)	//draw to memory
	{
		g_GraphProc.drawPixel(x, y, color);
	}
	else	//draw to screen
	{
		
	//	mwom_pixel(x,y,Change5551DataTo8888Data((WORD)color.data));//color.data);
		DG_AL_draw_pixel(x,y,color.data);//Change5551DataTo8888Data((WORD)color.data));
	}

	g_bUpdateScreenFlag = 1;

	return 0;
}

int UTFSaveScreen(WORD left, WORD top, LPUTFBITMAP lpBitmap, DWORD bUsedLater)
{
	UTFRECT rcScreen;

	UTFGetOSDRgnRect(&rcScreen);
	
	left += rcScreen.left;
	top += rcScreen.top;

	blt_osd_to_memory(lpBitmap->data, left, top, lpBitmap->Width, lpBitmap->Height);
	
	return 0;
}

int UTFPutScreen(WORD left, WORD top, LPUTFBITMAP lpBitmap, DWORD bUsedLater)
{
	UTFRECT rcScreen;

	g_bUpdateScreenFlag = 1;
	
	UTFGetOSDRgnRect(&rcScreen);
	
	left += rcScreen.left;
	top += rcScreen.top;

	blt_image_to_osd(lpBitmap->data, 0, 0, lpBitmap->Width, lpBitmap->Height, left, top);
	
	return 0;
}

int UTFBitBlt(LPUTFBITMAP lpSrcBmp, WORD wX, WORD wY, WORD wWidth, WORD wHeight, LPUTFBITMAP lpDstBmp, WORD wLeft, WORD wTop)
{
	return g_GraphProc.bitblt(lpSrcBmp, wX, wY, wWidth, wHeight, lpDstBmp, wLeft, wTop);
}

int UTFUpdateScreen(void)
{
	if(g_bUpdateScreenFlag)
	{
		g_bUpdateScreenFlag = 0;
		
		copy_memory_bitmap_to_screen();

		return 1;
	}
	
	return 0;
}

int UTFInstallPalette(BYTE pos, BYTE r, BYTE g, BYTE b, BYTE alpha, DWORD bUsedLater)
{
	UTFOSDCOL color;

	color.rgb.red = r;
	color.rgb.green = g;
	color.rgb.blue = b;
	color.rgb.alpha = alpha;
	UTFSetPalette(pos, color.data);
	
	return 0;
}

int UTFSetOSDAlpha(BYTE bNewAlpha, DWORD bUsedLater)
{
	mwom_set_osd_alpha(bNewAlpha);
	
	return 0;
}

int UTFGetOSDInfo(LPUTFOSDInfo lpInfo)
{
	if(lpInfo != NULL)
	{
		memcpy(lpInfo, &g_OSDInfo, sizeof(UTFOSDInfo));
		return 1;
	}

	return 0;
}

BYTE *UTF_OSMalloc(DWORD size)
{
	return (BYTE *)OS_Malloc(size);
}

void UTF_OSFree(BYTE *pAddr)
{
	OS_Free(pAddr);
}

int UTFAPI UTFCreateBitmap(WORD wWidth, WORD wHeight, LPUTFBITMAP lpBitmap, BYTE bInit)
{
	if(lpBitmap)
	{
		DWORD dwSize;

		if(bInit == 0)
		{
			if(lpBitmap->data != NULL)
			{
				return 0;
			}
		}

		lpBitmap->Width = wWidth;
		lpBitmap->Height = wHeight;
		lpBitmap->bitPerPixel = g_OSDInfo.bitPerPixel;
		lpBitmap->hDC = NULL;
		lpBitmap->hBitmap = NULL;

		dwSize = (lpBitmap->Width*lpBitmap->Height*lpBitmap->bitPerPixel)/8;
#if 0
#if (defined(UTF_MOMERY_BITMAP) && (UTF_MOMERY_BITMAP == TRUE))

		lpBitmap->pOrigin = (BYTE *)SOWMalloc(dwSize+32);//UTF_OSMalloc(dwSize+32);
#else

		lpBitmap->pOrigin = (BYTE *)UTF_OSMalloc(dwSize+32);//SOWMallocBitmap(dwSize+32);//
#endif
#else
if(g_IsMemoryDraw)
	lpBitmap->pOrigin = (BYTE *)SOWMalloc(dwSize+32);//UTF_OSMalloc(dwSize+32);
else
	lpBitmap->pOrigin = (BYTE *)UTF_OSMalloc(dwSize+32);//SOWMallocBitmap(dwSize+32);
#endif
		lpBitmap->data = lpBitmap->pOrigin;
		if(lpBitmap->pOrigin)
		{
			dwSize = (DWORD)lpBitmap->pOrigin;
			lpBitmap->data += (8-dwSize%8);

			return 1;
		}
	}
	
	return 0;
}

int UTFAPI UTFDeleteBitmap(LPUTFBITMAP lpBitmap)
{
	if(lpBitmap)
	{
		if(lpBitmap->data)
		{
//#if( !(defined(UTF_MOMERY_BITMAP) && (UTF_MOMERY_BITMAP == TRUE)))
			if(g_IsMemoryDraw)
			{	
				UTF_OSFree(lpBitmap->pOrigin);
				lpBitmap->data = NULL;
				lpBitmap->pOrigin = NULL;
			}
//#endif
		}

		return 1;
	}
	
	return 0;
}


void UTFAPI UTFSetMemoryBitmap(LPUTFBITMAP lpBitmap)
{
	if(lpBitmap)
	{
		memcpy(&g_GlobalBitmap, lpBitmap, sizeof(UTFBITMAP));

		if(g_GlobalBitmap.data)
			UTFSetScreenDraw(0);
		else
			UTFSetScreenDraw(1);
	}
	else
	{
		UTFSetScreenDraw(1);
		g_GlobalBitmap = g_GlobalBitmapBackup;
	}
}

void UTFGetMemoryBitmap(LPUTFBITMAP lpBitmap)
{
	if(lpBitmap)
	{
		memcpy(lpBitmap, &g_GlobalBitmap, sizeof(UTFBITMAP));
	}
}

/******************************************************************/

int UTFGraphInitialize(void)
{
	//Initialize OSD information
	g_OSDInfo.bitPerPixel = 8;
	g_OSDInfo.bGraphMode = UTF_DEPTH_8;
	g_OSDInfo.MaxBitNum = 16;
	g_OSDInfo.dwStyle = 0;
	
	g_bUpdateScreenFlag = 0;

	//Initialize global bitmap data
	/************吴小辉修改内存画图***************/
//#if (defined(UTF_MOMERY_BITMAP) && (UTF_MOMERY_BITMAP == TRUE))
if(g_IsMemoryDraw)
{
	UTFCreateBitmap(720, 576, &g_GlobalBitmap, 1);
	UTFSetMemoryBitmap(&g_GlobalBitmap);
}
//#else	
else
{
	g_GlobalBitmap.Width = 0;
	g_GlobalBitmap.Height = 0;
	g_GlobalBitmap.bitPerPixel = g_OSDInfo.bitPerPixel;
	g_GlobalBitmap.data = NULL;
	g_GlobalBitmap.hDC = NULL;
	g_GlobalBitmap.hBitmap = NULL;
	UTFSetScreenDraw(1);
}
//#endif	
	
	g_GlobalBitmapBackup = g_GlobalBitmap;



	return 1;
}

void UTFDareIsMemoryDraw(int flag)
{
	g_IsMemoryDraw = flag;
}

void UTFDareMemorySwitch(int flag)
{
	if(flag)	//内存画图
	{
		if(!g_GlobalBitmap.data)
		{
			UTFCreateBitmap(720, 576, &g_GlobalBitmap, 1);
			UTFSetMemoryBitmap(&g_GlobalBitmap);
			UTFSetScreenDraw(0);
			UTFDareIsMemoryDraw(0);
		}
		else		//随画随显
		{
			if(!g_GlobalBitmap.data)
			{
				g_GlobalBitmap.Width = 0;
				g_GlobalBitmap.Height = 0;
				g_GlobalBitmap.bitPerPixel = g_OSDInfo.bitPerPixel;
				g_GlobalBitmap.data = NULL;
				g_GlobalBitmap.hDC = NULL;
				g_GlobalBitmap.hBitmap = NULL;
				//g_GlobalBitmap.pOrigin = NULL;
				UTFSetScreenDraw(1);
				UTFDareIsMemoryDraw(1);
			}
		}
	}
}


void UTFAPI UTFSetGraphMode(UTFGRAPHMODE GraphMode)
{
	switch(GraphMode)
	{
	case UTF_DEPTH_16:
		g_OSDInfo.bGraphMode = GraphMode;
		g_OSDInfo.bitPerPixel = 16;

		g_GraphProc.drawPixel = UTFMemoryPixel16;
		g_GraphProc.drawLine = UTFMemoryLine16;
		g_GraphProc.drawRect = UTFMemoryDrawRect16;
		g_GraphProc.fillRect = UTFMemoryFillRect16;
		g_GraphProc.drawEllipse = UTFMemoryEllipseDraw16;
		g_GraphProc.fillEllipse = UTFMemoryEllipseFill16;
		g_GraphProc.fillPolygon = UTFMemoryPolygonFill16;
		g_GraphProc.drawRoundRect = UTFMemoryDrawRoundRect16;
		g_GraphProc.fillRoundRect = UTFMemoryFillRoundRect16;
		g_GraphProc.bitblt = UTFBitBlt16;
		break;

	default:
		g_OSDInfo.bGraphMode = UTF_DEPTH_8;
		g_OSDInfo.bitPerPixel = 8;

		g_GraphProc.drawPixel = UTFMemoryPixel8;
		g_GraphProc.drawLine = UTFMemoryLine8;
		g_GraphProc.drawRect = UTFMemoryDrawRect8;
		g_GraphProc.fillRect = UTFMemoryFillRect8;
		g_GraphProc.drawEllipse = UTFMemoryEllipseDraw8;
		g_GraphProc.fillEllipse = UTFMemoryEllipseFill8;
		g_GraphProc.fillPolygon = UTFMemoryPolygonFill8;
		g_GraphProc.drawRoundRect = UTFMemoryDrawRoundRect8;
		g_GraphProc.fillRoundRect = UTFMemoryFillRoundRect8;
		g_GraphProc.bitblt = UTFBitBlt8;
		break;
	}

	UTFPaletteSetColorMode(g_OSDInfo.bitPerPixel);
	UTFDrawAPISetColorMode(g_OSDInfo.bitPerPixel);
	
	UTFArabicSetColorMode(g_OSDInfo.bitPerPixel);
	UTFBIG5SetColorMode(g_OSDInfo.bitPerPixel);
	UTFCHNSetColorMode(g_OSDInfo.bitPerPixel);
	UTFENGSetColorMode(g_OSDInfo.bitPerPixel);
	UTFHINSetColorMode(g_OSDInfo.bitPerPixel);
	UTFKANADASetColorMode(g_OSDInfo.bitPerPixel);
	UTFMALAYASetColorMode(g_OSDInfo.bitPerPixel);
	UTFTAMILSetColorMode(g_OSDInfo.bitPerPixel);
	UTFTHAISetColorMode(g_OSDInfo.bitPerPixel);
}
extern void *mem_malloc(DWORD size);
#define MAX_MEMORY	(720*576*2+32)
#define ADDWIDTHEX 32

static BYTE g_MemoryPool[MAX_MEMORY+ADDWIDTHEX*2];
void *SOWMalloc(DWORD size)
{
	DWORD dwPtrAddr = (DWORD)g_MemoryPool;

	if(size > MAX_MEMORY)
		return 0;
		
	if(dwPtrAddr%ADDWIDTHEX)
	{
		dwPtrAddr += ADDWIDTHEX-dwPtrAddr%ADDWIDTHEX;
	}

	return (void *)dwPtrAddr;
}
/*{
	void *ptr;

	 ptr = mem_malloc(size);

	 return ptr;
}
*/
void SOWFree(void *p)
{
//	mem_free(p);
}

void *OS_Malloc(DWORD size)
{
	return (void *)DG_mem_malloc((D_UINT32)size);
}

void OS_Free(void *p)
{
	DG_mem_free(p);
}
/*
static BYTE g_bitmapPool[MAX_MEMORY+ADDWIDTHEX*2];

void *SOWMallocBitmap(DWORD size)
{
	DWORD dwPtrAddr = (DWORD)g_bitmapPool;

	if(size > MAX_MEMORY)
		return 0;
		
	if(dwPtrAddr%ADDWIDTHEX)
	{
		dwPtrAddr += ADDWIDTHEX-dwPtrAddr%ADDWIDTHEX;
	}

	return (void *)dwPtrAddr;
}
*/

DWORD UTFRGBAToBpp16_5551(BYTE r, BYTE g, BYTE b, BYTE alpha)
{
	BPP16COLOR colordata;

	colordata.color.a = (alpha == 0)?0:1;
	colordata.color.r = (r>>3);
	colordata.color.g = (g>>3);
	colordata.color.b = (b>>3);

	return colordata.wData;
}


