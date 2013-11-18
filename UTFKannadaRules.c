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
#include "UTFKannadaRules.h"
#include "UTFMemoryPriv.h"

typedef WORD ( *KANADATEXTFUNC ) (LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont, BYTE bDrawOut);

typedef enum
{
	KANNADA_NORMAL,			//this char align lonely
	KANNADA_CHAR,			//this char will place with right char and have space
	KANNADA_GLUE,			//this char will place with right char and no space
	KANNADA_FIRST_LINE,		//this char take width with first line
	KANNADA_TOP_R,			//char lie top of base char and align same right with base char
	KANNADA_BTM_R,			//char lie bottom of base char and align same right with base char
	KANNADA_LAST_LINE,		//this char take width with last line
	KANNADA_FIRST_LINE_EX,	//this char take width with first line right side and not glue with left char
	KANNADA_GLUE_RIGHT,		//this char will glue with a char at it right side
	KANNADA_DOWN_FIRST,		//this char take width with first line and look at down
	KANNADA_DOWN_FIRST_EX,	//this char take width with first line and look at down
	KANNADA_LEFT_SIDE,		//this char take width with left side line and right side
	KANNADA_LONELY_CHAR,	//this char let down first type char take width with full width
	KANNADA_FIRST_EXTEND,	//this char take width with first line right side and not glue with left char
}KANNADA_FONT_TYPE;
//Notice: this enum should't be change

static KANADATEXTFUNC KANADADrawTextFunc;
/*****************************************************************************
This function decide the code is avalible or no
*****************************************************************************/
static BYTE KANADAGetCharType(BYTE bCharCode, LPUTFLOGFONT lpFont)
{
	BYTE *pVect;
	WORD wVectSize;
	WORD i,index;

	index = bCharCode - 32;

	UTFMemCpyWORD(&wVectSize, (void *)lpFont->lfAddr1, 2);
	pVect = (BYTE *)(lpFont->lfAddr1+2);	
    for(i=0; i<index; i++)
    {
		pVect += 7;
    }

	if(pVect[0] == 0)
	{
		return KANNADA_TOP_R;
	}

	return pVect[6];
}

WORD KANADATextPreDone(LPTEXT pTextSrc, LPTEXT pTextBuffer, WORD bufferSize, LPUTFLOGFONT lpFont)
{
	BYTE bEngLish = TRUE;
	WORD wRealNum = 0;

	while(*pTextSrc)
	{
		if(*pTextSrc == KANNADA_BYTE)
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
			*pTextBuffer++ = KANNADA_BYTE;
			*pTextBuffer++ = *pTextSrc++;
			wRealNum += 2;
		}
	}

	return wRealNum;
}

static WORD KANADADrawTextFunc8(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont, BYTE bDrawOut)
{
	DWORD address,dwSize;
	WORD i,index,j,k,y;
	WORD wBaseLeft = 0;
	WORD wOldLeft = 0;
	WORD count,wVectSize;
	WORD textWidth = 0;
	WORD dx=0,dy=0;
	BYTE data,type,bDraw;
	BYTE bNextType;
	BYTE bShowMode;
	BYTE charactor;
	BYTE bDrawMode=1;
	UTFOSDCOL color;
	int iPrevCharType = -1;
	UTFRECT prevRect;
	BYTE *pVect,*pData;
	BYTE *ptrStart;
	BYTE *pTemp;
	BYTE *ptrRead;
	BYTE *ptrWrite;
	BYTE *ptrHeader;
	UTFBITMAP bitmap;

	if(bDrawOut)
	{
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

			if(pVect[0] == 0)	//If this char real width is 0, don't use it
			{
				continue;
			}

			bDraw = TRUE;
			type = pVect[6];
			switch(type)
			{
			case KANNADA_FIRST_LINE:
			case KANNADA_FIRST_LINE_EX:
			case KANNADA_GLUE_RIGHT:
			case KANNADA_DOWN_FIRST:
			case KANNADA_DOWN_FIRST_EX:
			case KANNADA_LEFT_SIDE:
			case KANNADA_FIRST_EXTEND:
				y = 0;
				if((iPrevCharType == KANNADA_FIRST_EXTEND) || (iPrevCharType == KANNADA_LONELY_CHAR))
				{
					if(!((type == KANNADA_DOWN_FIRST) || (type == KANNADA_DOWN_FIRST_EX) || (type == KANNADA_LEFT_SIDE)))
					{
						wBaseLeft = wBaseLeft+pVect[4]-pVect[5];
					}
				}
				else
				{
					wBaseLeft = wBaseLeft+pVect[4]-pVect[5];
				}
				break;
				
			case KANNADA_NORMAL:
			case KANNADA_GLUE:
			case KANNADA_CHAR:
			case KANNADA_LAST_LINE:
			case KANNADA_LONELY_CHAR:
				y = 0;
				break;

			case KANNADA_TOP_R:
				if(iPrevCharType < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.top-(pVect[3]-pVect[2]);
					wBaseLeft = prevRect.right-pVect[0];
				}
				break;

			case KANNADA_BTM_R:
				if(iPrevCharType < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom+1;
					wBaseLeft = prevRect.right-pVect[0];
				}
				break;
			}

			if(bDraw && bDrawOut)
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
			case KANNADA_GLUE:
			case KANNADA_CHAR:
			case KANNADA_LONELY_CHAR:
				prevRect.left = wBaseLeft;
				prevRect.right = wBaseLeft+pVect[0];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				iPrevCharType = type;

				wBaseLeft += pVect[4];
				textWidth += pVect[4];
				wOldLeft = wBaseLeft;
				break;

			case KANNADA_NORMAL:
				iPrevCharType = -1;
				
				wBaseLeft += pVect[0]+charInterval;
				textWidth += pVect[0]+charInterval;
				wOldLeft = wBaseLeft;
				break;

			case KANNADA_LAST_LINE:
				if(*pText)
				{
					textWidth += prevRect.right-wBaseLeft;
					wBaseLeft = prevRect.right;
					wOldLeft = wBaseLeft;
				}
				else
				{
					textWidth += pVect[0];
					wBaseLeft += pVect[0];
				}				
				break;

			case KANNADA_FIRST_LINE:
			case KANNADA_FIRST_LINE_EX:
			case KANNADA_GLUE_RIGHT:
			case KANNADA_DOWN_FIRST:
			case KANNADA_DOWN_FIRST_EX:
			case KANNADA_LEFT_SIDE:
			case KANNADA_FIRST_EXTEND:
				prevRect.left = wBaseLeft;
				prevRect.right = wBaseLeft+pVect[0];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				bNextType = KANNADA_CHAR;

				pTemp = pText+1;
				while(*pTemp > 31)
				{
					bNextType = KANADAGetCharType(*pTemp, lpFont);
					if((bNextType != KANNADA_TOP_R) && (bNextType != KANNADA_BTM_R))
						break;

					pTemp += 2;
				}

				bShowMode = 0;
				if((iPrevCharType == KANNADA_FIRST_EXTEND) || (iPrevCharType == KANNADA_LONELY_CHAR))
				{
					if((type == KANNADA_DOWN_FIRST) || (type == KANNADA_DOWN_FIRST_EX) || (type == KANNADA_LEFT_SIDE))
						bShowMode = 1;
				}
				else if((bNextType == KANNADA_FIRST_LINE_EX) || (bNextType == KANNADA_GLUE_RIGHT) || (bNextType == KANNADA_DOWN_FIRST) || (bNextType == KANNADA_DOWN_FIRST_EX) || (bNextType == KANNADA_LEFT_SIDE))
				{
					bShowMode = 2;
				}
				else if((bNextType == KANNADA_CHAR) || (bNextType == KANNADA_NORMAL))
				{
					bShowMode = 3;
				}				

				if(bShowMode == 0)
				{
					wBaseLeft += pVect[5];
				}
				else
				{
					wBaseLeft += pVect[0];
					if(bShowMode == 2)
					{
						wBaseLeft++;
						textWidth++;
					}
				}
				
				if(bShowMode == 1)
					textWidth += pVect[0];
				else
					textWidth += pVect[4];
				wOldLeft = wBaseLeft;
				iPrevCharType = type;
				break;

			default:
				wBaseLeft = wOldLeft;
				break;
			}
		}
	}

	if(bDrawOut && bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}

	return textWidth;
}

static WORD KANADADrawTextFunc16(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont, BYTE bDrawOut)
{
	DWORD address,dwSize;
	WORD i,index,j,k,y;
	WORD wBaseLeft = 0;
	WORD wOldLeft = 0;
	WORD count,wVectSize;
	WORD textWidth = 0;
	WORD dx=0,dy=0;
	BYTE data,type,bDraw;
	BYTE bNextType;
	BYTE bShowMode;
	BYTE charactor;
	BYTE bDrawMode=1;
	UTFOSDCOL color;
	int iPrevCharType = -1;
	UTFRECT prevRect;
	BYTE *pVect,*pData;
	BYTE *ptrStart;
	BYTE *pTemp;
	BYTE *ptrRead;
	WORD *ptrWrite;
	BYTE *ptrHeader;
	UTFBITMAP bitmap;

	if(bDrawOut)
	{
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

			if(pVect[0] == 0)	//If this char real width is 0, don't use it
			{
				continue;
			}

			bDraw = TRUE;
			type = pVect[6];
			switch(type)
			{
			case KANNADA_FIRST_LINE:
			case KANNADA_FIRST_LINE_EX:
			case KANNADA_GLUE_RIGHT:
			case KANNADA_DOWN_FIRST:
			case KANNADA_DOWN_FIRST_EX:
			case KANNADA_LEFT_SIDE:
			case KANNADA_FIRST_EXTEND:
				y = 0;
				if((iPrevCharType == KANNADA_FIRST_EXTEND) || (iPrevCharType == KANNADA_LONELY_CHAR))
				{
					if(!((type == KANNADA_DOWN_FIRST) || (type == KANNADA_DOWN_FIRST_EX) || (type == KANNADA_LEFT_SIDE)))
					{
						wBaseLeft = wBaseLeft+pVect[4]-pVect[5];
					}
				}
				else
				{
					wBaseLeft = wBaseLeft+pVect[4]-pVect[5];
				}
				break;
				
			case KANNADA_NORMAL:
			case KANNADA_GLUE:
			case KANNADA_CHAR:
			case KANNADA_LAST_LINE:
			case KANNADA_LONELY_CHAR:
				y = 0;
				break;

			case KANNADA_TOP_R:
				if(iPrevCharType < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.top-(pVect[3]-pVect[2]);
					wBaseLeft = prevRect.right-pVect[0];
				}
				break;

			case KANNADA_BTM_R:
				if(iPrevCharType < 0)
				{
					bDraw = FALSE;
				}
				else
				{
					y = prevRect.bottom+1;
					wBaseLeft = prevRect.right-pVect[0];
				}
				break;
			}

			if(bDraw && bDrawOut)
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
			case KANNADA_GLUE:
			case KANNADA_CHAR:
			case KANNADA_LONELY_CHAR:
				prevRect.left = wBaseLeft;
				prevRect.right = wBaseLeft+pVect[0];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				iPrevCharType = type;

				wBaseLeft += pVect[4];
				textWidth += pVect[4];
				wOldLeft = wBaseLeft;
				break;

			case KANNADA_NORMAL:
				iPrevCharType = -1;
				
				wBaseLeft += pVect[0]+charInterval;
				textWidth += pVect[0]+charInterval;
				wOldLeft = wBaseLeft;
				break;

			case KANNADA_LAST_LINE:
				if(*pText)
				{
					textWidth += prevRect.right-wBaseLeft;
					wBaseLeft = prevRect.right;
					wOldLeft = wBaseLeft;
				}
				else
				{
					textWidth += pVect[0];
					wBaseLeft += pVect[0];
				}				
				break;

			case KANNADA_FIRST_LINE:
			case KANNADA_FIRST_LINE_EX:
			case KANNADA_GLUE_RIGHT:
			case KANNADA_DOWN_FIRST:
			case KANNADA_DOWN_FIRST_EX:
			case KANNADA_LEFT_SIDE:
			case KANNADA_FIRST_EXTEND:
				prevRect.left = wBaseLeft;
				prevRect.right = wBaseLeft+pVect[0];
				prevRect.top = pVect[2];
				prevRect.bottom = pVect[3];
				bNextType = KANNADA_CHAR;

				pTemp = pText+1;
				while(*pTemp > 31)
				{
					bNextType = KANADAGetCharType(*pTemp, lpFont);
					if((bNextType != KANNADA_TOP_R) && (bNextType != KANNADA_BTM_R))
						break;

					pTemp += 2;
				}

				bShowMode = 0;
				if((iPrevCharType == KANNADA_FIRST_EXTEND) || (iPrevCharType == KANNADA_LONELY_CHAR))
				{
					if((type == KANNADA_DOWN_FIRST) || (type == KANNADA_DOWN_FIRST_EX) || (type == KANNADA_LEFT_SIDE))
						bShowMode = 1;
				}
				else if((bNextType == KANNADA_FIRST_LINE_EX) || (bNextType == KANNADA_GLUE_RIGHT) || (bNextType == KANNADA_DOWN_FIRST) || (bNextType == KANNADA_DOWN_FIRST_EX) || (bNextType == KANNADA_LEFT_SIDE))
				{
					bShowMode = 2;
				}
				else if((bNextType == KANNADA_CHAR) || (bNextType == KANNADA_NORMAL))
				{
					bShowMode = 3;
				}				

				if(bShowMode == 0)
				{
					wBaseLeft += pVect[5];
				}
				else
				{
					wBaseLeft += pVect[0];
					if(bShowMode == 2)
					{
						wBaseLeft++;
						textWidth++;
					}
				}
				
				if(bShowMode == 1)
					textWidth += pVect[0];
				else
					textWidth += pVect[4];
				wOldLeft = wBaseLeft;
				iPrevCharType = type;
				break;

			default:
				wBaseLeft = wOldLeft;
				break;
			}
		}
	}

	if(bDrawOut && bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}

	return textWidth;
}

void KANADATextFunc(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
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
		if(char1 == KANNADA_BYTE)
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
				}while((char1 == KANNADA_BYTE) && char2);
				*(lpBuffer+len) = 0;

				WidthEx = KANADADrawTextFunc(lpBuffer, x, top, totalWidth, height, charInterval, textColor, lpFont, 1);
				if(totalWidth < width)
				{
					totalWidth -= charInterval;
					x += charInterval;
				}
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
			}while((char1 > 0) && (char1 != KANNADA_BYTE));
			*(lpBuffer+len) = 0;

			if(height > lpFont->lfFontExTop)
			{
				ENGTextFunc(lpBuffer, x, top+lpFont->lfFontExTop, totalWidth, height-lpFont->lfFontExTop, charInterval, textColor, (LPUTFLOGFONT)lpFont->lfLogFontEx);
			}

			WidthEx = ENGTextWidth(lpBuffer, charInterval, (LPUTFLOGFONT)lpFont->lfLogFontEx);
			if(totalWidth < width)
			{
				totalWidth -= charInterval;
				x += charInterval;
			}
			totalWidth -= WidthEx;
			x += WidthEx;
		}
	}
}

WORD KANADATextWidth(LPTEXT pText, int charInterval, LPUTFLOGFONT lpFont)
{
	WORD textWidth = 0;
	BYTE char1,char2;
	WORD bufferSize;
	WORD len = 0;
	BYTE *lpBuffer;
	WORD WidthEx;

	while(*pText)
	{
		char1 = *pText;
		if(char1 == '\n')
			break;
		
		if(char1 == KANNADA_BYTE)
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
				}while((char1 == KANNADA_BYTE) && char2);
				*(lpBuffer+len) = 0;

				WidthEx = KANADADrawTextFunc(lpBuffer, 0, 0, 0, 0, charInterval, 0, lpFont, 0);
				if(textWidth)
				{
					textWidth += charInterval;
				}
				textWidth += WidthEx;
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
			}while((char1 > 0) && (char1 != KANNADA_BYTE));
			*(lpBuffer+len) = 0;

			WidthEx = ENGTextWidth(lpBuffer, charInterval, (LPUTFLOGFONT)lpFont->lfLogFontEx);
			if(textWidth)
			{
				textWidth += charInterval;
			}
			textWidth += WidthEx;
		}
	}

	return textWidth;
}
/*************************************************************************/
void UTFKANADASetColorMode(BYTE bitPerPixel)
{
	if(bitPerPixel == 8)
	{
		KANADADrawTextFunc = KANADADrawTextFunc8;
	}
	if(bitPerPixel == 16)
	{
		KANADADrawTextFunc = KANADADrawTextFunc16;
	}
}

