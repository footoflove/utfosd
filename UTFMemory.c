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
#include "UTFMemory.h"
#include "UTFMemoryPriv.h"

/***********************************************************************/
//define memory size and usage

#define BLOCK_SIZE		384
#define BLOCK_NUM		(535*2)
#define TOTAL_MEMSIZE	(BLOCK_SIZE * BLOCK_NUM)	// 200 KBytes

//data define
static const DWORD MEM_SIZE = TOTAL_MEMSIZE;
static DWORD UTFMemUsage[BLOCK_NUM];
static BYTE UTFMemSpace[TOTAL_MEMSIZE];

/***********************************************************************/
DWORD UTFMalloc(DWORD size)
{
	int blockCount;
	int i,j;
	//trace ("\n  UTF UTFMalloc  size : %x \n",size);
	if((size == 0) || (size > MEM_SIZE))
		return NULL;

	blockCount = size/BLOCK_SIZE;
	if(size % BLOCK_SIZE)
	{
		blockCount++;
	}

	for(i=0; i<BLOCK_NUM; i++)
	{
		if(UTFMemUsage[i] == 0)
		{
			int bFree = TRUE;

			if((i+blockCount) > BLOCK_NUM)
				break;

			for(j=1; j<blockCount; j++)
			{
				if(UTFMemUsage[i+j] != 0)
				{
					i += j;
					bFree = FALSE;
					break;
				}
			}

			if(bFree == TRUE)
			{
				DWORD address = (DWORD)&UTFMemSpace[i*BLOCK_SIZE];

				memset((void *)address, 0, blockCount*BLOCK_SIZE);
				for(j=0; j<blockCount; j++)
				{
					UTFMemUsage[i+j] = address;
				}

				return address;
			}
		}
	}

	return NULL;
}

void UTFFree(DWORD address)
{
	int i,bFound=FALSE;

	if(address == NULL)
		return;

	for(i=0; i<BLOCK_NUM; i++)
	{
		if(UTFMemUsage[i] == address)
		{
			UTFMemUsage[i] = 0;
			bFound = TRUE;
		}
		else if(bFound == TRUE)
		{
			break;
		}
	}
}

void UTFMemoryInit(void)
{
	memset((void *)UTFMemUsage, 0, sizeof(UTFMemUsage));
	memset((void *)UTFMemSpace, 0, sizeof(UTFMemSpace));
}

/*********************************************************************************/
/*    these source code for memory operate need by utfosd internal  **************/
/*********************************************************************************/
/* Function UTFMemCpyWORD use to copy two byte memory data to WORD type variable */
void UTFMemCpyWORD(WORD *dest, void *src, DWORD totalbyte)
{
	BYTE *ptr = (BYTE *)src;

	*dest = *(ptr+1);
	*dest <<= 8;
	*dest += *ptr;
}

/***********************************************************************/

