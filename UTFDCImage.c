
#include "UTFJpegDC.h"
#include "UTFJpegCommon.h"
#include "UTFJpegDecode.h"
#include "UTFDCImage.h"

void PrvRotate90CW_08( UInt8 *srcIP, UInt8 *trgIP, UInt32 w, UInt32 h, UInt32 srl, UInt32 trl );
void PrvRotate90CW_16( UInt16 *srcIP, UInt16 *trgIP, UInt32 w, UInt32 h, UInt32 srl, UInt32 trl );
void PrvRotate90CCW_08( UInt8 *srcIP, UInt8 *trgIP, UInt32 w, UInt32 h, UInt32 srl, UInt32 trl );
void PrvRotate90CCW_16( UInt16 *srcIP, UInt16 *trgIP, UInt32 w, UInt32 h, UInt32 srl, UInt32 trl );
void PrvRotate180_08( UInt8 *srcIP, UInt8 *trgIP, UInt32 w, UInt32 h, UInt32 srl, UInt32 trl );
void PrvRotate180_16( UInt16 *srcIP, UInt16 *trgIP, UInt32 w, UInt32 h, UInt32 srl, UInt32 trl );

Result UTF_DCIM_CreateImage( ImageType * imageP, unsigned short width, unsigned short height, EnvType * envP )
{
	Result res = resOk;
	UInt32 rowLen, colLen, size, sizeA;
	UInt8 * imgP = 0;

	//
	if( !envP || !width || !height ) return resNull;
	//
	MSet4( imageP, 0, sizeof(ImageType)>>2 );
	//
	rowLen = Round4( width ); 
	colLen = Round2( height );
	//
	imageP->width = width;
	imageP->height = height;
	imageP->rowByte = (UInt16)rowLen;
	//
	size = Round2( rowLen * colLen );		
	sizeA = Round16( size + (size>>1) );
	imgP = (UInt8*)envP->allocP( sizeA );
	if( !imgP ) { res = resNoMem; goto errExitL; }
	MSet4( imgP, 0, sizeA>>2 );
	imageP->dataYP = imgP;
	imageP->dataCbCrP = imgP + size;
	
	return res;
	
errExitL:	
	
	if( imageP->dataYP ) envP->freeP( imageP->dataYP );
	
	return res;
}

Result UTF_DCIM_KillImage( ImageType * imageP, EnvType * envP )
{
	// check
	if( !imageP || !envP ) return resNull;
	// 
	if( imageP->dataYP ) envP->freeP( imageP->dataYP );
	//
	MSet4( imageP, 0, sizeof(ImageType)>>2 );

	return resOk;
}

Boolean UTF_DCIM_IsImageCreated( ImageType * imageP )
{
	return (Boolean)((Int32)(imageP->width) > 0 && (Int32)(imageP->height) > 0);
}
