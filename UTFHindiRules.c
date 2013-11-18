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
#include "UTFHindiRules.h"
#include "UTFMemoryPriv.h"

typedef WORD ( *HINTEXTFUNC ) (LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont);

typedef enum
{
	HINDI_NORMAL,	//this char align lonely
	HINDI_GLUE,		//this char will place with right char and no space
	HINDI_LEFT,		//Left char glue width less than real width
	HINDI_RIGHT,	//Right char glue width less than real width
	HINDI_FIRST_LINE,//this char take width with first line
	HINDI_FIRST_LINE_EX,//this char take width with first line
	HINDI_TOP_L,	//char lie top of base char and align same left with base char
	HINDI_TOP_C,	//char lie top of base char and align center with base char
	HINDI_TOP_R,	//char lie top of base char and align same right with base char
	HINDI_TOP_DOT,	//char lie top of base char and align same right with base char
	HINDI_BTM_L,	//char lie bottom of base char and align same left with base char
	HINDI_BTM_C,	//char lie bottom of base char and align center with base char
	HINDI_BTM_R,	//char lie bottom of base char and align same right with base char
	HINDI_BTM_DOT,	//char lie bottom of base char and align same right with base char
	HINDI_BTM_EX	//char lie at same bottom and right with base char
}HINDI_FONT_TYPE;
//Notice: this enum should't be change

static HINTEXTFUNC HINDrawTextFunc;
/*****************************************************************************
This function decide the code is avalible or no
*****************************************************************************/
BYTE HindiCharGetType(BYTE byteChar, LPUTFLOGFONT lpFont)
{
	WORD i,wVectSize;
	BYTE *ptrStart;
	BYTE type,*ptr;
	BYTE index;

	UTFMemCpyWORD(&wVectSize, (void *)lpFont->lfAddr1, 2);
	ptrStart = (BYTE *)(lpFont->lfAddr1+2);

	if(byteChar > 31)
	{
		ptr = ptrStart;
		
		index = byteChar - 32;
		for(i=0; i<index; i++)
		{
			ptr += 7;
		}
		
		type = ptr[6];
		switch(type)
		{
		case HINDI_NORMAL:
			return 1;

		case HINDI_GLUE:
		case HINDI_LEFT:
		case HINDI_RIGHT:
		case HINDI_FIRST_LINE:
		case HINDI_FIRST_LINE_EX:
			return 2;

		default:
			return 0;
		}
	}

	return 0;
}

WORD HINTextPreDone(LPTEXT pTextSrc, LPTEXT pTextBuffer, WORD bufferSize, LPUTFLOGFONT lpFont)
{
	BYTE bEngLish = TRUE;
	WORD wRealNum = 0;

	while(*pTextSrc)
	{
		if(*pTextSrc == HINDI_BYTE)
		{
			if(bEngLish == TRUE)
			{
				bEngLish = FALSE;
			}
			else
			{
				bEngLish = TRUE;
			}
			
			pTextSrc++;
		}
		else if(bEngLish)
		{
			*pTextBuffer++ = *pTextSrc++;
			wRealNum++;
		}
		else if((*pTextSrc == ' ') || (*pTextSrc == '\n'))
		{
			*pTextBuffer++ = *pTextSrc++;
			wRealNum++;
		}
		else
		{
			*pTextBuffer++ = HINDI_BYTE;
			*pTextBuffer++ = *pTextSrc++;
			wRealNum += 2;
		}
	}

	return wRealNum;
}

WORD HINTextWidth(LPTEXT pText, int charInterval, LPUTFLOGFONT lpFont)
{
	WORD index,count = 0;
	WORD textWidth = 0;
	WORD i,wVectSize;
	BYTE char1,char2;
	BYTE *ptrStart;
	BYTE type,*ptr;
	WORD wStrlen;

	UTFMemCpyWORD(&wVectSize, (void *)lpFont->lfAddr1, 2);
	ptrStart = (BYTE *)(lpFont->lfAddr1+2);

	wStrlen = strlen((char *)pText);
	while(*pText)
	{
		char1 = *pText;
		if(char1 == '\n')
			break;
		
		if(char1 == HINDI_BYTE)
		{
			char2 = *(pText+1);
			if(char2 == 0)
			{
				break;
			}
			else if(char2 > 31)
			{
				ptr = ptrStart;
				
				index = char2 - 32;
				for(i=0; i<index; i++)
				{
					ptr += 7;
				}
				
				type = ptr[6];
				switch(type)
				{
				case HINDI_NORMAL:
					count++;
					textWidth += ptr[0];
					break;

				case HINDI_LEFT:
				case HINDI_FIRST_LINE:
				case HINDI_FIRST_LINE_EX:
					if(wStrlen > 2)
					{
						if(*(pText+2))
						{
							textWidth += ptr[4];
						}
						else
						{
							textWidth += ptr[0];
						}
					}
					else
					{
						textWidth += ptr[4];
					}
					break;

				case HINDI_GLUE:
				case HINDI_RIGHT:
					textWidth += ptr[4];
					break;

				default:
					break;
				}
			}
			
			pText += 2;
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
			}while((char1 > 0) && (char1 != HINDI_BYTE));
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

static WORD HINDrawTextFunc8(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
{
	DWORD address,dwSize;
	WORD i,index,j,k,y;
	WORD wBaseLeft = 0;
	WORD wOldLeft = 0;
	WORD dx=0,dy=0;
	WORD count,wVectSize;
	WORD textWidth = 0;
	BYTE charactor,bDrawMode=1;
	BYTE data,type,bDraw;
	UTFOSDCOL color;
	int iPrevChar = -1;
	UTFRECT prevRect;
	BYTE *pVect,*pData;
	BYTE *ptrStart;
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

	//start draw char
	prevRect.left = wBaseLeft;
	prevRect.right = wBaseLeft;
	prevRect.top = 0;
	prevRect.bottom = 0;

	UTFMemCpyWORD(&wVectSize, (void *)lpFont->lfAddr1, 2);
	ptrStart = (BYTE *)(lpFont->lfAddr1+2);
	pData = ptrStart+wVectSize;

	color.data = textColor;
	while(*pText)
	{
		pText++;
		charactor = *pText++;
		if(charactor == 0)
			break;
		
		if(charactor > 31)
		{
			index = charactor - 32;
			pVect = ptrStart;

			address = 0;
		    for(i=0; i<index; i++)
		    {
				address += pVect[1];
				pVect += 7;
		    }
		    address *= lpFont->lfHeight;

			bDraw = TRUE;
			type = pVect[6];
			switch(type)
			{
			case HINDI_RIGHT:
				y = 0;
				wBaseLeft = wBaseLeft+pVect[4]-pVect[0];
				break;

			case HINDI_FIRST_LINE:
			case HINDI_FIRST_LINE_EX:
				y = 0;
				wBaseLeft = wBaseLeft+pVect[4]-pVect[5];
				break;
				
			case HINDI_LEFT:
			case HINDI_NORMAL:
			case HINDI_GLUE:
				y = 0;
				break;

			case HINDI_TOP_L:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.top-(pVect[3]-pVect[2]);
					wBaseLeft = prevRect.left;
				}
				break;

			case HINDI_TOP_C:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.top-(pVect[3]-pVect[2]);
					wBaseLeft = prevRect.left+(prevRect.right-prevRect.left-pVect[0])/2;
				}
				break;

			case HINDI_TOP_R:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.top-(pVect[3]-pVect[2]);
					wBaseLeft = prevRect.right-pVect[0]-3;
				}
				break;

			case HINDI_TOP_DOT:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = (prevRect.top-(pVect[3]-pVect[2]))/2;
					wBaseLeft = prevRect.right-pVect[0]-2;
				}
				break;

			case HINDI_BTM_L:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom;
					wBaseLeft = prevRect.left;
				}
				break;

			case HINDI_BTM_C:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom;
					wBaseLeft = prevRect.left+(prevRect.right-prevRect.left-pVect[0])/2;
				}
				break;

			case HINDI_BTM_R:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom;
					wBaseLeft = prevRect.right-pVect[0];
				}
				break;

			case HINDI_BTM_DOT:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom+1;
					wBaseLeft = prevRect.right-pVect[0]-2;
				}
				break;

			case HINDI_BTM_EX:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom-(pVect[3]-pVect[2]);
					wBaseLeft = prevRect.right-pVect[0]-2;
				}
				break;
			}

			if(bDraw)
			{
				ptrRead = pData+address;
				ptrWrite = bitmap.data+(y+dy)*bitmap.Width+wBaseLeft+dx;
			
				for(i=0; i<height; i++)
				{
					count = 0;
					for(k=0; k<pVect[1]; k++)
					{
						data = *ptrRead++;					
		   				for(j=0; j<8; j++)
		  				{
		  					if(count >= pVect[0])
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

			//remember last rectangle area
			switch(type)
			{
			case HINDI_GLUE:
				prevRect.left = wBaseLeft;
				prevRect.right = wBaseLeft+pVect[0];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				iPrevChar = charactor;

				wBaseLeft += pVect[4];
				textWidth += pVect[4];
				wOldLeft = wBaseLeft;
				break;

			case HINDI_LEFT:
				prevRect.left = wBaseLeft;
				prevRect.right = wBaseLeft+pVect[4];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				iPrevChar = charactor;

				wBaseLeft += pVect[4];
				textWidth += pVect[4];
				wOldLeft = wBaseLeft;
				break;

			case HINDI_RIGHT:
				prevRect.left = wBaseLeft+pVect[0]-pVect[4];
				prevRect.right = wBaseLeft+pVect[0];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				iPrevChar = charactor;

				wBaseLeft += pVect[0];
				textWidth += pVect[4];
				wOldLeft = wBaseLeft;
				break;

			case HINDI_NORMAL:
				iPrevChar = -1;
				
				wBaseLeft += pVect[0]+charInterval;
				textWidth += pVect[0]+charInterval;
				wOldLeft = wBaseLeft;
				break;

			case HINDI_FIRST_LINE:
			case HINDI_FIRST_LINE_EX:
				prevRect.left = wBaseLeft;
				prevRect.right = wBaseLeft+pVect[0];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				iPrevChar = charactor;

				wBaseLeft += pVect[5];
				textWidth += pVect[5];
				wOldLeft = wBaseLeft;
				break;

			default:
				wBaseLeft = wOldLeft;
				break;
			}			
		}
	}

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}

	return textWidth;
} 

static WORD HINDrawTextFunc16(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
{
	DWORD address,dwSize;
	WORD i,index,j,k,y;
	WORD wBaseLeft = 0;
	WORD wOldLeft = 0;
	WORD dx=0,dy=0;
	WORD count,wVectSize;
	WORD textWidth = 0;
	BYTE charactor,bDrawMode=1;
	BYTE data,type,bDraw;
	UTFOSDCOL color;
	int iPrevChar = -1;
	UTFRECT prevRect;
	BYTE *pVect,*pData;
	BYTE *ptrStart;
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

	//start draw char
	prevRect.left = wBaseLeft;
	prevRect.right = wBaseLeft;
	prevRect.top = 0;
	prevRect.bottom = 0;

	UTFMemCpyWORD(&wVectSize, (void *)lpFont->lfAddr1, 2);
	ptrStart = (BYTE *)(lpFont->lfAddr1+2);
	pData = ptrStart+wVectSize;

	color.data = textColor;
	while(*pText)
	{
		pText++;
		charactor = *pText++;
		if(charactor == 0)
			break;
		
		if(charactor > 31)
		{
			index = charactor - 32;
			pVect = ptrStart;

			address = 0;
		    for(i=0; i<index; i++)
		    {
				address += pVect[1];
				pVect += 7;
		    }
		    address *= lpFont->lfHeight;

			bDraw = TRUE;
			type = pVect[6];
			switch(type)
			{
			case HINDI_RIGHT:
				y = 0;
				wBaseLeft = wBaseLeft+pVect[4]-pVect[0];
				break;

			case HINDI_FIRST_LINE:
			case HINDI_FIRST_LINE_EX:
				y = 0;
				wBaseLeft = wBaseLeft+pVect[4]-pVect[5];
				break;
				
			case HINDI_LEFT:
			case HINDI_NORMAL:
			case HINDI_GLUE:
				y = 0;
				break;

			case HINDI_TOP_L:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.top-(pVect[3]-pVect[2]);
					wBaseLeft = prevRect.left;
				}
				break;

			case HINDI_TOP_C:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.top-(pVect[3]-pVect[2]);
					wBaseLeft = prevRect.left+(prevRect.right-prevRect.left-pVect[0])/2;
				}
				break;

			case HINDI_TOP_R:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.top-(pVect[3]-pVect[2]);
					wBaseLeft = prevRect.right-pVect[0]-3;
				}
				break;

			case HINDI_TOP_DOT:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = (prevRect.top-(pVect[3]-pVect[2]))/2;
					wBaseLeft = prevRect.right-pVect[0]-2;
				}
				break;

			case HINDI_BTM_L:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom;
					wBaseLeft = prevRect.left;
				}
				break;

			case HINDI_BTM_C:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom;
					wBaseLeft = prevRect.left+(prevRect.right-prevRect.left-pVect[0])/2;
				}
				break;

			case HINDI_BTM_R:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom;
					wBaseLeft = prevRect.right-pVect[0];
				}
				break;

			case HINDI_BTM_DOT:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom+1;
					wBaseLeft = prevRect.right-pVect[0]-2;
				}
				break;

			case HINDI_BTM_EX:
				if(iPrevChar < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom-(pVect[3]-pVect[2]);
					wBaseLeft = prevRect.right-pVect[0]-2;
				}
				break;
			}

			if(bDraw)
			{
				ptrRead = pData+address;
				ptrWrite = (WORD *)bitmap.data;
				ptrWrite += (y+dy)*bitmap.Width+wBaseLeft+dx;
			
				for(i=0; i<height; i++)
				{
					count = 0;
					for(k=0; k<pVect[1]; k++)
					{
						data = *ptrRead++;					
		   				for(j=0; j<8; j++)
		  				{
		  					if(count >= pVect[0])
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

			//remember last rectangle area
			switch(type)
			{
			case HINDI_GLUE:
				prevRect.left = wBaseLeft;
				prevRect.right = wBaseLeft+pVect[0];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				iPrevChar = charactor;

				wBaseLeft += pVect[4];
				textWidth += pVect[4];
				wOldLeft = wBaseLeft;
				break;

			case HINDI_LEFT:
				prevRect.left = wBaseLeft;
				prevRect.right = wBaseLeft+pVect[4];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				iPrevChar = charactor;

				wBaseLeft += pVect[4];
				textWidth += pVect[4];
				wOldLeft = wBaseLeft;
				break;

			case HINDI_RIGHT:
				prevRect.left = wBaseLeft+pVect[0]-pVect[4];
				prevRect.right = wBaseLeft+pVect[0];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				iPrevChar = charactor;

				wBaseLeft += pVect[0];
				textWidth += pVect[4];
				wOldLeft = wBaseLeft;
				break;

			case HINDI_NORMAL:
				iPrevChar = -1;
				
				wBaseLeft += pVect[0]+charInterval;
				textWidth += pVect[0]+charInterval;
				wOldLeft = wBaseLeft;
				break;

			case HINDI_FIRST_LINE:
			case HINDI_FIRST_LINE_EX:
				prevRect.left = wBaseLeft;
				prevRect.right = wBaseLeft+pVect[0];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				iPrevChar = charactor;

				wBaseLeft += pVect[5];
				textWidth += pVect[5];
				wOldLeft = wBaseLeft;
				break;

			default:
				wBaseLeft = wOldLeft;
				break;
			}			
		}
	}

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}

	return textWidth;
}

void HINTextFunc(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
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
		if(char1 == HINDI_BYTE)
		{
			char2 = *(pText+1);
			if(char2 == 0)
			{
				break;
			}
			else
			{
				len = 0;
				lpBuffer = UTFGetTextTempPtr(&bufferSize);
				do
				{
					*(lpBuffer+len++) = *pText++;
					*(lpBuffer+len++) = *pText++;
					char1 = *pText;
					char2 = *(pText+1);
				}while((char1 == HINDI_BYTE) && char2);
				*(lpBuffer+len) = 0;

				WidthEx = HINDrawTextFunc(lpBuffer, x, top, totalWidth, height, charInterval, textColor, lpFont);
				totalWidth -= WidthEx;
				x += WidthEx;
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
			}while((char1 > 0) && (char1 != HINDI_BYTE));
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
void UTFHINSetColorMode(BYTE bitPerPixel)
{
	if(bitPerPixel == 8)
	{
		HINDrawTextFunc = HINDrawTextFunc8;
	}
	if(bitPerPixel == 16)
	{
		HINDrawTextFunc = HINDrawTextFunc16;
	}
}

