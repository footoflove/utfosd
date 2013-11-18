
#include "UTFJpegDC.h"
#include "UTFJpegCommon.h"

#ifdef _AASM_USE_

void MSet4( void * p, unsigned long val, unsigned long size )
{
	__asm
	{
		stmfd	   	sp!, {r0-r2}
		mov			r0, p
		mov			r1, val
		mov			r2, size
	rep:	
		str			r1, [r0], #4
		subs		r2, r2, #1
		bne			rep
		ldmfd   	sp!, {r0-r2}		
	}
}

void MCopy4( void * s, void * t, unsigned long size )
{
	__asm
	{
		stmfd	   	sp!, {r0-r3}
		mov			r0, s
		mov			r1, t
		mov			r2, size
	rep:
		ldr			r3, [r0], #4
		str			r3, [r1], #4
		subs		r2, r2, #1
		bne			rep
		ldmfd   	sp!, {r0-r3}
	}
}


#else

void MSet4( void * p, unsigned long val, unsigned long size )
{
	UInt32 * trgP = (UInt32*)p;
	while( size -- ) { *trgP = val; ++ trgP; }
}

void MCopy4( void * s, void * t, unsigned long size )
{
	UInt32 * srcP = (UInt32*)s;
	UInt32 * trgP = (UInt32*)t;
	while( size -- ) { *trgP = *srcP; ++ trgP; ++ srcP; }
}

#endif