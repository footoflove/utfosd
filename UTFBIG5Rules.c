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
#include "UTFBIG5Rules.h"
#include "UTFMemoryPriv.h"

static TEXTPROC BIG5DrawTextFunc;

/*****************************************************************************
This function decide the code is avalible or no
*****************************************************************************/
static BYTE BIG5IsAvalibleCode(BYTE bHighByte, BYTE bLowByte, DWORD *ptrAddr, LPUTFLOGFONT lpFont)
{
	BYTE bAvalible = FALSE;

	if((bHighByte > BIG5_BYTE) && (bHighByte < 0xC7))
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
		else if(((bLowByte >= 0x40) && (bLowByte < 0x7F)) || ((bLowByte > 0xA0) && (bLowByte < 0xFF)))
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

				if(bLowByte < 0x7F)
				{
					position = (bHighByte-0xA1)*157 + (bLowByte-0x40);
				}
				else
				{
					position = (bHighByte-0xA1)*157 + (bLowByte-0x62);
				}
				
				*ptrAddr = lpFont->lfAddr1+dwSize*position;
			}
		}
	}

	return bAvalible;
}

/*************************************************************************/
WORD BIG5TextWidth(LPTEXT pText, int charInterval, LPUTFLOGFONT lpFont)
{
	WORD count = 0;
	WORD textWidth = 0;
	BYTE char1,char2;

	while(*pText)
	{
		char1 = *pText;
		if(char1 == '\n')
			break;
		
		if(char1 > BIG5_BYTE)
		{
			char2 = *(pText+1);
			if(char2)
			{
				pText += 2;
				count++;
				textWidth += (WORD)lpFont->lfWidth;
			}
			else
			{
				break;
			}
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
			}while((char1 > 0) && (char1 <= BIG5_BYTE));
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

static void BIG5DrawTextFunc8(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
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
		
		if(BIG5IsAvalibleCode(char1, char2, &dwAddr, lpFont) == TRUE)
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

static void BIG5DrawTextFunc16(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
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
		
		if(BIG5IsAvalibleCode(char1, char2, &dwAddr, lpFont) == TRUE)
		{
			ptrRead = (BYTE *)dwAddr;
			ptrWrite = (WORD *)bitmap.data;
			ptrWrite += dy*bitmap.Width+dx+xstep;
			
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

void BIG5TextFunc(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
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
		if(char1 > BIG5_BYTE)
		{
			char2 = *(pText+1);
			if(char2)
			{
				len = 0;
				lpBuffer = UTFGetTextTempPtr(&bufferSize);
				do
				{
					*(lpBuffer+len++) = *pText++;
					*(lpBuffer+len++) = *pText++;
					char1 = *pText;
					char2 = *(pText+1);
				}while((char1 > BIG5_BYTE) && (char2 > 0));
				*(lpBuffer+len) = 0;

				BIG5DrawTextFunc(lpBuffer, x, top, totalWidth, height, charInterval, textColor, lpFont);
				
				WidthEx = charInterval+BIG5TextWidth(lpBuffer, charInterval, lpFont);
				totalWidth -= WidthEx;
				x += WidthEx;
			}
			else
			{
				break;
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
			}while((char1 > 0) && (char1 <= BIG5_BYTE));
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
void UTFBIG5SetColorMode(BYTE bitPerPixel)
{
	if(bitPerPixel == 8)
	{
		BIG5DrawTextFunc = BIG5DrawTextFunc8;
	}
	if(bitPerPixel == 16)
	{
		BIG5DrawTextFunc = BIG5DrawTextFunc16;
	}
}
