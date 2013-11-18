
#include "UTFJpegDC.h"
#include "UTFJpegCommon.h"
#include "UTFDCImage.h"
#include "UTFJpegDecode.h"

void UTFToStopDecoding()
{
	UTF_JPEG_StopDecoding();
}

unsigned long UTFLoadThumb( StorageType *srcP, ImageType *imgP, long w, long h, unsigned long flags, EnvType *envP )
{
	Result res = resNull;
	JpegInfoType jpgI;
	void * jpgP = 0; 
	OneStepCallbackPrc *prgP = 0;
	UInt32 sf = 0, reducePrcX, reducePrcY;

	if( !srcP || !imgP ) return 0;

	if( (flags & DCIM_PROGRESS) != DCIM_PROGRESS ) 
	{
		prgP = envP->onestepP;
		envP->onestepP = 0L;
	}

	if( resOk == (res = UTF_JPEG_Open( &jpgP, srcP, envP )) )
	{
		UTF_JPEG_GetInfo( jpgP, &jpgI, envP ); 
		if( jpgI.width <= w && jpgI.height <= h )
		{
			sf = 0;
		}
		else // fit to ...
		{
			reducePrcX = (jpgI.width << 8) / w;
			reducePrcY = (jpgI.height << 8) / h;
			reducePrcX = (reducePrcX > reducePrcY)? reducePrcX : reducePrcY;
			for( sf = 1; sf < 4; ++ sf ) if( (reducePrcX >> sf) < 256 ) break;
			-- sf; 
		}
		res = UTF_JPEG_GetThumbnail( jpgP, imgP, w, h, flags, envP ); 
		UTF_JPEG_Close( &jpgP, envP );
	}

	if( !envP->onestepP ) envP->onestepP = prgP;

	return res;
}

unsigned long UTFLoadJpeg( StorageType *srcP, ImageType *imgP, long w, long h, unsigned long flags, EnvType *envP )
{
	Result res = resNull;
	JpegInfoType jpgI;
	void * jpgP = 0;
	OneStepCallbackPrc *prgP = 0;
	UInt32 sf = 0, reducePrcX, reducePrcY;

	if( !srcP || !imgP ) return 0;

	if( (flags & DCIM_PROGRESS) != DCIM_PROGRESS ) 
	{
		prgP = envP->onestepP;
		envP->onestepP = 0L;
	}

	if( resOk == (res = UTF_JPEG_Open( &jpgP, srcP, envP )) )
	{
		UTF_JPEG_GetInfo( jpgP, &jpgI, envP ); 
		if( jpgI.width <= w && jpgI.height <= h )
		{
			sf = 0;
		}
		else // fit to ...
		{
			reducePrcX = (jpgI.width << 8) / w;
			reducePrcY = (jpgI.height << 8) / h;
			reducePrcX = (reducePrcX > reducePrcY)? reducePrcX : reducePrcY;
			if( (flags & DCIM_QUALITYZOOM) == DCIM_QUALITYZOOM )
			{
				for( sf = 1; sf < 4; ++ sf ) if( (reducePrcX >> sf) < 256 ) break;	// 100%
			}
			else
			{
				for( sf = 1; sf < 4; ++ sf ) if( (reducePrcX >> sf) < 170 ) break;	// 170 ~66%	// 130 ~51%
			}
			-- sf; 
		}
		if( (flags & DCIM_PROGRESS) == DCIM_PROGRESS && (flags & DCIM_NOTHUMB) != DCIM_NOTHUMB )
		{
			if( (flags & DCIM_NOZOOM) == DCIM_NOZOOM )
				UTF_JPEG_GetThumbnail( jpgP, imgP, jpgI.width>>sf, jpgI.height>>sf, flags|DCIM_ZOOMIN, envP ); 
			else
				UTF_JPEG_GetThumbnail( jpgP, imgP, w, h, flags|DCIM_ZOOMIN, envP );
			if( imgP->dataYP ) envP->freeP( imgP->dataYP );
			imgP->dataYP = imgP->dataCbCrP = 0L; imgP->width = imgP->height = imgP->rowByte = 0;
		}
		res = UTF_JPEG_Load( jpgP, imgP, w, h, flags, (UInt16)sf, envP ); 
		UTF_JPEG_Close( &jpgP, envP );
	}

	if( !envP->onestepP ) envP->onestepP = prgP;

	return res; 
}

void PrvWriteWord( UInt8 * trgP, Int16 w )
{
	*trgP++ = (UInt8)(w & 0xff);
	*trgP++ = (UInt8)(w >> 8);
}

void PrvWriteDWord( UInt8 * trgP, Int32 w )
{
	*trgP++ = (UInt8)(w & 0xff);
	*trgP++ = (UInt8)(w >> 8);
	*trgP++ = (UInt8)(w >> 16);
	*trgP++ = (UInt8)(w >> 24);
}

void UTFStoreAsBMP( ImageType * imgP, void ** trgTP, unsigned long * sizeP, EnvType * envP )
{
	UInt32 ii, jj, pos, rl, size;
	UInt8 * trgP = 0, * trgSP;
	UInt8 *srcYP=0, *srcCbCrP=0;
	
	if( !envP || !imgP || !trgTP || !sizeP ) return;

	rl = imgP->width * 3;
	rl = ((rl + 3)>>2)<<2;
	size = imgP->height * rl + 54;//sizeof(WinBitmapType);
	trgSP = trgP = (UInt8*)envP->allocP( size + 16 );
	if( !trgP ) return;

	PrvWriteWord( trgP, 0x4d42 );	trgP += 2;
	PrvWriteDWord( trgP, size );	trgP += 4;
	PrvWriteDWord( trgP, 0 );	trgP += 4;
	PrvWriteDWord( trgP, 54 );	trgP += 4;
	PrvWriteDWord( trgP, 40 );	trgP += 4;
	PrvWriteDWord( trgP, (Int32)imgP->width );	trgP += 4;
	PrvWriteDWord( trgP, -((Int32)imgP->height) );	trgP += 4;
	PrvWriteWord( trgP, 1 );	trgP += 2;
	PrvWriteWord( trgP, 24 );	trgP += 2;
	PrvWriteDWord( trgP, 0 );	trgP += 4;
	PrvWriteDWord( trgP, 0 );	trgP += 4;
	PrvWriteDWord( trgP, 3000 );	trgP += 4;
	PrvWriteDWord( trgP, 3000 );	trgP += 4;
	PrvWriteDWord( trgP, 0 );	trgP += 4;
	PrvWriteDWord( trgP, 0 );	trgP += 4;
	
	srcYP = imgP->dataYP;
	srcCbCrP = imgP->dataCbCrP;
	for( ii = 0; ii < imgP->height; ++ ii )
	{
		UInt8 y, cb, cr;
		int r, g, b;
		for( pos = jj = 0; jj < imgP->width; ++ jj )
		{
			y = *(srcYP + jj);
			if( !(jj & 1) )
			{
				cb = *(srcCbCrP + jj);
				cr = *(srcCbCrP + jj + 1);
			}
			r = (int)((double)y + 1.4 * ((double)cr - 128.));
			g = (int)((double)y - 0.3 * ((double)cb - 128.) - 0.7 * ((double)cr - 128.));
			b = (int)((double)y + 1.7 * ((double)cb - 128.));
			r = ( r < 0 )? 0 : ( r > 255 )? 255 : r;
			g = ( g < 0 )? 0 : ( g > 255 )? 255 : g;
			b = ( b < 0 )? 0 : ( b > 255 )? 255 : b;
			trgP[pos++] = (UInt8)b;
			trgP[pos++] = (UInt8)g;
			trgP[pos++] = (UInt8)r;
		}
		trgP += rl;
		srcYP += imgP->rowByte;
		if( ii&1 )
		{
			srcCbCrP += imgP->rowByte;
		}
	}
	
	*trgTP = (void*)trgSP;
	*sizeP = size;

	return;
}