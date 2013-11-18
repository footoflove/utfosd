#include "stbcfg.h"
#include <stdio.h>
#include <string.h>
#ifdef OPENTV_12
#include "opentv_12.h"
#else
#include "basetype.h"
#endif
/*#include "kal.h"
#include "retcodes.h"
#include "trace.h"
#include "osdlib.h"
#include "vidlib.h"
#include "gfxlib.h"
#include "gfxtypes.h"
#include "globals.h"
*/
#include "UTFTypeDef.h"
#include "UTFJpegDC.h"
#include "UTFJpegCommon.h"
#include "UTFDCImage.h"
#include "UTFJpegDecode.h"


/************************************************************************************/
// for allocation
static void * PrvAllocMem( unsigned long size )
{
    return (void *)OS_Malloc( (DWORD)size );
}

// for deleting
static void PrvFreeMem( void * memP )
{
    OS_Free(memP);
}

/************************************************************************************/
// It is an example! A set of functions for decoding from a source (.jpeg file) which is in memory
//	The decoder causes these functions so:
//
//		int size = storageP->getsizeP( storageP->dataP );	
// 
//		if( size < JpegSegmentSize )
//			storageP->resizeP( storageP->dataP, JpegSegmentSize );	// JpegSegmentSize no more 65K
//
//		BYTE *bufP = storageP->refreshP( storageP->dataP, 0, 2, &err );	// bufP is a pointer on begin of the .jpeg data (FFD8)
//
//
// return size of internal buffer; because .jpeg file in memory, return very big number. 
// The decoder will consider, that the buffer already contains all data
static unsigned long PrvGetSize( void * p )
{
    return 0xfffffffe;
}

// The decoder call this function to change the size of the internal buffer.
// That is to specify the maximal size (s) of data (p), which will be processed.
// As data already are in memory, this function does not carry out any actions. 
// Function should return 1 if it was possible to change successfully the size of the internal buffer, or 0 differently
static unsigned long PrvResize( void * p, unsigned long s )
{
    return 1;
}

// It is the main function. The decoder call it to update data in the internal buffer. 
// The decoder specifies a position from the beginning of a .jpeg file (pos) and the size (size) data, who are necessary for decompression
// Function should return the pointer on data and establish a code of result (err) in 1, if everything is all right (differently 0)
static unsigned char * PrvRefresh( void * p, unsigned long pos, unsigned long size, unsigned long * err)
{
    if( err ) *err = 1;
    return (unsigned char*)p + pos;
}





bool UTF_jpeg_decode(char *jpg,long imgsize, long rotate, void** bmpP)
{
	EnvType env;
	StorageType str;
	ImageType img;
	char * bufP;
//	void * bmpP = 0;
	unsigned long size = 0;
	int filesize;

	if(jpg==NULL)
		return 0;

//	bufP = (char*)OS_Malloc(imgsize);
//	if( bufP )
	{
		// read from file
		//fread( bufP, 1, filesize, f );
		//fclose( f );
//		memcpy(bufP,jpg,imgsize);
		// prepare JpegDecoder: memory alloc/free functions
		env.allocP = PrvAllocMem;
		env.freeP = PrvFreeMem;
		env.onestepP = 0; // don’t use progress
		// prepare JpegDecoder: read from memory functions
		str.dataP =jpg;// bufP;
		str.getsizeP = PrvGetSize;
		str.refreshP = PrvRefresh;
		str.resizeP = PrvResize;
		// clear output...
		img.dataYP = img.dataCbCrP = 0L;
		img.width = img.height = img.rowByte = 0;
		// JpegDecoder: load jpeg
		if( UTFLoadJpeg( &str, &img, 720, 576, 0, &env ) == 1 )
		{
			UTFStoreAsBMP( &img, bmpP, &size, &env );
			if( bmpP )
			{
			//	trace("\n  bmp is   ok  \n");
			}
			// free mem!
			if( img.dataYP ) 
				OS_Free( img.dataYP );
		}
		// free mem!
//		OS_Free( bufP);
	}
		// close file
	return 1;
}

BYTE bmp_16_img[720*576*2+32];

int change24bitmapto16bitmap (BYTE *bmpFile,int xpos,int ypos,int right,int bottom) 
{ 
	DWORD nWidth,nHeight;
	BYTE *pData,*pBmpDataStart;
	DWORD size;
	int bmWidthBytes;
	int bmBitsPixel;
	int nBit;
	int z; 
	WORD rgb1555;
	BYTE red1,green1,blue1;
	int y,x,flag_sort=0;
	WORD *ptemp_16_img;
	WORD   biBitCount=24; 
	DWORD Offsize=0;


	if(NULL==bmpFile)
	{
		return 0;
	}
	pBmpDataStart=bmpFile+10;
	Offsize=*pBmpDataStart|(*(pBmpDataStart+1)<<8)|(*(pBmpDataStart+2)<<16)|(*(pBmpDataStart+3)<<24);
	pData=bmpFile+Offsize;
	pBmpDataStart=bmpFile+28;
	biBitCount=*pBmpDataStart|(*(pBmpDataStart+1)<<8);
	

	bmpFile+=18;
	nWidth=*bmpFile|(*(bmpFile+1)<<8)|(*(bmpFile+2)<<16)|(*(bmpFile+3)<<24);
	bmpFile+=4;
	nHeight=*bmpFile|(*(bmpFile+1)<<8)|(*(bmpFile+2)<<16)|(*(bmpFile+3)<<24);
	if(nHeight&0x80000000)
	{
		nHeight=0xFFFFFFFF-nHeight+1;
		flag_sort=1;
	}

	if(nWidth % 4 !=0) 
	nWidth=nWidth+ (4-nWidth % 4); //修正位图宽度值 
	
	size=nWidth*biBitCount/8*nHeight; //nWidth*bih.biBitCount/8*nHeight; 
	if(nWidth>720||nHeight>576)
		return 0;

	bmWidthBytes=nWidth*biBitCount /8; 
	bmBitsPixel=biBitCount ; 
	nBit=bmBitsPixel/8; 
	memset(bmp_16_img,0,sizeof(bmp_16_img));
	ptemp_16_img=(WORD*)bmp_16_img;
		
	for ( y=0;y<nHeight;y++) 
	{ 
		for ( x=0;x<nWidth;x++) 
		{ 
			if(flag_sort==0)
				z=nHeight-y-1; 
			else
				z=y;

			red1		= pData[x*nBit+2+z*bmWidthBytes];
			green1		= pData[x*nBit+1+z*bmWidthBytes];
			blue1		= pData[x*nBit+z*bmWidthBytes];
			rgb1555=UTFRGBAToBpp16_5551(red1,green1,blue1,0xFF);
			*ptemp_16_img=rgb1555;
			ptemp_16_img++;

		} 
	} 

//	if((xpos+nWidth)>720||(ypos+nHeight)>576)
//		return 0;
	UTFDrawBmp1555(bmp_16_img,nWidth,nHeight,xpos,ypos,xpos+nWidth,ypos+nHeight);
	return 1; 
}


