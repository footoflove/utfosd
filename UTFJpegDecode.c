#include "UTFJpegDC.h"
#include "UTFJpegCommon.h"
#include "UTFDCImage.h"
#include "UTFJpegDecode.h"

/////////////////////////////////////////////
//
volatile UInt32	g_decodingState = 0;	

// Funcs //////////////////////////////////////////////////////////////////////////////////////////
//
void	PrvQuickZoom8( void* jpgP, UInt8 * srcP, UInt8 * trgP, UInt32 srcRL, UInt32 srcLim, UInt8 );
void	PrvQuickZoom16( void* jpgP, UInt8 * srcCbP, UInt8 * srcCrP, UInt8 * trgP, UInt32 srcRL, UInt32 srcLim, UInt32, UInt8 );
void	PrvZoomCalcFactorAW( UInt32 src, UInt32 trg, UInt32 * scaleFactorP, UInt32 *sizeP );
void	PrvZoomClose( JPEG_FormatType *jpgP, EnvType *envP );
Result	PrvZoomPrepare( JPEG_FormatType * jpgP, UInt32 srcW, UInt32 srcH, UInt32 trgW, UInt32 trgH, UInt8 zoomIn, UInt8 zoomNo, UInt32 rotateAng, EnvType *envP );
//
Result 	Cmpl_SingleScan1( JPEG_FormatType * jpgP, ImageType * imageP, UInt8 * duLineT[], UInt32 linePos, UInt32 lineNum, UInt32 lineSize[] );
Result 	Cmpl_SingleScan3( JPEG_FormatType * jpgP, ImageType * imageP, UInt8 * duLineT[], UInt32 linePos, UInt32 lineNum, UInt32 lineSize[] );
Result 	Cmpl_SingleScan4( JPEG_FormatType * jpgP, ImageType * imageP, UInt8 * duLineT[], UInt32 linePos, UInt32 lineNum, UInt32 lineSize[] );
//
Result 	IDCT_QuickDecode8x8( Int32 * srcP, UInt8 * trgP,  UInt8 * nzNumVec, UInt32 len );
Result 	IDCT_QuickDecode4x4( Int32 * srcP, UInt8 * trgP,  UInt8 * nzNumVec, UInt32 len );
Result 	IDCT_QuickDecode2x2( Int32 * srcP, UInt8 * trgP,  UInt8 * nzNumVec, UInt32 len );
Result 	IDCT_QuickDecodeThumb( Int32 * srcP, UInt8 * trgP,  UInt8 * nzNumVec, UInt32 len );
//
UInt8 	LL_ReadByte( UInt8 * srcP );
UInt16 	LL_ReadWord( UInt8 * srcP );
UInt32 	LL_ReadDWord( UInt8 * srcP );
//
UInt16 	ReadBlockLen( UInt8 * bufP );
Result 	PrvCreateHufTree( JPEG_HufTreeType *hufTreeP, JPEG_HufTableType *hufTableP );
Result 	ReadAPP0Block( UInt8 * bufP, JPEG_FormatPtr jpgP, UInt32 );
Result	ReadAPP0xBlock( UInt8 * bufP, JPEG_FormatPtr jpgP, UInt32 idx );
Result  PrvGetIFDTags( UInt8 * SegmentStartP, UInt32 IFDOffset, UInt16 ExifInfoSize, Boolean IsIntelByteAlign, Boolean IsSubIFD, JPEG_ThumbPtr infoP );
Result  PrvAnalizeExifTag( UInt16 Tag, UInt32 ComponentNumber, UInt32 Data, UInt8 * SegmentStartP, UInt16 ExifInfoSize, Boolean IsIntelByteAlign, Boolean IsSubIFD, JPEG_ThumbPtr infoP );
Result  ReadAPP1Block( UInt8 * bufP, JPEG_FormatType * jpgP, UInt32 idxA );
Result 	ReadSOF0Block( UInt8 * bufP, JPEG_FormatType * jpgP );
Result 	ReadSOSBlock( UInt8 * bufP, JPEG_FormatType * jpgP );
Result 	ReadDHTBlock( UInt8 * bufP, JPEG_FormatType * jpgP, EnvType * envP );
Result 	ReadDQTBlock( UInt8 * bufP, JPEG_FormatType * jpgP );
Result 	ReadDRIBlock( UInt8 * bufP, JPEG_FormatType * jpgP );
//
Result 	JPEG_JpegScan( JPEG_FormatType * jpgP, StorageType * storageP, EnvType * envP );
//
UInt32	PrvDeHufNextUnit( UInt32 hufLen, UInt32 unitHi, UInt32 *hufCodeM );
Result	PrvJpegDecodeBaselineScan( JPEG_FormatType * jpgP, ImageType * imageP, UInt32 scaleFactor, CopyProcType * PrvCopyProc, OneStepCallbackPrc *OneStepProc, EnvType *envP );
Result	PrvLoadThumb( JPEG_FormatType *jpgP, ImageType *imageP, UInt32 w, UInt32 h, UInt32 flag, EnvType *envP );

////////////////////////////////////////////////////////
//

UInt32 decodingState;			

////////////////////////////////////////////////////////
//
#define Rounding 		0x8080 //32896
#define Norm( x )		(UInt8)(((x)<0x100)?0:((x)>0xff00)?0xff:(x)>>8)
#define MUL_ISEC2(x)	((x)>>1)
#define MUL_ISEC6(x)	(x)
//#define MUL_IC4(x)		(x)
#define MUL_IC4(x)		((x>>1) + (x>>2))
//#define MUL_ISEC6(x)	(x + (x>>1) - (x>>2))

Result IDCT_QuickDecodeThumb( Int32 *srcP, UInt8 *trgP,  UInt8 *t0, UInt32 t1 )
{
	register Int32 a0;
	
	a0 = (*srcP + Rounding);
	*trgP = Norm( a0 );
		
	return resOk;
}

Result IDCT_QuickDecode2x2( Int32 *srcP, UInt8 *trgP,  UInt8 *t0, UInt32 len )
{
	register Int32 a0;
	register Int32 b0, b1;
	register Int32 c0;
	Int32 *tmpP = srcP, *tmpC = srcP;

	//
	a0 = MUL_ISEC2(*(srcP + 1));
	b0 = *srcP;
	a0 = a0 + a0 + a0;
	*tmpP = b0 + a0;
	*(tmpP + 1) = b0 - a0;
	a0 = MUL_ISEC2(*(srcP + 9));
	b0 = *(srcP + 8);
	a0 = a0 + a0 + a0;
	*(tmpP + 2) = b0 + a0;
	*(tmpP + 3) = b0 - a0;

	//
	c0 = *tmpC + Rounding;										
	b0 = *(tmpC + 2);
	a0 = MUL_ISEC2(b0);															
	a0 = b0 + a0 + a0;	
	b0 = (c0 + a0); 
	b1 = (c0 - a0); 
	*trgP = Norm( b0 );
	*(trgP + len) = Norm( b1 );
	c0 = *(tmpC + 1) + Rounding;										
	b0 = *(tmpC + 3);
	a0 = MUL_ISEC2(b0);															
	a0 = b0 + a0 + a0;	
	b0 = c0 + a0; 
	b1 = c0 - a0; 
	*(trgP + 1) = Norm( b0 );
	*(trgP + len + 1) = Norm( b1 );

	return resOk;
}

Result IDCT_QuickDecode4x4( Int32 *srcP, UInt8 *trgP,  UInt8 *nzNumVec, UInt32 len )
{
	register Int32 a0, a1, a2, a3;
	register Int32 b0, b1, b2, b3;
	register Int32 c0, c1, c2, c3;
	register UInt32 i, pos;
	Int32 * tmpP = srcP, * tmpC = srcP;

	for( i = 4; i; -- i, tmpP += 4, srcP += 8, ++ nzNumVec )
	{
		if( * nzNumVec <= 1 )
		{
			*tmpP = *(tmpP + 1) = *(tmpP + 2) = *(tmpP + 3) = *srcP;
		}
		else
		{
			a0 = *srcP;							
			a2 = *(srcP + 2);					
			b0 = a0;
			b2 = MUL_IC4( a2 );
			b3 = a2;
			a0 = b0 + b2 + b3;	
			a1 = b0 + b2;		
			a2 = b0 - b2;					
			a3 = b0 - b2 - b3;
			b0 = *(srcP + 1);
			b2 = *(srcP + 3);
			c0 = a0;
			c1 = a2;
			c2 = a3;
			c3 = a1;
			a1 = b0 - b2;
			a2 = - b2;
			a3 = b0 + b2;
			a0 = MUL_ISEC2( b0 );
			a2 = MUL_ISEC6( a2 );
			b0 = a0 + a2;
			b2 = a0 - a2;					
			a0 = MUL_IC4( b0 );
			a1 = MUL_IC4( a1 );			
			b1 = a1;	
			b2 = a0 + b2;
			a1 = a0 + b1;
			a2 = b1 + b2;			
			a3 = b2 + a3;	
			*tmpP = c0 + a3;
			*(tmpP + 1) = c1 + a1;
			*(tmpP + 2) = c2 - a0;
			*(tmpP + 3) = c3 - a2;
		}
	}

	tmpP = tmpC;
	for( i = 0; i < 4; ++ i, ++ trgP, ++ tmpP )
	{
		a0 = *tmpP;										
		a2 = *(tmpP + 8);		
		b0 = a0 + Rounding;									
		b2 = MUL_IC4( a2 );
		b3 = a2;
		a0 = b0 + b2 + b3;
		a1 = b0 + b2;				
		a2 = b0 - b2;			
		a3 = b0 - b2 - b3;
		c0 = a0;
		c1 = a2;
		c2 = a3;
		c3 = a1;
		b0 = *(tmpP + 4);
		b2 = *(tmpP + 12);		
		a1 = b0 - b2;
		a2 = - b2;
		a3 = b0 + b2;
		a0 = MUL_ISEC2( b0 );
		a2 = MUL_ISEC6( a2 );
		b0 = a0 + a2;
		b2 = a0 - a2;					
		a0 = MUL_IC4( b0 );		
		a1 = MUL_IC4( a1 );			
		b1 = a1;
		b2 = a0 + b2;
		a1 = a0 + b1;				
		a2 = b1 + b2;			
		a3 = b2 + a3;	
		b0 = (c0 + a3);
		b1 = (c1 + a1);
		b2 = (c2 - a0);
		b3 = (c3 - a2);
		*trgP = Norm( b0 ); pos = len;
		*(trgP + pos) = Norm( b1 ); pos += len;
		*(trgP + pos) = Norm( b2 ); pos += len;
		*(trgP + pos) = Norm( b3 );
	}

	return resOk;
}

Result IDCT_QuickDecode8x8( Int32 *srcP, UInt8 *trgP,  UInt8 *nzNumVec, UInt32 len )
{
	register Int32 a0, a1, a2, a3;
	register Int32 b0, b1, b2, b3;
	register Int32 c0, c1, c2, c3;
	register UInt32 i, pos;
	Int32 * tmpP = srcP, * tmpC = srcP;

	for( i = 8; i; -- i, tmpP += 8, srcP += 8, ++ nzNumVec )
	{
		if( * nzNumVec <= 1 )
		{
			*(tmpP + 1) = *(tmpP + 2) = *(tmpP + 3) = *(tmpP + 4) = *(tmpP + 5) = *(tmpP + 6) = *(tmpP + 7) = *srcP;
		}
		else
		{
			a0 = *srcP;
			a1 = *(srcP + 4);
			a2 = *(srcP + 2);
			a3 = *(srcP + 6);
			b2 = a2 - a3;
			b3 = a2 + a3;
			b0 = a0 + a1;		
			b1 = a0 - a1;		
			b2 = MUL_IC4( b2 );
			a0 = b0 + b2 + b3;
			a1 = b1 + b2;	
			a2 = b1 - b2;			
			a3 = b0 - b2 - b3;
			b0 = *(srcP + 1);	
			b1 = *(srcP + 5);	
			b2 = *(srcP + 3);		
			b3 = *(srcP + 7);		
			c0 = a0;	
			c1 = a1;		
			c2 = a2;			
			c3 = a3;
			a0 = b0 - b3;	
			a1 = b0 + b3 - b1 - b2;	
			a2 = b1 - b2;			
			a3 = b0 + b3 + b1 + b2;
			a0 = MUL_ISEC2( a0 );	
			a2 = MUL_ISEC6( a2 );
			b0 = a0 + a2;		
			b2 = a0 - a2;					
			a0 = MUL_IC4( b0 );	
			a1 = MUL_IC4( a1 );			
			b1 = a1;	
			b2 = a0 + b2;
			a1 = a0 + b1;		
			a2 = b1 + b2;		
			a3 = b2 + a3;	
			*tmpP = c0 + a3;
			*(tmpP + 1) = c1 + a2;
			*(tmpP + 2) = c2 + a1;
			*(tmpP + 3) = c3 + a0;
			*(tmpP + 4) = c3 - a0;
			*(tmpP + 5) = c2 - a1;
			*(tmpP + 6) = c1 - a2;
			*(tmpP + 7) = c0 - a3;
		}
	}

	tmpP = tmpC;
	for( i = 0; i < 8; ++ i, ++ trgP, ++ tmpP )
	{
		a0 = *tmpP;				
		a1 = *(tmpP + 32);				
		a2 = *(tmpP + 16);			
		a3 = *(tmpP + 48);
		b2 = a2 - a3;			
		b3 = a2 + a3;
		b0 = a0 + a1 + Rounding;	
		b1 = a0 - a1 + Rounding;	
		b2 = MUL_IC4( b2 );
		a0 = b0 + b2 + b3;			
		a1 = b1 + b2;				
		a2 = b1 - b2;			
		a3 = b0 - b2 - b3;
		c0 = a0;					
		c1 = a1;					
		c2 = a2;				
		c3 = a3;
		b0 = *(tmpP + 8);				
		b1 = *(tmpP + 40);				
		b2 = *(tmpP + 24);			
		b3 = *(tmpP + 56);
		a0 = b0 - b3;				
		a1 = b0 + b3 - b1 - b2;		
		a2 = b1 - b2;			
		a3 = b0 + b3 + b1 + b2;
		a0 = MUL_ISEC2( a0 );								
		a2 = MUL_ISEC6( a2 );
		b0 = a0 + a2;										
		b2 = a0 - a2;					
		a0 = MUL_IC4(b0);			
		a1 = MUL_IC4( a1 );			
		b1 = a1;
		b2 = a0 + b2;
		a1 = a0 + b1;				
		a2 = b1 + b2;			
		a3 = b2 + a3;	
		b0 = (c0 + a3);
		b1 = (c1 + a2);
		b2 = (c2 + a1);
		b3 = (c3 + a0);
		a0 = (c3 - a0);
		a1 = (c2 - a1);
		a2 = (c1 - a2);
		a3 = (c0 - a3);
		*trgP = Norm( b0 ); pos = len; 	
		*(trgP + pos) = Norm( b1 ); pos += len; 	
		*(trgP + pos) = Norm( b2 ); pos += len; 	
		*(trgP + pos) = Norm( b3 ); pos += len; 	
		*(trgP + pos) = Norm( a0 ); pos += len; 	
		*(trgP + pos) = Norm( a1 ); pos += len; 	
		*(trgP + pos) = Norm( a2 ); pos += len; 	
		*(trgP + pos) = Norm( a3 );				
	}

	return resOk;
}

#undef Rounding
#undef Norm
#undef MUL_IC4
#undef MUL_ISEC2
#undef MUL_ISEC6

//////////////////////////////////////////////////////////////////////////////
//

#define singleScaleFactor 		1024 // не делай меньше, тогда не хватает точности для T3
#define singleScaleFactor2x		10 

void PrvQuickZoom8( void * jpgIP, UInt8 * srcP, UInt8 * trgP, UInt32 srcRL, UInt32 srcLim, UInt8 colorSpace )
{
	UInt32 i, j, w, h, tl, tc, sh, line, prevline;
	JPEG_FormatType *jpgP = (JPEG_FormatType*)jpgIP;
	UInt32 lineOffCount, srcOffLine, trgOffLine;	
	UInt16 *colIP;
	UInt8 *srcPt, *trgPt;
	
	h = jpgP->imgSIN;
	w = jpgP->imgSJN;
	tl = jpgP->imgSTOLD;
	tc = jpgP->imgSTOCD;
	sh = jpgP->imgSH;
	srcPt = srcP;
	srcOffLine = line = 0;
	lineOffCount = jpgP->imgSNT;	
	trgOffLine = jpgP->imgSPT;
	for( i = jpgP->imgSIT; i < h; ++ i )
	{
		prevline = line;
		line = lineOffCount >> singleScaleFactor2x;
		if( line >= srcLim ) 
		{
			jpgP->imgSPT = trgOffLine;
			jpgP->imgSNT = lineOffCount - (srcLim << singleScaleFactor2x);
			jpgP->imgSIT = i;
			return;
		}
		lineOffCount += sh;

		if( prevline != line ) 
		{
			srcOffLine = line * srcRL; 
			srcPt = srcP + srcOffLine;
		}
		colIP = jpgP->imgColY;
		trgPt = trgP + trgOffLine;
		trgOffLine += tl;
	
		if( colorSpace == asYCbCr )
		{
			for( j = 0; j < w; ++ j, trgPt += tc, ++ colIP )
			{
				*trgPt = (UInt8)*(srcPt + *colIP);
			}
		}
		else // asYCcCe
		{
			for( j = 0; j < w; ++ j, trgPt += tc, ++ colIP )
			{
				*trgPt = 255 - (UInt8)*(srcPt + *colIP);
			}
		}
	}
	jpgP->imgSIT = i;
}

void PrvQuickZoom16( void *jpgIP, UInt8 * srcCbP, UInt8 * srcCrP, UInt8 * trgP, UInt32 srcRL, UInt32 srcLim, UInt32 linePos, UInt8 colorSpace )
{
	UInt32 i, j, w, h, tl, tc, sh, line, prevline;
	JPEG_FormatType *jpgP = (JPEG_FormatType*)jpgIP;
	UInt32 lineOffCount, srcOffLine, trgOffLine;	
	UInt16 *colIP;
	UInt8 *srcP1t, *srcP2t, *trgPt;
	
	if( !srcLim )
	{
		if( !(linePos&1) && ((linePos+1) < jpgP->imgH) ) return;
		++srcLim;
	}

	w = jpgP->imgCJN;
	h = jpgP->imgCIN;
	tl = jpgP->imgCTOLD;
	tc = jpgP->imgCTOCD;
	sh = jpgP->imgSH;
	srcP1t = srcCbP;
	srcP2t = srcCrP;
	srcOffLine = line = 0;
	lineOffCount = jpgP->imgCNT;	
	trgOffLine = jpgP->imgCPT; 
	for( i = jpgP->imgCIT; i < h; ++ i )
	{
		prevline = line;     
		line = lineOffCount >> singleScaleFactor2x;
		if( line >= srcLim ) 
		{
			jpgP->imgCNT = lineOffCount - (srcLim << singleScaleFactor2x);
			jpgP->imgCPT = trgOffLine;
			jpgP->imgCIT = i;
			return;
		}
		lineOffCount += sh;

		if( prevline != line ) 
		{
			srcOffLine = line * srcRL; 
			srcP1t = srcCbP + srcOffLine;
			srcP2t = srcCrP + srcOffLine;
		}
		colIP = jpgP->imgColC;
		trgPt = trgP + trgOffLine;
		trgOffLine += tl;

		if( colorSpace == asYCbCr )
		{
			for( j = 0; j < w; j += 2, trgPt += tc, ++ colIP ) 
			{
				*trgPt = (UInt8)(*(srcP1t + *colIP)); 
				*(trgPt + 1) = (UInt8)(*(srcP2t + *colIP)); 
			}
		}
		else	// asYCcCe
		{
			for( j = 0; j < w; j += 2, trgPt += tc, ++ colIP ) 
			{
				*trgPt = 255 - (UInt8)(*(srcP1t + *colIP)); 
				*(trgPt + 1) = 255 - (UInt8)(*(srcP2t + *colIP)); 
			}
		}
	}
	jpgP->imgCIT = i;
}

void PrvZoomCalcFactorAW( UInt32 src, UInt32 trg, UInt32 * scaleFactorP, UInt32 *sizeP )
{
	if( !trg || !src || !sizeP ) return;

	*scaleFactorP = (src << singleScaleFactor2x) / trg;
	if( !*scaleFactorP ) return;

	while( 1 )
	{
		*sizeP = (src << singleScaleFactor2x) / *scaleFactorP;				
		if( *sizeP <= trg ) break;
		++ (*scaleFactorP);
	}
	if( *sizeP > 1 && *sizeP&1 ) -- *sizeP;
}

void PrvZoomClose( JPEG_FormatType *jpgP, EnvType *envP )
{
	if( jpgP->imgColY ) envP->freeP( jpgP->imgColY );
	if( jpgP->imgColC ) envP->freeP( jpgP->imgColC );
}

Result PrvZoomPrepare( JPEG_FormatType *jpgP, UInt32 srcEW, UInt32 srcEH, UInt32 trgW, UInt32 trgH, UInt8 zoomIn, UInt8 zoomNo, UInt32 rotateAng, EnvType *envP )
{
	UInt32 colnOffCount, w=0, h=0, rl=0, sfW=0, sfH=0, srcW=srcEW, srcH=srcEH;
	Int32 i;

	if( !envP || !srcW || !trgW || !srcH || !trgH || !jpgP ) return resNull;

	// check rotate
	if( (rotateAng & DCIM_IsRotate90)  ) 
	{
		Swapv( srcW, srcH );
	}

	// scale factor
	if( (srcW > trgW || srcH > trgH || zoomIn) && !zoomNo ) 
	{ 
		PrvZoomCalcFactorAW( srcW, trgW, &sfW, &w ); 
		PrvZoomCalcFactorAW( srcH, trgH, &sfH, &h ); 
	}
	else  
	{ 
		w = srcW; h = srcH; 
		sfW = sfH = singleScaleFactor; 
	}
	//
	rl = Round4( w );
	jpgP->imgW = w;
	jpgP->imgH = h;
	jpgP->imgRotate = rotateAng & DCIM_IsRotate;
	jpgP->imgSNT = jpgP->imgSPT = jpgP->imgSIT = jpgP->imgCNT = jpgP->imgCPT = jpgP->imgCIT = 0;
	//
	if( jpgP->mcuInfo.repVC[compY] == 1 && jpgP->mcuInfo.repHC[compY] == 1 ) 
	{
		jpgP->imgCLM = 1;
		jpgP->imgCCM = 1;
	}
	else if( jpgP->mcuInfo.repVC[compY] == 2 && jpgP->mcuInfo.repHC[compY] == 2 ) 
	{
		jpgP->imgCLM = 0;
		jpgP->imgCCM = 0;
	}
	else if( jpgP->mcuInfo.repVC[compY] == 1 && jpgP->mcuInfo.repHC[compY] == 2 ) 
	{
		jpgP->imgCLM = 1;
		jpgP->imgCCM = 0;
	}
	else if( jpgP->mcuInfo.repVC[compY] == 2 && jpgP->mcuInfo.repHC[compY] == 1 ) 
	{
		jpgP->imgCLM = 0;
		jpgP->imgCCM = 1;
	}
	else 
		return resFail;
	//
	if( (rotateAng & DCIM_Rotate90CCW) == DCIM_Rotate90CCW )
	{
		jpgP->imgSIN = w;
		jpgP->imgSJN = h;
		jpgP->imgSW = sfH;
		jpgP->imgSH = sfW;
		jpgP->imgSTOLD = 1;
		jpgP->imgSTOCD = rl;
		jpgP->imgCIN = w >> 1;
		jpgP->imgCJN = h;
		jpgP->imgCTOLD = 2;
		jpgP->imgCTOCD = rl;
		jpgP->imgColY = (UInt16*)envP->allocP( h << 1 );
		jpgP->imgColC = (UInt16*)envP->allocP( h << 1 );
		if( !jpgP->imgColY || !jpgP->imgColC ) return resFail;
		colnOffCount = 0;
		for( i = (Int32)h-1; i >= 0; -- i )
		{
			jpgP->imgColY[i] = (UInt16)(colnOffCount >> singleScaleFactor2x); 
			colnOffCount += sfH;
		}
		colnOffCount = 0;
		for( i = (Int32)(h>>1)-1; i >= 0; -- i )
		{
			jpgP->imgColC[i] = (UInt16)(colnOffCount >> singleScaleFactor2x); 
			colnOffCount += (sfH<<jpgP->imgCCM);
		}
	}
	else if( (rotateAng & DCIM_Rotate90CW) == DCIM_Rotate90CW )
	{
		jpgP->imgSIN = w;
		jpgP->imgSJN = h;
		jpgP->imgSW = sfH;
		jpgP->imgSH = sfW;
		jpgP->imgSTOLD = -1;
		jpgP->imgSTOCD = rl;
		jpgP->imgSPT = w - 1;
		jpgP->imgCPT = w - 2;
		jpgP->imgCIN = w >> 1;
		jpgP->imgCJN = h;
		jpgP->imgCTOLD = -2;
		jpgP->imgCTOCD = rl;
		jpgP->imgColY = (UInt16*)envP->allocP( h << 1 );
		jpgP->imgColC = (UInt16*)envP->allocP( h << 1 );
		if( !jpgP->imgColY || !jpgP->imgColC ) return resFail;
		colnOffCount = 0;
		for( i = 0; i < (Int32)h; ++i )
		{
			jpgP->imgColY[i] = (UInt16)(colnOffCount >> singleScaleFactor2x); 
			colnOffCount += sfH;
		}
		colnOffCount = 0;
		for( i = 0; i < (Int32)h; ++i )
		{
			jpgP->imgColC[i] = (UInt16)(colnOffCount >> singleScaleFactor2x); 
			colnOffCount += (sfH<<jpgP->imgCCM);
		}
	}
	else if( (rotateAng & DCIM_Rotate180) == DCIM_Rotate180 )
	{
		jpgP->imgSIN = h;
		jpgP->imgSJN = w;
		jpgP->imgSW = sfW;
		jpgP->imgSH = sfH;
		jpgP->imgSTOLD = -((Int32)rl);
		jpgP->imgSTOCD = -1;
		jpgP->imgSPT = rl * (h - 1) + w - 1;
		jpgP->imgCPT = rl * ((h>>1) - 1) + w - 2;
		jpgP->imgCIN = h >> 1;
		jpgP->imgCJN = w;
		jpgP->imgCTOLD = -(Int32)rl;
		jpgP->imgCTOCD = -2;
		jpgP->imgColY = (UInt16*)envP->allocP( w << 1 );
		jpgP->imgColC = (UInt16*)envP->allocP( w << 1 );
		if( !jpgP->imgColY || !jpgP->imgColC ) return resFail;
		colnOffCount = 0;
		for( i = 0; i < (Int32)w; ++ i )
		{
			jpgP->imgColY[i] = (UInt16)(colnOffCount >> singleScaleFactor2x); 
			colnOffCount += sfW;
		}
		colnOffCount = 0;
		for( i = 0; i < (Int32)w; ++ i )
		{
			jpgP->imgColC[i] = (UInt16)(colnOffCount >> singleScaleFactor2x); 
			colnOffCount += (sfW<<jpgP->imgCCM);
		}
	}
	else	// no rotate
	{
		jpgP->imgSW = sfW;
		jpgP->imgSH = sfH;
		jpgP->imgSIN = h;
		jpgP->imgSJN = w;
		jpgP->imgSTOLD = rl;
		jpgP->imgSTOCD = 1;
		jpgP->imgCIN = h >> 1;
		jpgP->imgCJN = w;
		jpgP->imgCTOLD = rl;
		jpgP->imgCTOCD = 2;
		jpgP->imgColY = (UInt16*)envP->allocP( w << 1 );
		jpgP->imgColC = (UInt16*)envP->allocP( w << 1 );
		if( !jpgP->imgColY || !jpgP->imgColC ) return resFail;
		colnOffCount = 0;
		for( i = 0; i < (Int32)w; ++ i )
		{
			jpgP->imgColY[i] = (UInt16)(colnOffCount >> singleScaleFactor2x); 
			colnOffCount += sfW;
		}
		colnOffCount = 0;
		for( i = 0; i < (Int32)w; ++ i )
		{
			jpgP->imgColC[i] = (UInt16)(colnOffCount >> singleScaleFactor2x); 
			colnOffCount += (sfW<<jpgP->imgCCM);
		}
	}
	//
	return resOk;
}

#undef singleScaleFactor
#undef singleScaleFactor2x 

// Grayscale
Result Cmpl_SingleScan1( JPEG_FormatType * jpgP, ImageType * imageP, UInt8 * duLineT[], UInt32 linePos, UInt32 lineNum, UInt32 lineSize[] )
{
	PrvQuickZoom8( jpgP, (UInt8*)*duLineT, imageP->dataYP, *(lineSize + compY), lineNum, asY );  
	return resOk;	
}

// RGB
Result Cmpl_SingleScan3( JPEG_FormatType * jpgP, ImageType * imageP, UInt8 * duLineT[], UInt32 linePos, UInt32 lineNum, UInt32 lineSize[] )
{
	PrvQuickZoom8( jpgP, (UInt8*)*duLineT, imageP->dataYP, *(lineSize + compY), lineNum, asYCbCr );  
	PrvQuickZoom16( jpgP, (UInt8*)*(duLineT + 2), (UInt8*)*(duLineT + 4), imageP->dataCbCrP, *(lineSize + compCb) << jpgP->imgCLM, lineNum >> 1, linePos, asYCbCr );  
	return resOk;	
}

// CMYK
Result Cmpl_SingleScan4( JPEG_FormatType * jpgP, ImageType * imageP, UInt8 * duLineT[], UInt32 linePos, UInt32 lineNum, UInt32 lineSize[] )
{
	PrvQuickZoom8( jpgP, (UInt8*)*duLineT, imageP->dataYP, *(lineSize + compY), lineNum, asYCcCe );  
	PrvQuickZoom16( jpgP, (UInt8*)*(duLineT + 2), (UInt8*)*(duLineT + 4), imageP->dataCbCrP, *(lineSize + compCb) << jpgP->imgCLM, lineNum >> 1, linePos, asYCcCe );  
	return resOk;	
}

////////////////////////////////////////////////////////////////////////////
//

UInt8 LL_ReadByte( register UInt8 * srcP )
{
	return * srcP;
}

UInt16 LL_ReadWord( register UInt8 * srcP )
{
	register UInt16 res;
	
	res = *srcP++;
	res <<= 8;
	res += *srcP;
	
	return res;
}

UInt32 LL_ReadDWord( register UInt8 * srcP )
{
	UInt32 res;

	res = *srcP++;
	res <<= 8;
	res += *srcP++;
	res <<= 8;
	res += *srcP++;
	res <<= 8;
	res += *srcP;
	
	return res;
}

UInt16 ReadBlockLen( UInt8 * bufP )
{
	return LL_ReadWord( bufP + 2 ) + 2;				
}

Result ReadAPP0Block( UInt8 * bufP, JPEG_FormatPtr jpgP, UInt32 idx )
{
	UInt16 len, w, h;
	
	len = LL_ReadWord( bufP + 2 ) + 2;				
	if( 18 > len ) return resMayBeNoJFIF; 
	if( jpegAPP0Sign != LL_ReadDWord( bufP + 4 ) ) return  resNoJFIF;	

	if(	1 != LL_ReadByte( bufP + 9 ) || 2 < LL_ReadByte( bufP + 10 ) ) return resUnknownVerJFIF; 

	w = (UInt16)LL_ReadByte( bufP + 14 );
	h = (UInt16)LL_ReadByte( bufP + 15 );
	if( w && h ) 
	{
		jpgP->thumb.thumbnail = 3;			// RGB
		jpgP->thumb.thumbnailIdx = idx + 16;
		jpgP->thumb.thumbnailW = w;
		jpgP->thumb.thumbnailH = h;
	}

	return resOk;
}

Result ReadAPP0xBlock( UInt8 * bufP, JPEG_FormatPtr jpgP, UInt32 idx )	
{
	UInt16 len, exc;
	
	len = LL_ReadWord( bufP + 2 ) + 2;				
	if( 18 > len ) return resMayBeNoJFIF; 
	if( jpegAPP0Sign != LL_ReadDWord( bufP + 4 ) ) 
		if( jpegAPP0SignXX != LL_ReadDWord( bufP + 4 ) ) return  resNoJFIF;	
	exc = LL_ReadByte( bufP + 9 );
	if( exc == 0x10 ) 
	{ 
		jpgP->thumb.thumbnail = 1; 
		jpgP->thumb.thumbnailIdx = idx + 10; 
		jpgP->thumb.thumbnailW = 0;
		jpgP->thumb.thumbnailH = 0;
	} 
	else if( exc == 0x11 ) 
	{ 
		jpgP->thumb.thumbnail = 2; 
		jpgP->thumb.thumbnailIdx = idx + 12; 
		jpgP->thumb.thumbnailW = (UInt32)LL_ReadByte( bufP + 10 );
		jpgP->thumb.thumbnailH = (UInt32)LL_ReadByte( bufP + 11 );
	} 
	else if( exc == 0x12 ) 
	{ 
		jpgP->thumb.thumbnail = 3; 
		jpgP->thumb.thumbnailIdx = idx + 12; 
		jpgP->thumb.thumbnailW = (UInt32)LL_ReadByte( bufP + 10 );
		jpgP->thumb.thumbnailH = (UInt32)LL_ReadByte( bufP + 11 );
	} 

	return resOk;
}
#define PrvGetWord( n, block, IsIntelByteAlign )  n = LL_ReadWord( block ); block += 2; if( IsIntelByteAlign ) n = ByteSwap16( n );  
#define PrvGetDWord( n, block, IsIntelByteAlign ) n = LL_ReadDWord( block ); block += 4; if( IsIntelByteAlign ) n = ByteSwap32( n );  

Result PrvGetIFDTags( UInt8 * SegmentStartP, UInt32 IFDOffset, UInt16 ExifInfoSize, Boolean IsIntelByteAlign, Boolean IsSubIFD, JPEG_ThumbPtr infoP )
{
	Result res = resOk;
  	UInt8 * blockP;
  	UInt16 IFDEntryNum, tag, DataFormat, t;
  	UInt32 ii, NumberOfComponents, Data;
    
    DBG_assert0( SegmentStartP );
    DBG_assert0( infoP );
    
	if( IFDOffset >= ExifInfoSize ) return resInvalidBlock;

	if( IFDOffset )
  	{
  		// 1.1.
    	blockP = SegmentStartP + IFDOffset;
    	PrvGetWord( IFDEntryNum, blockP, IsIntelByteAlign );
      	// 1.2.
    	for( ii = 0; ii < IFDEntryNum; ++ ii )
    	{
    		// 1.2.1
    		PrvGetWord( tag, blockP, IsIntelByteAlign );
      		PrvGetWord( DataFormat, blockP, IsIntelByteAlign );
      		PrvGetDWord( NumberOfComponents, blockP, IsIntelByteAlign );
      		PrvGetDWord( Data, blockP, IsIntelByteAlign );
			
          	if( !IsIntelByteAlign && ( DataFormat == 3 || DataFormat == 8 ) )
          	{
            	t = ( ((UInt16)( Data >> 16 )) & 0xffff );
            	Data = (((UInt16)(Data & 0xffff)) << 16 );
            	Data |= t;
          	}

			if( DataFormat == 2 || DataFormat == 5 || DataFormat == 10 || DataFormat == 12 )
				if( Data > ExifInfoSize || Data < 26 ) 
					continue; 
			
     		res = PrvAnalizeExifTag( tag, NumberOfComponents, Data, SegmentStartP, ExifInfoSize, IsIntelByteAlign, IsSubIFD, infoP );
    	}
    	// 1.3.
    	PrvGetDWord( Data, blockP, IsIntelByteAlign );
    	if( Data )
	    	res = PrvGetIFDTags( SegmentStartP, Data, ExifInfoSize, IsIntelByteAlign, IsSubIFD, infoP );
	}
  	
  	return res;
}

Result PrvAnalizeExifTag( UInt16 Tag, UInt32 ComponentNumber, UInt32 Data, UInt8 * SegmentStartP, UInt16 ExifInfoSize, Boolean IsIntelByteAlign, Boolean IsSubIFD, JPEG_ThumbPtr infoP )
{
  	if( !IsSubIFD )
  	{
  		switch( Tag )
  		{
     		// Thumbnail
     		case 0x100:
     		{
     			infoP->thumbnailW = Data;
     			break;
     		}
     		case 0x101:
     		{
	     		infoP->thumbnailH = Data;
     			break;
     		}
     		case 0x103:	// type
     		{
     			infoP->thumbnail = (Data == 6)? 1 : ((Data == 1)? 3 : 0);
     			break;
     		}
      		case 0x106:	// type 2
      		{
      			if( infoP->thumbnail == 3 )
	      			infoP->thumbnail = (Data == 2)? 3 : 0;		
	      		break;
      		}
      		case 0x201:		// start of jpeg thumb
      		{
      			if( infoP->thumbnail == 1 )
	      			infoP->thumbnailIdx = Data;
	      		break;
      		}
      		case 0x111:		// start of RGB thumb
      		{
      			if( infoP->thumbnail == 3 )
	      			infoP->thumbnailIdx = Data;
	      		break;
      		}
	    	default: break;
	    }
  	}
  
	return resOk;
}

Result ReadAPP1Block( UInt8 * bufP, JPEG_FormatType * jpgP, UInt32 idxA )
{
	Result res = resOk;
	Boolean IsIntelByteAlign;
  	UInt8 * segmentStartP;
  	UInt16 temp, len;
  	UInt32 IFDOffset, n;

	DBG_assert0( bufP );
	DBG_assert0( jpgP );

	// 1. prepare
	len = LL_ReadWord( bufP + 2 ) + 2;				
 	// 1.1 Exif? (0x45786966 0000)
 	n = LL_ReadDWord( bufP + 4 ); // Exif ID
 	if( (n != jpegExifSign && n != jpegEXIFSign) || LL_ReadWord( bufP + 8 ) ) return resNoExif;
	if( len < 22 && len >= 10 ) return resOk; 
  	bufP += 10;
  	
  	// 2. Tiff title
  	segmentStartP = bufP;

    // 3. Intel/Motorola
    temp = LL_ReadWord( bufP );
    bufP += 2;
    if( temp == 0x4949 ) // 'II' 0x4949 Big Endian ( Intel )
    	IsIntelByteAlign = 1;
    else if( temp == 0x4d4d ) // 'MM' 0x4d4d Little Endian ( Motorolla )
    	IsIntelByteAlign = 0;
    else 
    	return resExifUnsupportByteAlign;

	// 4. 002A
    PrvGetWord( temp, bufP, IsIntelByteAlign );
    if( temp != 0x002a ) return resExifCorrupt;

    // 5. first IFD
    PrvGetDWord( IFDOffset, bufP, IsIntelByteAlign );

	// 6. Read EXIF tags
    PrvGetIFDTags( segmentStartP, IFDOffset, len-10, IsIntelByteAlign, 0, &jpgP->thumb );

 	// 7. update thumbnail info
	if( jpgP->thumb.thumbnailIdx ) jpgP->thumb.thumbnailIdx += idxA + 10;
  
    return res;
}

#undef PrvGetWord
#undef PrvGetDWord

Result ReadSOF0Block( UInt8 * bufP, JPEG_FormatType * jpgP )
{
	UInt8 tmp;
	UInt16 len;	
	UInt32 idx = 0;
	UInt32 hMax = 0, vMax = 0, hSeq[jpegMaxComponentNum], vSeq[jpegMaxComponentNum];
	UInt32 i, j, pos, id;
	UInt32 width, height;

	len = LL_ReadWord( bufP + 2 ) + 2;
	jpgP->jpegInfo.bitPerUnit = (UInt16)LL_ReadByte( bufP + 4 );
	if( 8 != jpgP->jpegInfo.bitPerUnit ) return resUnsupportPrecision;
	jpgP->jpegInfo.height = LL_ReadWord( bufP + 5 );
	jpgP->jpegInfo.width = LL_ReadWord( bufP + 7 );
	if( !jpgP->jpegInfo.width || !jpgP->jpegInfo.height ) return resInvalidWidthHeight;
	jpgP->jpegInfo.compNum = (UInt16)LL_ReadByte( bufP + 9 );
	//if( 1 != jpgP->jpegInfo.compNum && 3 != jpgP->jpegInfo.compNum ) return resUnsupportColorComp;
	if( jpgP->jpegInfo.compNum > jpegMaxComponentNum ) return resUnsupportColorComp;
	if( jpgP->jpegInfo.compNum * 3/*длина блока компонента*/ + 10 != len ) return resInvalidBlock;
	idx += 10;
	for( pos = 0, i = 0; i < jpgP->jpegInfo.compNum; ++ i, idx += 3 )
	{
		id = LL_ReadByte( bufP + idx );
		jpgP->mcuInfo.compId[i] = id;
		tmp = LL_ReadByte( bufP + idx + 1 );
		hSeq[i] = HIPART( tmp ); vSeq[i] = LOPART( tmp );
		if( hSeq[i] > 2 || vSeq[i] > 2 ) return resUnsupportedDiskretVal;
		jpgP->mcuInfo.repVC[i] = (UInt8)vSeq[i];
		jpgP->mcuInfo.repHC[i] = (UInt8)hSeq[i];
		hMax = ( hMax < hSeq[i] )? hSeq[i] : hMax;
		vMax = ( vMax < vSeq[i] )? vSeq[i] : vMax;
		tmp = LL_ReadByte( bufP + idx + 2 );
		jpgP->mcuInfo.seqMCU[i] = hSeq[i] * vSeq[i];	
		jpgP->mcuInfo.compPos[i] = pos;
		for( j = 0; j < jpgP->mcuInfo.seqMCU[i]; ++ j, ++ pos )
		{
			jpgP->mcuInfo.numQuantTbl[pos] = tmp;
		}
		if( pos >= jpegMaxDiscretVal ) return resInvalidBlock;
		jpgP->mcuInfo.DC[i] = 0;
	}
	jpgP->mcuInfo.vMax = vMax;
	jpgP->mcuInfo.hMax = hMax;
	jpgP->mcuInfo.duInMcu = vMax * hMax;	
	jpgP->mcuInfo.sizeX = hMax << 3;		
	jpgP->mcuInfo.sizeY = vMax << 3;		
	jpgP->mcuInfo.sizeMCU = (jpgP->mcuInfo.duInMcu) << 6;
	for( i = 0; i < jpgP->jpegInfo.compNum; ++ i )	
	{	
		jpgP->mcuInfo.seqHC[i] = (UInt8)(1 - jpgP->mcuInfo.repHC[i] / hMax);
		jpgP->mcuInfo.seqVC[i] = (UInt8)(1 - jpgP->mcuInfo.repVC[i] / vMax);
	}
	width = jpgP->jpegInfo.width;
	height = jpgP->jpegInfo.height;
	jpgP->jpegInfo.heightM = ( 1 == vMax )? Round8( height ) : (( 2 == vMax )? Round16( height ) : height); 
	jpgP->jpegInfo.widthM = ( 1 == hMax )? Round8( width ) : (( 2 == hMax )? Round16( width ) : width);

	width = jpgP->jpegInfo.widthM / jpgP->mcuInfo.sizeX;
	height = jpgP->jpegInfo.heightM / jpgP->mcuInfo.sizeY;
	
	jpgP->jpegInfo.numMCUx = (UInt32)width;		
	jpgP->jpegInfo.numMCUy = (UInt32)height;	
	jpgP->jpegInfo.numMCU = width * height;		

	return resOk;
}


Result ReadSOSBlock( UInt8 * bufP, JPEG_FormatType * jpgP )
{
	UInt8 tmp;
	UInt16 len;
	UInt32 idx = 0;
	UInt32 i, j, pos, id;
	UInt32 numScan;

	len = LL_ReadWord( bufP + 2 ) + 2;				
	numScan = (UInt16)LL_ReadByte( bufP + 4 );
	if( 1 > numScan || resInvalidNumScanComp < numScan ) return resInvalidNumScanComp; // 1..3 компоненты на скан
	if( len != (8 + (numScan << 1)) ) return resInvalidSizeSOSBlock;
	idx = 5;
	jpgP->mcuInfo.compNumInScan = numScan;
	for( j = 0; j < jpegMaxComponentNum; ++ j ) jpgP->mcuInfo.compMcuId[j] = 0xff;
	for( pos = 0, i = 0; i < numScan; ++ i, idx += 2 )
	{
		id = LL_ReadByte( bufP + idx );
		for( j = 0; j < jpgP->jpegInfo.compNum; ++ j )
		{
			if( id == jpgP->mcuInfo.compId[j] ) break;
		}
		jpgP->mcuInfo.compMcuId[i] = j;
		tmp = LL_ReadByte( bufP + idx + 1 );
		pos = jpgP->mcuInfo.compPos[jpgP->mcuInfo.compMcuId[i]];
		for( j = 0; j < jpgP->mcuInfo.seqMCU[jpgP->mcuInfo.compMcuId[i]]; ++ j, ++ pos )
		{
			jpgP->mcuInfo.numHufTblDC[pos] = HIPART( tmp );		// первая таблица - для DC
			jpgP->mcuInfo.numHufTblAC[pos] = LOPART( tmp );		// вторая таблица - для AC
		}
		if( pos >= jpegMaxDiscretVal ) return resInvalidSizeSOSBlock;
	}

	jpgP->scanInfo.scanSSS = (UInt32)LL_ReadByte( bufP + idx );
	jpgP->scanInfo.scanSSE = (UInt32)LL_ReadByte( bufP + idx + 1 );
	tmp = LL_ReadByte( bufP + idx + 2 );
	jpgP->scanInfo.scanSAH = HIPART( tmp );
	jpgP->scanInfo.scanSAL = LOPART( tmp );
	if( CHK_FLAG( jpgP, jpegProgressive ) )
	{
		if( (!jpgP->scanInfo.scanSSS || !jpgP->scanInfo.scanSSE) && (jpgP->scanInfo.scanSSS != jpgP->scanInfo.scanSSE) ) return resSpecSelectionError;
		if( jpgP->scanInfo.scanSSS > jpgP->scanInfo.scanSSE ) return resSpecSelectionError;
		if( jpgP->scanInfo.scanSSS && 1 != numScan ) return resSpecSelectionError;
		if( jpgP->scanInfo.scanSSE > 63 ) return resSpecSelectionError;
		if( jpgP->scanInfo.scanSAH > 13 || jpgP->scanInfo.scanSAL > 13 ) return resSucesApproxError;
		if( jpgP->scanInfo.scanSAH && jpgP->scanInfo.scanSAH < jpgP->scanInfo.scanSAL ) return resSucesApproxError;
	}

	++ jpgP->scanInfo.numScans;

	return resOk;
}

Result PrvCreateHufTree( JPEG_HufTreeType * hufTreeP, JPEG_HufTableType * hufTableP )
{
	UInt16 i, j, code;
	UInt16 pos, size;
	UInt16 mask;
	UInt8 hufSize[256];

	if( !hufTreeP || !hufTableP ) return resNull;

	for( pos = 0, i = 0; i < 16; i++ ) 
	{
		for( j = 1; j <= hufTableP->dhtBits[i]; ++ j, ++ pos )
		{
			hufSize[pos] = (UInt8)(i + 1);
		}
	}
	hufTreeP->hufLen = pos;
	hufSize[pos] = 0;

	code = 0;	
	pos = 0;
	size = hufSize[0];
	while( hufSize[pos] ) 
	{
		mask = ~(0xffff >> size);
		while( hufSize[pos] == size ) 
		{
			hufTreeP->hufCode[pos] = (~(((UInt16)code) << (16 - size))) & mask;
			++ code;
			++ pos;
		}
	    code <<= 1;
		++ size;
	}

	for( pos = 0; hufSize[pos]; ++ pos ) 
	{
		hufTreeP->hufData[hufTreeP->hufLen - pos] = (UInt16)hufTableP->dhtCodes[pos];
		hufTreeP->hufSize[hufTreeP->hufLen - pos] = hufSize[pos];
	}

	return resOk;
}


Result ReadDHTBlock( UInt8 * bufP, JPEG_FormatType * jpgP, EnvType * envP )
{
	Result err = resOk;
	UInt16 len;	
	UInt32 idx = 0, i, hufCount;
	Int32 dhtLen;
	JPEG_HufTableType * hufTableP;		
	
	len = LL_ReadWord( bufP + 2 ) + 2;				
	dhtLen = len - 4;	
	if( 0 >= dhtLen ) return resInvalidSizeHufTable;

	hufTableP = (JPEG_HufTableType*)envP->allocP(sizeof(JPEG_HufTableType)); 
	if( !hufTableP ) return resNoMem;
	
	idx = 4;
	
	while( dhtLen > 0 )
	{
		hufTableP->dhtInfo = (UInt16)LL_ReadByte( bufP + idx );
		if( LOPART( hufTableP->dhtInfo ) >= jpegMaxHufTableNum ) { err = resInvalidNumHufTable; goto exitL; }// если указан некорректный номер таблицы
		++ idx;
		hufCount = 0;
		for( i = 0; i < 16; ++ i )
		{
			hufTableP->dhtBits[i] = (UInt16)LL_ReadByte( bufP + idx + i );
			hufCount += hufTableP->dhtBits[i];
		}
		idx += 16;
		for( i = 0; i < hufCount; ++ i )
		{
			hufTableP->dhtCodes[i] = (UInt16)LL_ReadByte( bufP + idx + i );
		}
		idx += hufCount;
		if(  HIPART( hufTableP->dhtInfo ) ) 
			err = PrvCreateHufTree( &jpgP->scanInfo.hufACTree[LOPART( hufTableP->dhtInfo )], hufTableP );
		else 
			err = PrvCreateHufTree( &jpgP->scanInfo.hufDCTree[LOPART( hufTableP->dhtInfo )], hufTableP );
		dhtLen -= (17 + hufCount);
		++ jpgP->jpegInfo.hufNum;
	}
	if( dhtLen != 0 ) { err = resDehufCorrupt; goto exitL; }

exitL:

	if( hufTableP ) envP->freeP(hufTableP); 
	
	return err;
}

Result ReadDQTBlock( UInt8 * bufP, JPEG_FormatType * jpgP )
{
	Int32 dqtLen;
	UInt32 i, dqtCount;
	UInt16 len;
	UInt16 * quantTableP, n;
	UInt8 dqtInfo;
	UInt32 idx = 0;
	UInt32 * unzigzagP = jpgP->unZigZagT;
	UInt8 scalingP[] = jpegCosScaledTableForRGB16;
	
	len = LL_ReadWord( bufP + 2 ) + 2;				
	dqtLen = (Int32)len - 4;
	if( 0 >= dqtLen ) return resInvalidSizeQuantTable;

	idx = 4; 
	while( dqtLen > 0 )
	{
		dqtInfo = LL_ReadByte( bufP + idx );
		if( 0 != HIPART( dqtInfo ) ) return res16bitQuantTable; 
		dqtCount = (UInt16)LOPART( dqtInfo );
		if( dqtCount >= jpegMaxTableNum ) return resInvalidNumQuantTable;
		jpgP->scanInfo.quantTable[dqtCount].quantInfo = dqtInfo;
		++ idx;
		quantTableP = jpgP->scanInfo.quantTable[dqtCount].quantTable;
		for( i = 0; i < 64; ++ i )
		{
			n = (((UInt16)LL_ReadByte( bufP + idx + i )) * scalingP[unzigzagP[i]]) >> 2; 
			n = (n > 255)? 255 : n;
			quantTableP[unzigzagP[i]] = n; 
		}
		idx += 64;
		dqtLen -= 65;
		++ jpgP->jpegInfo.qntNum;
	}
	if( dqtLen != 0 ) return resInvalidSizeQuantTable;

	return resOk;
}

Result ReadDRIBlock( UInt8 * bufP, JPEG_FormatType * jpgP )
{
	UInt16 len;
	
	len = LL_ReadWord( bufP + 2 ) + 2;				
	if( 6 != len ) return resInvalidLenDRI;
	jpgP->jpegInfo.resetInterval = LL_ReadWord( bufP + 4 );

	return resOk;
}

Result JPEG_JpegScan( JPEG_FormatType * jpgP, StorageType * storageP, EnvType * envP )
{
	Result err = resFail;
	UInt8 * bufP;		
	UInt32 indexA = 0;	
	UInt32 index = 0;	
	UInt16 blockLen = 0;
	UInt16 blockId = 0;	

	DBG_assert0( jpgP );
	DBG_assert0( storageP );
	
	if( resOk != (err = storageP->resizeP( storageP->dataP, jpegSegmentSize )) ) return err;

	bufP = storageP->refreshP( storageP->dataP, jpgP->scanInfo.scanIndex, jpegSegmentSize, &err );
	if( resOk != err && resEOB != err ) return err;

	indexA = jpgP->scanInfo.scanIndex;
	index = 0;

	if( 0 == indexA ) if( 0xff != bufP[0] || 0xd8 != bufP[1] ) return resNoJPEG;

	while( 1 )
	{
		while( 1 ) 
		{
			if( jpegBlockSign == bufP[index] ) break;
			++ indexA;
			++ index;
			if( index >= jpegReserveSize ) return resLossesData;
		} 

		blockId = LL_ReadWord( bufP + index ); 
		
		if( jpegRST0 <= blockId && jpegRST7 >= blockId ) 
		{
			SET_FLAG( jpgP, jpegRSTxPresent );
			indexA += 2;
			index += 2;
		}
		else if( jpegSOI == blockId )	
		{
			if( 0 != GET_FLAGS( jpgP ) || CHK_FLAG( jpgP, jpegSOIPresent ) || indexA )  return resNoJPEG;
			SET_FLAG( jpgP, jpegSOIPresent );
			indexA += 2;
			index += 2;	
		}
		else if( jpegEOI == blockId )	
		{
			if( CHK_FLAG( jpgP, jpegEOIPresent ) )  return resNoJPEG;
			SET_FLAG( jpgP, jpegEOIPresent );
			indexA += 2;
			index += 2;		
			err = resEndOfJpeg;
			break;
		}
		else if( jpegAPP0 == blockId )	
		{
			blockLen = ReadBlockLen( bufP + index );
			if( resOk != (err = storageP->resizeP( storageP->dataP, blockLen )) ) return err;
			bufP = storageP->refreshP( storageP->dataP, indexA, blockLen + jpegReserveSize, 0 );
			index = 0;
			if( !CHK_FLAG( jpgP, jpegAPP0Present ) ) 
			{ 
				if( resOk != (err = ReadAPP0Block( bufP, jpgP, indexA )) ) break;		
			} 
			else if( !jpgP->thumb.thumbnail )
			{
				if( resOk != (err = ReadAPP0xBlock( bufP, jpgP, indexA )) ) break;		
			}
			indexA += blockLen;
			index += blockLen;				
			SET_FLAG( jpgP, jpegAPP0Present );
		}
		else if( jpegAPP1 == blockId )		// EXIF 
		{
			blockLen = ReadBlockLen( bufP + index );
			if( resOk != (err = storageP->resizeP( storageP->dataP, blockLen )) ) return err;
			bufP = storageP->refreshP( storageP->dataP, indexA, blockLen + jpegReserveSize, 0 );
			index = 0;
			if( !CHK_FLAG( jpgP, jpegAPP1Present ) ) 
			{ 
				//if( resOk != (err = ReadAPP1Block( bufP, jpgP, indexA )) ) break;		
				err = ReadAPP1Block( bufP, jpgP, indexA );
				if( resOk != err && resNoExif != err ) break;
			} 
			indexA += blockLen;
			index += blockLen;
			SET_FLAG( jpgP, jpegAPP1Present );
		}
		else if( jpegSOF0 == blockId )		
		{
			if( CHK_FLAG( jpgP, jpegSOF0Present ) )  return resDoubledBlock;
			blockLen = ReadBlockLen( bufP + index );
			if( resOk != (err = storageP->resizeP( storageP->dataP, blockLen )) ) return err;
			bufP = storageP->refreshP( storageP->dataP, indexA, blockLen + jpegReserveSize, 0 );
			index = 0;
			if( resOk != (err = ReadSOF0Block( bufP, jpgP )) ) break;		
			indexA += blockLen;
			index += blockLen;				
			SET_FLAG( jpgP, jpegSOF0Present );
		} 
		else if( jpegSOF1 == blockId )	
		{
			return resUnknownVerJFIF; 
		}
		else if( jpegSOF2 == blockId )  
		{
			if( CHK_FLAG( jpgP, jpegSOF2Present ) )  return resDoubledBlock;
			blockLen = ReadBlockLen( bufP + index );
			if( resOk != (err = storageP->resizeP( storageP->dataP, blockLen )) ) return err;
			bufP = storageP->refreshP( storageP->dataP, indexA, blockLen + jpegReserveSize, 0 ); 
			index = 0;
			if( resOk != (err = ReadSOF0Block( bufP, jpgP )) ) break; 
			indexA += blockLen;
			index += blockLen;
			SET_FLAG( jpgP, jpegSOF2Present );
		}
		else if( jpegSOS == blockId )		
		{
			if( (!CHK_FLAG( jpgP, jpegSOF0Present ) && !CHK_FLAG( jpgP, jpegSOF2Present ))|| !CHK_FLAG( jpgP, jpegDHTPresent ) || !CHK_FLAG( jpgP, jpegDQTPresent ) ) 
				return resBlockFamilyCorrupt; 
			blockLen = ReadBlockLen( bufP + index );				
			if( resOk != (err = storageP->resizeP( storageP->dataP, blockLen )) ) return err;
			bufP = storageP->refreshP( storageP->dataP, indexA, blockLen + jpegReserveSize, 0 ); 
			index = 0;
			if( resOk != (err = ReadSOSBlock( bufP, jpgP )) ) break;	
			indexA += blockLen;
			index += blockLen;
			SET_FLAG( jpgP, jpegSOSPresent );
			if( CHK_FLAG( jpgP, jpegSOF2Present ) ) SET_FLAG( jpgP, jpegProgressive );
			else if( jpgP->jpegInfo.compNum != jpgP->mcuInfo.compNumInScan ) SET_FLAG( jpgP, jpegMultipleScan );
			else SET_FLAG( jpgP, jpegSingleScan );
            err = resStartOfScan;
            break;
		}
		else if( jpegDHT == blockId )	
		{
			blockLen = ReadBlockLen( bufP + index );
			if( resOk != (err = storageP->resizeP( storageP->dataP, blockLen )) ) return err;
			bufP = storageP->refreshP( storageP->dataP, indexA, blockLen + jpegReserveSize, 0 ); 
			index = 0;
			if( resOk != (err = ReadDHTBlock( bufP, jpgP, envP )) ) break;		
			indexA += blockLen;
			index += blockLen;
			SET_FLAG( jpgP, jpegDHTPresent );
		}
		else if( jpegDQT == blockId )
		{
			blockLen = ReadBlockLen( bufP + index );
			if( resOk != (err = storageP->resizeP( storageP->dataP, blockLen )) ) return err;
			bufP = storageP->refreshP( storageP->dataP, indexA, blockLen + jpegReserveSize, 0 ); 
			index = 0;
			if( resOk != (err = ReadDQTBlock( bufP, jpgP )) ) break;		
			indexA += blockLen;
			index += blockLen;
			SET_FLAG( jpgP, jpegDQTPresent );
		}
		else if( jpegDRI == blockId )	
		{
			blockLen = ReadBlockLen( bufP + index );
			if( resOk != (err = storageP->resizeP( storageP->dataP, blockLen )) ) return err;
			bufP = storageP->refreshP( storageP->dataP, indexA, blockLen + jpegReserveSize, 0 ); 
			index = 0;
			if( resOk != (err = ReadDRIBlock( bufP, jpgP )) ) break;
			indexA += blockLen;
			index += blockLen;
			SET_FLAG( jpgP, jpegDRIPresent );
		}
		else if(	jpegSOF3 == blockId ||   
					jpegSOF5 == blockId ||   
					jpegSOF6 == blockId ||   
					jpegSOF7 == blockId ||   
					jpegJPG  == blockId ||   
					jpegSOF9 == blockId ||   
					jpegSOF10 == blockId ||  
					jpegSOF11 == blockId ||   
					jpegDAC   == blockId ||  
					jpegSOF13 == blockId || 
					jpegSOF14 == blockId || 
					jpegSOF15 == blockId ||  
					jpegDNL == blockId ||  
					jpegTEM == blockId )
		{
			index += 2;
		}
		else if(	jpegAPP2 == blockId ||
					jpegAPP3 == blockId ||
					jpegAPP4 == blockId ||
					jpegAPP5 == blockId ||
					jpegAPP6 == blockId ||
					jpegAPP7 == blockId ||
					jpegAPP8 == blockId ||
					jpegAPP9 == blockId ||
					jpegAPP10 == blockId ||
					jpegAPP11 == blockId ||
					jpegAPP12 == blockId ||
					jpegAPP13 == blockId ||
					jpegAPP14 == blockId ||
					jpegAPP15 == blockId ||
					jpegCOM == blockId )
		{
			blockLen = ReadBlockLen( bufP + index );
			indexA += blockLen;
			if( resOk != (err = storageP->resizeP( storageP->dataP, blockLen )) ) return err;
			bufP = storageP->refreshP( storageP->dataP, indexA, jpegReserveSize, 0 ); 
			index = 0;
		}
		else 
		{
			indexA += 2;
			index += 2;
		}
	}

	jpgP->scanInfo.scanIndex = indexA;
	
	return err;
}

// read bits
#define BitSkipNbits( b, i, n ) { (b) <<= (n); (i) += (n); }
#define BitReadNextByte( dp, d, rp, rf, s ) { if( *(dp) == 0xff ) { while( *(dp) == 0xff ) ++ (dp); \
	if( !*(dp) ) { (d) += ((UInt32)0x000000ff << (s)); ++(dp); } \
	else if( (*(dp) >= 0xd0 && *(dp) <= 0xd7) || *(dp) == 0xd9 ) { (rp)=(dp); (rf) = 1;  ++(dp); } } \
	else { (d) += (((UInt32)((UInt8)*((dp)))) << (s)); ++ (dp); } }
#define BitCacheUpdate( dp, d, i, rp, rf ) { if( (i) >= 8 ) {  \
	if( (i) < 16 ) { (i) -= 8; BitReadNextByte( (dp), (d), (rp), (rf), (i) ); } \
	else if( (i) < 24 ) { (i) -= 16; BitReadNextByte( (dp), (d), (rp), (rf), (i) + 8 ); BitReadNextByte( (dp), (d), (rp), (rf), (i) ); } \
	else if( (i) < 32 ) { (i) -= 24; BitReadNextByte( (dp), (d), (rp), (rf), (i) + 16 );BitReadNextByte( (dp), (d), (rp), (rf), (i) + 8 ); BitReadNextByte( (dp), (d), (rp), (rf), (i) );} \
	else { (i) = 0; BitReadNextByte( (dp), (d), (rp), (rf), 24 );BitReadNextByte( (dp), (d), (rp), (rf), 16 ); BitReadNextByte( (dp), (d), (rp), (rf), 8 );	BitReadNextByte( (dp), (d), (rp), (rf), 0 ); } } }
#define BitCacheChkAndUpdate( dp, d, i, rp, rf, n ) { if( (i) > (32 - (n)) ) BitCacheUpdate( dp, d, i, rp, rf ); }
#define BitReadAndInvertNbits( d, i, r, n ) { (r) = (d); (r) = (( ((UInt32)(r)) & 0x80000000 )? ((UInt32)(r)) >> (32 - (n)) : -((Int32)((~((UInt32)(r))) >> (32 - (n))))); (d) <<= (n); (i) += (n); }

#ifdef _AASM_USE_

UInt32 PrvDeHufNextUnit( UInt32 hufLen, UInt32 unitHi, UInt32 *hufCodeM ) 
{
	__asm
	{
		stmfd	   	sp!, {r1-r7}
		
		mov			r0, hufLen
		mov			r1, unitHi
		mov			r2, hufCodeM
		
		add			r0, r0, #8
		mov			r3, r1, lsr #16	
		mov			r1, r3, lsl #16			
		
	rep:
		ldmia		r2!, {r4-r7}

		tst			r3, r4;	
		beq			exit;	
		subs		r0, r0, #1;	
		tst			r1, r4;	
		beq			exit;	
		subs		r0, r0, #1; 
		tst			r3, r5;	
		beq			exit;	
		subs		r0, r0, #1;	
		tst			r1, r5;	
		beq			exit;	
		subs		r0, r0, #1; 
		tst			r3, r6;	
		beq			exit;	
		subs		r0, r0, #1;	
		tst			r1, r6;	
		beq			exit;	
		subs		r0, r0, #1; 
		tst			r3, r7;	
		beq			exit;	
		subs		r0, r0, #1;	
		tst			r1, r7;	
		beq			exit;	
		subs		r0, r0, #1; 
		cmp			r0, #8
		bhi			rep		
		
	exit:

		ldmfd   	sp!, {r1-r7}
		sub			r0, r0, #8
	}
}

#else

UInt32 PrvDeHufNextUnit( UInt32 hufLen, UInt32 unitHi, UInt32 *hufCodeM ) 
{
	register UInt32 unitLo;

	unitHi &= 0xffff0000;
	unitLo = unitHi >> 16;

	while( hufLen > 1 )
	{
		if( 0 == (unitLo & *hufCodeM) ) 
			return (Int32)(hufLen);
		if( 0 == (unitHi & *hufCodeM) ) 
			return (Int32)(hufLen - 1);
		++ hufCodeM;
		hufLen -= 2;
	} 

	if( hufLen > 0 )
		if( 0 == (unitLo & *hufCodeM) ) 
			return (Int32)(hufLen);

	return 0; 
}

#endif

#define BLS_RESET_DH() { rpos = 0; if( !rstxF ) { while( *dataP != 0xff && (++rpos) < jpegLastReserv ) ++ dataP; \
 while( *dataP == 0xff && (++rpos) < jpegLastReserv ) ++ dataP; if( rpos < jpegLastReserv ) { \
 if( (*dataP >= 0xd0 && *dataP <= 0xd7) || *dataP == 0xd9 ) { rstxF = 1; rstxP = dataP; } } } \
 if( !rstxF ) { err = resLossesData; goto exitL; } regIndex = 32; regData = 0; dataP = rstxP + 1; rstxP = 0; rstxF = 0; \
 delta = *(jpgP->mcuInfo.DC + compY) = *(jpgP->mcuInfo.DC + compCr) = *(jpgP->mcuInfo.DC + compCb) = 0; }


Result PrvJpegDecodeBaselineScan( JPEG_FormatType *jpgP, ImageType *imageP, UInt32 scaleFactor, CopyProcType *PrvCopyProc, OneStepCallbackPrc *OneStepProc, EnvType *envP )
{
	Result				err = resOk;
	Int32 				Diff;
	UInt32				i, j, size, res, rpos, rstInterval, delta = 0;
	UInt32 				n, midx, idx, skipLimit;		
	UInt32				numComp, id, vc, hc;
	UInt32				ii, jj, kk, ll, pos = 0;
	UInt32				sizeBuf, indexBuf, lineNum, prevLineC, prevLineS;
	UInt32				mcuJpegX, mcuJpegY, mcuJpegDY, mcuNum, duJpegSX;
	UInt8				*strP;
	JPEG_HufTreeType 	*hufTreeP; 	
	UInt16	 			*scaleTblP;	
	IDCTProcType		*idctProcP;
	UInt32				regData;
	UInt32				regIndex;
	UInt8				*dataP;
	UInt8				*rstxP;
	UInt32				rstxF;
	UInt32				lenSC[jpegMaxComponentNum];
	UInt8				*duLineP[jpegMaxComponentNum<<1], *duLineTP[jpegMaxComponentNum<<1];
	Int32				tVector[64];		
	UInt8				nzNumVec[8];		
	const UInt8			unZigzag[] = jpegUnZigzagTable;		
	const UInt8			skipLimits[] = { 64, 20, 4, 0 };

	DBG_assert0( jpgP );
	DBG_assert0( imageP );
	DBG_assert0( PrvCopyProc );
	DBG_assert0( PrvDecodeProc );
	DBG_assert0( scaleFactor <= 3 );
	
	//
	indexBuf = jpgP->scanInfo.scanIndex;
	sizeBuf = (jpgP->mcuInfo.sizeMCU * (jpgP->jpegInfo.numMCUx + 1)) << 1; 
	if( jpgP->storageP->getsizeP(jpgP->storageP->dataP) < sizeBuf ) 
		if( resOk != (err = jpgP->storageP->resizeP( jpgP->storageP->dataP, sizeBuf )) ) return err;
	if( (strP = (UInt8*)jpgP->storageP->refreshP( jpgP->storageP->dataP, indexBuf, sizeBuf, 0 )) == 0 ) goto exitL;	
	
	//
	regIndex = 32;
	regData = 0;
	dataP = (UInt8*)strP;
	rstxP = 0;
	rstxF = 0;
	
	//
	if( 0 == scaleFactor ) idctProcP = IDCT_QuickDecode8x8;
	else if( 1 == scaleFactor ) idctProcP = IDCT_QuickDecode4x4;
	else if( 2 == scaleFactor ) idctProcP = IDCT_QuickDecode2x2;
	else if( 3 == scaleFactor ) idctProcP = IDCT_QuickDecodeThumb;
	else return resFail;
	
	//
	for( i = 0; i < jpgP->jpegInfo.compNum; ++ i )
	{	
		*(lenSC + i) = (jpgP->jpegInfo.widthM >> *(jpgP->mcuInfo.seqHC + i)) >> scaleFactor;
	}

	//
	MSet4( duLineTP, 0, 6 );
	for( i = 0; i < jpegMaxComponentNum; ++ i )
	{
		j = i << 1;
		size = ((*(jpgP->mcuInfo.seqMCU + i) << 6) * jpgP->jpegInfo.numMCUx) >> (scaleFactor + scaleFactor);
		*(duLineTP + j)= (UInt8*)envP->allocP(size + jpegLastReserv); 
		if( !*(duLineTP + j) ) { err = resNoMem; goto exitL; }
		if( *jpgP->mcuInfo.repVC > 1 ) *(duLineTP + j + 1) = *(duLineTP + j) + (size>>1);
	}

	//
	skipLimit = skipLimits[scaleFactor];

	//
	prevLineC = prevLineS = 0;
	duJpegSX = 8 >> scaleFactor;
	mcuJpegX = jpgP->jpegInfo.numMCUx;					
	mcuJpegY = jpgP->jpegInfo.numMCUy;					
	mcuJpegDY = jpgP->mcuInfo.sizeY >> scaleFactor;
	rstInterval = jpgP->jpegInfo.resetInterval;			
	numComp = jpgP->mcuInfo.compNumInScan;

	//
	for( delta = 0, mcuNum = 0, lineNum = 0, i = 0; i < mcuJpegY; ++ i, lineNum += mcuJpegDY )
	{
		MCopy4( (UInt32*)duLineTP, (UInt32*)duLineP, jpgP->jpegInfo.compNum << 1 );
		for( j = 0; j < mcuJpegX; ++ j, ++ delta, ++ mcuNum )
		{
			//
			if( g_decodingState == 0 ) { res = resBreak; goto exitL; }
			// reset
			if( delta == rstInterval && rstInterval ) 
			{ 
				BLS_RESET_DH();
			}
			// decode MCU
			for( jj = 0; jj < numComp; ++ jj )
			{
				id = *(jpgP->mcuInfo.compMcuId + jj);
				pos = *(jpgP->mcuInfo.compPos + id);
				vc = *(jpgP->mcuInfo.repVC + id);
				hc = *(jpgP->mcuInfo.repHC + id);
				ll = id << 1;
				for( ii = 0; ii < vc; ++ ii, ++ ll )
				{
					for( kk = 0; kk < hc; ++ kk, ++ pos )
					{
						// decode DU
						MSet4( tVector, 0, 64 );
						MSet4( nzNumVec, 0, 2 );
						scaleTblP = (jpgP->scanInfo.quantTable + *(jpgP->mcuInfo.numQuantTbl + pos))->quantTable;
						BitCacheUpdate( dataP, regData, regIndex, rstxP, rstxF );
						hufTreeP = jpgP->scanInfo.hufDCTree + *(jpgP->mcuInfo.numHufTblDC + pos);
						res = PrvDeHufNextUnit( hufTreeP->hufLen, regData, (UInt32*)hufTreeP->hufCode );
						if( res == 0 ) { err = resHufCodeNotFound; goto exitL; }
						n = *(hufTreeP->hufSize + res);
						BitSkipNbits( regData, regIndex, n );
						n = *(hufTreeP->hufData + res); 
						if( n ) 
						{
							BitReadAndInvertNbits( regData, regIndex, Diff, n ); // и приводим его к нужному значению
							*(jpgP->mcuInfo.DC + id) += (Int32)Diff;
						}
						*tVector = ((Int32)*(jpgP->mcuInfo.DC + id) * *scaleTblP) << 2;
						++ *nzNumVec;
						hufTreeP = jpgP->scanInfo.hufACTree + *(jpgP->mcuInfo.numHufTblAC + pos);
						idx = 1;
						while( idx < 64 )
						{
							BitCacheUpdate( dataP, regData, regIndex, rstxP, rstxF );
							res = PrvDeHufNextUnit( hufTreeP->hufLen, regData, (UInt32*)hufTreeP->hufCode );
							if( res == 0 ) { err = resHufCodeNotFound; goto exitL; }
							n = *(hufTreeP->hufSize + res);
							BitSkipNbits( regData, regIndex, n );
							n = *(hufTreeP->hufData + res); 
							if( n == 0x00 ) idx = 64;			
							else if( n == 0xf0 ) idx += 16;	
							else 
							{
								idx += HIPART((UInt8)n);			
								//if( idx > 63 ) idx = 63;
								n = LOPART((UInt8)n);			
								BitCacheChkAndUpdate( dataP, regData, regIndex, rstxP, rstxF, n );
								if( idx >= skipLimit ) 			
								{
									BitSkipNbits( regData, regIndex, n );
								}
								else
								{
									midx = (UInt32)*(unZigzag + idx);
									BitReadAndInvertNbits( regData, regIndex, Diff, n ); 
									*(tVector + midx) = ((Int16)Diff * (Int16)*(scaleTblP + midx)) << 2;
									++ *(nzNumVec + (midx >> 3));	
								}
								++ idx;
							}
						}
						idctProcP( tVector, *(duLineP + ll), nzNumVec, *(lenSC + id) );
						*(duLineP + ll) += duJpegSX;
					}
				}
			}
		}
		// copy
		PrvCopyProc( jpgP, imageP, duLineTP, lineNum, mcuJpegDY, lenSC );
		//progress
		if( OneStepProc && (i&1) ) 
		{
			if( !OneStepProc( (void*)imageP, prevLineS, jpgP->imgSIT, prevLineC, jpgP->imgCIT, jpgP->imgRotate ) ) { err = resBreak; goto exitL; }
			prevLineS = jpgP->imgSIT;
			prevLineC = jpgP->imgCIT;
		}
		
		if( rstxF  && rstInterval ) 
			indexBuf += (UInt32)(rstxP - strP);
		else 
			indexBuf += (UInt32)(dataP - strP);
			
		if( i == (mcuJpegY - 1) ) indexBuf -= 4 - (regIndex >> 3);
			
		jpgP->scanInfo.scanIndex = indexBuf;
		if( (strP = (UInt8*)jpgP->storageP->refreshP( jpgP->storageP->dataP, indexBuf, sizeBuf, 0 )) == 0 ) goto exitL;
		rstxP = dataP = strP;
	}

	if( OneStepProc ) 
		OneStepProc( (void*)imageP, prevLineS, jpgP->imgSIT, prevLineC, jpgP->imgCIT, jpgP->imgRotate );

exitL:

	if( duLineTP[0] ) envP->freeP( duLineTP[0] );
	if( duLineTP[2] ) envP->freeP( duLineTP[2] );
	if( duLineTP[4] ) envP->freeP( duLineTP[4] );
	if( duLineTP[6] ) envP->freeP( duLineTP[6] );

	return err;
}

#undef BitSkipNbits
#undef BitReadNextByte
#undef BitCacheUpdate
#undef BitCacheChkAndUpdate
#undef BitReadAndInvertNbits


UInt32 _PrvGetSize( void * p ); UInt32 _PrvGetSize( void * p ) { return 100000000; }
UInt32 _PrvResize( void * p, unsigned long s ); UInt32 _PrvResize( void * p, unsigned long s ) { return 1; }
UInt8* _PrvRefresh( void * p, unsigned long pos, unsigned long size, unsigned long * err); UInt8* _PrvRefresh( void * p, unsigned long pos, unsigned long size, unsigned long * err) { return (unsigned char*)p + pos; }

Result PrvLoadThumb( JPEG_FormatType *jpgP, ImageType *imageP, UInt32 w, UInt32 h, UInt32 flag, EnvType *envP )
{
	Result res = resOk;
	UInt8 * bufP = 0;
	UInt32 indexBuf;
	
	if( !jpgP || !imageP || !envP ) return resNull;
	if( !jpgP->thumb.thumbnailIdx ) return resFail;

	// prepare
	indexBuf = jpgP->thumb.thumbnailIdx;
	if( jpgP->storageP->getsizeP( jpgP->storageP->dataP ) < jpegSegmentSize ) 
		if( resOk != (res = jpgP->storageP->resizeP( jpgP->storageP->dataP, jpegSegmentSize )) ) goto exitL;	
	// 
	if( (bufP = (UInt8*)jpgP->storageP->refreshP( jpgP->storageP->dataP, indexBuf, jpegSegmentSize, 0 )) == 0) goto exitL;	

	// thumb type
	if( jpgP->thumb.thumbnail == 1 )	// jpeg
	{
		StorageType *strP=0L;
		void *jpgTP=0L;
		// create storage type
		strP = (StorageType*)envP->allocP( sizeof(StorageType) );
		if( strP )
		{
			//
			strP->dataP = bufP;
			strP->getsizeP = _PrvGetSize;
			strP->refreshP = _PrvRefresh;
			strP->resizeP = _PrvResize;
			// open and load image
			res = UTF_JPEG_Open( &jpgTP, strP, envP );		
			if( res == resOk )
			{
				res = UTF_JPEG_Load( jpgTP, imageP, w, h, flag, 0, envP );
				UTF_JPEG_Close( &jpgTP, envP );
			}
			envP->freeP( strP );
		}
	}
	else { res = resFail; goto exitL; }		// h-z
	
exitL:
	
	return res;
}

Result UTF_JPEG_GetThumbnail( void * jpgH, ImageType * imageP, unsigned long w, unsigned long h, unsigned long flags, EnvType * envP )
{
	Result res = resBreak;
	JPEG_FormatType * jpgP;
	
	if( !jpgH || !imageP || !envP ) return resNull;
	
	jpgP = (JPEG_FormatType*)jpgH;
		
	if( jpgP->thumb.thumbnailIdx ) 
	{
		if( jpgP->thumb.thumbnail ) 
		{
			res = PrvLoadThumb( jpgP, imageP, w, h, flags, envP ); 
		}
	}	

	return res;
}

Result UTF_JPEG_Load( void * jpgH, ImageType * imageP, unsigned long w, unsigned long h, unsigned long flags, unsigned short scaleFactor, EnvType * envP )
{
	Result err = resOk;
	JPEG_FormatType * jpgP = 0L;
	//UInt32 size = 0;
	UInt8 zoomIn = 0, noZoom = 0;
	Int16 crCnvT[] = jpegCrConvertTable;
	Int16 cbCnvT[] = jpegCbConvertTable;
	
	DBG_assert0( jpgH );
	DBG_assert0( imageP );
	
	if( !jpgH || !imageP || !envP ) return resNull;
	if( scaleFactor > 3 ) return resFail;
	
	jpgP = (JPEG_FormatType*)jpgH;
		
	if( (!CHK_FLAG( jpgP, jpegSOF0Present ) && !CHK_FLAG( jpgP, jpegSOF2Present )) || !CHK_FLAG( jpgP, jpegDHTPresent ) ||
		!CHK_FLAG( jpgP, jpegDQTPresent ) || !CHK_FLAG( jpgP, jpegSOSPresent ) ) return resWantOfBlocks;


	jpgP->mcuInfo.DC[compY] = jpgP->mcuInfo.DC[compCr] = jpgP->mcuInfo.DC[compCb] = 0;
	
	MCopy4( (UInt32*)cbCnvT, (UInt32*)jpgP->cbCnv, 128 );
	MCopy4( (UInt32*)crCnvT, (UInt32*)jpgP->crCnv, 128 );

	//
	zoomIn = (UInt8)((flags & DCIM_ZOOMIN) == DCIM_ZOOMIN);
	noZoom = (UInt8)((flags & DCIM_NOZOOM) == DCIM_NOZOOM);
	if( scaleFactor && !zoomIn )
	{
		if( (jpgP->width>>scaleFactor) < w || (jpgP->height>>scaleFactor) < h ) zoomIn = 1;
	}

	//
	g_decodingState = 1;

	if( CHK_FLAG( jpgP, jpegSingleScan ) )
	{
		// 
		if( 1 == jpgP->jpegInfo.compNum ) // grayscale
		{
			if( resOk != (err = PrvZoomPrepare( jpgP, jpgP->width>>scaleFactor, jpgP->height>>scaleFactor, w, h, zoomIn, noZoom, (flags&DCIM_IsRotate), envP )) ) goto exit1L;
			if( resOk != (err = UTF_DCIM_CreateImage( imageP, (UInt16)(jpgP->imgW), (UInt16)(jpgP->imgH), envP )) ) goto exit1L;
			MSet4( imageP->dataCbCrP, 0x80808080, (Round4(jpgP->imgW) * Round2(jpgP->imgH)) >> 3 );
			err = PrvJpegDecodeBaselineScan( jpgP, imageP, scaleFactor, Cmpl_SingleScan1, envP->onestepP, envP );
		}
		else if( 3 == jpgP->jpegInfo.compNum ) // rgb
		{
			if( resOk != (err = PrvZoomPrepare( jpgP, jpgP->width>>scaleFactor, jpgP->height>>scaleFactor, w, h, zoomIn, noZoom, (flags&DCIM_IsRotate), envP )) ) goto exit1L;
			if( resOk != (err = UTF_DCIM_CreateImage( imageP, (UInt16)(jpgP->imgW), (UInt16)(jpgP->imgH), envP )) ) goto exit1L;
			err = PrvJpegDecodeBaselineScan( jpgP, imageP, scaleFactor, Cmpl_SingleScan3, envP->onestepP, envP );
		}
		else if( 4 == jpgP->jpegInfo.compNum ) // cmyk
		{
			if( resOk != (err = PrvZoomPrepare( jpgP, jpgP->width>>scaleFactor, jpgP->height>>scaleFactor, w, h, zoomIn, noZoom, (flags&DCIM_IsRotate), envP )) ) goto exit1L;
			if( resOk != (err = UTF_DCIM_CreateImage( imageP, (UInt16)(jpgP->imgW), (UInt16)(jpgP->imgH), envP )) ) goto exit1L;
			err = PrvJpegDecodeBaselineScan( jpgP, imageP, scaleFactor, Cmpl_SingleScan4, envP->onestepP, envP );
		}
		else
		{
			err = resNoJPEG;
		}

exit1L:
		PrvZoomClose( jpgP, envP );
	}
	else 
	{
		err = resNoJPEG;
	}
	
	//
	g_decodingState = 0;

	return err;
}


Result UTF_JPEG_Open( void ** jpgHP, StorageType * srcP, EnvType * envP )
{
	Result err = resOk;
	JPEG_FormatType * jpgP;
	UInt32 unZigzag[] = jpegUnZigzagTable;		
		
	DBG_assert0( jpgHP );
	DBG_assert0( srcP );
	
	*jpgHP = 0L;
	
	jpgP = (JPEG_FormatType*)envP->allocP( sizeof( struct JPEG_FormatType ) );
	if( !jpgP ) return resNoMem;
	MSet4( jpgP, 0, sizeof( struct JPEG_FormatType ) >> 2 );

	jpgP->storageP = srcP;		

	CLR_FLAGS( jpgP );

	MCopy4( unZigzag, jpgP->unZigZagT, 64 );

	err = JPEG_JpegScan( jpgP, srcP, envP );
    if( resStartOfScan != err ) goto exitErrL;

	if( (!CHK_FLAG( jpgP, jpegSOF0Present ) && !CHK_FLAG( jpgP, jpegSOF2Present )) || !CHK_FLAG( jpgP, jpegDHTPresent ) ||
		!CHK_FLAG( jpgP, jpegDQTPresent ) || !CHK_FLAG( jpgP, jpegSOSPresent ) )
	{
		err = resWantOfBlocks;
		goto exitErrL;
	}

	*jpgHP = jpgP;

	// JPEG part
	jpgP->len = sizeof( JPEG_FormatType );
	jpgP->width = jpgP->jpegInfo.width; 
	jpgP->height = jpgP->jpegInfo.height;
	jpgP->bitPerUnit = 8;
	jpgP->components = jpgP->jpegInfo.compNum;
	jpgP->planes = 1;

	return resOk;

exitErrL:

	if( jpgP ) envP->freeP( jpgP );	
	
	return err;
}

Result UTF_JPEG_Close( void ** jpgHP, EnvType * envP )
{
	DBG_assert0( jpgHP );
	DBG_assert0( *jpgHP );

	if( *jpgHP ) envP->freeP( *jpgHP );
	
	*jpgHP = 0L;
	
	return resOk;
}

Result UTF_JPEG_GetInfo( void * jpgH, JpegInfoType * infP, EnvType * envP )
{
	JPEG_FormatType * jpgP;
	
	if( !jpgH || !infP ) return resNull;
	
	jpgP = (JPEG_FormatType*)jpgH;
	if( !jpgP ) return resNull;
	
	infP->width = (UInt16)jpgP->width;
	infP->height = (UInt16)jpgP->height;
	infP->cNum = (UInt16)jpgP->components;

	return resOk;
}

Result UTF_JPEG_StopDecoding()
{
	g_decodingState = 0;

	return resOk;
}

Result UTF_JPEG_IsJPEG( StorageType * srcP )
{
	UInt8 * bufP = 0;
	Result err;
	
	if( !srcP ) return resNull;
	
	bufP = srcP->refreshP( srcP->dataP, 0, 2, &err );
	if( resOk != err && resEOB != err ) return resNull;

	if( 0xff != bufP[0] || 0xd8 != bufP[1] ) return resNull;
	
	return resOk;
}
