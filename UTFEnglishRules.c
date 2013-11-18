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
#include "UTFEnglishRules.h"
#include "UTFDrawAPI.h"
#include "UTFMemoryPriv.h"

TEXTPROC ENGTextFunc;
/*************************************************************************/
WORD ENGTextWidth(LPTEXT pText, int charInterval, LPUTFLOGFONT lpFont)
{
	WORD index,count = 0;
	WORD textWidth = 0;
	BYTE charactor;
	BYTE *pVect;

	if(lpFont->lfAddr1 == NULL)
		return 0;

	pVect = (BYTE *)(lpFont->lfAddr1+2);
	while(*pText)
	{
		charactor = *pText;
		if(charactor == '\n')
			break;

		if((charactor > 31) || (charactor < 5))
		{
			count++;
			
			if(charactor < 5)
			{
				index = charactor - 1;
			}
			else
		    {
				index = charactor - 28;
			}

			textWidth += pVect[index<<1];
		}
		
		pText++;
	}

	if(count > 1)
	{
		textWidth += (count-1)*charInterval;
	}

	return textWidth;
}

static void ENGTextFunc8(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
{
	WORD i,j,k,count;
	WORD startx = 0;
	WORD position;
	WORD wVectSize;
	WORD dx=0,dy=0;
	DWORD dwSize;
	BYTE data,bDrawMode=1;
	BYTE *pVect,*pData;
	BYTE *ptrWrite;
	BYTE *ptrHeader;
	UTFBITMAP bitmap;

	if(lpFont->lfAddr1 == NULL)
		return;

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

	UTFMemCpyWORD(&wVectSize, (void *)lpFont->lfAddr1, 2);
	
	pVect = (BYTE *)(lpFont->lfAddr1+2);
	pData = pVect+wVectSize;
	while(*pText != NULL)
	{
		if((*pText > 31) || (*pText < 5))
		{
			DWORD address = 0;

			if(*pText < 5)
			{
				position = *pText - 1;
			}
			else
		    {
				position = *pText - 28;
			}

		    for(i=0; i<position; i++)
		    {
				address += pVect[(i<<1)+1];
		    }			
		    address *= lpFont->lfHeight;

			ptrWrite = bitmap.data+dy*bitmap.Width+dx+startx;
			for(i=0; i<height; i++)
			{
				BYTE ByteNum = pVect[(position<<1)+1];

				count = 0;
				for(k=0; k<ByteNum; k++)
				{
					data = pData[address];
					
	   				for(j=0; j<8; j++)
	  				{
	  					if(count >= pVect[position<<1])
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
					address++;
				}
				
				ptrWrite += bitmap.Width;
			}
			
			startx += pVect[position<<1] + charInterval;
		}
		
		pText++;
	}

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}

static void ENGTextFunc16(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
{
	WORD i,j,k,count;
	WORD startx = 0;
	WORD position;
	WORD wVectSize;
	WORD dx=0,dy=0;
	DWORD dwSize;
	BYTE data,bDrawMode=1;
	BYTE *pVect,*pData;
	WORD *ptrWrite;
	BYTE *ptrHeader;
	UTFBITMAP bitmap;

	if(lpFont->lfAddr1 == NULL)
		return;

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

	UTFMemCpyWORD(&wVectSize, (void *)lpFont->lfAddr1, 2);
	
	pVect = (BYTE *)(lpFont->lfAddr1+2);
	pData = pVect+wVectSize;
	while(*pText != NULL)
	{
		if((*pText > 31) || (*pText < 5))
		{
			DWORD address = 0;

			if(*pText < 5)
			{
				position = *pText - 1;
			}
			else
		    {
				position = *pText - 28;
			}

		    for(i=0; i<position; i++)
		    {
				address += pVect[(i<<1)+1];
		    }			
		    address *= lpFont->lfHeight;

			ptrWrite = (WORD *)bitmap.data;
			ptrWrite += dy*bitmap.Width+dx+startx;
			for(i=0; i<height; i++)
			{
				BYTE ByteNum = pVect[(position<<1)+1];

				count = 0;
				for(k=0; k<ByteNum; k++)
				{
					data = pData[address];
					
	   				for(j=0; j<8; j++)
	  				{
	  					if(count >= pVect[position<<1])
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
					address++;
				}
				
				ptrWrite += bitmap.Width;
			}
			
			startx += pVect[position<<1] + charInterval;
		}
		
		pText++;
	}

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}
/*************************************************************************/
void UTFENGSetColorMode(BYTE bitPerPixel)
{
	if(bitPerPixel == 8)
	{
		ENGTextFunc = ENGTextFunc8;
	}
	if(bitPerPixel == 16)
	{
		ENGTextFunc = ENGTextFunc16;
	}
}

