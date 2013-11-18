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
#include "UTFGraphPort.h"
#include "UTFGraphPortPriv.h"
#include "UTFGifDraw.h"
#include "UTFGifDrawPriv.h"

typedef struct
{
	BYTE pixel;
	WORD previous;
	WORD next;
}GIFINFO;

/**************************************************************************/
static UTFOSDCOL g_OutColor;
static BYTE g_GIFAlpha = 255;
static UTFCOLOR g_GIFPal[256];
static GIFINFO g_GIFInfoTable[4096];

static char g_cRowOffset[5]  = {0,8,8,4,2};
static char g_cFirstRow[5] = {0,0,4,2,1};
/**************************************************************************/

static void UTFGifPutPixel(BYTE pixel, LPUTFGIFFILE lpGIFInfo)
{
	if(lpGIFInfo->m_cPackedField & 0x40)
	{
		if(++lpGIFInfo->m_x >= lpGIFInfo->m_iWidth)
		{
			DWORD dwOldRow = lpGIFInfo->m_iRow;
			
			lpGIFInfo->m_x = 0;
			lpGIFInfo->m_iRow += g_cRowOffset[lpGIFInfo->m_iPass];
			
			if(lpGIFInfo->m_iRow >= lpGIFInfo->m_iHeight)
			{
				lpGIFInfo->m_iPass++;
				lpGIFInfo->m_iRow = g_cFirstRow[lpGIFInfo->m_iPass];
			}
			if(lpGIFInfo->m_iRow >= lpGIFInfo->m_iHeight1)
				return;

		#if(RUN_PC == FALSE)
			lpGIFInfo->m_pcBitmap += (lpGIFInfo->m_iRow - dwOldRow)*lpGIFInfo->bitmap.Width;
		#endif
		}
		
		if(pixel != lpGIFInfo->m_filterColor)
		{
		#if(RUN_PC == TRUE)
			g_OutColor.data = g_GIFPal[pixel];
			UTFPixelDraw(lpGIFInfo->m_x+lpGIFInfo->m_nPosX+lpGIFInfo->m_nOffX, lpGIFInfo->m_iRow+lpGIFInfo->m_nPosY+lpGIFInfo->m_nOffY, g_OutColor, 0);
		#else
			*(lpGIFInfo->m_pcBitmap+lpGIFInfo->m_x) = g_GIFPal[pixel];
		#endif
		}
	}
	else
	{
	#if(RUN_PC == TRUE)
		if(pixel != lpGIFInfo->m_filterColor)
		{
			g_OutColor.data = g_GIFPal[pixel];
			UTFPixelDraw(lpGIFInfo->m_x+lpGIFInfo->m_nPosX+lpGIFInfo->m_nOffX, lpGIFInfo->m_y+lpGIFInfo->m_nPosY+lpGIFInfo->m_nOffY, g_OutColor, 0);
		}
		
		if(++lpGIFInfo->m_x >= lpGIFInfo->m_iWidth)
		{
			lpGIFInfo->m_x = 0;
			lpGIFInfo->m_y++;
		}
	#else
		if(pixel != lpGIFInfo->m_filterColor)
		{
			*(lpGIFInfo->m_pcBitmap) = g_GIFPal[pixel];
		}
		lpGIFInfo->m_pcBitmap++;
	#endif
	}
}

static BYTE UTFGifGetByte(LPUTFGIFFILE lpGIFInfo)
{
	if(lpGIFInfo->m_uReadByte >= lpGIFInfo->m_uBlockSize)
	{
		lpGIFInfo->m_uBlockSize = *lpGIFInfo->m_pcGifTrack++;
		lpGIFInfo->m_uReadByte  = 0;
		lpGIFInfo->m_iTotalReadByte += lpGIFInfo->m_uBlockSize+1;
		if(lpGIFInfo->m_iTotalReadByte > lpGIFInfo->m_iGifSize)
		{
			lpGIFInfo->m_iTotalReadByte -= lpGIFInfo->m_uBlockSize+1;
			return 0;
		}
		if(lpGIFInfo->m_uBlockSize == 0)
		{
			lpGIFInfo->m_pcGifTrack--;
			lpGIFInfo->m_iTotalReadByte--;
			return 0;
		}
	}
	lpGIFInfo->m_uReadByte++;
	
	return *lpGIFInfo->m_pcGifTrack++;
}

static DWORD UTFGifGetCode(LPUTFGIFFILE lpGIFInfo)
{
	DWORD tmp1,tmp = 1;
	
	if(lpGIFInfo->m_uRemain >= lpGIFInfo->m_uBitSize)
	{
		tmp <<= lpGIFInfo->m_uBitSize;
		tmp1 = lpGIFInfo->m_cCurentByte & (tmp-1);
		lpGIFInfo->m_cCurentByte >>= lpGIFInfo->m_uBitSize;
		lpGIFInfo->m_uRemain -= lpGIFInfo->m_uBitSize;
	}
	else
	{
		tmp1 = lpGIFInfo->m_cCurentByte;
		lpGIFInfo->m_cCurentByte = UTFGifGetByte(lpGIFInfo);
		if(8 >= (lpGIFInfo->m_uBitSize - lpGIFInfo->m_uRemain))
		{
			tmp <<= (lpGIFInfo->m_uBitSize - lpGIFInfo->m_uRemain);
			tmp1 += (((DWORD)(lpGIFInfo->m_cCurentByte & (tmp-1))) << lpGIFInfo->m_uRemain);
			lpGIFInfo->m_cCurentByte >>= (lpGIFInfo->m_uBitSize - lpGIFInfo->m_uRemain);
			lpGIFInfo->m_uRemain = 8 - (lpGIFInfo->m_uBitSize - lpGIFInfo->m_uRemain);
		}
		else
		{
			tmp1 += (((DWORD)(lpGIFInfo->m_cCurentByte)) << lpGIFInfo->m_uRemain);
			lpGIFInfo->m_cCurentByte = UTFGifGetByte(lpGIFInfo);
			tmp <<= lpGIFInfo->m_uBitSize - lpGIFInfo->m_uRemain - 8;
			tmp1 += (((DWORD)(lpGIFInfo->m_cCurentByte & (tmp-1))) << (lpGIFInfo->m_uRemain+8));
			lpGIFInfo->m_cCurentByte >>= lpGIFInfo->m_uBitSize - lpGIFInfo->m_uRemain-8;
			lpGIFInfo->m_uRemain = 8-(lpGIFInfo->m_uBitSize - lpGIFInfo->m_uRemain-8);
		}
	}
	return tmp1;
}

static int UTFGifTakeIt(LPUTFGIFFILE lpGIFInfo)
{
	DWORD uLocalColorTableSize;
	WORD code,oldcode,code1;
	int iFinishCode,iResetCode,i;
	int iPrimaryTableSize,iTableSize;
	UTFOSDCOL color;
	int iColorIndex;
	UTFBITMAP tempbmp;

	lpGIFInfo->m_iLeft = *(lpGIFInfo->m_pcGifTrack+1)*256 + *lpGIFInfo->m_pcGifTrack;
	lpGIFInfo->m_pcGifTrack+=2;
	
	lpGIFInfo->m_iTop  = *(lpGIFInfo->m_pcGifTrack+1)*256 + *lpGIFInfo->m_pcGifTrack; 
	lpGIFInfo->m_pcGifTrack+=2;
	
	lpGIFInfo->m_iWidth = *(lpGIFInfo->m_pcGifTrack+1)*256 + *lpGIFInfo->m_pcGifTrack;
	lpGIFInfo->m_pcGifTrack+=2;

	if((lpGIFInfo->m_iWidth+lpGIFInfo->m_nPosX) > lpGIFInfo->m_iWidth1)
	{
		return 0;
	}
	
	lpGIFInfo->m_iHeight = *(lpGIFInfo->m_pcGifTrack+1)*256 + *lpGIFInfo->m_pcGifTrack;
	lpGIFInfo->m_pcGifTrack+=2;
	
	if((lpGIFInfo->m_iHeight+lpGIFInfo->m_nPosY) > lpGIFInfo->m_iHeight1)
		return 0;

#if(RUN_PC == FALSE)
	if(UTFCreateBitmap(lpGIFInfo->m_iWidth, lpGIFInfo->m_iHeight, &lpGIFInfo->bitmap, 1) == 0)
		return 0;

	UTFGetMemoryBitmap(&tempbmp);
	if(tempbmp.data)	//draw to memory
		UTFBitBlt(&tempbmp, lpGIFInfo->m_nPosX+lpGIFInfo->m_iLeft, lpGIFInfo->m_nPosY+lpGIFInfo->m_iTop, lpGIFInfo->bitmap.Width, lpGIFInfo->bitmap.Height, &lpGIFInfo->bitmap, 0, 0);
	else				//draw to screen		zhoulei edit 08-10-14根据边距来保存背景
		UTFSaveScreen(lpGIFInfo->m_nPosX+lpGIFInfo->m_iLeft, lpGIFInfo->m_nPosY+lpGIFInfo->m_iTop, &lpGIFInfo->bitmap, 0);
#endif

	lpGIFInfo->m_cPackedField = *lpGIFInfo->m_pcGifTrack++;
	lpGIFInfo->m_iTotalReadByte += 9;

	memset(g_GIFInfoTable, 0, sizeof(g_GIFInfoTable));
	if(lpGIFInfo->m_cPackedField & 0x80)
	{
		uLocalColorTableSize = 1;
		uLocalColorTableSize <<= (lpGIFInfo->m_cPackedField & 7)+1;
		if(uLocalColorTableSize > 256)
			return 0;

		lpGIFInfo->m_filterColor = -1;
		if(lpGIFInfo->m_bTransparentIndex)
		{
			lpGIFInfo->m_filterColor = lpGIFInfo->m_iTransparentIndex;
		}

		lpGIFInfo->m_iTotalReadByte += uLocalColorTableSize*3;
		color.rgb.alpha = g_GIFAlpha;

		iColorIndex = UTFGetColorIndex();

		lpGIFInfo->m_iTotalColor = uLocalColorTableSize;
		for(i=0;i<uLocalColorTableSize;i++)
		{
			color.rgb.red = *lpGIFInfo->m_pcGifTrack++;
			color.rgb.green = *lpGIFInfo->m_pcGifTrack++;
			color.rgb.blue = *lpGIFInfo->m_pcGifTrack++;

			g_GIFPal[i] = UTFFillPaletteFunc(color.data, i+iColorIndex);
		}
	}
	else 
	{
		BYTE *pcGlobalColor = lpGIFInfo->m_pcGlobalColorTable;

		if(lpGIFInfo->m_iGlobalColorSize > 256)
			return 0;

		lpGIFInfo->m_filterColor = -1;
		if(lpGIFInfo->m_bTransparentIndex)
		{
			lpGIFInfo->m_filterColor = lpGIFInfo->m_iTransparentIndex;
		}
		
		color.rgb.alpha = g_GIFAlpha;
		iColorIndex = UTFGetColorIndex();

		lpGIFInfo->m_iTotalColor = lpGIFInfo->m_iGlobalColorSize;
		for(i=0; i<lpGIFInfo->m_iGlobalColorSize; i++)
		{
			color.rgb.red = *pcGlobalColor++;
			color.rgb.green = *pcGlobalColor++;
			color.rgb.blue = *pcGlobalColor++;

			g_GIFPal[i] = UTFFillPaletteFunc(color.data, i+iColorIndex);
		}
	}

#if(RUN_PC == TRUE)
	lpGIFInfo->m_pcBitmap  =(WORD*) lpGIFInfo->bitmap.data;
#endif

	lpGIFInfo->m_uPrimaryBitSize = lpGIFInfo->m_uBitSize = *lpGIFInfo->m_pcGifTrack++;
	lpGIFInfo->m_iTotalReadByte++;
	iPrimaryTableSize = iTableSize = (1<<lpGIFInfo->m_uBitSize)+2;
	iFinishCode = iTableSize -1;
	iResetCode  = iFinishCode-1;

	lpGIFInfo->m_uPrimaryBitSize++;
	lpGIFInfo->m_uBitSize++;
	lpGIFInfo->m_uRemain=0;
	lpGIFInfo->m_cCurentByte=0;
	lpGIFInfo->m_uBlockSize=0;
	lpGIFInfo->m_uReadByte=1;
	lpGIFInfo->m_x = 0;
	lpGIFInfo->m_y = 0;
	lpGIFInfo->m_iPass = 1;
	lpGIFInfo->m_iRow = 0;
	
	while((code=UTFGifGetCode(lpGIFInfo))!=iFinishCode)
	{
		if(code==iResetCode)
		{
			lpGIFInfo->m_uBitSize = lpGIFInfo->m_uPrimaryBitSize;
			iTableSize = iPrimaryTableSize;
			oldcode = UTFGifGetCode(lpGIFInfo);
			if(oldcode > iTableSize)
				return 0;
			UTFGifPutPixel((BYTE)oldcode, lpGIFInfo);
			continue;
		}
		if(code < iTableSize) //<code> exist in the string g_GIFInfoTable
		{
			WORD code2 = 0;
			
			code1 = code;
			while(code1 >= iPrimaryTableSize)
			{
				g_GIFInfoTable[code1].next = code2;
				code2 = code1;
				code1 = g_GIFInfoTable[code1].previous;
				if(code1 >= code2)
					return 0;
			}
			UTFGifPutPixel((BYTE)code1, lpGIFInfo);
			while(code2 != 0)
			{
				UTFGifPutPixel(g_GIFInfoTable[code2].pixel, lpGIFInfo);
				code2 = g_GIFInfoTable[code2].next;
			}
			g_GIFInfoTable[iTableSize].pixel = (BYTE)code1;
			g_GIFInfoTable[iTableSize].previous = oldcode;
			iTableSize++;
			if(iTableSize == (0x0001 << lpGIFInfo->m_uBitSize))
				lpGIFInfo->m_uBitSize++;
			if(lpGIFInfo->m_uBitSize > 12)
				lpGIFInfo->m_uBitSize = 12;
			oldcode = code;
		} 
		else    //<code> doesn't exist in the string g_GIFInfoTable
		{
			WORD code2 = 0;
		
			code1 = oldcode;
			while(code1 >= iPrimaryTableSize)
			{
				g_GIFInfoTable[code1].next = code2;
				code2 = code1;
				code1 = g_GIFInfoTable[code1].previous;
				if(code1 >= code2)
					return 0;
			}
			UTFGifPutPixel((BYTE)code1, lpGIFInfo);
			while(code2 != 0)
			{
				UTFGifPutPixel(g_GIFInfoTable[code2].pixel, lpGIFInfo);
				code2 = g_GIFInfoTable[code2].next;
			}
			UTFGifPutPixel((BYTE)code1, lpGIFInfo);
			g_GIFInfoTable[iTableSize].pixel = (BYTE)code1;
			g_GIFInfoTable[iTableSize].previous = oldcode;
			iTableSize++;
			if(iTableSize == (0x0001<<lpGIFInfo->m_uBitSize))
				lpGIFInfo->m_uBitSize++;
			if(lpGIFInfo->m_uBitSize > 12)
				lpGIFInfo->m_uBitSize = 12;
			oldcode = code;
		}
	}

	lpGIFInfo->m_pcGifTrack++;
	lpGIFInfo->m_iTotalReadByte++;

	return TRUE;
}

static int UTFLoadGifFile(BYTE *pData, DWORD dataLen, LPUTFGIFFILE lpGIFInfo)
{
	char name[7];

	lpGIFInfo->m_pcGlobalColorTable = NULL;
	lpGIFInfo->m_pcGif = NULL;
	lpGIFInfo->m_iTotalReadByte = 0;
	lpGIFInfo->m_iGifSize = 0;
	lpGIFInfo->m_iGlobalColorSize = 0;

	memcpy(name, pData, 6);
	name[6] = 0;
	pData += 6;
	if(strcmp(name, "GIF87a") != 0 && strcmp(name, "GIF89a") != 0)
		return FALSE;
	
	lpGIFInfo->m_iGifWidth = *(pData+1)*256 + *pData;
	lpGIFInfo->m_iGifHeight = *(pData+3)*256 + *(pData+2);
	pData += 4;
	
	lpGIFInfo->m_iBackgroundColor = *(pData+1);
	if(*pData & 0x80)
	{
		lpGIFInfo->m_iGlobalColorSize = 0x01 << ((*pData & 0x07)+1);
		lpGIFInfo->m_pcGlobalColorTable = pData+3;

		if(dataLen <= ((DWORD)3*lpGIFInfo->m_iGlobalColorSize+13))
		{
			lpGIFInfo->m_pcGlobalColorTable = 0;
			lpGIFInfo->m_iGlobalColorSize   = 0;

			return FALSE;
		}
	}

	lpGIFInfo->m_iGifSize = dataLen - 3 * lpGIFInfo->m_iGlobalColorSize-12;
	lpGIFInfo->m_pcGifTrack = lpGIFInfo->m_pcGif = pData+3 * lpGIFInfo->m_iGlobalColorSize+3;

	return TRUE;
}

static int UTFGifNextFrame(LPUTFGIFFILE lpGIFInfo)
{
	BYTE cSize;
	
	if(lpGIFInfo->m_pcGif == NULL)
		return 0;
	
l1:	if(lpGIFInfo->m_iTotalReadByte > lpGIFInfo->m_iGifSize)
	{
		lpGIFInfo->m_pcGifTrack = lpGIFInfo->m_pcGif;
		lpGIFInfo->m_iTotalReadByte = 0;
		return 0;
	}
	lpGIFInfo->m_iTotalReadByte++;
	
	switch(*lpGIFInfo->m_pcGifTrack++)
	{
	case 0x2C:
		{
			int iRet = UTFGifTakeIt(lpGIFInfo);

		#if(RUN_PC == FALSE)
			if(lpGIFInfo->bitmap.data)
			{
				UTFBITMAP tempbmp;
			#if 0//vivian add only to test 20080605
				UTFCreateBitmap(lpGIFInfo->bitmap.Width, lpGIFInfo->bitmap.Height, &tempbmp, 1);
				UTFSetMemoryBitmap(&tempbmp);
				UTFGetMemoryBitmap(&tempbmp);
				if(tempbmp.data)	//draw to memory
					UTFBitBlt(&lpGIFInfo->bitmap, 0, 0, lpGIFInfo->bitmap.Width, lpGIFInfo->bitmap.Height, &tempbmp, 0, 0);
				UTFPutScreen(lpGIFInfo->m_nPosX, lpGIFInfo->m_nPosY, &tempbmp, 0);
				UTFDeleteBitmap(&tempbmp);
				UTFSetMemoryBitmap(NULL);
			#else
				UTFGetMemoryBitmap(&tempbmp);
				if(tempbmp.data)	//draw to memory
					UTFBitBlt(&lpGIFInfo->bitmap, 0, 0, lpGIFInfo->bitmap.Width, lpGIFInfo->bitmap.Height, &tempbmp, lpGIFInfo->m_nPosX+lpGIFInfo->m_iLeft, lpGIFInfo->m_nPosY+lpGIFInfo->m_iTop);
				else				//draw to screen		//zhoulei edited 08-10-14  居中显示
					UTFPutScreen(lpGIFInfo->m_nPosX+lpGIFInfo->m_iLeft, lpGIFInfo->m_nPosY+lpGIFInfo->m_iTop, &lpGIFInfo->bitmap, 0);
				UTFDeleteBitmap(&lpGIFInfo->bitmap);
			#endif
			}
		#endif
			return iRet;
		}
		break;
		
	case 0x21:
			lpGIFInfo->m_iTotalReadByte++;
			switch(*lpGIFInfo->m_pcGifTrack++)
			{ 
			case 0xF9:
				lpGIFInfo->m_pcGifTrack++;	//block size
				lpGIFInfo->m_iDisposalMethod = (*lpGIFInfo->m_pcGifTrack) & 28;
				lpGIFInfo->m_bTransparentIndex = (*lpGIFInfo->m_pcGifTrack++) & 1;
				lpGIFInfo->m_iDelayTime = *(lpGIFInfo->m_pcGifTrack+1)*256 + *lpGIFInfo->m_pcGifTrack;
				lpGIFInfo->m_pcGifTrack += 2;
				lpGIFInfo->m_iTransparentIndex = *lpGIFInfo->m_pcGifTrack++;
				lpGIFInfo->m_iTotalReadByte += 5;
				break;
				
			case 0xFE:
				while((cSize = *lpGIFInfo->m_pcGifTrack)!=0)
				{
					lpGIFInfo->m_pcGifTrack += cSize+1;
					lpGIFInfo->m_iTotalReadByte += cSize+1;
					if(lpGIFInfo->m_iTotalReadByte > lpGIFInfo->m_iGifSize)
						return 0;
				}
				break;
				
			case 0x01:
				lpGIFInfo->m_pcGifTrack += 13;
				lpGIFInfo->m_iTotalReadByte += 13;
				while((cSize = *lpGIFInfo->m_pcGifTrack)!=0)
				{
					lpGIFInfo->m_pcGifTrack += cSize+1;
					lpGIFInfo->m_iTotalReadByte += cSize+1;
					if(lpGIFInfo->m_iTotalReadByte > lpGIFInfo->m_iGifSize)
						return 0;
				}
				break;
				
			case 0xFF:
				lpGIFInfo->m_pcGifTrack += 12;
				lpGIFInfo->m_iTotalReadByte += 12;
				while((cSize = *lpGIFInfo->m_pcGifTrack)!=0)
				{
					lpGIFInfo->m_pcGifTrack += cSize+1;
					lpGIFInfo->m_iTotalReadByte += cSize+1;
					if(lpGIFInfo->m_iTotalReadByte > lpGIFInfo->m_iGifSize)
						return 0;
				}
				break;
				
			default: 
				return FALSE;
			}
			
			lpGIFInfo->m_pcGifTrack++;
			lpGIFInfo->m_iTotalReadByte++;
			if(lpGIFInfo->m_iTotalReadByte > lpGIFInfo->m_iGifSize)
				return 0;
			goto l1;
			break;
			
	case 0x3B:
		lpGIFInfo->m_pcGifTrack = lpGIFInfo->m_pcGif;
		lpGIFInfo->m_iTotalReadByte = 0;
		goto l1;
		
	case 0:  
		goto l1;
		
	default:
		return FALSE;
	}
}

static void UTFGifInit(LPUTFGIFFILE lpGIFInfo, WORD x, WORD y)
{
	UTFRECT rcOSDRgn;

	UTFGetFullScreenRect(&rcOSDRgn);

	lpGIFInfo->m_nPosX 				= x;
	lpGIFInfo->m_nPosY 				= y;
	lpGIFInfo->m_iTotalReadByte		= 0;
	lpGIFInfo->m_filterColor		= -1;
	lpGIFInfo->m_iTotalColor		= 0;
	lpGIFInfo->m_pcGlobalColorTable	= 0;
	lpGIFInfo->m_pcGif				= 0;
	lpGIFInfo->m_iGifSize			= 0;
	lpGIFInfo->m_iGlobalColorSize	= 0;
	lpGIFInfo->m_bTransparentIndex 	= FALSE;
	lpGIFInfo->m_iDelayTime 		= 0;
	lpGIFInfo->m_bDataOk			= TRUE;
	lpGIFInfo->m_bEnablePlay		= 1;
	lpGIFInfo->m_pcBitmap 			= NULL;
	lpGIFInfo->m_iWidth1			= rcOSDRgn.right-rcOSDRgn.left;
	lpGIFInfo->m_iHeight1			= rcOSDRgn.bottom-rcOSDRgn.top;
	lpGIFInfo->bitmap.data			= NULL;

	if(UTFGetScreenDraw())
	{
		UTFGetOSDRgnRect(&rcOSDRgn);

		lpGIFInfo->m_nOffX = rcOSDRgn.left;
		lpGIFInfo->m_nOffY = rcOSDRgn.top;
	}
	else
	{
		lpGIFInfo->m_nOffX = 0;
		lpGIFInfo->m_nOffY = 0;
	}
}

void UTFGIFSetAlpha(BYTE bAlpha)
{
	g_GIFAlpha = bAlpha;
}

char UTFAPI UTFGIFOpen(LPUTFGIFFILE lpGIFInfo, char *pGIFData, DWORD dwSize, WORD x, WORD y)
{
	int bLoadOK;

	if(lpGIFInfo == NULL)
		return FALSE;
	
	UTFGifInit(lpGIFInfo, x, y);
	
	bLoadOK = UTFLoadGifFile((BYTE *)pGIFData, dwSize, lpGIFInfo);
	if(bLoadOK == FALSE)
	{
		lpGIFInfo->m_bDataOk = FALSE;
	}

	return bLoadOK;
}

void UTFAPI UTFGIFPlay(LPUTFGIFFILE lpGIFInfo)
{
	if(lpGIFInfo)
	{
		if(lpGIFInfo->m_bDataOk && lpGIFInfo->m_bEnablePlay)
		{
			UTFGifNextFrame(lpGIFInfo);
		}
	}
}

void UTFAPI UTFGIFStop(LPUTFGIFFILE lpGIFInfo)
{
	if(lpGIFInfo)
	{
		if(lpGIFInfo->m_bDataOk && lpGIFInfo->m_bEnablePlay)
		{
			lpGIFInfo->m_pcGifTrack = lpGIFInfo->m_pcGif;
			lpGIFInfo->m_iTotalReadByte = 0;
			lpGIFInfo->m_bEnablePlay = 0;
			
			UTFGifNextFrame(lpGIFInfo);
		}
	}
}

void UTFAPI UTFGIFStart(LPUTFGIFFILE lpGIFInfo)
{
	if(lpGIFInfo)
	{
		if(lpGIFInfo->m_bDataOk && !lpGIFInfo->m_bEnablePlay)
		{			
			lpGIFInfo->m_pcGifTrack = lpGIFInfo->m_pcGif;
			lpGIFInfo->m_iTotalReadByte = 0;
			lpGIFInfo->m_bEnablePlay = 1;
		}
	}
}

void UTFAPI UTFGIFGetImageInfo(LPUTFGIFFILE lpGIFInfo, LPUTFIMAGEINFO lpInfo)
{
	if((lpGIFInfo == NULL) || (lpInfo == NULL))
		return;

	lpInfo->dwNumberOfColor = 0;
	lpInfo->dwWidth = 0;
	lpInfo->dwHeight = 0;

	if(lpGIFInfo->m_bDataOk)
	{
		lpInfo->dwWidth = lpGIFInfo->m_iGifWidth;
		lpInfo->dwHeight = lpGIFInfo->m_iGifHeight;
		lpInfo->dwNumberOfColor = lpGIFInfo->m_iTotalColor;
	}
}

