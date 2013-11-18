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
#include "UTFPalette.h"
#include "UTFGraphPort.h"
#include "UTFGraphPortPriv.h"
#include "UTFDrawAPI.h"
#include "UTFDrawTextPriv.h"
#include "UTFEnglishRules.h"
#include "UTFCHNRules.h"
#include "UTFMemoryPriv.h"

static TEXTPROC CHNDrawTextFunc;

/*****************************************************************************
This function decide the code is avalible or no
*****************************************************************************/
static BYTE CHNIsAvalibleCode(BYTE bHighByte, BYTE bLowByte, DWORD *ptrAddr, LPUTFLOGFONT lpFont)
{
	BYTE bAvalible = FALSE;

	if(bHighByte > CHN_BYTE)
	{
		if((bLowByte > CHN_BYTE) && (bLowByte < 0xFF))
		{
			if(lpFont->lfAddr2)
			{
				BYTE *ptrLibData = (BYTE *)lpFont->lfAddr2;
				WORD i,wTotal;

				UTFMemCpyWORD(&wTotal, ptrLibData, 2);
				ptrLibData += 2;
				
				if(wTotal)
				{
					for(i=0; i<wTotal; i++)
					{
						if((bHighByte == *ptrLibData) && (bLowByte == *(ptrLibData+1)))
						{
							bAvalible = TRUE;
							if(ptrAddr)
							{
								DWORD dwByteNum;
								DWORD dwSize;

								dwByteNum = lpFont->lfWidth/8;
								if(lpFont->lfWidth%8)
								{
									dwByteNum++;
								}
								dwSize = dwByteNum*lpFont->lfHeight;
								*ptrAddr = lpFont->lfAddr2+2*(wTotal+1)+dwSize*i;
							}
							break;
						}
						ptrLibData += 2;
					}
				}
			}
			else if((bHighByte < 0xAA) || ((bHighByte >= 0xB0) && (bHighByte < 0xF8)))
			{
				bAvalible = TRUE;
				if(ptrAddr)
				{
					WORD position;
					DWORD dwByteNum;
					DWORD dwSize;

					dwByteNum = lpFont->lfWidth/8;
					if(lpFont->lfWidth%8)
					{
						dwByteNum++;
					}
					dwSize = dwByteNum*lpFont->lfHeight;

					if(bHighByte < 0xAA)
					{
						position = (bHighByte-0xA1)*94 + (bLowByte-0xA1);
					}
					else
					{
						position = (bHighByte-0xA7)*94 + (bLowByte-0xA1);
					}
					
					*ptrAddr = lpFont->lfAddr1+dwSize*position;
				}
			}
		}
	}

	return bAvalible;
}

/*************************************************************************/
WORD CHNTextWidth(LPTEXT pText, int charInterval, LPUTFLOGFONT lpFont)
{
	WORD count = 0;
	WORD textWidth = 0;
	BYTE char1,char2;

	while(*pText)
	{
		char1 = *pText;
		if(char1 == '\n')
			break;
		
		if(char1 > CHN_BYTE)
		{
			char2 = *(pText+1);
			if(char2 > CHN_BYTE)
			{
				pText++;
				count++;
				textWidth += (WORD)lpFont->lfWidth;
			}
			else if(char2 == 0)
			{
				break;
			}
			
			pText++;
		}
		else
		{
			WORD bufferSize;
			WORD len = 0;
			BYTE *lpBuffer;
			
			lpBuffer = UTFGetTextTempPtr(&bufferSize);
			do
			{
				*(lpBuffer+len++) = *pText++;
				char1 = *pText;
				count++;
			}while((char1 > 0) && (char1 <= CHN_BYTE));
			*(lpBuffer+len) = 0;

			textWidth += ENGTextWidth(lpBuffer, 0, (LPUTFLOGFONT)lpFont->lfLogFontEx);
		}
	}

	if(count > 1)
	{
		textWidth += (count-1)*charInterval;
	}

	return textWidth;
}

static void CHNDrawTextFunc8(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
{
	WORD i,j,k;
	WORD count;
	WORD xstep = 0;
	WORD dx=0,dy=0;
	DWORD dwAddr;
	DWORD dwSize;
	BYTE bDrawMode=1;
	BYTE char1,char2;
	BYTE data,bByteNum;
	BYTE *ptrRead;
	BYTE *ptrWrite;
	BYTE *ptrHeader;
	UTFBITMAP bitmap;

	if(height > lpFont->lfHeight)
	{
		height = lpFont->lfHeight;
	}

	UTFGetMemoryBitmap(&bitmap);
	if(bitmap.data)
	{
		bDrawMode = 0;
		dx = left;
		dy = top;
	}
	else
	{
		if(width%32)
		{
			WORD wOldWidth = width;
			UTFRECT rcFullScreen;

			UTFGetFullScreenRect(&rcFullScreen);
			
			width += 32-width%32;
			if((left+width) > (rcFullScreen.right-rcFullScreen.left))
			{
				width = wOldWidth;
			}
		}

		dwSize = width*height;
		ptrHeader = (BYTE *)SOWMalloc(dwSize);
		if(ptrHeader == NULL)
			return;

		bitmap.data = ptrHeader;
		bitmap.Width = width;
		bitmap.Height = height;
		bitmap.bitPerPixel = 8;
		UTFSaveScreen(left, top, &bitmap, 0);
	}

	bByteNum = lpFont->lfWidth/8;
	if(lpFont->lfWidth%8)
	{
		bByteNum++;
	}
	
	while(*pText)
	{
		char1 = *pText;
		char2 = *(pText+1);
		
		if(CHNIsAvalibleCode(char1, char2, &dwAddr, lpFont) == TRUE)
		{
			ptrRead = (BYTE *)dwAddr;
			ptrWrite = bitmap.data+dy*bitmap.Width+dx+xstep;
			
			for(i=0; i<height; i++)
			{
				count = 0;
				for(k=0; k<bByteNum; k++)
				{
					data = *ptrRead++;					
	   				for(j=0; j<8; j++)
	  				{
	  					if(count >= lpFont->lfWidth)
						{
							break;
	  					}
						
						if(data & 0x80)
						{
							*(ptrWrite+count) = (BYTE)textColor;
						}
						count++;
						data <<= 1;
					}
				}
				ptrWrite += bitmap.Width;
			}
		}

		pText += 2;
		xstep += lpFont->lfWidth+charInterval;

		if(char2 == 0)
		{
			break;
		}
	}

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}

static void CHNDrawTextFunc16(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
{
	WORD i,j,k;
	WORD count;
	WORD xstep = 0;
	WORD dx=0,dy=0;
	DWORD dwAddr;
	DWORD dwSize;
	BYTE bDrawMode=1;
	BYTE char1,char2;
	BYTE data,bByteNum;
	BYTE *ptrRead;
	WORD *ptrWrite;
	BYTE *ptrHeader;
	UTFBITMAP bitmap;

	if(height > lpFont->lfHeight)
	{
		height = lpFont->lfHeight;
	}

	UTFGetMemoryBitmap(&bitmap);
	if(bitmap.data)
	{
		bDrawMode = 0;
		dx = left;
		dy = top;
	}
	else
	{
	/*	if(width%32)
		{
			WORD wOldWidth = width;
			UTFRECT rcFullScreen;

			UTFGetFullScreenRect(&rcFullScreen);
			
			width += 32-width%32;
			if((left+width) > (rcFullScreen.right-rcFullScreen.left))
			{
				width = wOldWidth;
			}
		}
*/
		dwSize = width*height;
		ptrHeader = (BYTE *)SOWMalloc(dwSize<<1);
		if(ptrHeader == NULL)
			return;

		bitmap.data = ptrHeader;
		bitmap.Width = width;
		bitmap.Height = height;
		bitmap.bitPerPixel = 16;
		UTFSaveScreen(left, top, &bitmap, 0);
	}

	bByteNum = lpFont->lfWidth/8;
	if(lpFont->lfWidth%8)
	{
		bByteNum++;
	}
	
	while(*pText)
	{
		char1 = *pText;
		char2 = *(pText+1);
		
		if(CHNIsAvalibleCode(char1, char2, &dwAddr, lpFont) == TRUE)//返回该字点阵地址
		{
			ptrRead = (BYTE *)dwAddr;
			ptrWrite = (WORD *)bitmap.data;
			ptrWrite += dy*bitmap.Width+dx+xstep;
			
			for(i=0; i<height; i++)		//对该中文字符逐行扫描每个像素
			{
				count = 0;
				for(k=0; k<bByteNum; k++)
				{
					data = *ptrRead++;					
	   				for(j=0; j<8; j++)	//对于每个像素(2个字节)
	  				{
	  					if(count >= lpFont->lfWidth)
						{
							break;
	  					}
						
						if(data & 0x80)//判断该像素是否需要画出(什么颜色)
						{
							*(ptrWrite+count) = (WORD)textColor;
						}
						count++;
						data <<= 1;
					}
				}
				ptrWrite += bitmap.Width;
			}
		}

		pText += 2;
		xstep += lpFont->lfWidth+charInterval;

		if(char2 == 0)
		{
			break;
		}
	}

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}

void CHNTextFunc(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
{
	WORD x = left;
	WORD y = top;
	WORD bufferSize;
	WORD len = 0;
	WORD totalWidth = width;
	WORD WidthEx;
	BYTE char1,char2;
	BYTE *lpBuffer;

	while(*pText)
	{
		char1 = *pText;
		if(char1 > CHN_BYTE)
		{
			char2 = *(pText+1);
			if(char2 > CHN_BYTE)
			{
				len = 0;
				lpBuffer = UTFGetTextTempPtr(&bufferSize);
				do
				{
					*(lpBuffer+len++) = *pText++;
					*(lpBuffer+len++) = *pText++;
					char1 = *pText;
					char2 = *(pText+1);
				}while((char1 > CHN_BYTE) && (char2 > CHN_BYTE));
				*(lpBuffer+len) = 0;

				CHNDrawTextFunc(lpBuffer, x, top, totalWidth, height, charInterval, textColor, lpFont);
				
				WidthEx = charInterval+CHNTextWidth(lpBuffer, charInterval, lpFont);
				totalWidth -= WidthEx;
				x += WidthEx;
			}
			else if(char2 == 0)
			{
				break;
			}
			else
			{
				pText++;
			}
		}
		else
		{
			len = 0;
			lpBuffer = UTFGetTextTempPtr(&bufferSize);
			do
			{
				*(lpBuffer+len++) = *pText++;
				char1 = *pText;
			}while((char1 > 0) && (char1 <= CHN_BYTE));
			*(lpBuffer+len) = 0;

			if(height > lpFont->lfFontExTop)
			{
				ENGTextFunc(lpBuffer, x, top+lpFont->lfFontExTop, totalWidth, height-lpFont->lfFontExTop, charInterval, textColor, (LPUTFLOGFONT)lpFont->lfLogFontEx);
			}

			WidthEx = charInterval+ENGTextWidth(lpBuffer, charInterval, (LPUTFLOGFONT)lpFont->lfLogFontEx);
			totalWidth -= WidthEx;
			x += WidthEx;
		}
	}
}
/*************************************************************************/
void UTFCHNSetColorMode(BYTE bitPerPixel)
{
	if(bitPerPixel == 8)
	{
		CHNDrawTextFunc = CHNDrawTextFunc8;
	}
	if(bitPerPixel == 16)
	{
		CHNDrawTextFunc = CHNDrawTextFunc16;
	}
}
