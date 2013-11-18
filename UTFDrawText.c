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
#include "UTFDrawAPI.h"
#include "UTFDrawText.h"
#include "UTFDrawTextPriv.h"
#include "UTFEnglishRules.h"
#include "UTFArabicRules.h"
#include "UTFCHNRules.h"
#include "UTFBIG5Rules.h"
#include "UTFRUSRules.h"
#include "UTFHindiRules.h"
#include "UTFTamilRules.h"
#include "UTFMalayalamRules.h"
#include "UTFKannadaRules.h"
#include "UTFTHAIRules.h"

#define TEXT_BUFFER_SIZE	5120
#define TEMP_BUFFER_SIZE	(TEXT_BUFFER_SIZE>>1)

typedef struct
{
	TEXTPROC m_DrawOneLineText;
	TEXTWIDTH m_GetTextWidth;
	TEXTPREDONE m_TextPreDone;
	DRAWTEXT m_DrawText;
	DECODETEXT m_DecodeText;
	ADDENDLLIPSE m_AddEndllipse;
	COPYDATAFORPREDONE m_CopyToPreDone;
}TEXTFUNC;

static TEXTFUNC g_TextFunc = {0};
static UTFLOGFONT g_TextFont;
static UTFCOLOR g_TextColor,g_TextColorCode;
static BYTE g_iCharInterval = 1,g_bRowSpace = 2;
static BYTE g_TextFinalData[TEXT_BUFFER_SIZE+150];
static BYTE g_TextBuffer[TEXT_BUFFER_SIZE];
static BYTE g_TempBuffer[TEXT_BUFFER_SIZE/2];
static BYTE g_TempCopyData[TEXT_BUFFER_SIZE/2];
/************************************************************************/

BYTE *UTFGetTextTempPtr(WORD *lpTempSize)
{
	*lpTempSize = TEMP_BUFFER_SIZE;
	return g_TempBuffer;
}
/************************************************************************/

void UTFAPI UTFSetTextColor(UTFCOLOR textColor)
{
	g_TextColorCode = textColor;
	g_TextColor = UTFFillPaletteFunc(textColor, UTFGetColorIndex());
}

UTFCOLOR UTFAPI UTFGetTextColor(void)
{
	return g_TextColorCode;
}

void UTFAPI UTFSetTextInterval(BYTE interval)
{
	g_iCharInterval = interval;
}

void UTFAPI UTFSetTextRowSpace(BYTE space)
{
	g_bRowSpace = space;
}

BYTE UTFAPI UTFGetTextInterval(void)
{
	return g_iCharInterval;
}

BYTE UTFAPI UTFGetTextRowSpace(void)
{
	return g_bRowSpace;
}

void UTFAPI UTFDrawText(LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat)
{
	if(g_TextFunc.m_DrawText)
	{
		UTFNormalizeRect(lpRect);
		g_TextFunc.m_DrawText(lpText, lpRect, uFormat);
	}
//	DG_draw_text(lpText,0xc0c0,lpRect->left,lpRect->top);
}

static void UTFCopyPreDoneData(LPTEXT lpStart, LPTEXT lpCurPtr)
{
	if(g_TextFunc.m_CopyToPreDone)
	{
		g_TextFunc.m_CopyToPreDone(lpStart, lpCurPtr);
	}
}

static WORD UTFTextCopy(LPTEXT lpText, BYTE *pBuffer, WORD bufferSize, DWORD uFormat)
{
	WORD wReadBytes = 0;
	
	if(g_TextFunc.m_TextPreDone)
	{
		BYTE *ptrRead = lpText;
		BYTE *ptrWrite = pBuffer;
		WORD wStrBytes = 0;
		WORD wWriteBytes = 0;
		WORD wTotalBuffer = bufferSize;
		BYTE *lpStart = lpText;

		while(*lpText)
		{
			if(*lpText == '\n')
			{
				if(wReadBytes)
				{
					*lpText = 0;
					UTFCopyPreDoneData(lpStart, ptrRead);
					wStrBytes = g_TextFunc.m_TextPreDone(g_TempCopyData, ptrWrite, wTotalBuffer, &g_TextFont);
					wTotalBuffer -= wStrBytes+1;
					wWriteBytes += wStrBytes+1;
					ptrWrite += wStrBytes;
					if((uFormat & DTUI_NO_RETURN) && (uFormat & DTUI_SINGLELINE))
						*ptrWrite++ = ' ';
					else
						*ptrWrite++ = '\n';
					
					*lpText = '\n';
					ptrRead = lpText+1;
					wReadBytes = 0;
				}
				else
				{
					if((uFormat & DTUI_NO_RETURN) && (uFormat & DTUI_SINGLELINE))
						*ptrWrite++ = ' ';
					else	
						*ptrWrite++ = *ptrRead;

					ptrRead++;
					wWriteBytes++;
				}
			}
			else
			{
				wReadBytes++;
			}
			
			lpText++;
		}

		if(wReadBytes)
		{
			UTFCopyPreDoneData(lpStart, ptrRead);
			wStrBytes = g_TextFunc.m_TextPreDone(g_TempCopyData, ptrWrite, wTotalBuffer, &g_TextFont);
			*(ptrWrite+wStrBytes) = 0;
			wWriteBytes += wStrBytes;
		}
		
		wReadBytes = wWriteBytes;
		pBuffer[wReadBytes] = 0;
	}
	else
	{
		while(*lpText)
		{
			if(*lpText == '\n')
			{
				if((uFormat & DTUI_NO_RETURN) && (uFormat & DTUI_SINGLELINE))
					pBuffer[wReadBytes++] = ' ';
				else
					pBuffer[wReadBytes++] = *lpText;
			}
			else
			{
				pBuffer[wReadBytes++] = *lpText;
			}
			lpText++;
			
			if(wReadBytes >= (bufferSize-1))
				break;
		}
		pBuffer[wReadBytes] = 0;
	}

	return wReadBytes;
}

static WORD UTFTextGetWidthEx(LPTEXT lpText)
{
	if(g_TextFunc.m_GetTextWidth == NULL)
		return 0;

	return g_TextFunc.m_GetTextWidth(lpText, g_iCharInterval, &g_TextFont);
}

WORD UTFAPI UTFGetTextWidth(LPTEXT lpText)
{
	WORD totalLine = 1;
	WORD Width[40] = {0};
	int i, whichLine = 0;
	WORD iWidth = 0;
	WORD wReadBytes;
	BYTE *ptr = g_TextBuffer;
	
	if(g_TextFunc.m_GetTextWidth == NULL)
		return 0;

	wReadBytes = UTFTextCopy(lpText, g_TextBuffer, TEXT_BUFFER_SIZE, 0);
	for(i=0; i<TEXT_BUFFER_SIZE; i++)
	{
		if(g_TextBuffer[i] == 0)
		{
			Width[whichLine] = UTFTextGetWidthEx(ptr);
			break;
		}
		
		if(g_TextBuffer[i] == '\n')
		{
			totalLine++;
			g_TextBuffer[i] = 0;
			Width[whichLine++] = UTFTextGetWidthEx(ptr);
			g_TextBuffer[i] = '\n';
			ptr = &g_TextBuffer[i+1];
		}
	}

	for(i=0; i<totalLine; i++)
	{
		if(iWidth < Width[i])
		{
			iWidth = Width[i];
		}
	}
	
	return iWidth;
}

static WORD UTFGetCharWidth(LPTEXT lpText)
{
	return g_TextFunc.m_GetTextWidth(lpText, 0, &g_TextFont);
}

/***********************************************************************/
WORD UTFAPI UTFDecodeTextInRect(LPTEXT lpTxtSrc, LPTEXT lpBuffer, DWORD dwBufferSize, LPUTFRECT lpRect, DWORD uFormat)
{
	if(g_TextFunc.m_DecodeText)
	{
		DWORD size = sizeof(g_TextFinalData);
		WORD wTotal;

		memset(g_TextFinalData, 0, size);

		UTFNormalizeRect(lpRect);
		wTotal = g_TextFunc.m_DecodeText(lpTxtSrc, lpRect, uFormat);

		if((lpBuffer != NULL) && (dwBufferSize > 0))
		{
			if(size <= dwBufferSize)
			{
				memcpy(lpBuffer, g_TextFinalData, size);
			}
			else
			{
				memcpy(lpBuffer, g_TextFinalData, dwBufferSize-1);
				lpBuffer[dwBufferSize-1] = 0;
			}
		}

		return wTotal;
	}

	return 0;
}

int UTFAPI UTFGetDecodedRowText(LPTEXT lpTxtSrc, DWORD dwSrcSize, LPTEXT lpBuffer, DWORD dwBufferSize, WORD rowId)
{
	BYTE *ptr = lpTxtSrc;
	WORD wWhichRow = 0;
	DWORD i;

	if((lpBuffer == NULL) || (dwBufferSize == 0))
	{
		return 0;
	}

	if((lpTxtSrc == NULL) || (dwSrcSize == 0))
	{
		memset(lpBuffer, 0, dwBufferSize);
		return 0;
	}

	if(lpTxtSrc[0] == 0)
	{
		memset(lpBuffer, 0, dwBufferSize);
		return 0;
	}

	for(i=0; i<dwSrcSize; i++)
	{
		if(*(lpTxtSrc+i))
		{
			//If ptr are point to zero data, point to first none zero data
			if(*ptr == 0)
			{
				ptr = lpTxtSrc+i;
			}
		}
		else
		{
			//If find current rowid, copy data to buffer and return
			if(wWhichRow == rowId)
			{
				WORD wStrlen = strlen((char *)ptr);

				if(wStrlen == 0)
				{
					memset(lpBuffer, 0, dwBufferSize);
				}
				else if(wStrlen < dwBufferSize)
				{
					strcpy((char *)lpBuffer, (char *)ptr);
				}
				else
				{
					memcpy(lpBuffer, ptr, dwBufferSize-1);
					lpBuffer[dwBufferSize-1] = 0;
				}
				return 1;
			}

			ptr = lpTxtSrc+i;
			wWhichRow++;
		}
	}

	//If not find current rowid, clear buffer data to zero
	memset(lpBuffer, 0, dwBufferSize);

	return 0;
}

/***********************************************************************/
static BYTE UTFCharCanbePlaceFirst(BYTE character)
{
	char symbol[] = {',', ';', '?', '!'};
	WORD i;
	
	for(i=0; i<4; i++)
	{
		if(character == symbol[i])
		{
			return 0;
		}
	}

	return 1;
}

/***********************************************************************/
static void UTFAddEndllipse(LPTEXT lpText, WORD width, BYTE bFlag)
{
	if(g_TextFunc.m_AddEndllipse)
	{
		g_TextFunc.m_AddEndllipse(lpText, width, bFlag);
	}
}

static void UTFDrawTextInter(LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat)
{
	WORD left, top;
	WORD width, height;
	WORD i, textWidth = 0;
	UTFRECT rcScreen;
	WORD totalRow;
	BYTE endllipse[4] = {'.','.','.',0};

	if(uFormat & DTUI_NODRAW)
		return;

	width = lpRect->right - lpRect->left;
	height = lpRect->bottom - lpRect->top;
	
	if((g_TextFunc.m_DrawOneLineText == NULL) || (lpText == NULL))
	{
		return;
	}

	if(uFormat & DTUI_NOT_CALCRECT)
	{
		totalRow = 1;
		strcpy((char *)g_TextFinalData, (char *)lpText);
	}
	else
	{
		totalRow = UTFDecodeTextInRect(lpText, NULL, 0, lpRect, uFormat);
	}

	UTFGetOSDRgnRect(&rcScreen);
#if(RUN_PC == TRUE)
	if(!UTFGetScreenDraw())
	{
		UTFOffsetRect(&rcScreen, -rcScreen.left, -rcScreen.top);
	}
#else
	UTFOffsetRect(&rcScreen, -rcScreen.left, -rcScreen.top);
#endif
	
	if(uFormat & DTUI_SINGLELINE)
	{
		textWidth = UTFTextGetWidthEx(g_TextFinalData);

		if(uFormat & DTUI_CENTER)
			left = lpRect->left + ((width - textWidth)>>1);
		else if(uFormat & DTUI_RIGHT)
		{
			if(width > textWidth)
				left = lpRect->right-textWidth-1;
			else
				left = lpRect->left;
		}
		else
			left = lpRect->left;
		if(left < lpRect->left)
		{
			left = lpRect->left;
		}
		left += rcScreen.left;
		
		if(uFormat & DTUI_VCENTER)
			top = (WORD)(lpRect->top + ((height - g_TextFont.lfHeight)>>1));
		else if(uFormat & DTUI_BOTTOM)
			top = (WORD)(lpRect->bottom - g_TextFont.lfHeight);
		else
			top = lpRect->top;
		if(top < lpRect->top)
		{
			top = lpRect->top;
		}
		top += rcScreen.top;

		g_TextFunc.m_DrawOneLineText(g_TextFinalData, left, top, textWidth, (WORD)g_TextFont.lfHeight, g_iCharInterval, g_TextColor, &g_TextFont);
	}
	else
	{
		DWORD size;
		WORD totalHeight;
		WORD charWidth;
		BYTE bFlag = 0;
		BYTE bFlag1 = 0;
		BYTE *ptr;

		//get end character width of "..."
		charWidth = UTFTextGetWidthEx(endllipse);

		//calculate how many line text can be display in rectangle
		totalHeight = (WORD)(totalRow*g_TextFont.lfHeight);
		if(totalRow > 1)
		{
			totalHeight += (totalRow-1)*g_bRowSpace;
		}
		
		if(totalRow == 1)
		{
			textWidth = UTFTextGetWidthEx(g_TextFinalData);
			if(textWidth > width)
			{
				bFlag1 = 1;
			}
		}
		else if(totalHeight > height)
		{
			while(totalRow > 1)
			{
				bFlag1 = 1;
				totalHeight -= g_TextFont.lfHeight+g_bRowSpace;
				totalRow--;
				if(totalHeight <= height)
					break;
			}
		}

		if(uFormat & DTUI_VCENTER)
			top = lpRect->top + ((height - totalHeight)>>1);
		else if(uFormat & DTUI_BOTTOM)
			top = lpRect->bottom - totalHeight;
		else
			top = lpRect->top;
		if(top < lpRect->top)
		{
			top = lpRect->top;
		}

		//start display every line on screen
		ptr = g_TextFinalData;
		size = sizeof(g_TextFinalData);
		for(i=0; i<size; i++)
		{
			if( !g_TextFinalData[i] )
			{
				totalRow--;
				bFlag = 0;
				
				textWidth = UTFTextGetWidthEx(ptr);
				if((textWidth+charWidth) > width)
				{
					bFlag = 1;
				}
				
				if(uFormat & DTUI_END_ELLIPSIS)
				{
					if((totalRow == 0) && bFlag1)
					{
						textWidth += charWidth;
					}
				}
				if(textWidth > width)
				{
					textWidth = width;
				}
				
				if(uFormat & DTUI_CENTER)
					left = lpRect->left + ((width - textWidth)>>1);
				else if(uFormat & DTUI_RIGHT)
				{
					if(width > textWidth)
						left = lpRect->right-textWidth-1;
					else
						left = lpRect->left;
				}
				else
					left = lpRect->left;
				left += rcScreen.left;
				
				if(uFormat & DTUI_END_ELLIPSIS)
				{
					// add end ellipsis character
					if((totalRow == 0) && bFlag1)
					{
						UTFAddEndllipse(ptr, width, bFlag);
					}
				}

				g_TextFunc.m_DrawOneLineText(ptr, left, top+rcScreen.top, textWidth, lpRect->bottom-top, g_iCharInterval, g_TextColor, &g_TextFont);

				//if text tail come arrive, break the display
				if(totalRow == 0)
				{
					break;
				}

				ptr = &g_TextFinalData[i+1];
				top += g_TextFont.lfHeight+g_bRowSpace;
			}
		}
	}
}

/***********************************************************************/
static void UTFENGCopyPreDoneData(LPTEXT lpStart, LPTEXT lpCurPtr)
{
	int iStrlen = strlen((char *)lpCurPtr);

	if(iStrlen > TEMP_BUFFER_SIZE)
	{
		memcpy(g_TempCopyData, lpCurPtr, TEMP_BUFFER_SIZE);
		g_TempCopyData[TEMP_BUFFER_SIZE-1] = 0;
	}
	else
	{
		strcpy((char *)g_TempCopyData, (char *)lpCurPtr);
	}
}

static void UTFENGEndllipse(LPTEXT lpText, WORD width, BYTE bFlag)
{
	WORD wStrLen;

	if(lpText == NULL)
		return;

	wStrLen = strlen((char *)lpText);
	if(wStrLen == 0)
		return;

	if(bFlag)
	{
		BYTE endllipse[4] = {'.','.','.',0};
		WORD wWidth;
		WORD wWD = 0;
		WORD charWD = 0;
		BYTE *ptr = lpText;
		BYTE c[2] = {0};
		int i;
	
		wWidth = UTFTextGetWidthEx(endllipse);
		wWD = UTFTextGetWidthEx(lpText);
		
		for(i=(wStrLen-1); i>=0; i--)
		{
			c[0] = lpText[i];
			charWD = UTFGetCharWidth(c);
			wWD -= charWD;
			
			if((wWD+wWidth) <= width)
			{
				ptr += i;
				break;
			}
			wWD -= g_iCharInterval;
		}

		strcpy((char *)ptr, "...");
	}
	else
	{
		strcat((char *)lpText, "...");
	}
}

static WORD UTFENGDecodeText(LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat)
{
	WORD width, height;
	WORD i, textWidth = 0;
	WORD wReadBytes;
	WORD charWidth;
	WORD wStrlen;
	WORD wTotal = 0;
	BYTE c[2] = {0, 0};

	width = lpRect->right - lpRect->left;
	height = lpRect->bottom - lpRect->top;
	
	wReadBytes = UTFTextCopy(lpText, g_TextBuffer, TEXT_BUFFER_SIZE, uFormat);

	if(uFormat & DTUI_SINGLELINE)
	{
		textWidth = UTFTextGetWidthEx(g_TextBuffer);
		
		if(textWidth <= width)
		{
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}
				
				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}			
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}
		else
		{
			WORD iWidth = 0;

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				BYTE endllipse[4] = {'.','.','.',0};
				
				iWidth = UTFTextGetWidthEx(endllipse);
			}
			
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}
				
				c[0] = g_TextBuffer[i];
				charWidth = UTFGetCharWidth(c);
				if(uFormat & DTUI_END_ELLIPSIS)
				{
					charWidth += g_iCharInterval;
				}
				
				if((iWidth+charWidth) <= width)
				{
					iWidth += charWidth;
					if(!(uFormat & DTUI_END_ELLIPSIS))
					{
						iWidth += g_iCharInterval;
					}
				}
				else
				{
					break;
				}

				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}

		return 1;
	}
	else
	{
		BYTE *ptr = g_TextBuffer;
		WORD totalRow = 1;
		WORD iWidth = 0;
		WORD totalByte = wReadBytes+1;
		WORD wordWidth;
		
		for(i=0; i<totalByte; i++)
		{
			if( !g_TextBuffer[i] )
			{
				break;
			}

			if(g_TextBuffer[i] == '\n')
			{
				totalRow++;
				iWidth = 0;

				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				g_TextBuffer[i] = '\n';

				memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
				wTotal += wStrlen;
				g_TextFinalData[wTotal++] = 0;

				ptr = &g_TextBuffer[i+1];
			}
			else if(g_TextBuffer[i] == ' ')
			{
				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i] = ' ';
				
				// If text length large than window width, cut text 
				if((iWidth+wordWidth) > width)
				{
					// If any space in text, make current word in new line
					totalRow++;
					iWidth = wordWidth+g_iCharInterval;
					
					g_TextFinalData[wTotal++] = 0;
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
						
					ptr = &g_TextBuffer[i];
				}
				else
				{
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
					iWidth += wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}
				}
			}
			else
			{
				WORD oldI = i;
				BYTE data = g_TextBuffer[i+1];
				
				g_TextBuffer[i+1] = 0;
				wStrlen = strlen((char *)ptr);
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i+1] = data;
				if((iWidth+wordWidth) > width)
				{
					WORD j,WD = width+1;

					if(iWidth)
					{
						for(j=i+1; j<totalByte; j++)
						{
							if((g_TextBuffer[j] == 0) || (g_TextBuffer[j] == ' ') || (g_TextBuffer[j] == '\n'))
							{
								data = g_TextBuffer[j];
								g_TextBuffer[j] = 0;
								WD = UTFTextGetWidthEx(ptr);
								g_TextBuffer[j] = data;
								break;
							}
						}
					}

					if(WD > width)
					{
						if(ptr == &g_TextBuffer[oldI])
						{
							i++;
							
							data = g_TextBuffer[i];
							g_TextBuffer[i] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[i] = data;
						}
						else
						{
							i = oldI;
							wStrlen--;
						}

						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						g_TextFinalData[wTotal++] = 0;
						
						ptr = &g_TextBuffer[i];
					}
					else
					{
						i = j-1;
						g_TextFinalData[wTotal++] = 0;
						ptr++;
					}

					iWidth = 0;
					totalRow++;
				}
			}
		}

		wStrlen = strlen((char *)ptr);
		if(wStrlen)
		{
			memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
			wTotal += wStrlen;
		}
		
		g_TextFinalData[wTotal] = 0;
		g_TextFinalData[wTotal+1] = 0;

		return totalRow;
	}

	return 0;
}

/***********************************************************************/
static void UTFARBEndllipse(LPTEXT lpText, WORD width, BYTE bFlag)
{
	WORD wStrLen;

	if(lpText == NULL)
		return;

	wStrLen = strlen((char *)lpText);
	if(wStrLen == 0)
		return;

	if(bFlag)
	{
		BYTE endllipse[4] = {'.','.','.',0};
		WORD wWidth;
		WORD wWD = 0;
		WORD charWD = 0;
		BYTE *ptr = lpText;
		BYTE c[2] = {0};
		int i;
	
		wWidth = UTFTextGetWidthEx(endllipse);
		for(i=0; i<wStrLen; i++)
		{
			c[0] = lpText[i];
			charWD = UTFGetCharWidth(c);
			wWD += charWD;
			if(ARBIsEnglishChar(lpText[i]))
			{
				wWD += g_iCharInterval;
			}
			
			if((wWD+wWidth) > width)
			{
				ptr += i;
				break;
			}
		}

		strcpy((char *)ptr, "...");
	}
	else
	{
		strcat((char *)lpText, "...");
	}
}

static WORD UTFARBDecodeText(LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat)
{
	WORD width, height;
	WORD i, textWidth = 0;
	WORD wReadBytes;
	WORD charWidth;
	WORD wStrlen;
	WORD wTotal = 0;
	BYTE c[2] = {0, 0};

	width = lpRect->right - lpRect->left;
	height = lpRect->bottom - lpRect->top;
	
	wReadBytes = UTFTextCopy(lpText, g_TextBuffer, TEXT_BUFFER_SIZE, uFormat);
	if(uFormat & DTUI_SINGLELINE)
	{
		textWidth = UTFTextGetWidthEx(g_TextBuffer);

		if(textWidth <= width)
		{
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}
				
				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}			
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}
		else
		{
			WORD iWidth = 0;

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				BYTE endllipse[4] = {'.','.','.',0};
				
				iWidth = UTFTextGetWidthEx(endllipse);
			}
			
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}
				
				c[0] = g_TextBuffer[i];
				charWidth = UTFGetCharWidth(c);
				if(uFormat & DTUI_END_ELLIPSIS)
				{
					if(ARBIsEnglishChar(g_TextBuffer[i]))
					{
						charWidth += g_iCharInterval;
					}
				}
				
				if((iWidth+charWidth) <= width)
				{
					iWidth += charWidth;
					if(!(uFormat & DTUI_END_ELLIPSIS))
					{
						if(ARBIsEnglishChar(g_TextBuffer[i]))
						{
							iWidth += g_iCharInterval;
						}
					}
				}
				else
				{
					break;
				}

				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}

		return 1;
	}
	else
	{
		BYTE *ptr = g_TextBuffer;
		WORD totalRow = 1;
		WORD iWidth = 0;
		WORD totalByte = wReadBytes+1;
		WORD wordWidth;
		
		for(i=0; i<totalByte; i++)
		{
			if( !g_TextBuffer[i] )
			{
				break;
			}

			if(g_TextBuffer[i] == '\n')
			{
				totalRow++;
				iWidth = 0;

				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				g_TextBuffer[i] = '\n';

				memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
				wTotal += wStrlen;
				g_TextFinalData[wTotal++] = 0;

				ptr = &g_TextBuffer[i+1];
			}
			else if(g_TextBuffer[i] == ' ')
			{
				BYTE data = g_TextBuffer[i+1];

				g_TextBuffer[i+1] = 0;
				wStrlen = strlen((char *)ptr);
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i+1] = data;
				
				// If text length large than window width, cut text 
				if((iWidth+wordWidth) > width)
				{
					// If any space in text, make current word in new line
					totalRow++;
					iWidth = wordWidth;
					
					g_TextFinalData[wTotal++] = 0;
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
						
					ptr = &g_TextBuffer[i+1];
				}
				else
				{
					iWidth += wordWidth;
					
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i+1];					
				}
			}
			else
			{
				WORD oldI = i;
				BYTE data = g_TextBuffer[i+1];
				
				g_TextBuffer[i+1] = 0;
				wStrlen = strlen((char *)ptr);
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i+1] = data;
				if((iWidth+wordWidth) > width)
				{
					WORD j,WD = width+1;

					if(iWidth)
					{
						for(j=i+1; j<totalByte; j++)
						{
							if((g_TextBuffer[j] == 0) || (g_TextBuffer[j] == ' ') || (g_TextBuffer[j] == '\n'))
							{
								data = g_TextBuffer[j];
								g_TextBuffer[j] = 0;
								WD = UTFTextGetWidthEx(ptr);
								g_TextBuffer[j] = data;
								break;
							}
						}
					}

					if(WD > width)
					{
						if(ptr == &g_TextBuffer[oldI])
						{
							i++;
							
							data = g_TextBuffer[i];
							g_TextBuffer[i] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[i] = data;
						}
						else
						{
							i = oldI;
							wStrlen--;
						}

						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						g_TextFinalData[wTotal++] = 0;
						
						ptr = &g_TextBuffer[i];
					}
					else
					{
						i = j-1;
						g_TextFinalData[wTotal++] = 0;
					}

					iWidth = 0;
					totalRow++;
				}
			}
		}

		wStrlen = strlen((char *)ptr);
		if(wStrlen)
		{
			memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
			wTotal += wStrlen;
		}
		
		g_TextFinalData[wTotal] = 0;
		g_TextFinalData[wTotal+1] = 0;

		return totalRow;
	}

	return 0;
}

/***********************************************************************/
static void UTFCHNEndllipse(LPTEXT lpText, WORD width, BYTE bFlag)
{
	WORD wStrLen;

	if(lpText == NULL)
		return;

	wStrLen = strlen((char *)lpText);
	if(wStrLen == 0)
		return;

	if(bFlag)
	{
		BYTE endllipse[4] = {'.','.','.',0};
		WORD wWidth;
		WORD wWD = 0;
		WORD charWD = 0;
		BYTE *ptr = lpText;
		BYTE c[2] = {0};
		int i;
	
		wWidth = UTFTextGetWidthEx(endllipse);
		wWD = UTFTextGetWidthEx(lpText);
		
		for(i=(wStrLen-1); i>=0; i--)
		{
			if(lpText[i] > CHN_BYTE)
			{
				i--;
				charWD = (WORD)g_TextFont.lfWidth;
			}
			else
			{
				c[0] = lpText[i];
				charWD = UTFGetCharWidth(c);
			}
			wWD -= charWD;
			
			if((wWD+wWidth) <= width)
			{
				ptr += i;
				break;
			}
			wWD -= g_iCharInterval;
		}

		strcpy((char *)ptr, "...");
	}
	else
	{
		strcat((char *)lpText, "...");
	}
}

static WORD UTFCHNDecodeText(LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat)
{
	WORD width, height;
	WORD i, textWidth = 0;
	WORD wReadBytes;
	WORD charWidth;
	WORD wStrlen;
	WORD wTotal = 0;
	BYTE bOldByte;
	BYTE c[3] = {0, 0, 0};

	width = lpRect->right - lpRect->left;
	height = lpRect->bottom - lpRect->top;
	
	wReadBytes = UTFTextCopy(lpText, g_TextBuffer, TEXT_BUFFER_SIZE, uFormat);
	
	if(uFormat & DTUI_SINGLELINE)
	{
		textWidth = UTFTextGetWidthEx(g_TextBuffer);
		
		if(textWidth <= width)
		{
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}
		else
		{
			WORD iWidth = 0;

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				BYTE endllipse[4] = {'.','.','.',0};
				
				iWidth = UTFTextGetWidthEx(endllipse);
			}
			
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}
				
				bOldByte = g_TextBuffer[i];
				
				if(bOldByte > CHN_BYTE)
				{
					if(g_TextBuffer[i+1] > CHN_BYTE)
					{
						i++;
						charWidth = (WORD)g_TextFont.lfWidth;
					}
					else if(g_TextBuffer[i+1])
					{
						continue;
					}
					else
					{
						break;
					}
				}
				else
				{
					c[0] = g_TextBuffer[i];
					c[1] = 0;
					charWidth = UTFGetCharWidth(c);
				}
				if(uFormat & DTUI_END_ELLIPSIS)
				{
					charWidth += g_iCharInterval;
				}
				
				if((iWidth+charWidth) <= width)
				{
					iWidth += charWidth;
					if(!(uFormat & DTUI_END_ELLIPSIS))
					{
						iWidth += g_iCharInterval;
					}
				}
				else
				{
					break;
				}

				if(bOldByte > CHN_BYTE)
				{
					g_TextFinalData[wTotal++] = bOldByte;
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
				}
				else
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
				}
			}

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}

		return 1;
	}
	else
	{
		BYTE *ptr = g_TextBuffer;
		WORD totalRow = 1;
		WORD iWidth = 0;
		WORD totalByte = wReadBytes+1;
		WORD wordWidth;
		BYTE bPrevHaveCHN = FALSE;
		WORD newI = 0;
		
		for(i=0; i<totalByte; i++)
		{
			if( !g_TextBuffer[i] )
			{
				break;
			}
			
			if(g_TextBuffer[i] == '\n')
			{
				totalRow++;
				iWidth = 0;
				bPrevHaveCHN = FALSE;

				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				g_TextBuffer[i] = '\n';

				memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
				wTotal += wStrlen;
				g_TextFinalData[wTotal++] = 0;
				
				ptr = &g_TextBuffer[i+1];
			}
			else if(g_TextBuffer[i] == ' ')
			{
				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i] = ' ';

				// If text length large than window width, cut text 
				if((iWidth+wordWidth) > width)
				{
					// If any space in text, make current word in new line
					totalRow++;
					iWidth = wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}

					g_TextFinalData[wTotal++] = 0;
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
					if(g_TextBuffer[i+1] > CHN_BYTE)
					{
						c[0] = g_TextBuffer[i];
						c[1] = 0;
						iWidth += UTFGetCharWidth(c)+g_iCharInterval;

						g_TextFinalData[wTotal++] = *ptr++;
					}

					bPrevHaveCHN = FALSE;
				}
				else
				{
					iWidth += wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}
					
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;

					ptr = &g_TextBuffer[i];
					if(g_TextBuffer[i+1] > CHN_BYTE)
					{
						c[0] = g_TextBuffer[i];
						c[1] = 0;
						iWidth += UTFGetCharWidth(c)+g_iCharInterval;

						g_TextFinalData[wTotal++] = *ptr++;
					}
				}
			}
			else
			{
				WORD oldI = i;
				BYTE data;

				bOldByte = g_TextBuffer[i];
				if(bOldByte > CHN_BYTE)
				{
					if(g_TextBuffer[i+1] > CHN_BYTE)
					{
						wordWidth = (WORD)g_TextFont.lfWidth;
						bPrevHaveCHN = TRUE;
					}
					else if(g_TextBuffer[i+1])
					{
						continue;
					}
					else
					{
						break;
					}
				}
				else
				{
					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wordWidth = UTFTextGetWidthEx(ptr);
					g_TextBuffer[i+1] = data;
				}

				if((iWidth+wordWidth) > width)
				{
					WORD j,WD = width+1;
					BYTE bFound = 0;

					if(iWidth)//If any text had been copyed
					{
						WORD wStart = i+1;

						if(bOldByte > CHN_BYTE)
						{
							wStart++;
						}

						//find current string for it tail
						for(j=wStart; j<totalByte; j++)
						{
							if((g_TextBuffer[j] == 0) || (g_TextBuffer[j] == ' ') || (g_TextBuffer[j] == '\n'))
							{
								bFound = 1;
							}
							else if(bOldByte > CHN_BYTE)
							{
							}
							else if(g_TextBuffer[j] > CHN_BYTE)
							{
								bFound = 3;
							}

							if(bFound)
							{
								data = g_TextBuffer[j];
								g_TextBuffer[j] = 0;
								WD = UTFTextGetWidthEx(ptr);
								g_TextBuffer[j] = data;
								break;
							}
						}
					}

					if(WD > width)
					{
						BYTE *pTemp;

						if(ptr == &g_TextBuffer[oldI])
						{
							if(bOldByte > CHN_BYTE)
							{
								if(iWidth)
								{
									wStrlen = 0;
								}
								else
								{
									wStrlen = 2;
								}
							}
							else
							{
								wStrlen = 1;
							}
							i = oldI+wStrlen-1;
							pTemp = &g_TextBuffer[i+1];
						}
						else if((bPrevHaveCHN == TRUE) && (bOldByte <= CHN_BYTE))
						{
							i = newI-1;
							data = g_TextBuffer[newI];
							g_TextBuffer[newI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[newI] = data;
							pTemp = &g_TextBuffer[newI];
						}
						else
						{
							i = oldI-1;
							data = g_TextBuffer[oldI];
							g_TextBuffer[oldI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[oldI] = data;
							pTemp = &g_TextBuffer[oldI];
						}

						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						g_TextFinalData[wTotal++] = 0;

						ptr = pTemp;
					}
					else
					{
						i = j-1;
						if(bFound == 3)
						{
							i--;
						}

						g_TextFinalData[wTotal++] = 0;								
					}

					totalRow++;
					iWidth = 0;
					bPrevHaveCHN = FALSE;
				}
				else if(bOldByte > CHN_BYTE)
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i++];
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
					ptr += 2;
					
					iWidth += wordWidth+g_iCharInterval;
					if(g_TextBuffer[i+1] <= CHN_BYTE)
					{
						newI = i+1;
					}
				}
				else if(g_TextBuffer[i+1] > CHN_BYTE)
				{
					iWidth += wordWidth+g_iCharInterval;

					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wStrlen = strlen((char *)ptr);
					g_TextBuffer[i+1] = data;
							
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					ptr += wStrlen;
				}
			}
		}

		wStrlen = strlen((char *)ptr);
		if(wStrlen)
		{
			memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
			wTotal += wStrlen;
		}
		
		g_TextFinalData[wTotal] = 0;
		g_TextFinalData[wTotal+1] = 0;

		return totalRow;
	}

	return 0;
}

/***********************************************************************/
static void UTFBIG5Endllipse(LPTEXT lpText, WORD width, BYTE bFlag)
{
	WORD wStrLen;

	if(lpText == NULL)
		return;

	wStrLen = strlen((char *)lpText);
	if(wStrLen == 0)
		return;

	if(bFlag)
	{
		BYTE endllipse[4] = {'.','.','.',0};
		WORD wWidth;
		WORD wWD = 0;
		WORD charWD = 0;
		BYTE *ptr = lpText;
		BYTE c[2] = {0};
		BYTE bOldByte;
		int i;
	
		wWidth = UTFTextGetWidthEx(endllipse);
		for(i=0; i<wStrLen; i++)
		{
			bOldByte = lpText[i];
			if(bOldByte > BIG5_BYTE)
			{
				if(lpText[i+1])
				{
					i++;
					charWD = (WORD)g_TextFont.lfWidth;
				}
				else
				{
					break;
				}
			}
			else
			{
				c[0] = lpText[i];
				charWD = UTFGetCharWidth(c);
			}
			wWD += charWD;
			
			if((wWD+wWidth) <= width)
			{
				ptr++;
				if(bOldByte > BIG5_BYTE)
				{
					ptr++;
				}
			}
			else
			{
				break;
			}
			
			wWD += g_iCharInterval;
		}

		strcpy((char *)ptr, "...");
	}
	else
	{
		strcat((char *)lpText, "...");
	}
}

static WORD UTFBIG5DecodeText(LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat)
{
	WORD width, height;
	WORD i, textWidth = 0;
	WORD wReadBytes;
	WORD charWidth;
	WORD wStrlen;
	WORD wTotal = 0;
	BYTE bOldByte;
	BYTE c[3] = {0, 0, 0};

	width = lpRect->right - lpRect->left;
	height = lpRect->bottom - lpRect->top;
	
	wReadBytes = UTFTextCopy(lpText, g_TextBuffer, TEXT_BUFFER_SIZE, uFormat);
	
	if(uFormat & DTUI_SINGLELINE)
	{
		textWidth = UTFTextGetWidthEx(g_TextBuffer);
		
		if(textWidth <= width)
		{
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}
		else
		{
			WORD iWidth = 0;

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				BYTE endllipse[4] = {'.','.','.',0};
				
				iWidth = UTFTextGetWidthEx(endllipse);
			}
			
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}
				
				bOldByte = g_TextBuffer[i];
				
				if(bOldByte > BIG5_BYTE)
				{
					if(g_TextBuffer[i+1])
					{
						i++;
						charWidth = (WORD)g_TextFont.lfWidth;
					}
					else
					{
						break;
					}
				}
				else
				{
					c[0] = g_TextBuffer[i];
					c[1] = 0;
					charWidth = UTFGetCharWidth(c);
				}
				if(uFormat & DTUI_END_ELLIPSIS)
				{
					charWidth += g_iCharInterval;
				}
				
				if((iWidth+charWidth) <= width)
				{
					iWidth += charWidth;
					if(!(uFormat & DTUI_END_ELLIPSIS))
					{
						iWidth += g_iCharInterval;
					}
				}
				else
				{
					break;
				}

				if(bOldByte > BIG5_BYTE)
				{
					g_TextFinalData[wTotal++] = bOldByte;
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
				}
				else
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
				}
			}

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}

		return 1;
	}
	else
	{
		BYTE *ptr = g_TextBuffer;
		WORD totalRow = 1;
		WORD iWidth = 0;
		WORD totalByte = wReadBytes+1;
		WORD wordWidth;
		BYTE bPrevHaveBIG5 = FALSE;
		WORD newI = 0;
		
		for(i=0; i<totalByte; i++)
		{
			if(!g_TextBuffer[i])
			{
				break;
			}
			
			if(g_TextBuffer[i] == '\n')
			{
				totalRow++;
				iWidth = 0;
				bPrevHaveBIG5 = FALSE;

				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				g_TextBuffer[i] = '\n';

				memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
				wTotal += wStrlen;
				g_TextFinalData[wTotal++] = 0;
				
				ptr = &g_TextBuffer[i+1];
			}
			else if(g_TextBuffer[i] == ' ')
			{
				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i] = ' ';
				
				// If text length large than window width, cut text 
				if((iWidth+wordWidth) > width)
				{
					// If any space in text, make current word in new line
					totalRow++;
					iWidth = wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}

					g_TextFinalData[wTotal++] = 0;
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
					if(g_TextBuffer[i+1] > BIG5_BYTE)
					{
						c[0] = g_TextBuffer[i];
						c[1] = 0;
						iWidth += UTFGetCharWidth(c)+g_iCharInterval;

						g_TextFinalData[wTotal++] = *ptr++;
					}
					
					bPrevHaveBIG5 = FALSE;
				}
				else
				{
					iWidth += wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}

					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;

					ptr = &g_TextBuffer[i];
					if(g_TextBuffer[i+1] > BIG5_BYTE)
					{
						c[0] = g_TextBuffer[i];
						c[1] = 0;
						iWidth += UTFGetCharWidth(c)+g_iCharInterval;

						g_TextFinalData[wTotal++] = *ptr++;
					}
				}
			}
			else
			{
				WORD oldI = i;
				BYTE data;

				bOldByte = g_TextBuffer[i];
				if(bOldByte > BIG5_BYTE)
				{
					if(g_TextBuffer[i+1])
					{
						wordWidth = (WORD)g_TextFont.lfWidth;
						bPrevHaveBIG5 = TRUE;
					}
					else
					{
						break;
					}
				}
				else
				{
					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wordWidth = UTFTextGetWidthEx(ptr);
					g_TextBuffer[i+1] = data;
				}
				
				if((iWidth+wordWidth) > width)
				{
					WORD j,WD = width+1;
					BYTE bFound = 0;

					if(iWidth)//If any text had been copyed
					{
						WORD wStart = i+1;

						if(bOldByte > BIG5_BYTE)
						{
							wStart++;
						}

						//find current string for it tail
						for(j=wStart; j<totalByte; j++)
						{
							if((g_TextBuffer[j] == 0) || (g_TextBuffer[j] == ' ') || (g_TextBuffer[j] == '\n'))
							{
								bFound = 1;
							}
							else if(bOldByte > BIG5_BYTE)
							{
							}
							else if(g_TextBuffer[j] > BIG5_BYTE)
							{
								bFound = 3;
							}

							if(bFound)
							{
								data = g_TextBuffer[j];
								g_TextBuffer[j] = 0;
								WD = UTFTextGetWidthEx(ptr);
								g_TextBuffer[j] = data;
								break;
							}
						}
					}
					
					if(WD > width)
					{
						BYTE *pTemp;
						
						if(ptr == &g_TextBuffer[oldI])
						{
							if(bOldByte > BIG5_BYTE)
							{
								if(iWidth)
								{
									wStrlen = 0;
								}
								else
								{
									wStrlen = 2;
								}
							}
							else
							{
								wStrlen = 1;
							}
							i = oldI+wStrlen-1;
							pTemp = &g_TextBuffer[i+1];
						}
						else if((bPrevHaveBIG5 == TRUE) && (bOldByte <= BIG5_BYTE))
						{
							i = newI-1;
							data = g_TextBuffer[newI];
							g_TextBuffer[newI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[newI] = data;
							pTemp = &g_TextBuffer[newI];
						}
						else
						{
							i = oldI-1;
							data = g_TextBuffer[oldI];
							g_TextBuffer[oldI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[oldI] = data;
							pTemp = &g_TextBuffer[oldI];
						}

						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						g_TextFinalData[wTotal++] = 0;

						ptr = pTemp;
					}
					else
					{
						i = j-1;
						if(bFound == 3)
						{
							i--;
						}
						g_TextFinalData[wTotal++] = 0;
					}

					totalRow++;
					iWidth = 0;
					bPrevHaveBIG5 = FALSE;
				}
				else if(bOldByte > BIG5_BYTE)
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i++];
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
					ptr += 2;
					
					iWidth += wordWidth+g_iCharInterval;
					if(g_TextBuffer[i+1] <= BIG5_BYTE)
					{
						newI = i+1;
					}
				}
				else if(g_TextBuffer[i+1] > BIG5_BYTE)
				{
					iWidth += wordWidth+g_iCharInterval;

					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wStrlen = strlen((char *)ptr);
					g_TextBuffer[i+1] = data;
							
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					ptr += wStrlen;
				}
			}
		}

		wStrlen = strlen((char *)ptr);
		if(wStrlen)
		{
			memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
			wTotal += wStrlen;
		}
		
		g_TextFinalData[wTotal] = 0;
		g_TextFinalData[wTotal+1] = 0;

		return totalRow;
	}

	return 0;
}

/***********************************************************************/
static void UTFHindiCopyPreDoneData(LPTEXT lpStart, LPTEXT lpCurPtr)
{
	int iStrlen = strlen((char *)lpCurPtr);
	BYTE bEnglish = TRUE;
	WORD wTempBufferSize = TEMP_BUFFER_SIZE;
	BYTE bStartPos = 0;

	while(lpStart < lpCurPtr)
	{
		if(*lpStart == HINDI_BYTE)
		{
			if(bEnglish == TRUE)
			{
				bEnglish = FALSE;
			}
			else
			{
				bEnglish = TRUE;
			}
		}
		lpStart++;
	}

	if(bEnglish == FALSE)
	{
		wTempBufferSize--;
		g_TempCopyData[0] = HINDI_BYTE;
		bStartPos++;
	}

	if(iStrlen > wTempBufferSize)
	{
		memcpy(g_TempCopyData+bStartPos, lpCurPtr, wTempBufferSize);
		g_TempCopyData[TEMP_BUFFER_SIZE-1] = 0;
	}
	else
	{
		g_TempCopyData[bStartPos] = 0;
		strcpy((char *)g_TempCopyData+bStartPos, (char *)lpCurPtr);
	}
}

static void UTFHindiEndllipse(LPTEXT lpText, WORD width, BYTE bFlag)
{
	WORD wStrLen;

	if(lpText == NULL)
		return;

	wStrLen = strlen((char *)lpText);
	if(wStrLen == 0)
		return;

	if(bFlag)
	{
		BYTE endllipse[4] = {'.','.','.',0};
		WORD wWidth;
		WORD i,wWD = 0;
		WORD charWD = 0;
		BYTE *ptr = lpText;
		BYTE c[3] = {0};
		BYTE bOldByte;
		BYTE interval;
		BYTE bMode;
	
		wWidth = UTFTextGetWidthEx(endllipse);
		for(i=0; i<wStrLen; i++)
		{
			interval = g_iCharInterval;
			
			bOldByte = lpText[i];
			if(bOldByte == HINDI_BYTE)
			{
				if(lpText[i+1])
				{
					bMode = HindiCharGetType(lpText[i+1], &g_TextFont);
					if(bMode != 1)
					{
						interval = 0;
					}

					if(bMode)
					{
						c[0] = lpText[i];
						c[1] = lpText[i+1];
						c[2] = 0;
						charWD = UTFGetCharWidth(c);
					}
					else
					{
						charWD = 0;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				c[0] = lpText[i];
				c[1] = 0;
				charWD = UTFGetCharWidth(c);
			}
			wWD += charWD;

			if((wWD+wWidth) <= width)
			{
				ptr++;
				if(bOldByte == HINDI_BYTE)
				{
					ptr++;
					i++;
				}
			}
			else
			{
				break;
			}
			
			wWD += interval;
		}

		strcpy((char *)ptr, "...");
	}
	else
	{
		strcat((char *)lpText, "...");
	}
}

static WORD UTFHindiDecodeText(LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat)
{
	WORD width, height;
	WORD i, textWidth = 0;
	WORD wReadBytes;
	WORD charWidth;
	WORD wStrlen;
	WORD wTotal = 0;
	BYTE bOldByte;
	BYTE interval;
	BYTE bMode;
	BYTE c[3] = {0, 0, 0};

	width = lpRect->right - lpRect->left;
	height = lpRect->bottom - lpRect->top;
	
	wReadBytes = UTFTextCopy(lpText, g_TextBuffer, TEXT_BUFFER_SIZE, uFormat);

	if(uFormat & DTUI_SINGLELINE)
	{
		textWidth = UTFTextGetWidthEx(g_TextBuffer);
		
		if(textWidth <= width)
		{
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}
		else
		{
			WORD iWidth = 0;

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				BYTE endllipse[4] = {'.','.','.',0};
				
				iWidth = UTFTextGetWidthEx(endllipse);
			}
			
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				interval = g_iCharInterval;

				bOldByte = g_TextBuffer[i];
				if(bOldByte == HINDI_BYTE)
				{
					if(g_TextBuffer[i+1])
					{
						bMode = HindiCharGetType(g_TextBuffer[i+1], &g_TextFont);
						if(bMode != 1)
						{
							interval = 0;
						}

						if(bMode)
						{
							c[0] = g_TextBuffer[i];
							c[1] = g_TextBuffer[i+1];
							c[2] = 0;
							charWidth = UTFGetCharWidth(c);
						}
						else
						{
							charWidth = 0;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					c[0] = g_TextBuffer[i];
					c[1] = 0;
					charWidth = UTFGetCharWidth(c);
				}
				if(uFormat & DTUI_END_ELLIPSIS)
				{
					charWidth += interval;
				}

				if((iWidth+charWidth) <= width)
				{
					iWidth += charWidth;
					if(!(uFormat & DTUI_END_ELLIPSIS))
					{
						iWidth += interval;
					}
				}
				else
				{
					break;
				}

				if(bOldByte == HINDI_BYTE)
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
					g_TextFinalData[wTotal++] = g_TextBuffer[i+1];
					i++;
				}
				else
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
				}
			}

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}

		return 1;
	}
	else
	{
		BYTE *ptr = g_TextBuffer;
		WORD totalRow = 1;
		WORD iWidth = 0;
		WORD totalByte = wReadBytes+1;
		WORD wordWidth;
		BYTE bPrevHaveHindi = FALSE;
		WORD newI = 0;

		for(i=0; i<totalByte; i++)
		{
			if( !g_TextBuffer[i] )
			{
				break;
			}
			
			if(g_TextBuffer[i] == '\n')
			{
				totalRow++;
				iWidth = 0;

				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				g_TextBuffer[i] = '\n';

				memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
				wTotal += wStrlen;
				g_TextFinalData[wTotal++] = 0;
				
				ptr = &g_TextBuffer[i+1];
				
				bPrevHaveHindi = FALSE;
			}
			else if(g_TextBuffer[i] == ' ')
			{
				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i] = ' ';
				
				// If text length large than window width, cut text 
				if((iWidth+wordWidth) > width)
				{
					// If any space in text, make current word in new line
					totalRow++;
					iWidth = wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}

					g_TextFinalData[wTotal++] = 0;
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
					
					bPrevHaveHindi = FALSE;
				}
				else
				{
					iWidth += wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}
					
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
				}
			}
			else
			{
				WORD oldI = i;
				BYTE data;

				bOldByte = g_TextBuffer[i];
				if(bOldByte == HINDI_BYTE)
				{
					if(g_TextBuffer[i+1])
					{
						data = g_TextBuffer[i+2];
						g_TextBuffer[i+2] = 0;
						wordWidth = UTFTextGetWidthEx(ptr);
						g_TextBuffer[i+2] = data;
						
						bPrevHaveHindi = TRUE;
					}
					else
					{
						break;
					}
				}
				else
				{
					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wordWidth = UTFTextGetWidthEx(ptr);
					g_TextBuffer[i+1] = data;
				}
				
				if((iWidth+wordWidth) > width)
				{
					WORD j,WD = width+1;
					BYTE bFound = 0;

					if(iWidth)//If any text had been copyed
					{
						WORD wStart = i+1;

						if(bOldByte == HINDI_BYTE)
						{
							wStart++;
						}

						//find current string for it tail
						for(j=wStart; j<totalByte; j++)
						{
							if((g_TextBuffer[j] == 0) || (g_TextBuffer[j] == ' ') || (g_TextBuffer[j] == '\n'))
							{
								bFound = 1;
							}
							else if(bOldByte == HINDI_BYTE)
							{
							}
							else if(g_TextBuffer[j] == HINDI_BYTE)
							{
								bFound = 3;
							}

							if(bFound)
							{
								data = g_TextBuffer[j];
								g_TextBuffer[j] = 0;
								WD = UTFTextGetWidthEx(ptr);
								g_TextBuffer[j] = data;
								break;
							}
						}
					}
					
					if(WD > width)
					{
						BYTE *pTemp;
						
						if(ptr == &g_TextBuffer[oldI])
						{
							if(bOldByte == HINDI_BYTE)
							{
								if(iWidth)
								{
									wStrlen = 0;
								}
								else
								{
									wStrlen = 2;
								}
							}
							else
							{
								wStrlen = 1;
							}
							i = oldI+wStrlen-1;
							pTemp = &g_TextBuffer[i+1];
						}
						else if((bPrevHaveHindi == TRUE) && (bOldByte != HINDI_BYTE))
						{
							i = newI-1;
							data = g_TextBuffer[newI];
							g_TextBuffer[newI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[newI] = data;
							pTemp = &g_TextBuffer[newI];
						}
						else
						{
							i = oldI-1;
							data = g_TextBuffer[oldI];
							g_TextBuffer[oldI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[oldI] = data;
							pTemp = &g_TextBuffer[oldI];
						}

						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						g_TextFinalData[wTotal++] = 0;

						ptr = pTemp;
					}
					else
					{
						i = j-1;
						if(bFound == 3)
						{
							i--;
						}
						g_TextFinalData[wTotal++] = 0;
					}

					totalRow++;
					iWidth = 0;
					bPrevHaveHindi = FALSE;
				}
				else if(bOldByte == HINDI_BYTE)
				{
					i++;
					
					if(g_TextBuffer[i+1] != HINDI_BYTE)
					{
						newI = i+1;

						iWidth += wordWidth+g_iCharInterval;

						data = g_TextBuffer[i+1];
						g_TextBuffer[i+1] = 0;
						wStrlen = strlen((char *)ptr);
						g_TextBuffer[i+1] = data;
								
						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						ptr += wStrlen;
					}
				}
				else if(g_TextBuffer[i+1] == HINDI_BYTE)
				{
					iWidth += wordWidth+g_iCharInterval;

					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wStrlen = strlen((char *)ptr);
					g_TextBuffer[i+1] = data;
							
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					ptr += wStrlen;
				}
			}
		}

		wStrlen = strlen((char *)ptr);
		if(wStrlen)
		{
			memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
			wTotal += wStrlen;
		}
		
		g_TextFinalData[wTotal] = 0;
		g_TextFinalData[wTotal+1] = 0;

		return totalRow;
	}

	return 0;
}

/***********************************************************************/
static void UTFTamilCopyPreDoneData(LPTEXT lpStart, LPTEXT lpCurPtr)
{
	int iStrlen = strlen((char *)lpCurPtr);
	BYTE bEnglish = TRUE;
	WORD wTempBufferSize = TEMP_BUFFER_SIZE;
	BYTE bStartPos = 0;

	while(lpStart < lpCurPtr)
	{
		if(*lpStart == TAMIL_BYTE)
		{
			if(bEnglish == TRUE)
			{
				bEnglish = FALSE;
			}
			else
			{
				bEnglish = TRUE;
			}
		}
		lpStart++;
	}

	if(bEnglish == FALSE)
	{
		wTempBufferSize--;
		g_TempCopyData[0] = TAMIL_BYTE;
		bStartPos++;
	}

	if(iStrlen > wTempBufferSize)
	{
		memcpy(g_TempCopyData+bStartPos, lpCurPtr, wTempBufferSize);
		g_TempCopyData[TEMP_BUFFER_SIZE-1] = 0;
	}
	else
	{
		g_TempCopyData[bStartPos] = 0;
		strcpy((char *)g_TempCopyData+bStartPos, (char *)lpCurPtr);
	}
}

static void UTFTamilEndllipse(LPTEXT lpText, WORD width, BYTE bFlag)
{
	WORD wStrLen;

	if(lpText == NULL)
		return;

	wStrLen = strlen((char *)lpText);
	if(wStrLen == 0)
		return;

	if(bFlag)
	{
		BYTE endllipse[4] = {'.','.','.',0};
		WORD wWidth;
		WORD i,wWD = 0;
		WORD charWD = 0;
		BYTE *ptr = lpText;
		BYTE c[3] = {0};
		BYTE bOldByte;
		BYTE interval;
		BYTE bMode;
	
		wWidth = UTFTextGetWidthEx(endllipse);
		for(i=0; i<wStrLen; i++)
		{
			interval = g_iCharInterval;
			
			bOldByte = lpText[i];
			if(bOldByte == TAMIL_BYTE)
			{
				if(lpText[i+1])
				{
					bMode = TAMILCharGetType(lpText[i+1], &g_TextFont);
					if(bMode != 1)
					{
						interval = 0;
					}

					if(bMode)
					{
						c[0] = lpText[i];
						c[1] = lpText[i+1];
						c[2] = 0;
						charWD = UTFGetCharWidth(c);
					}
					else
					{
						charWD = 0;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				c[0] = lpText[i];
				c[1] = 0;
				charWD = UTFGetCharWidth(c);
			}
			wWD += charWD;

			if((wWD+wWidth) <= width)
			{
				ptr++;
				if(bOldByte == TAMIL_BYTE)
				{
					ptr++;
					i++;
				}
			}
			else
			{
				break;
			}
			
			wWD += interval;
		}

		strcpy((char *)ptr, "...");
	}
	else
	{
		strcat((char *)lpText, "...");
	}
}

static WORD UTFTamilDecodeText(LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat)
{
	WORD width, height;
	WORD i, textWidth = 0;
	WORD wReadBytes;
	WORD charWidth;
	WORD wStrlen;
	WORD wTotal = 0;
	BYTE bOldByte;
	BYTE interval;
	BYTE bMode;
	BYTE c[3] = {0, 0, 0};

	width = lpRect->right - lpRect->left;
	height = lpRect->bottom - lpRect->top;
	
	wReadBytes = UTFTextCopy(lpText, g_TextBuffer, TEXT_BUFFER_SIZE, uFormat);

	if(uFormat & DTUI_SINGLELINE)
	{
		textWidth = UTFTextGetWidthEx(g_TextBuffer);
		
		if(textWidth <= width)
		{
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}
		else
		{
			WORD iWidth = 0;

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				BYTE endllipse[4] = {'.','.','.',0};
				
				iWidth = UTFTextGetWidthEx(endllipse);
			}
			
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				interval = g_iCharInterval;

				bOldByte = g_TextBuffer[i];
				if(bOldByte == TAMIL_BYTE)
				{
					if(g_TextBuffer[i+1])
					{
						bMode = TAMILCharGetType(g_TextBuffer[i+1], &g_TextFont);
						if(bMode != 1)
						{
							interval = 0;
						}

						if(bMode)
						{
							c[0] = g_TextBuffer[i];
							c[1] = g_TextBuffer[i+1];
							c[2] = 0;
							charWidth = UTFGetCharWidth(c);
						}
						else
						{
							charWidth = 0;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					c[0] = g_TextBuffer[i];
					c[1] = 0;
					charWidth = UTFGetCharWidth(c);
				}
				if(uFormat & DTUI_END_ELLIPSIS)
				{
					charWidth += interval;
				}

				if((iWidth+charWidth) <= width)
				{
					iWidth += charWidth;
					if(!(uFormat & DTUI_END_ELLIPSIS))
					{
						iWidth += interval;
					}
				}
				else
				{
					break;
				}

				if(bOldByte == TAMIL_BYTE)
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
					g_TextFinalData[wTotal++] = g_TextBuffer[i+1];
					i++;
				}
				else
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
				}
			}

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}

		return 1;
	}
	else
	{
		BYTE *ptr = g_TextBuffer;
		WORD totalRow = 1;
		WORD iWidth = 0;
		WORD totalByte = wReadBytes+1;
		WORD wordWidth;
		BYTE bPrevHaveTamil = FALSE;
		WORD newI = 0;

		for(i=0; i<totalByte; i++)
		{
			if( !g_TextBuffer[i] )
			{
				break;
			}
			
			if(g_TextBuffer[i] == '\n')
			{
				totalRow++;
				iWidth = 0;

				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				g_TextBuffer[i] = '\n';

				memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
				wTotal += wStrlen;
				g_TextFinalData[wTotal++] = 0;
				
				ptr = &g_TextBuffer[i+1];
				
				bPrevHaveTamil = FALSE;
			}
			else if(g_TextBuffer[i] == ' ')
			{
				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i] = ' ';
				
				// If text length large than window width, cut text 
				if((iWidth+wordWidth) > width)
				{
					// If any space in text, make current word in new line
					totalRow++;
					iWidth = wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}

					g_TextFinalData[wTotal++] = 0;
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
					
					bPrevHaveTamil = FALSE;
				}
				else
				{
					iWidth += wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}
					
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
				}
			}
			else
			{
				WORD oldI = i;
				BYTE data;

				bOldByte = g_TextBuffer[i];
				if(bOldByte == TAMIL_BYTE)
				{
					if(g_TextBuffer[i+1])
					{
						data = g_TextBuffer[i+2];
						g_TextBuffer[i+2] = 0;
						wordWidth = UTFTextGetWidthEx(ptr);
						g_TextBuffer[i+2] = data;
						
						bPrevHaveTamil = TRUE;
					}
					else
					{
						break;
					}
				}
				else
				{
					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wordWidth = UTFTextGetWidthEx(ptr);
					g_TextBuffer[i+1] = data;
				}
				
				if((iWidth+wordWidth) > width)
				{
					WORD j,WD = width+1;
					BYTE bFound = 0;

					if(iWidth)//If any text had been copyed
					{
						WORD wStart = i+1;

						if(bOldByte == TAMIL_BYTE)
						{
							wStart++;
						}

						//find current string for it tail
						for(j=wStart; j<totalByte; j++)
						{
							if((g_TextBuffer[j] == 0) || (g_TextBuffer[j] == ' ') || (g_TextBuffer[j] == '\n'))
							{
								bFound = 1;
							}
							else if(bOldByte == TAMIL_BYTE)
							{
							}
							else if(g_TextBuffer[j] == TAMIL_BYTE)
							{
								bFound = 3;
							}

							if(bFound)
							{
								data = g_TextBuffer[j];
								g_TextBuffer[j] = 0;
								WD = UTFTextGetWidthEx(ptr);
								g_TextBuffer[j] = data;
								break;
							}
						}
					}
					
					if(WD > width)
					{
						BYTE *pTemp;
						
						if(ptr == &g_TextBuffer[oldI])
						{
							if(bOldByte == TAMIL_BYTE)
							{
								if(iWidth)
								{
									wStrlen = 0;
								}
								else
								{
									wStrlen = 2;
								}
							}
							else
							{
								wStrlen = 1;
							}
							i = oldI+wStrlen-1;
							pTemp = &g_TextBuffer[i+1];
						}
						else if((bPrevHaveTamil == TRUE) && (bOldByte != TAMIL_BYTE))
						{
							i = newI-1;
							data = g_TextBuffer[newI];
							g_TextBuffer[newI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[newI] = data;
							pTemp = &g_TextBuffer[newI];
						}
						else
						{
							i = oldI-1;
							data = g_TextBuffer[oldI];
							g_TextBuffer[oldI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[oldI] = data;
							pTemp = &g_TextBuffer[oldI];
						}

						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						g_TextFinalData[wTotal++] = 0;

						ptr = pTemp;
					}
					else
					{
						i = j-1;
						if(bFound == 3)
						{
							i--;
						}
						g_TextFinalData[wTotal++] = 0;
					}

					totalRow++;
					iWidth = 0;
					bPrevHaveTamil = FALSE;
				}
				else if(bOldByte == TAMIL_BYTE)
				{
					i++;
					
					if(g_TextBuffer[i+1] != TAMIL_BYTE)
					{
						newI = i+1;

						iWidth += wordWidth+g_iCharInterval;

						data = g_TextBuffer[i+1];
						g_TextBuffer[i+1] = 0;
						wStrlen = strlen((char *)ptr);
						g_TextBuffer[i+1] = data;
								
						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						ptr += wStrlen;
					}
				}
				else if(g_TextBuffer[i+1] == TAMIL_BYTE)
				{
					iWidth += wordWidth+g_iCharInterval;

					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wStrlen = strlen((char *)ptr);
					g_TextBuffer[i+1] = data;
							
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					ptr += wStrlen;
				}
			}
		}

		wStrlen = strlen((char *)ptr);
		if(wStrlen)
		{
			memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
			wTotal += wStrlen;
		}
		
		g_TextFinalData[wTotal] = 0;
		g_TextFinalData[wTotal+1] = 0;

		return totalRow;
	}

	return 0;
}

/***********************************************************************/
static void UTFMalayalamCopyPreDoneData(LPTEXT lpStart, LPTEXT lpCurPtr)
{
	int iStrlen = strlen((char *)lpCurPtr);
	BYTE bEnglish = TRUE;
	WORD wTempBufferSize = TEMP_BUFFER_SIZE;
	BYTE bStartPos = 0;

	while(lpStart < lpCurPtr)
	{
		if(*lpStart == MALAYALAM_BYTE)
		{
			if(bEnglish == TRUE)
			{
				bEnglish = FALSE;
			}
			else
			{
				bEnglish = TRUE;
			}
		}
		lpStart++;
	}

	if(bEnglish == FALSE)
	{
		wTempBufferSize--;
		g_TempCopyData[0] = MALAYALAM_BYTE;
		bStartPos++;
	}

	if(iStrlen > wTempBufferSize)
	{
		memcpy(g_TempCopyData+bStartPos, lpCurPtr, wTempBufferSize);
		g_TempCopyData[TEMP_BUFFER_SIZE-1] = 0;
	}
	else
	{
		g_TempCopyData[bStartPos] = 0;
		strcpy((char *)g_TempCopyData+bStartPos, (char *)lpCurPtr);
	}
}

static void UTFMalayalamEndllipse(LPTEXT lpText, WORD width, BYTE bFlag)
{
	WORD wStrLen;

	if(lpText == NULL)
		return;

	wStrLen = strlen((char *)lpText);
	if(wStrLen == 0)
		return;

	if(bFlag)
	{
		BYTE endllipse[4] = {'.','.','.',0};
		WORD wWidth;
		WORD i,wWD = 0;
		WORD charWD = 0;
		BYTE *ptr = lpText;
		BYTE c[3] = {0};
		BYTE bOldByte;
		BYTE interval;
		BYTE bMode;
	
		wWidth = UTFTextGetWidthEx(endllipse);
		for(i=0; i<wStrLen; i++)
		{
			interval = g_iCharInterval;
			
			bOldByte = lpText[i];
			if(bOldByte == MALAYALAM_BYTE)
			{
				if(lpText[i+1])
				{
					bMode = MALAYACharGetType(lpText[i+1], &g_TextFont);
					if(bMode != 1)
					{
						interval = 0;
					}

					if(bMode)
					{
						c[0] = lpText[i];
						c[1] = lpText[i+1];
						c[2] = 0;
						charWD = UTFGetCharWidth(c);
					}
					else
					{
						charWD = 0;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				c[0] = lpText[i];
				c[1] = 0;
				charWD = UTFGetCharWidth(c);
			}
			wWD += charWD;

			if((wWD+wWidth) <= width)
			{
				ptr++;
				if(bOldByte == MALAYALAM_BYTE)
				{
					ptr++;
					i++;
				}
			}
			else
			{
				break;
			}
			
			wWD += interval;
		}

		strcpy((char *)ptr, "...");
	}
	else
	{
		strcat((char *)lpText, "...");
	}
}

static WORD UTFMalayalamDecodeText(LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat)
{
	WORD width, height;
	WORD i, textWidth = 0;
	WORD wReadBytes;
	WORD charWidth;
	WORD wStrlen;
	WORD wTotal = 0;
	BYTE bOldByte;
	BYTE interval;
	BYTE bMode;
	BYTE c[3] = {0, 0, 0};

	width = lpRect->right - lpRect->left;
	height = lpRect->bottom - lpRect->top;
	
	wReadBytes = UTFTextCopy(lpText, g_TextBuffer, TEXT_BUFFER_SIZE, uFormat);

	if(uFormat & DTUI_SINGLELINE)
	{
		textWidth = UTFTextGetWidthEx(g_TextBuffer);
		
		if(textWidth <= width)
		{
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}
		else
		{
			WORD iWidth = 0;

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				BYTE endllipse[4] = {'.','.','.',0};
				
				iWidth = UTFTextGetWidthEx(endllipse);
			}
			
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				interval = g_iCharInterval;

				bOldByte = g_TextBuffer[i];
				if(bOldByte == MALAYALAM_BYTE)
				{
					if(g_TextBuffer[i+1])
					{
						bMode = MALAYACharGetType(g_TextBuffer[i+1], &g_TextFont);
						if(bMode != 1)
						{
							interval = 0;
						}

						if(bMode)
						{
							c[0] = g_TextBuffer[i];
							c[1] = g_TextBuffer[i+1];
							c[2] = 0;
							charWidth = UTFGetCharWidth(c);
						}
						else
						{
							charWidth = 0;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					c[0] = g_TextBuffer[i];
					c[1] = 0;
					charWidth = UTFGetCharWidth(c);
				}
				if(uFormat & DTUI_END_ELLIPSIS)
				{
					charWidth += interval;
				}

				if((iWidth+charWidth) <= width)
				{
					iWidth += charWidth;
					if(!(uFormat & DTUI_END_ELLIPSIS))
					{
						iWidth += interval;
					}
				}
				else
				{
					break;
				}

				if(bOldByte == MALAYALAM_BYTE)
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
					g_TextFinalData[wTotal++] = g_TextBuffer[i+1];
					i++;
				}
				else
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
				}
			}

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}

		return 1;
	}
	else
	{
		BYTE *ptr = g_TextBuffer;
		WORD totalRow = 1;
		WORD iWidth = 0;
		WORD totalByte = wReadBytes+1;
		WORD wordWidth;
		BYTE bPrevHaveMalayalam = FALSE;
		WORD newI = 0;

		for(i=0; i<totalByte; i++)
		{
			if( !g_TextBuffer[i] )
			{
				break;
			}
			
			if(g_TextBuffer[i] == '\n')
			{
				totalRow++;
				iWidth = 0;

				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				g_TextBuffer[i] = '\n';

				memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
				wTotal += wStrlen;
				g_TextFinalData[wTotal++] = 0;
				
				ptr = &g_TextBuffer[i+1];
				
				bPrevHaveMalayalam = FALSE;
			}
			else if(g_TextBuffer[i] == ' ')
			{
				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i] = ' ';
				
				// If text length large than window width, cut text 
				if((iWidth+wordWidth) > width)
				{
					// If any space in text, make current word in new line
					totalRow++;
					iWidth = wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}

					g_TextFinalData[wTotal++] = 0;
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
					
					bPrevHaveMalayalam = FALSE;
				}
				else
				{
					iWidth += wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}
					
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
				}
			}
			else
			{
				WORD oldI = i;
				BYTE data;

				bOldByte = g_TextBuffer[i];
				if(bOldByte == MALAYALAM_BYTE)
				{
					if(g_TextBuffer[i+1])
					{
						data = g_TextBuffer[i+2];
						g_TextBuffer[i+2] = 0;
						wordWidth = UTFTextGetWidthEx(ptr);
						g_TextBuffer[i+2] = data;
						
						bPrevHaveMalayalam = TRUE;
					}
					else
					{
						break;
					}
				}
				else
				{
					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wordWidth = UTFTextGetWidthEx(ptr);
					g_TextBuffer[i+1] = data;
				}
				
				if((iWidth+wordWidth) > width)
				{
					WORD j,WD = width+1;
					BYTE bFound = 0;

					if(iWidth)//If any text had been copyed
					{
						WORD wStart = i+1;

						if(bOldByte == MALAYALAM_BYTE)
						{
							wStart++;
						}

						//find current string for it tail
						for(j=wStart; j<totalByte; j++)
						{
							if((g_TextBuffer[j] == 0) || (g_TextBuffer[j] == ' ') || (g_TextBuffer[j] == '\n'))
							{
								bFound = 1;
							}
							else if(bOldByte == MALAYALAM_BYTE)
							{
							}
							else if(g_TextBuffer[j] == MALAYALAM_BYTE)
							{
								bFound = 3;
							}

							if(bFound)
							{
								data = g_TextBuffer[j];
								g_TextBuffer[j] = 0;
								WD = UTFTextGetWidthEx(ptr);
								g_TextBuffer[j] = data;
								break;
							}
						}
					}
					
					if(WD > width)
					{
						BYTE *pTemp;
						
						if(ptr == &g_TextBuffer[oldI])
						{
							if(bOldByte == MALAYALAM_BYTE)
							{
								if(iWidth)
								{
									wStrlen = 0;
								}
								else
								{
									wStrlen = 2;
								}
							}
							else
							{
								wStrlen = 1;
							}
							i = oldI+wStrlen-1;
							pTemp = &g_TextBuffer[i+1];
						}
						else if((bPrevHaveMalayalam == TRUE) && (bOldByte != MALAYALAM_BYTE))
						{
							i = newI-1;
							data = g_TextBuffer[newI];
							g_TextBuffer[newI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[newI] = data;
							pTemp = &g_TextBuffer[newI];
						}
						else
						{
							i = oldI-1;
							data = g_TextBuffer[oldI];
							g_TextBuffer[oldI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[oldI] = data;
							pTemp = &g_TextBuffer[oldI];
						}

						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						g_TextFinalData[wTotal++] = 0;

						ptr = pTemp;
					}
					else
					{
						i = j-1;
						if(bFound == 3)
						{
							i--;
						}
						g_TextFinalData[wTotal++] = 0;
					}

					totalRow++;
					iWidth = 0;
					bPrevHaveMalayalam = FALSE;
				}
				else if(bOldByte == MALAYALAM_BYTE)
				{
					i++;
					
					if(g_TextBuffer[i+1] != MALAYALAM_BYTE)
					{
						newI = i+1;

						iWidth += wordWidth+g_iCharInterval;

						data = g_TextBuffer[i+1];
						g_TextBuffer[i+1] = 0;
						wStrlen = strlen((char *)ptr);
						g_TextBuffer[i+1] = data;
								
						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						ptr += wStrlen;
					}
				}
				else if(g_TextBuffer[i+1] == MALAYALAM_BYTE)
				{
					iWidth += wordWidth+g_iCharInterval;

					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wStrlen = strlen((char *)ptr);
					g_TextBuffer[i+1] = data;
							
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					ptr += wStrlen;
				}
			}
		}

		wStrlen = strlen((char *)ptr);
		if(wStrlen)
		{
			memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
			wTotal += wStrlen;
		}
		
		g_TextFinalData[wTotal] = 0;
		g_TextFinalData[wTotal+1] = 0;

		return totalRow;
	}

	return 0;
}

/***********************************************************************/
static void UTFKannadaCopyPreDoneData(LPTEXT lpStart, LPTEXT lpCurPtr)
{
	int iStrlen = strlen((char *)lpCurPtr);
	BYTE bEnglish = TRUE;
	WORD wTempBufferSize = TEMP_BUFFER_SIZE;
	BYTE bStartPos = 0;

	while(lpStart < lpCurPtr)
	{
		if(*lpStart == KANNADA_BYTE)
		{
			if(bEnglish == TRUE)
			{
				bEnglish = FALSE;
			}
			else
			{
				bEnglish = TRUE;
			}
		}
		lpStart++;
	}

	if(bEnglish == FALSE)
	{
		wTempBufferSize--;
		g_TempCopyData[0] = KANNADA_BYTE;
		bStartPos++;
	}

	if(iStrlen > wTempBufferSize)
	{
		memcpy(g_TempCopyData+bStartPos, lpCurPtr, wTempBufferSize);
		g_TempCopyData[TEMP_BUFFER_SIZE-1] = 0;
	}
	else
	{
		g_TempCopyData[bStartPos] = 0;
		strcpy((char *)g_TempCopyData+bStartPos, (char *)lpCurPtr);
	}
}

static void UTFKannadaEndllipse(LPTEXT lpText, WORD width, BYTE bFlag)
{
	WORD wStrLen;

	if(lpText == NULL)
		return;

	wStrLen = strlen((char *)lpText);
	if(wStrLen == 0)
		return;

	if(wStrLen < 3)
	{
		memset(lpText, '.', wStrLen);
		return;
	}

	if(bFlag)
	{
		BYTE endllipse[4] = {'.','.','.',0};
		WORD wWidth;
		WORD i,wWD = 0;
		BYTE *ptr = lpText;
		BYTE bOldByte;
	
		wWD = UTFTextGetWidthEx(endllipse)+g_iCharInterval;
		for(i=0; i<wStrLen; i++)
		{
			if(lpText[i] == KANNADA_BYTE)
			{
				bOldByte = lpText[i+2];
				lpText[i+2] = 0;
				wWidth = UTFTextGetWidthEx(lpText);
				lpText[i+2] = bOldByte;
			}
			else
			{
				bOldByte = lpText[i+1];
				lpText[i+1] = 0;
				wWidth = UTFTextGetWidthEx(lpText);
				lpText[i+1] = bOldByte;
			}
			
			if((wWD+wWidth) <= width)
			{
				ptr++;
				if(lpText[i] == KANNADA_BYTE)
				{
					ptr++;
					i++;
				}
			}
			else
			{
				break;
			}
		}

		strcpy((char *)ptr, "...");
	}
	else
	{
		strcat((char *)lpText, "...");
	}
}

static WORD UTFKannadaDecodeText(LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat)
{
	WORD width, height;
	WORD i, textWidth = 0;
	WORD wReadBytes;
	WORD charWidth;
	WORD wStrlen;
	WORD wTotal = 0;
	BYTE bOldByte;

	width = lpRect->right - lpRect->left;
	height = lpRect->bottom - lpRect->top;
	
	wReadBytes = UTFTextCopy(lpText, g_TextBuffer, TEXT_BUFFER_SIZE, uFormat);

	if(uFormat & DTUI_SINGLELINE)
	{
		textWidth = UTFTextGetWidthEx(g_TextBuffer);
		
		if(textWidth <= width)
		{
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}
		else
		{
			WORD iWidth = 0;

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				BYTE endllipse[4] = {'.','.','.',0};
				
				iWidth = UTFTextGetWidthEx(endllipse)+g_iCharInterval;
			}
			
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				if(g_TextBuffer[i] == KANNADA_BYTE)
				{
					bOldByte = g_TextBuffer[i+2];
					g_TextBuffer[i+2] = 0;
					charWidth = UTFTextGetWidthEx(g_TextBuffer);
					g_TextBuffer[i+2] = bOldByte;
				}
				else
				{
					bOldByte = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					charWidth = UTFTextGetWidthEx(g_TextBuffer);
					g_TextBuffer[i+1] = bOldByte;
				}

				if((iWidth+charWidth) > width)
				{
					break;
				}

				if(g_TextBuffer[i] == KANNADA_BYTE)
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
					g_TextFinalData[wTotal++] = g_TextBuffer[i+1];
					i++;
				}
				else
				{
					g_TextFinalData[wTotal++] = g_TextBuffer[i];
				}
			}

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}

		return 1;
	}
	else
	{
		BYTE *ptr = g_TextBuffer;
		WORD totalRow = 1;
		WORD iWidth = 0;
		WORD totalByte = wReadBytes+1;
		WORD wordWidth;
		BYTE bPrevHaveKannada = FALSE;
		WORD newI = 0;

		for(i=0; i<totalByte; i++)
		{
			if( !g_TextBuffer[i] )
			{
				break;
			}
			
			if(g_TextBuffer[i] == '\n')
			{
				totalRow++;
				iWidth = 0;

				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				g_TextBuffer[i] = '\n';

				memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
				wTotal += wStrlen;
				g_TextFinalData[wTotal++] = 0;
				
				ptr = &g_TextBuffer[i+1];
				
				bPrevHaveKannada = FALSE;
			}
			else if(g_TextBuffer[i] == ' ')
			{
				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i] = ' ';
				
				// If text length large than window width, cut text 
				if((iWidth+wordWidth) > width)
				{
					// If any space in text, make current word in new line
					totalRow++;
					iWidth = wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}

					g_TextFinalData[wTotal++] = 0;
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
					
					bPrevHaveKannada = FALSE;
				}
				else
				{
					iWidth += wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}
					
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
				}
			}
			else
			{
				WORD oldI = i;
				BYTE data;

				bOldByte = g_TextBuffer[i];
				if(bOldByte == KANNADA_BYTE)
				{
					if(g_TextBuffer[i+1])
					{
						data = g_TextBuffer[i+2];
						g_TextBuffer[i+2] = 0;
						wordWidth = UTFTextGetWidthEx(ptr);
						g_TextBuffer[i+2] = data;
						
						bPrevHaveKannada = TRUE;
					}
					else
					{
						break;
					}
				}
				else
				{
					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wordWidth = UTFTextGetWidthEx(ptr);
					g_TextBuffer[i+1] = data;
				}
				
				if((iWidth+wordWidth) > width)
				{
					WORD j,WD = width+1;
					BYTE bFound = 0;

					if(iWidth)//If any text had been copyed
					{
						WORD wStart = i+1;

						if(bOldByte == KANNADA_BYTE)
						{
							wStart++;
						}

						//find current string for it tail
						for(j=wStart; j<totalByte; j++)
						{
							if((g_TextBuffer[j] == 0) || (g_TextBuffer[j] == ' ') || (g_TextBuffer[j] == '\n'))
							{
								bFound = 1;
							}
							else if(bOldByte == KANNADA_BYTE)
							{
							}
							else if(g_TextBuffer[j] == KANNADA_BYTE)
							{
								bFound = 3;
							}

							if(bFound)
							{
								data = g_TextBuffer[j];
								g_TextBuffer[j] = 0;
								WD = UTFTextGetWidthEx(ptr);
								g_TextBuffer[j] = data;
								break;
							}
						}
					}
					
					if(WD > width)
					{
						BYTE *pTemp;
						
						if(ptr == &g_TextBuffer[oldI])
						{
							if(bOldByte == KANNADA_BYTE)
							{
								if(iWidth)
								{
									wStrlen = 0;
								}
								else
								{
									wStrlen = 2;
								}
							}
							else
							{
								wStrlen = 1;
							}
							i = oldI+wStrlen-1;
							pTemp = &g_TextBuffer[i+1];
						}
						else if((bPrevHaveKannada == TRUE) && (bOldByte != KANNADA_BYTE))
						{
							i = newI-1;
							data = g_TextBuffer[newI];
							g_TextBuffer[newI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[newI] = data;
							pTemp = &g_TextBuffer[newI];
						}
						else
						{
							i = oldI-1;
							data = g_TextBuffer[oldI];
							g_TextBuffer[oldI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[oldI] = data;
							pTemp = &g_TextBuffer[oldI];
						}

						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						g_TextFinalData[wTotal++] = 0;

						ptr = pTemp;
					}
					else
					{
						i = j-1;
						if(bFound == 3)
						{
							i--;
						}
						g_TextFinalData[wTotal++] = 0;
					}

					totalRow++;
					iWidth = 0;
					bPrevHaveKannada = FALSE;
				}
				else if(bOldByte == KANNADA_BYTE)
				{
					i++;
					
					if(g_TextBuffer[i+1] != KANNADA_BYTE)
					{
						newI = i+1;

						iWidth += wordWidth+g_iCharInterval;

						data = g_TextBuffer[i+1];
						g_TextBuffer[i+1] = 0;
						wStrlen = strlen((char *)ptr);
						g_TextBuffer[i+1] = data;
								
						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						ptr += wStrlen;
					}
				}
				else if(g_TextBuffer[i+1] == KANNADA_BYTE)
				{
					iWidth += wordWidth+g_iCharInterval;

					data = g_TextBuffer[i+1];
					g_TextBuffer[i+1] = 0;
					wStrlen = strlen((char *)ptr);
					g_TextBuffer[i+1] = data;
							
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					ptr += wStrlen;
				}
			}
		}

		wStrlen = strlen((char *)ptr);
		if(wStrlen)
		{
			memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
			wTotal += wStrlen;
		}
		
		g_TextFinalData[wTotal] = 0;
		g_TextFinalData[wTotal+1] = 0;

		return totalRow;
	}

	return 0;
}

static void UTFThaiEndllipse(LPTEXT lpText, WORD width, BYTE bFlag)
{
	WORD wStrLen;

	if(lpText == NULL)
		return;

	wStrLen = strlen((char *)lpText);
	if(wStrLen == 0)
		return;

	if(wStrLen < 3)
	{
		memset(lpText, '.', wStrLen);
		return;
	}

	if(bFlag)
	{
		BYTE endllipse[4] = {'.','.','.',0};
		WORD wWidth;
		WORD i,wWD = 0;
		BYTE *ptr = lpText;
		BYTE bOldByte;
	
		wWD = UTFTextGetWidthEx(endllipse)+g_iCharInterval;
		for(i=0; i<wStrLen; i++)
		{
			bOldByte = lpText[i+1];
			lpText[i+1] = 0;
			wWidth = UTFTextGetWidthEx(lpText);
			lpText[i+1] = bOldByte;
			
			if((wWD+wWidth) <= width)
			{
				ptr++;
			}
			else
			{
				break;
			}
		}

		strcpy((char *)ptr, "...");
	}
	else
	{
		strcat((char *)lpText, "...");
	}
}

static WORD UTFThaiDecodeText(LPTEXT lpText, LPUTFRECT lpRect, DWORD uFormat)
{
	WORD width, height;
	WORD i, textWidth = 0;
	WORD wReadBytes;
	WORD charWidth;
	WORD wStrlen;
	WORD wTotal = 0;
	BYTE bOldByte;

	width = lpRect->right - lpRect->left;
	height = lpRect->bottom - lpRect->top;
	
	wReadBytes = UTFTextCopy(lpText, g_TextBuffer, TEXT_BUFFER_SIZE, uFormat);

	if(uFormat & DTUI_SINGLELINE)
	{
		textWidth = UTFTextGetWidthEx(g_TextBuffer);
		
		if(textWidth <= width)
		{
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}
		else
		{
			WORD iWidth = 0;

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				BYTE endllipse[4] = {'.','.','.',0};
				
				iWidth = UTFTextGetWidthEx(endllipse)+g_iCharInterval;
			}
			
			for(i=0; i<wReadBytes; i++)
			{
				if(g_TextBuffer[i] == '\n')
				{
					break;
				}

				bOldByte = g_TextBuffer[i+1];
				g_TextBuffer[i+1] = 0;
				charWidth = UTFTextGetWidthEx(g_TextBuffer);
				g_TextBuffer[i+1] = bOldByte;

				if((iWidth+charWidth) > width)
				{
					break;
				}
				g_TextFinalData[wTotal++] = g_TextBuffer[i];
			}

			if(uFormat & DTUI_END_ELLIPSIS)
			{
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
				g_TextFinalData[wTotal++] = '.';
			}
			g_TextFinalData[wTotal] = 0;
			g_TextFinalData[wTotal+1] = 0;
		}

		return 1;
	}
	else
	{
		BYTE *ptr = g_TextBuffer;
		WORD totalRow = 1;
		WORD iWidth = 0;
		WORD totalByte = wReadBytes+1;
		WORD wordWidth;
		WORD newI = 0;

		for(i=0; i<totalByte; i++)
		{
			if( !g_TextBuffer[i] )
			{
				break;
			}
			
			if(g_TextBuffer[i] == '\n')
			{
				totalRow++;
				iWidth = 0;

				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				g_TextBuffer[i] = '\n';

				memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
				wTotal += wStrlen;
				g_TextFinalData[wTotal++] = 0;
				
				ptr = &g_TextBuffer[i+1];
			}
			else if(g_TextBuffer[i] == ' ')
			{
				g_TextBuffer[i] = 0;
				wStrlen = strlen((char *)ptr);
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i] = ' ';
				
				// If text length large than window width, cut text 
				if((iWidth+wordWidth) > width)
				{
					// If any space in text, make current word in new line
					totalRow++;
					iWidth = wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}

					g_TextFinalData[wTotal++] = 0;
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
				}
				else
				{
					iWidth += wordWidth;
					if(wordWidth)
					{
						iWidth += g_iCharInterval;
					}
					
					memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
					wTotal += wStrlen;
					
					ptr = &g_TextBuffer[i];
				}
			}
			else
			{
				WORD oldI = i;
				BYTE data;

				data = g_TextBuffer[i+1];
				g_TextBuffer[i+1] = 0;
				wordWidth = UTFTextGetWidthEx(ptr);
				g_TextBuffer[i+1] = data;
				
				if((iWidth+wordWidth) > width)
				{
					WORD j,WD = width+1;
					BYTE bFound = 0;

					if(iWidth)//If any text had been copyed
					{
						WORD wStart = i+1;

						//find current string for it tail
						for(j=wStart; j<totalByte; j++)
						{
							if((g_TextBuffer[j] == 0) || (g_TextBuffer[j] == ' ') || (g_TextBuffer[j] == '\n'))
							{
								bFound = 1;
							}

							if(bFound)
							{
								data = g_TextBuffer[j];
								g_TextBuffer[j] = 0;
								WD = UTFTextGetWidthEx(ptr);
								g_TextBuffer[j] = data;
								break;
							}
						}
					}

					if(WD > width)
					{
						BYTE *pTemp;
						
						if(ptr == &g_TextBuffer[oldI])
						{
							wStrlen = 1;
							i = oldI+wStrlen-1;
							pTemp = &g_TextBuffer[i+1];
						}
						else
						{
							i = oldI-1;
							data = g_TextBuffer[oldI];
							g_TextBuffer[oldI] = 0;
							wStrlen = strlen((char *)ptr);
							g_TextBuffer[oldI] = data;
							pTemp = &g_TextBuffer[oldI];
						}

						memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
						wTotal += wStrlen;
						g_TextFinalData[wTotal++] = 0;

						ptr = pTemp;
					}
					else
					{
						i = j-1;
						g_TextFinalData[wTotal++] = 0;
					}

					totalRow++;
					iWidth = 0;
				}
			}
		}

		wStrlen = strlen((char *)ptr);
		if(wStrlen)
		{
			memcpy(&g_TextFinalData[wTotal], ptr, wStrlen);
			wTotal += wStrlen;
		}
		
		g_TextFinalData[wTotal] = 0;
		g_TextFinalData[wTotal+1] = 0;

		return totalRow;
	}

	return 0;
}

/***********************************************************************/
static void UTFDVBArabicPreCopyData(LPTEXT lpStart, LPTEXT lpCurPtr)
{
	int iStrlen = strlen((char *)lpCurPtr);
	int i, iRealCode;

	if(iStrlen >= TEMP_BUFFER_SIZE)
	{
		iStrlen = TEMP_BUFFER_SIZE-1;
	}

	for(i=0; i<iStrlen; i++)
	{
		iRealCode = DVBArabicToWinArabic(*lpCurPtr);
		if(iRealCode > 0)
		{
			g_TempCopyData[i] = iRealCode;
		}
		else
		{
			g_TempCopyData[i] = *lpCurPtr;
		}

		lpCurPtr++;
	}

	g_TempCopyData[iStrlen] = 0;
}

/***********************************************************************/
static WORD UTFDVBTextPreDone(LPTEXT pTextSrc, LPTEXT pTextBuffer, WORD bufferSize, LPUTFLOGFONT lpFont)
{
	WORD wRealNum=0;

	while(*pTextSrc)
	{
		if((*pTextSrc < 0x80) || (*pTextSrc > 0x9F))
		{
			wRealNum++;
			*pTextBuffer++ = *pTextSrc;
		}

		pTextSrc++;
	}

	return wRealNum;
}

/***********************************************************************/
void UTFResetTextFunc(LPUTFLOGFONT lpFont, BYTE bForceSetFunc)
{
	BYTE lfCharSetOld = g_TextFont.lfCharSet;
	
	memcpy(&g_TextFont, lpFont, sizeof(UTFLOGFONT));
	if(bForceSetFunc == FALSE)
	{
		if(lfCharSetOld == g_TextFont.lfCharSet)
		{
			return;
		}
	}
	
	switch(g_TextFont.lfCharSet)
	{
	case ANSI_CHARSET_UI:
		g_TextFunc.m_TextPreDone = NULL;
		g_TextFunc.m_CopyToPreDone = UTFENGCopyPreDoneData;
		g_TextFunc.m_AddEndllipse = UTFENGEndllipse;
		g_TextFunc.m_DrawOneLineText = ENGTextFunc;
		g_TextFunc.m_GetTextWidth = ENGTextWidth;
		g_TextFunc.m_DrawText = UTFDrawTextInter;
		g_TextFunc.m_DecodeText = UTFENGDecodeText;
		break;

	case DEFAULT_CHARSET_UI:
		g_TextFunc.m_TextPreDone = NULL;
		g_TextFunc.m_CopyToPreDone = UTFENGCopyPreDoneData;
		g_TextFunc.m_AddEndllipse = UTFENGEndllipse;
		g_TextFunc.m_DrawOneLineText = ENGTextFunc;
		g_TextFunc.m_GetTextWidth = ENGTextWidth;
		g_TextFunc.m_DrawText = UTFDrawTextInter;
		g_TextFunc.m_DecodeText = UTFENGDecodeText;
		break;

	case GREEK_CHARSET_UI:
		g_TextFunc.m_TextPreDone = NULL;
		g_TextFunc.m_CopyToPreDone = UTFENGCopyPreDoneData;
		g_TextFunc.m_AddEndllipse = UTFENGEndllipse;
		g_TextFunc.m_DrawOneLineText = ENGTextFunc;
		g_TextFunc.m_GetTextWidth = ENGTextWidth;
		g_TextFunc.m_DrawText = UTFDrawTextInter;
		g_TextFunc.m_DecodeText = UTFENGDecodeText;
		break;

	case RUSSIAN_CHARSET_UI:
		g_TextFunc.m_CopyToPreDone = UTFENGCopyPreDoneData;
		g_TextFunc.m_AddEndllipse = UTFENGEndllipse;
		g_TextFunc.m_DrawOneLineText = ENGTextFunc;
		g_TextFunc.m_GetTextWidth = ENGTextWidth;
		g_TextFunc.m_DrawText = UTFDrawTextInter;
		g_TextFunc.m_TextPreDone = RUSTextUnicodeToANSI;
		g_TextFunc.m_DecodeText = UTFENGDecodeText;
		break;

	case TURKISH_CHARSET_UI:
		g_TextFunc.m_TextPreDone = NULL;
		g_TextFunc.m_CopyToPreDone = UTFENGCopyPreDoneData;
		g_TextFunc.m_AddEndllipse = UTFENGEndllipse;
		g_TextFunc.m_DrawOneLineText = ENGTextFunc;
		g_TextFunc.m_GetTextWidth = ENGTextWidth;
		g_TextFunc.m_DrawText = UTFDrawTextInter;
		g_TextFunc.m_DecodeText = UTFENGDecodeText;
		break;

	case EASTEUROPE_CHARSET_UI:
		g_TextFunc.m_TextPreDone = NULL;
		g_TextFunc.m_CopyToPreDone = UTFENGCopyPreDoneData;
		g_TextFunc.m_AddEndllipse = UTFENGEndllipse;
		g_TextFunc.m_DrawOneLineText = ENGTextFunc;
		g_TextFunc.m_GetTextWidth = ENGTextWidth;
		g_TextFunc.m_DrawText = UTFDrawTextInter;
		g_TextFunc.m_DecodeText = UTFENGDecodeText;
		break;

	case THAI_CHARSET_UI:
		g_TextFunc.m_TextPreDone = THAITextPreDone;
		g_TextFunc.m_CopyToPreDone = UTFENGCopyPreDoneData;
		g_TextFunc.m_AddEndllipse = UTFThaiEndllipse;
		g_TextFunc.m_DrawOneLineText = THAITextFunc;
		g_TextFunc.m_GetTextWidth = THAITextWidth;
		g_TextFunc.m_DrawText = UTFDrawTextInter;
		g_TextFunc.m_DecodeText = UTFThaiDecodeText;
		break;

	case ARABIC_CHARSET_UI:
		g_TextFunc.m_CopyToPreDone = UTFENGCopyPreDoneData;
		g_TextFunc.m_AddEndllipse = UTFARBEndllipse;
		g_TextFunc.m_DrawOneLineText = ARBTextFunc;
		g_TextFunc.m_GetTextWidth = ARBTextWidth;
		g_TextFunc.m_TextPreDone = ARBTextPreDone;
		g_TextFunc.m_DrawText = UTFDrawTextInter;
		g_TextFunc.m_DecodeText = UTFARBDecodeText;
		break;

	case DVB_ISO_LATIN_CHARSET:
	case DVB_ISO_CYRILLIC_CHARSET:
	case DVB_ISO_GREEK_CHARSET:
	case DVB_ISO_HEBREW_CHARSET:
	case DVB_ISO_LATIN_N5:
	case ISO_8859_1_CHARSET_UI:
	case ISO_8859_2_CHARSET_UI:
	case ISO_8859_3_CHARSET_UI:
	case ISO_8859_4_CHARSET_UI:
	case ISO_8859_10_CHARSET_UI:
	case ISO_8859_13_CHARSET_UI:
	case ISO_8859_14_CHARSET_UI:
	case ISO_8859_15_CHARSET_UI:
		g_TextFunc.m_TextPreDone = UTFDVBTextPreDone;
		g_TextFunc.m_CopyToPreDone = UTFENGCopyPreDoneData;
		g_TextFunc.m_AddEndllipse = UTFENGEndllipse;
		g_TextFunc.m_DrawOneLineText = ENGTextFunc;
		g_TextFunc.m_GetTextWidth = ENGTextWidth;
		g_TextFunc.m_DrawText = UTFDrawTextInter;
		g_TextFunc.m_DecodeText = UTFENGDecodeText;
		break;

	case DVB_ISO_ARABIC_CHARSET:
		g_TextFunc.m_CopyToPreDone = UTFDVBArabicPreCopyData;
		g_TextFunc.m_AddEndllipse = UTFARBEndllipse;
		g_TextFunc.m_DrawOneLineText = ARBTextFunc;
		g_TextFunc.m_GetTextWidth = ARBTextWidth;
		g_TextFunc.m_TextPreDone = ARBTextPreDone;
		g_TextFunc.m_DrawText = UTFDrawTextInter;
		g_TextFunc.m_DecodeText = UTFARBDecodeText;
		break;

	case CHINESEBIG5_CHARSET_UI:
		if((g_TextFont.lfLogFontEx == NULL) || ((g_TextFont.lfAddr1 == NULL) && (g_TextFont.lfAddr2 == NULL)))
		{
			g_TextFunc.m_DrawOneLineText = NULL;
			g_TextFunc.m_GetTextWidth = NULL;
			g_TextFunc.m_TextPreDone = NULL;
			g_TextFunc.m_DrawText = NULL;
			g_TextFunc.m_DecodeText = NULL;
			g_TextFunc.m_AddEndllipse = NULL;
			g_TextFunc.m_CopyToPreDone = NULL;
		}
		else
		{
			g_TextFunc.m_TextPreDone = NULL;
			g_TextFunc.m_CopyToPreDone = UTFENGCopyPreDoneData;
			g_TextFunc.m_AddEndllipse = UTFBIG5Endllipse;
			g_TextFunc.m_DrawOneLineText = BIG5TextFunc;
			g_TextFunc.m_GetTextWidth = BIG5TextWidth;
			g_TextFunc.m_DrawText = UTFDrawTextInter;
			g_TextFunc.m_DecodeText = UTFBIG5DecodeText;
			if(g_TextFont.lfFontExTop >= g_TextFont.lfHeight)
			{
				g_TextFont.lfFontExTop = 0;
			}
		}
		break;
		
	case GB2312_CHARSET_UI:
		if((g_TextFont.lfLogFontEx == NULL) || ((g_TextFont.lfAddr1 == NULL) && (g_TextFont.lfAddr2 == NULL)))
		{
			g_TextFunc.m_DrawOneLineText = NULL;
			g_TextFunc.m_GetTextWidth = NULL;
			g_TextFunc.m_TextPreDone = NULL;
			g_TextFunc.m_DrawText = NULL;
			g_TextFunc.m_DecodeText = NULL;
			g_TextFunc.m_AddEndllipse = NULL;
			g_TextFunc.m_CopyToPreDone = NULL;
		}
		else
		{
			g_TextFunc.m_TextPreDone = NULL;
			g_TextFunc.m_CopyToPreDone = UTFENGCopyPreDoneData;
			g_TextFunc.m_AddEndllipse = UTFCHNEndllipse;
			g_TextFunc.m_DrawOneLineText = CHNTextFunc;
			g_TextFunc.m_GetTextWidth = CHNTextWidth;
			g_TextFunc.m_DrawText = UTFDrawTextInter;
			g_TextFunc.m_DecodeText = UTFCHNDecodeText;
			if(g_TextFont.lfFontExTop >= g_TextFont.lfHeight)
			{
				g_TextFont.lfFontExTop = 0;
			}
		}
		break;

	case HINDI_CHARSET_UI:
		if((g_TextFont.lfLogFontEx == NULL) || (g_TextFont.lfAddr1 == NULL))
		{
			g_TextFunc.m_DrawOneLineText = NULL;
			g_TextFunc.m_GetTextWidth = NULL;
			g_TextFunc.m_TextPreDone = NULL;
			g_TextFunc.m_DrawText = NULL;
			g_TextFunc.m_DecodeText = NULL;
			g_TextFunc.m_AddEndllipse = NULL;
			g_TextFunc.m_CopyToPreDone = NULL;
		}
		else
		{
			g_TextFunc.m_CopyToPreDone = UTFHindiCopyPreDoneData;
			g_TextFunc.m_AddEndllipse = UTFHindiEndllipse;
			g_TextFunc.m_DrawOneLineText = HINTextFunc;
			g_TextFunc.m_GetTextWidth = HINTextWidth;
			g_TextFunc.m_DrawText = UTFDrawTextInter;
			g_TextFunc.m_TextPreDone = HINTextPreDone;
			g_TextFunc.m_DecodeText = UTFHindiDecodeText;
			if(g_TextFont.lfFontExTop >= g_TextFont.lfHeight)
			{
				g_TextFont.lfFontExTop = 0;
			}
		}
		break;

	case TAMIL_CHARSET_UI:
		if((g_TextFont.lfLogFontEx == NULL) || (g_TextFont.lfAddr1 == NULL))
		{
			g_TextFunc.m_DrawOneLineText = NULL;
			g_TextFunc.m_GetTextWidth = NULL;
			g_TextFunc.m_TextPreDone = NULL;
			g_TextFunc.m_DrawText = NULL;
			g_TextFunc.m_DecodeText = NULL;
			g_TextFunc.m_AddEndllipse = NULL;
			g_TextFunc.m_CopyToPreDone = NULL;
		}
		else
		{
			g_TextFunc.m_CopyToPreDone = UTFTamilCopyPreDoneData;
			g_TextFunc.m_AddEndllipse = UTFTamilEndllipse;
			g_TextFunc.m_DrawOneLineText = TAMILTextFunc;
			g_TextFunc.m_GetTextWidth = TAMILTextWidth;
			g_TextFunc.m_DrawText = UTFDrawTextInter;
			g_TextFunc.m_TextPreDone = TAMILTextPreDone;
			g_TextFunc.m_DecodeText = UTFTamilDecodeText;
			if(g_TextFont.lfFontExTop >= g_TextFont.lfHeight)
			{
				g_TextFont.lfFontExTop = 0;
			}
		}
		break;

	case MALAYALAM_CHARSET_UI:
		if((g_TextFont.lfLogFontEx == NULL) || (g_TextFont.lfAddr1 == NULL))
		{
			g_TextFunc.m_DrawOneLineText = NULL;
			g_TextFunc.m_GetTextWidth = NULL;
			g_TextFunc.m_TextPreDone = NULL;
			g_TextFunc.m_DrawText = NULL;
			g_TextFunc.m_DecodeText = NULL;
			g_TextFunc.m_AddEndllipse = NULL;
			g_TextFunc.m_CopyToPreDone = NULL;
		}
		else
		{
			g_TextFunc.m_CopyToPreDone = UTFMalayalamCopyPreDoneData;
			g_TextFunc.m_AddEndllipse = UTFMalayalamEndllipse;
			g_TextFunc.m_DrawOneLineText = MALAYATextFunc;
			g_TextFunc.m_GetTextWidth = MALAYATextWidth;
			g_TextFunc.m_DrawText = UTFDrawTextInter;
			g_TextFunc.m_TextPreDone = MALAYATextPreDone;
			g_TextFunc.m_DecodeText = UTFMalayalamDecodeText;
			if(g_TextFont.lfFontExTop >= g_TextFont.lfHeight)
			{
				g_TextFont.lfFontExTop = 0;
			}
		}
		break;

	case KANNADA_CHARSET_UI:
		if((g_TextFont.lfLogFontEx == NULL) || (g_TextFont.lfAddr1 == NULL))
		{
			g_TextFunc.m_DrawOneLineText = NULL;
			g_TextFunc.m_GetTextWidth = NULL;
			g_TextFunc.m_TextPreDone = NULL;
			g_TextFunc.m_DrawText = NULL;
			g_TextFunc.m_DecodeText = NULL;
			g_TextFunc.m_AddEndllipse = NULL;
			g_TextFunc.m_CopyToPreDone = NULL;
		}
		else
		{
			g_TextFunc.m_CopyToPreDone = UTFKannadaCopyPreDoneData;
			g_TextFunc.m_AddEndllipse = UTFKannadaEndllipse;
			g_TextFunc.m_DrawOneLineText = KANADATextFunc;
			g_TextFunc.m_GetTextWidth = KANADATextWidth;
			g_TextFunc.m_DrawText = UTFDrawTextInter;
			g_TextFunc.m_TextPreDone = KANADATextPreDone;
			g_TextFunc.m_DecodeText = UTFKannadaDecodeText;
			if(g_TextFont.lfFontExTop >= g_TextFont.lfHeight)
			{
				g_TextFont.lfFontExTop = 0;
			}
		}
		break;

	default:
		g_TextFunc.m_AddEndllipse = NULL;
		g_TextFunc.m_CopyToPreDone = NULL;
		g_TextFunc.m_DrawOneLineText = NULL;
		g_TextFunc.m_GetTextWidth = NULL;
		g_TextFunc.m_TextPreDone = NULL;
		g_TextFunc.m_DrawText = NULL;
		g_TextFunc.m_DecodeText = NULL;
		break;
	}
}

