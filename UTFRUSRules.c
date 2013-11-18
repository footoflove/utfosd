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
#include "UTFRUSRules.h"

/**************************************************************************/
#define TOTAL_CODE		66

//////////// standard russian table for unicode to ansi code
static WORD UnicodeTable[TOTAL_CODE*2] = 
{
	0xA7F1,0xFF, 0xA7F0,0xFE, 0xA7EF,0xFD, 0xA7EE,0xFC,
	0xA7ED,0xFB, 0xA7EC,0xFA, 0xA7EB,0xF9, 0xA7EA,0xF8,
	0xA7E9,0xF7, 0xA7E8,0xF6, 0xA7E7,0xF5, 0xA7E6,0xF4,
	0xA7E5,0xF3, 0xA7E4,0xF2, 0xA7E3,0xF1, 0xA7E2,0xF0,
	0xA7E1,0xEF, 0xA7E0,0xEE, 0xA7DF,0xED, 0xA7DE,0xEC,
	0xA7DD,0xEB, 0xA7DC,0xEA, 0xA7DB,0xE9, 0xA7DA,0xE8,
	0xA7D9,0xE7, 0xA7D8,0xE6, 0xA7D6,0xE5, 0xA7D5,0xE4,
	0xA7D4,0xE3, 0xA7D3,0xE2, 0xA7D2,0xE1, 0xA7D1,0xE0,
	0xA7C1,0xDF, 0xA7C0,0xDE, 0xA7BF,0xDD, 0xA7BE,0xDC,
	0xA7BD,0xDB, 0xA7BC,0xDA, 0xA7BB,0xD9, 0xA7BA,0xD8,
	0xA7B9,0xD7, 0xA7B8,0xD6, 0xA7B7,0xD5, 0xA7B6,0xD4,
	0xA7B5,0xD3, 0xA7B4,0xD2, 0xA7B3,0xD1, 0xA7B2,0xD0,
	0xA7B1,0xCF, 0xA7B0,0xCE, 0xA7AF,0xCD, 0xA7AE,0xCC,
	0xA7AD,0xCB, 0xA7AC,0xCA, 0xA7AB,0xC9, 0xA7AA,0xC8,
	0xA7A9,0xC7, 0xA7A8,0xC6, 0xA7A6,0xC5, 0xA7A5,0xC4,
	0xA7A4,0xC3, 0xA7A3,0xC2, 0xA7A2,0xC1, 0xA7A1,0xC0,
	0xA7D7,0xB8, 0xA7A7,0xA8
};

/*************************************************************************
** This function will return the ansi code same as unicode data character
*************************************************************************/
static int RUSUnicodeToANSI(WORD wUnicode)
{
	WORD i;
	
	for(i=0; i<TOTAL_CODE; i++)
	{	
		if(wUnicode == UnicodeTable[i*2])
		{
			return UnicodeTable[i*2+1];
		}
	}
	
	return (-1);
}
/**************************************************************************/

WORD RUSTextUnicodeToANSI(LPTEXT pTextSrc, LPTEXT pTextBuffer, WORD bufferSize, LPUTFLOGFONT lpFont)
{
	WORD wRealNum = 0;
	WORD wUnicode;
	int iANSICode;

	while(*pTextSrc)
	{
		if(*pTextSrc == 0xA7)
		{
			if(*(pTextSrc+1))
			{
				wUnicode = *pTextSrc;
				wUnicode <<= 8;
				wUnicode += *(pTextSrc+1);
				
				iANSICode = RUSUnicodeToANSI(wUnicode);
				if(iANSICode > 0)
				{
					*pTextBuffer++ = (BYTE)iANSICode;
					wRealNum++;
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
			*pTextBuffer++ = *pTextSrc++;
			wRealNum++;
		}		
	}

	return wRealNum;
}

