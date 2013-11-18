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
#include "UTFArabicRules.h"
#include "UTFDrawTextPriv.h"
#include "UTFDrawAPI.h"
#include "UTFMemoryPriv.h"

/***********************************************************************************/
typedef struct 
{
	WORD code;
	int lonelycode;
	int frontcode;
	int backcode;
	int middlecode;
}UTFCODEDATA;

#define INTER_CODE_COUNT 	78
#define BACK_LONELY_COUNT 	49
#define FRONT_LONELY_COUNT	24
#define LR_CODE_COUNT 		5

TEXTPROC ARBTextFunc;
/***********************************************************************************/
static BYTE g_RCode[BACK_LONELY_COUNT] = 
{
	0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD,
	0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
	0x19, 0x1A, 0x1C, 0x1E, 0x24, 0x25, 0x26, 0x27, 0x2C, 0x39, 0x3A,
	0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46,
	0x47, 0x48, 0x4B
};

static BYTE g_DSideCode[ LR_CODE_COUNT ] = { 0x11, 0x12, 0x13, 0x2C, 0x42 };
static BYTE g_FrontLonelyCode[FRONT_LONELY_COUNT] =
{
	0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD,
	0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x2C, 0x42
};

static UTFCODEDATA g_CodeMap[ INTER_CODE_COUNT ] = 
{
	{ 0, 0, 0, 0, 0 },				{ 0X20, -96, -96, -96, -96 },
	{ 0X2C, -84, -84, -84, -84 },	{ 0X2E, -82, -82, -82, -82 },
	{ 0X2F, -81, -81, -81, -81 },	{ 0X30, -80, -80, -80, -80 },
	{ 0X31, -79, -79, -79, -79 },	{ 0X32, -78, -78, -78, -78 },
	{ 0X33, -77, -77, -77, -77 },	{ 0X34, -76, -76, -76, -76 },
	{ 0X35, -75, -7, -75, -75 },	{ 0X36, -74, -74, -74, -74 },
	{ 0X37, -73, -73, -73, -73 },	{ 0X38, -72, -72, -72, -72 },
	{ 0X39, -71, -71, -71, -71 },	{ 0X3C, -68, -68, -68, -68 },
	{ 0X3E, -66, -66, -66, -66 },	{ 0X91, 105, 105, 105, 105 },
	{ 0X92, 106, 106, 106, 106 },	{ 0XA1, 72, 72, 72, 72 },
	{ 0XBA, 64, 64, 64, 64 },		{ 0XBF, 92, 92, 92, 92 },
	{ 0XC1, 77, 77, 77, 77 },		{ 0XC2, 83, 83, 84, 84 },
	{ 0XC3, 69, 69, 70, 70 },		{ 0XC4, 78, 78, 78, 78 },
	{ 0XC5, 107, 107, 108, 108 },	{ 0XC6, 74, 75, 76, 75 },
	{ 0XC7, 1, 1, 2, 2 },			{ 0XC8, 3, 4, 3, 4 },
	{ 0XC9, 87, 87, 88, 88},		{ 0XCA, 5, 6, 5, 6 },
	{ 0XCB, 7, 8, 7, 8 },			{ 0XCC, 9, 10, 11, 10 },
	{ 0XCD, 12, 13, 14, 13 },		{ 0XCE, 15, 16, 17, 16 },
	{ 0XCF, 18, 18, 18, 18 },		{ 0XD0, 19, 19, 19, 19 },
	{ 0XD1, 20, 20, 20, 20 },		{ 0XD2, 21, 21, 21, 21 },
	{ 0XD3, 22, 23, 22, 23 },		{ 0XD4, 24, 25, 24, 25 },
	{ 0XD5, 26, 27, 26, 27 },		{ 0XD6, 28, 29, 28, 29 },
	{ 0XD7, 63, 63, 63, 63 },		{ 0XD8, 30, 30, 30, 30 },
	{ 0XD9, 31, 31, 31, 31 },		{ 0XDA, 32, 33, 34, 35 },
	{ 0XDB, 36, 37, 38, 39 },		{ 0XDC, 71, 71, 71, 71 },
	{ 0XDD, 40, 41, 40, 42 },		{ 0XDE, 43, 44, 43, 45 },
	{ 0XDF, 46, 47, 46, 47 },		{ 0XE1, 48, 49, 48, 49 },
	{ 0XE3, 50, 51, 50, 51 },		{ 0XE4, 52, 53, 52, 53 },
	{ 0XE5, 54, 55, 56, 57 },		{ 0XE6, 58, 58, 58, 58 },
	{ 0XEC, 85, 85, 86, 86 },		{ 0XED, 59, 60, 61, 60 },
	{ 0XF0, 111, 111, 111, 111 },	{ 0XF1, 112, 112, 112, 112 },
	{ 0XF2, 113, 113, 113, 113 },	{ 0XF3, 114, 114, 114, 114 },
	{ 0XF5, 115, 115, 115, 115 },	{ 0XF6, 116, 116, 116, 116 },
	{ 0XF7, 62, 62, 62, 62 },		{ 0XF8, 93, 93, 93, 93 },
	{ 0XFA, 117, 117, 117, 117 },	{ 0XE1C2, 79, 79, 80, 80 },
	{ 0XE1C3, 67, 67, 68, 68 },		{ 0XE1C5, 109, 109, 110, 110 },
	{ 0XE1C7, 81, 81, 82, 82 },		{ 0X90, -112, -110, -111, -109 },
	{ 0X98, 97, 99, 98, 100 },		{ 0X8E, 104, 104, 104, 104 },
	{ 0X81, 101, 102, 101, 102 },	{ 0X8D, 94, 96, 95, 96 }
};

static BYTE *g_ptrCodeData;
static BYTE g_cTempData[2048];
/***********************************************************************************/

static int BelongToRSide( BYTE code )
{
	WORD low, high = BACK_LONELY_COUNT;
	
	if( code < 0x80 )
	{
		return TRUE;
	}
	
	code &= 0x7F;
	for( low = 0; low < high; low++ )
	{
		if( code == g_RCode[low] )
		{
			return TRUE;
		}
	}
	
	return FALSE;
}

static int BelongToDoubleSide( BYTE code )
{
	WORD low, high = LR_CODE_COUNT;
	
	if( code < 0x80 )
	{
		return TRUE;
	}
	
	code &= 0x7F;
	for( low = 0; low < high; low++ )
	{
		if( code == g_DSideCode[low] )
		{
			return TRUE;
		}
	}
	
	return FALSE;
}

static BYTE FindCodeID( WORD code )
{
	WORD low, high = INTER_CODE_COUNT;
	
	if( code < 0x80 )
	{
		return (BYTE)code;
	}
	
	for(low=0; low<high; low++)
	{
		if( code == g_CodeMap[low].code )
		{
			return ( low | 0x80 );
		}
	}
	
	return ( code & 0x7F );
}

static int BelongToLSide( BYTE code )
{
	WORD low, high = FRONT_LONELY_COUNT;
	
	if( code < 0x80 )
	{
		return TRUE;
	}
	
	code &= 0x7F;
	for( low = 0; low < high; low++ )
	{
		if( code == g_FrontLonelyCode[low] )
		{
			return TRUE;
		}
	}
	
	return FALSE;
}

static void InterCodeChange( BYTE *lpTable, BYTE *lpMatrix, int total )
{
	BYTE bFrontFlag = FALSE;
	BYTE bNextFlag = FALSE;
	BYTE bFrontLR = FALSE;
	int index, iTemp;
	
	if( total <= 0 )
		return;
	
	if( total == 1 )
	{
		if( lpTable[0] < 0x80 )
		{
			lpMatrix[0] = lpTable[0];
		}
		else
		{
			lpMatrix[0] = g_CodeMap[lpTable[0]-0x80].lonelycode + 0x80;
		}
		
		return;
	}
	
	for( index=0; index<total; index++ )
	{
		if( lpTable[index] < 0x80 )
		{
			lpMatrix[index] = lpTable[index];
		}
		else
		{
			bNextFlag = TRUE;
			if( index+1 >=total )
			{
				bNextFlag = FALSE;
			}
			else if(BelongToLSide(lpTable[index+1]))
			{
				bNextFlag = FALSE;
			}

			if( bFrontFlag && bNextFlag )
			{
				lpMatrix[index] = g_CodeMap[lpTable[index]-0x80].middlecode+0x80;
			}
			else if( bFrontFlag && !bNextFlag )
			{
				lpMatrix[index] = g_CodeMap[lpTable[index]-0x80].backcode+0x80;
			}
			else if( !bFrontFlag && bNextFlag )
			{
				lpMatrix[index] = g_CodeMap[lpTable[index]-0x80].frontcode+0x80;
			}
			else if( !bFrontFlag && !bNextFlag )
			{
				lpMatrix[index] = g_CodeMap[lpTable[index]-0x80].lonelycode+0x80;
			}
		}

		bFrontFlag = 1-BelongToRSide(lpTable[index]);
		if((lpTable[index] == 0x82) && (lpMatrix[index] != 90+0x80) && bFrontLR )
		{
			for( iTemp=index+1; iTemp<total; iTemp++ )
			{
				if( !BelongToDoubleSide( lpTable[iTemp] )) 
				{
					lpMatrix[index] = 90;
					break;
				}
			}
		}
		
		if( !BelongToDoubleSide( lpTable[index] ) )
		{
			bFrontLR = TRUE;
		}
	}
}

static void InterCodeToFontCode( BYTE *text, WORD character_number, BYTE *lpRealCodeBuffer, WORD *real_len, LPUTFLOGFONT lpFont )
{
	char cTemp;
	int iTemp,id = 0;
	WORD i,code;
	WORD string_len;
	WORD iRealTextLen = 0;
	BYTE bWord_Width;

	if( character_number <= 0 ) 
	{
		*real_len = 0;
		return;
	}

	string_len = character_number;
	while( id < string_len )
	{
		BYTE character = 0x00;
		
		if( id < (string_len-1) )
		{
			character = (BYTE)text[id + 1];
		}
		
		if((text[id] == 0xe1) && ((character == 0xc2)||(character == 0xc3)||(character == 0xc5)||(character == 0xc7)))
		{
			bWord_Width = 2;
			code = (0xe1<<8)|character;
		}
		else
		{
			bWord_Width= 1;
			code = text[id];
		}
		
		id += bWord_Width;
		g_ptrCodeData[ iRealTextLen++ ] = FindCodeID( code );
	}
	
	id = 0;
	while( id < iRealTextLen )
	{
		if( BelongToDoubleSide( g_ptrCodeData[id] ) )
		{
			iTemp = id;
			while( id < iRealTextLen )
			{
				if( !BelongToDoubleSide(g_ptrCodeData[ id]) )
					break;
				id++;
			}
			
			string_len = id - iTemp;
			for( id=0; id<string_len/2; id++ )
			{
				cTemp = g_ptrCodeData[iTemp+id];
				g_ptrCodeData[iTemp+id] = g_ptrCodeData[iTemp+string_len-1-id];
				g_ptrCodeData[iTemp+string_len-1-id] = cTemp;
			}
			id = string_len + iTemp;
		}
		id++;
	}

	InterCodeChange( g_ptrCodeData, g_cTempData, iRealTextLen );

	id = 0;
	for( i=0; i<iRealTextLen; i++ )
	{
		lpRealCodeBuffer[ id++ ] = g_cTempData[ iRealTextLen-1-i ];
	}
	iRealTextLen = id;

	lpRealCodeBuffer[iRealTextLen] = 0;
	*real_len = iRealTextLen;
}

/*************************************************************************/
static int ARBFindCode( WORD code )
{
	WORD low, high = INTER_CODE_COUNT;
	
	for(low=0; low<high; low++)
	{
		if( code == g_CodeMap[low].code )
		{
			return 1;
		}
	}
	
	return 0;
}

WORD ARBTextPreDone(LPTEXT pTextSrc, LPTEXT pTextBuffer, WORD bufferSize, LPUTFLOGFONT lpFont)
{
	WORD textLen = strlen((char *)pTextSrc);
	BYTE *ptrRead = pTextSrc;
	BYTE *ptrWrite = pTextBuffer;
	BYTE bArabicType = FALSE;
	WORD wTotalRead = 0;
	WORD wReadByte = 0;
	WORD realNum;
	BYTE bEnglishCode;

	if(textLen >= bufferSize)
		return 0;

	g_ptrCodeData = UTFGetTextTempPtr(&realNum);

	while(*pTextSrc)
	{
		bEnglishCode = FALSE;
		if(*pTextSrc < 0x80)
		{
			if(ARBFindCode(*pTextSrc))
			{
				bEnglishCode = TRUE;
			}
		}
		
		if(bEnglishCode)
		{
			if(bArabicType == FALSE)
			{
				*ptrWrite++ = *pTextSrc;
				wTotalRead++;
			}
			else
			{
				wReadByte++;
			}
		}
		else if(*pTextSrc < 0x80)
		{
			if((bArabicType == TRUE) && wReadByte)
			{
				InterCodeToFontCode( ptrRead, wReadByte, ptrWrite, &realNum, lpFont );
				wTotalRead += realNum;
				ptrWrite += realNum;
			}

			bArabicType = FALSE;
			*ptrWrite++ = *pTextSrc;
			wTotalRead++;
		}
		else
		{
			if(bArabicType == FALSE)
			{
				bArabicType = TRUE;
				ptrRead = pTextSrc;
				wReadByte = 0;
			}
			
			wReadByte++;
		}

		pTextSrc++;
	}

	if((bArabicType == TRUE) && wReadByte)
	{
		InterCodeToFontCode( ptrRead, wReadByte, ptrWrite, &realNum, lpFont );
		wTotalRead += realNum;
	}

	pTextBuffer[wTotalRead] = 0;

	return wTotalRead;
}

/*************************************************************************/
static char ARBHaveWidth(BYTE code)
{
	if(code >= 0xEF)
	{
		return 0;
	}

	return 1;
}

char ARBIsEnglishChar(BYTE code)
{
	if(code < 0x10)
		return 1;
	
	if((code > 0x1F) && (code < 0x80))
		return 1;

	return 0;
}

/*************************************************************************/
WORD ARBTextWidth(LPTEXT pText, int charInterval, LPUTFLOGFONT lpFont)
{
	WORD textLen = strlen((char *)pText);
	WORD i,textWidth = 0;
	BYTE charactor;
	BYTE *pWidth;

	if(lpFont->lfAddr1 == NULL)
		return 0;	

	pWidth = (BYTE *)(lpFont->lfAddr1+2);
	for( i=0; i<textLen; i++ )
	{
		charactor = pText[i];
		if(charactor == '\n')
			break;

		if(ARBHaveWidth(charactor))
		{
			textWidth += pWidth[charactor];
		}

		if(ARBIsEnglishChar(charactor))
		{
			textWidth += charInterval;
		}
	}

	return textWidth;
}

static void ARBTextFunc8(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
{
	WORD arabic_char_number;
	WORD xstep = 0;
	WORD i,j,k,col;
	WORD wWidthSize;
	WORD dx=0,dy=0;
	DWORD position;
	DWORD locate;
	DWORD maxByte;
	DWORD textSize = 0;
	DWORD dwSize;
	BYTE bDrawMode=1;
	BYTE character,data;
	BYTE *pWidth;
	BYTE *ptrRead;
	BYTE *ptrWrite;
	BYTE *ptrHeader;
	UTFBITMAP bitmap;

	if(lpFont->lfAddr1 == NULL)
		return;

	arabic_char_number = strlen((char *)pText);
	if(arabic_char_number <= 0)
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

	UTFMemCpyWORD(&wWidthSize, (void *)lpFont->lfAddr1, 2);
	pWidth = (BYTE *)(lpFont->lfAddr1+2);
	
	maxByte = pWidth[0]/8;
	if(pWidth[0]%8)
	{
		maxByte++;
	}

	for( i = 0; i < arabic_char_number; i++ )
	{
		character = pText[i];
		locate = character*lpFont->lfHeight*maxByte;
		ptrRead = pWidth+wWidthSize+locate;
		if((textSize+pWidth[character]) > width)
			break;

		ptrWrite = bitmap.data+dy*bitmap.Width+dx+xstep;
		for( j=0; j<height; j++ )
		{
			position = 0;
			for(k=0; k<maxByte; k++)
			{
				data = *ptrRead++;					
				for( col=0; col< 8; col++ )
				{
					if((position + col) >= pWidth[character])
						break;
						
					if(data & 0x80)
					{
						*(ptrWrite+position+col) = (BYTE)textColor;
					}

					data <<= 1;
				}
				position += 8;
			}
			ptrWrite += bitmap.Width;
		}

		if(ARBHaveWidth(character))
		{
			xstep += pWidth[character];
			textSize += pWidth[character];
		}

		if(ARBIsEnglishChar(character))
		{
			xstep += charInterval;
			textSize += charInterval;
		}
	}

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}

static void ARBTextFunc16(LPTEXT pText, WORD left, WORD top, WORD width, WORD height, WORD charInterval, UTFCOLOR textColor, LPUTFLOGFONT lpFont)
{
	WORD arabic_char_number;
	WORD xstep = 0;
	WORD i,j,k,col;
	WORD wWidthSize;
	WORD dx=0,dy=0;
	DWORD position;
	DWORD locate;
	DWORD maxByte;
	DWORD textSize = 0;
	DWORD dwSize;
	BYTE bDrawMode=1;
	BYTE character,data;
	BYTE *pWidth;
	BYTE *ptrRead;
	WORD *ptrWrite;
	BYTE *ptrHeader;
	UTFBITMAP bitmap;

	if(lpFont->lfAddr1 == NULL)
		return;

	arabic_char_number = strlen((char *)pText);
	if(arabic_char_number <= 0)
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

	UTFMemCpyWORD(&wWidthSize, (void *)lpFont->lfAddr1, 2);
	pWidth = (BYTE *)(lpFont->lfAddr1+2);
	
	maxByte = pWidth[0]/8;
	if(pWidth[0]%8)
	{
		maxByte++;
	}

	for( i = 0; i < arabic_char_number; i++ )
	{
		character = pText[i];
		locate = character*lpFont->lfHeight*maxByte;
		ptrRead = pWidth+wWidthSize+locate;
		if((textSize+pWidth[character]) > width)
			break;

		ptrWrite = (WORD *)bitmap.data;
		ptrWrite += dy*bitmap.Width+dx+xstep;
		for( j=0; j<height; j++ )
		{
			position = 0;
			for(k=0; k<maxByte; k++)
			{
				data = *ptrRead++;					
				for( col=0; col< 8; col++ )
				{
					if((position + col) >= pWidth[character])
						break;
						
					if(data & 0x80)
					{
						*(ptrWrite+position+col) = (WORD)textColor;
					}

					data <<= 1;
				}
				position += 8;
			}
			ptrWrite += bitmap.Width;
		}

		if(ARBHaveWidth(character))
		{
			xstep += pWidth[character];
			textSize += pWidth[character];
		}

		if(ARBIsEnglishChar(character))
		{
			xstep += charInterval;
			textSize += charInterval;
		}
	}

	if(bDrawMode)
	{
		UTFPutScreen(left, top, &bitmap, 0);
		SOWFree((void *)ptrHeader);
	}
}

void UTFArabicSetColorMode(BYTE bitPerPixel)
{
	if(bitPerPixel == 8)
	{
		ARBTextFunc = ARBTextFunc8;
	}
	if(bitPerPixel == 16)
	{
		ARBTextFunc = ARBTextFunc16;
	}
}

/*************************************************************************/
// ISO8859-6 Arabic font function code

#define DVB_ARABIC_TOTAL_CODE		25

//////////// standard dvb ara table for unicode to ansi code
static BYTE CodeMatchTable[DVB_ARABIC_TOTAL_CODE<<1] = 
{
	0xAC,0xA1, 0xBB,0xBA, 0xD7,0xD8, 0xD8,0xD9, 0xD9,0xDA,
	0xDA,0xDB, 0xE0,0xDC, 0xE1,0xDD, 0xE2,0xDE, 0xE3,0xDF,
	0xE4,0xE1, 0xE5,0xE3, 0xE6,0xE4, 0xE7,0xE5, 0xE8,0xE6,
	0xE9,0xEC, 0xEA,0xED, 0xEB,0xF0, 0xEC,0xF1, 0xED,0xF2,
	0xEE,0xF3, 0xEF,0xF5, 0xF0,0xF6, 0xF1,0xF8, 0xF2,0xFA
};

/*************************************************************************
** This function will return the windows code same as ISO8859-6 character
*************************************************************************/
int DVBArabicToWinArabic(BYTE cCode)
{
	WORD i;
	
	for(i=0; i<DVB_ARABIC_TOTAL_CODE; i++)
	{	
		if(cCode == CodeMatchTable[i*2])
		{
			return CodeMatchTable[i*2+1];
		}
	}

	return (-1);
}

