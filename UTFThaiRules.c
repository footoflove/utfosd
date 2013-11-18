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
#include "UTFDrawAPI.h"
#include "UTFTHAIRules.h"
#include "UTFGraphPortPriv.h"
#include "UTFMemoryPriv.h"

typedef WORD ( *THAITEXTFUNC ) (LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont, BYTE bDrawOut);

typedef enum
{
	THAI_TOP_LINE,			//this char will lie in top row
	THAI_TOP_LINE_C_BR,		//this char will lie in second row and center position align base char right side
	THAI_BASE_LINE_NORMAL,	//this char align lonely
	THAI_BASE_LINE_GLUE,	//this char type is not be used really
	THAI_BASE_LINE_RSVH,	//this char align lonely but it right side has very high line
	THAI_BASE_LINE_RSL,		//this char align lonely and it's right side has vertical line
	THAI_BTM_LINE,			//this char will lie in fourth row
	THAI_ANSI_CHAR,			//this char is base latin character
	THAI_BASE_LINE_RSL_GLUETOP//this char align lonely and it's right side has vertical line and the second line char will glue bottom with it
}THAI_FONT_TYPE;

static THAITEXTFUNC THAIDrawTextFunc;
/*************************************************************************
** This function will return the ansi code same as unicode data character
*************************************************************************/
static int ThaiUnicodeToANSI(WORD wUnicode)
{
	if((wUnicode > 0x0E00) && (wUnicode < 0x0E5C))
	{
		return (0xA0+wUnicode-0x0E00);
	}
	
	return (-1);
}
/**************************************************************************/
static BYTE THAIGetCharType(BYTE character, LPUTFLOGFONT lpFont)
{
	BYTE *pVect;
	WORD i,index;
	BYTE *ptrStart;

	if(character == 0)
		return THAI_ANSI_CHAR;

	if((character > 4) && (character < 0x20))
		return THAI_ANSI_CHAR;

	ptrStart = (BYTE *)(lpFont->lfAddr1+2);

	if(character >= 0x20)
		index = character - 28;
	else
		index = character - 1;

	pVect = ptrStart;
    for(i=0; i<index; i++)
    {
		pVect += 7;
    }
	return pVect[6];
}

WORD THAITextPreDone(LPTEXT pTextSrc, LPTEXT pTextBuffer, WORD bufferSize, LPUTFLOGFONT lpFont)
{
	WORD wRealNum = 0;
	WORD i,wUnicode;
	int iANSICode;
	LPTEXT pTextBuf = pTextBuffer;

	while(*pTextSrc)
	{
		if(*pTextSrc == 0x0E)
		{
			if(*(pTextSrc+1))
			{
				wUnicode = *pTextSrc;
				wUnicode <<= 8;
				wUnicode += *(pTextSrc+1);
				
				iANSICode = ThaiUnicodeToANSI(wUnicode);
				if(iANSICode > 0)
				{
					if(iANSICode == 0xD3)
					{
						if(pTextBuf < pTextBuffer)
						{
							BYTE *ptrTopChar = NULL;
							BYTE type,enable = 1;

							for(i=0; i<5; i++)
							{
								type = THAIGetCharType(*(pTextBuffer-1-i), lpFont);
								switch(type)
								{
								case THAI_TOP_LINE:
								case THAI_TOP_LINE_C_BR:
									ptrTopChar = pTextBuffer-1-i;
									enable = 0;
									break;

								case THAI_BASE_LINE_NORMAL:
								case THAI_BASE_LINE_RSVH:
								case THAI_BASE_LINE_RSL:
								case THAI_ANSI_CHAR:
								case THAI_BASE_LINE_RSL_GLUETOP:
									enable = 0;
									break;
								}
								if(!enable)
									break;
							}
							
							if(ptrTopChar != NULL)
							{
								BYTE *ptrTemp = pTextBuffer;
								
								while(ptrTemp > ptrTopChar)
								{
									*ptrTemp = *(ptrTemp-1);
									ptrTemp--;
								}
								*ptrTopChar = 0xED;
							}
							else
							{
								*pTextBuffer = 0xED;
							}
							pTextBuffer++;
						}
						else
						{
							*pTextBuffer++ = 0xED;
						}
						*pTextBuffer++ = 0xD2;
						wRealNum += 2;
					}
					else
					{
						*pTextBuffer++ = (BYTE)iANSICode;
						wRealNum++;
					}
					pTextSrc += 2;
				}
				else
				{
					*pTextBuffer++ = *pTextSrc++;
					wRealNum++;
				}
			}
			else
			{
				*pTextBuffer++ = *pTextSrc++;
				wRealNum++;
			}
		}
		else
		{
			if(*pTextSrc == 0xD3)
			{
				pTextSrc++;
				if(pTextBuf < pTextBuffer)
				{
					BYTE *ptrTopChar = NULL;
					BYTE type,enable = 1;

					for(i=0; i<5; i++)
					{
						type = THAIGetCharType(*(pTextBuffer-1-i), lpFont);
						switch(type)
						{
						case THAI_TOP_LINE:
						case THAI_TOP_LINE_C_BR:
							ptrTopChar = pTextBuffer-1-i;
							enable = 0;
							break;

						case THAI_BASE_LINE_NORMAL:
						case THAI_BASE_LINE_RSVH:
						case THAI_BASE_LINE_RSL:
						case THAI_ANSI_CHAR:
						case THAI_BASE_LINE_RSL_GLUETOP:
							enable = 0;
							break;
						}
						if(!enable)
							break;
					}
					
					if(ptrTopChar != NULL)
					{
						BYTE *ptrTemp = pTextBuffer;
						
						while(ptrTemp > ptrTopChar)
						{
							*ptrTemp = *(ptrTemp-1);
							ptrTemp--;
						}
						*ptrTopChar = 0xED;
					}
					else
					{
						*pTextBuffer = 0xED;
					}
					pTextBuffer++;
				}
				else
				{
					*pTextBuffer++ = 0xED;
				}
				*pTextBuffer++ = 0xD2;
				wRealNum += 2;
			}
			else
			{
				*pTextBuffer++ = *pTextSrc++;
				wRealNum++;
			}
		}		
	}

	return wRealNum;
}

static char THAIHaveBaseChar(LPTEXT pText, BYTE *ptrStart)
{
	BYTE charactor,type;
	BYTE *pVect;
	WORD i,index;

	while(*pText)
	{
		charactor = *pText++;

		if((charactor > 4) && (charactor < 0x20))
			continue;

		if(charactor >= 0x20)
			index = charactor - 28;
		else
			index = charactor - 1;

		pVect = ptrStart;
	    for(i=0; i<index; i++)
	    {
			pVect += 7;
	    }
		type = pVect[6];
		switch(type)
		{
		case THAI_BASE_LINE_NORMAL:
		case THAI_BASE_LINE_RSVH:
		case THAI_BASE_LINE_RSL:
		case THAI_ANSI_CHAR:
		case THAI_BASE_LINE_RSL_GLUETOP:
			return 1;
		}
	}
	
	return 0;
}

static int THAIGetCharHeight(BYTE character, BYTE *ptrStart)
{
	BYTE charactor,type;
	BYTE *pVect;
	WORD i,index;

	if(character == 0)
		return -1;
	if((character > 4) && (character < 0x20))
		return -1;

	if(character >= 0x20)
		index = character - 28;
	else
		index = character - 1;

	pVect = ptrStart;
    for(i=0; i<index; i++)
    {
		pVect += 7;
    }
	type = pVect[6];
	switch(type)
	{
	case THAI_TOP_LINE:
	case THAI_TOP_LINE_C_BR:
		return (pVect[3]-pVect[2]);
	}
	
	return -1;
}

static WORD THAIDrawTextFunc8(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont, BYTE bDrawOut)
{
	DWORD address,dwSize;
	WORD wBaseLeft = 0;
	WORD wOldLeft = 0;
	WORD count,wVectSize;
	WORD textWidth = 0;
	WORD wRCTypeLeft = 0;
	WORD dx=0,dy=0;
	BYTE bWhichLine = 2;
	BYTE bPrevRCType = 0;
	BYTE charactor,deltaY = 1;
	BYTE data,type,bDraw;
	BYTE bDrawMode=1;
	int i,index,j,k,x,y;
	int iPrevCharType = -1;
	UTFOSDCOL color;
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
	if(lpFont->lfHeight > 20)
	{
		deltaY = (lpFont->lfHeight-16)/6;
	}
	if(height == lpFont->lfHeight)
	{
		height += deltaY;
	}

	if(bDrawOut)
	{
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

	if(height > lpFont->lfHeight)
	{
		height = lpFont->lfHeight;
	}

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
		charactor = *pText++;

		if((charactor > 4) && (charactor < 0x20))
			continue;

		if(charactor >= 0x20)
			index = charactor - 28;
		else
			index = charactor - 1;

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
		case THAI_BASE_LINE_NORMAL:
		case THAI_BASE_LINE_RSVH:
		case THAI_BASE_LINE_RSL:
		case THAI_ANSI_CHAR:
		case THAI_BASE_LINE_RSL_GLUETOP:
			y = deltaY;
			bPrevRCType = 0;
			break;

		case THAI_TOP_LINE:
			if(iPrevCharType < 0)
			{
				bDraw = FALSE;
			}
			else
			{
				BYTE bType = 0;

				if(iPrevCharType == THAI_BASE_LINE_RSVH)
				{
					if(bWhichLine == 2)
					{
						bType = 1;
					}
				}

				if(bType)
				{
					y = prevRect.top;
					wBaseLeft = prevRect.left;
				}
				else
				{
					int iNextHeight = THAIGetCharHeight(*pText, ptrStart);
					
					y = prevRect.top-(pVect[3]-pVect[2]);
					if((iPrevCharType == THAI_BASE_LINE_RSL_GLUETOP) && (bWhichLine == 2))
						;
					else
						y -= deltaY;
					if(y < 0)
					{
						y = 0;
					}
					else if(iNextHeight > 0)
					{
						if(y < (iNextHeight+deltaY))
						{
							y = iNextHeight+deltaY;
						}
					}
					if(bPrevRCType && (charactor == 0xE9))
						wBaseLeft = wRCTypeLeft;
					else
						wBaseLeft = prevRect.right-pVect[0];
				}
				bPrevRCType = 0;
			}
			break;

		case THAI_TOP_LINE_C_BR:
			if(iPrevCharType < 0)
			{
				bDraw = FALSE;
			}
			else
			{
				BYTE bType = 0;

				if(iPrevCharType == THAI_BASE_LINE_RSVH)
				{
					if(bWhichLine == 2)
					{
						bType = 1;
					}
				}

				if(bType)
				{
					y = prevRect.top;
					wBaseLeft = prevRect.left;
				}
				else
				{
					int iNextHeight = THAIGetCharHeight(*pText, ptrStart);
					
					y = prevRect.top-(pVect[3]-pVect[2]);
					if((iPrevCharType == THAI_BASE_LINE_RSL_GLUETOP) && (bWhichLine == 2))
						;
					else
						y -= deltaY;
					if(y < 0)
					{
						y = 0;
					}
					else if(iNextHeight > 0)
					{
						if(y < (iNextHeight+deltaY))
						{
							y = iNextHeight+deltaY;
						}
					}
					wBaseLeft = prevRect.right-pVect[0]/2;
					
					if(bWhichLine == 2)
					{
						bPrevRCType = 1;
						wRCTypeLeft = wBaseLeft;
					}
				}
			}
			break;

		case THAI_BTM_LINE:
			if(iPrevCharType < 0)
			{
				bDraw = FALSE;
			}
			else
			{
				y = prevRect.bottom+deltaY;
				wBaseLeft = prevRect.right-pVect[0];
				if(height >= lpFont->lfHeight)
				{
					if((y+pVect[3]-pVect[2]) > height)
					{
						y = height-(pVect[3]-pVect[2]);
					}
				}
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
		case THAI_BASE_LINE_RSVH:
		case THAI_BASE_LINE_RSL:
		case THAI_BASE_LINE_NORMAL:
		case THAI_BASE_LINE_RSL_GLUETOP:
			iPrevCharType = type;

			prevRect.left = wBaseLeft;
			prevRect.right = wBaseLeft+pVect[5];
			prevRect.top = pVect[2]+deltaY;
			prevRect.bottom = pVect[3]+deltaY;

			wBaseLeft += pVect[0];
			textWidth += pVect[0];
			if(THAIHaveBaseChar(pText, ptrStart))
			{
				wBaseLeft += charInterval;
				textWidth += charInterval;
			}
			wOldLeft = wBaseLeft;
			bWhichLine = 2;
			break;

		case THAI_ANSI_CHAR:
			iPrevCharType = -1;

			wBaseLeft += pVect[0];
			textWidth += pVect[0];
			if(THAIHaveBaseChar(pText, ptrStart))
			{
				wBaseLeft += charInterval;
				textWidth += charInterval;
			}
			wOldLeft = wBaseLeft;
			bWhichLine = 2;
			break;

		case THAI_TOP_LINE_C_BR:
			if(iPrevCharType == THAI_BASE_LINE_RSVH)
			{
				if(bWhichLine == 2)
				{
					prevRect.right = prevRect.left+pVect[0];
				}
			}
			else
			{
				prevRect.top -= (pVect[3]-pVect[2]);
				if((iPrevCharType == THAI_BASE_LINE_RSL_GLUETOP) && (bWhichLine == 2))
					;
				else
					prevRect.top -= deltaY;
			}
			wBaseLeft = wOldLeft;
			if(!THAIHaveBaseChar(pText, ptrStart))
			{
				textWidth += pVect[0]/2;
			}
			bWhichLine = 1;
			break;

		case THAI_TOP_LINE:
			if(iPrevCharType == THAI_BASE_LINE_RSVH)
			{
				if(bWhichLine == 2)
				{
					prevRect.right = prevRect.left+pVect[0];
				}
			}
			else
			{
				prevRect.top -= (pVect[3]-pVect[2]);
				if((iPrevCharType == THAI_BASE_LINE_RSL_GLUETOP) && (bWhichLine == 2))
					;
				else
					prevRect.top -= deltaY;
			}
			wBaseLeft = wOldLeft;
			bWhichLine = 1;
			break;

		default:
			wBaseLeft = wOldLeft;
			break;
		}
	}

	if(bDrawOut && bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}

	return textWidth;
}

static WORD THAIDrawTextFunc16(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont, BYTE bDrawOut)
{
	DWORD address,dwSize;
	WORD wBaseLeft = 0;
	WORD wOldLeft = 0;
	WORD count,wVectSize;
	WORD textWidth = 0;
	WORD wRCTypeLeft = 0;
	WORD dx=0,dy=0;
	BYTE bWhichLine = 2;
	BYTE bPrevRCType = 0;
	BYTE charactor,deltaY = 1;
	BYTE data,type,bDraw;
	BYTE bDrawMode=1;
	int i,index,j,k,x,y;
	int iPrevCharType = -1;
	UTFOSDCOL color;
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
	if(lpFont->lfHeight > 20)
	{
		deltaY = (lpFont->lfHeight-16)/6;
	}
	if(height == lpFont->lfHeight)
	{
		height += deltaY;
	}

	if(bDrawOut)
	{
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

	if(height > lpFont->lfHeight)
	{
		height = lpFont->lfHeight;
	}

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
		charactor = *pText++;

		if((charactor > 4) && (charactor < 0x20))
			continue;

		if(charactor >= 0x20)
			index = charactor - 28;
		else
			index = charactor - 1;

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
		case THAI_BASE_LINE_NORMAL:
		case THAI_BASE_LINE_RSVH:
		case THAI_BASE_LINE_RSL:
		case THAI_ANSI_CHAR:
		case THAI_BASE_LINE_RSL_GLUETOP:
			y = deltaY;
			bPrevRCType = 0;
			break;

		case THAI_TOP_LINE:
			if(iPrevCharType < 0)
			{
				bDraw = FALSE;
			}
			else
			{
				BYTE bType = 0;

				if(iPrevCharType == THAI_BASE_LINE_RSVH)
				{
					if(bWhichLine == 2)
					{
						bType = 1;
					}
				}

				if(bType)
				{
					y = prevRect.top;
					wBaseLeft = prevRect.left;
				}
				else
				{
					int iNextHeight = THAIGetCharHeight(*pText, ptrStart);
					
					y = prevRect.top-(pVect[3]-pVect[2]);
					if((iPrevCharType == THAI_BASE_LINE_RSL_GLUETOP) && (bWhichLine == 2))
						;
					else
						y -= deltaY;
					if(y < 0)
					{
						y = 0;
					}
					else if(iNextHeight > 0)
					{
						if(y < (iNextHeight+deltaY))
						{
							y = iNextHeight+deltaY;
						}
					}
					if(bPrevRCType && (charactor == 0xE9))
						wBaseLeft = wRCTypeLeft;
					else
						wBaseLeft = prevRect.right-pVect[0];
				}
				bPrevRCType = 0;
			}
			break;

		case THAI_TOP_LINE_C_BR:
			if(iPrevCharType < 0)
			{
				bDraw = FALSE;
			}
			else
			{
				BYTE bType = 0;

				if(iPrevCharType == THAI_BASE_LINE_RSVH)
				{
					if(bWhichLine == 2)
					{
						bType = 1;
					}
				}

				if(bType)
				{
					y = prevRect.top;
					wBaseLeft = prevRect.left;
				}
				else
				{
					int iNextHeight = THAIGetCharHeight(*pText, ptrStart);
					
					y = prevRect.top-(pVect[3]-pVect[2]);
					if((iPrevCharType == THAI_BASE_LINE_RSL_GLUETOP) && (bWhichLine == 2))
						;
					else
						y -= deltaY;
					if(y < 0)
					{
						y = 0;
					}
					else if(iNextHeight > 0)
					{
						if(y < (iNextHeight+deltaY))
						{
							y = iNextHeight+deltaY;
						}
					}
					wBaseLeft = prevRect.right-pVect[0]/2;
					
					if(bWhichLine == 2)
					{
						bPrevRCType = 1;
						wRCTypeLeft = wBaseLeft;
					}
				}
			}
			break;

		case THAI_BTM_LINE:
			if(iPrevCharType < 0)
			{
				bDraw = FALSE;
			}
			else
			{
				y = prevRect.bottom+deltaY;
				wBaseLeft = prevRect.right-pVect[0];
				if(height >= lpFont->lfHeight)
				{
					if((y+pVect[3]-pVect[2]) > height)
					{
						y = height-(pVect[3]-pVect[2]);
					}
				}
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
		case THAI_BASE_LINE_RSVH:
		case THAI_BASE_LINE_RSL:
		case THAI_BASE_LINE_NORMAL:
		case THAI_BASE_LINE_RSL_GLUETOP:
			iPrevCharType = type;

			prevRect.left = wBaseLeft;
			prevRect.right = wBaseLeft+pVect[5];
			prevRect.top = pVect[2]+deltaY;
			prevRect.bottom = pVect[3]+deltaY;

			wBaseLeft += pVect[0];
			textWidth += pVect[0];
			if(THAIHaveBaseChar(pText, ptrStart))
			{
				wBaseLeft += charInterval;
				textWidth += charInterval;
			}
			wOldLeft = wBaseLeft;
			bWhichLine = 2;
			break;

		case THAI_ANSI_CHAR:
			iPrevCharType = -1;

			wBaseLeft += pVect[0];
			textWidth += pVect[0];
			if(THAIHaveBaseChar(pText, ptrStart))
			{
				wBaseLeft += charInterval;
				textWidth += charInterval;
			}
			wOldLeft = wBaseLeft;
			bWhichLine = 2;
			break;

		case THAI_TOP_LINE_C_BR:
			if(iPrevCharType == THAI_BASE_LINE_RSVH)
			{
				if(bWhichLine == 2)
				{
					prevRect.right = prevRect.left+pVect[0];
				}
			}
			else
			{
				prevRect.top -= (pVect[3]-pVect[2]);
				if((iPrevCharType == THAI_BASE_LINE_RSL_GLUETOP) && (bWhichLine == 2))
					;
				else
					prevRect.top -= deltaY;
			}
			wBaseLeft = wOldLeft;
			if(!THAIHaveBaseChar(pText, ptrStart))
			{
				textWidth += pVect[0]/2;
			}
			bWhichLine = 1;
			break;

		case THAI_TOP_LINE:
			if(iPrevCharType == THAI_BASE_LINE_RSVH)
			{
				if(bWhichLine == 2)
				{
					prevRect.right = prevRect.left+pVect[0];
				}
			}
			else
			{
				prevRect.top -= (pVect[3]-pVect[2]);
				if((iPrevCharType == THAI_BASE_LINE_RSL_GLUETOP) && (bWhichLine == 2))
					;
				else
					prevRect.top -= deltaY;
			}
			wBaseLeft = wOldLeft;
			bWhichLine = 1;
			break;

		default:
			wBaseLeft = wOldLeft;
			break;
		}
	}

	if(bDrawOut && bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}

	return textWidth;
}

void THAITextFunc(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
{
	THAIDrawTextFunc(pText, left, top, width, height, charInterval, textColor, lpFont, 1);
}

WORD THAITextWidth(LPTEXT pText, int charInterval, LPUTFLOGFONT lpFont)
{
	return THAIDrawTextFunc(pText, 0, 0, 0, 0, charInterval, 0, lpFont, 0);
}

/*************************************************************************/
void UTFTHAISetColorMode(BYTE bitPerPixel)
{
	if(bitPerPixel == 8)
	{
		THAIDrawTextFunc = THAIDrawTextFunc8;
	}
	if(bitPerPixel == 16)
	{
		THAIDrawTextFunc = THAIDrawTextFunc16;
	}
}

