
#ifndef _UTF_JPEG_COMMON_H
#define _UTF_JPEG_COMMON_H

#ifdef DBG
#define DBG_assert0(x)		ASSERT(x)
#else
#define DBG_assert0(x)		
#endif

#define Round2( w )		((((w)+1)>>1)<<1)
// выравнивание по границе 4
#define Round4( w )		((((w)+3)>>2)<<2)
// выравнивание по границе 8
#define Round8( w )		((((w)+7)>>3)<<3)
// выравнивание по границе 16
#define Round16( w )	((((w)+15)>>4)<<4)
// выравнивание по границе 32
#define Round32( w )	((((w)+31)>>5)<<5)

// LittleEndian <-> BigEndian
#ifndef ByteSwap32 
#define ByteSwap32(n) \
	( ((((unsigned long) n) << 24) & 0xFF000000) |	\
	  ((((unsigned long) n) <<  8) & 0x00FF0000) |	\
	  ((((unsigned long) n) >>  8) & 0x0000FF00) |	\
	  ((((unsigned long) n) >> 24) & 0x000000FF) )
#endif

#ifndef ByteSwap16 
#define ByteSwap16(n) \
	( ((((unsigned short) n) << 8) & 0xFF00) | \
	  ((((unsigned short) n) >> 8) & 0x00FF) )
#endif    

// 
#define Swapv( w, h ) (w) ^= (h); \
	(h) ^= (w); \
	(w) ^= (h)

typedef unsigned char	UInt8;
typedef signed char		Int8;
typedef unsigned short	UInt16;
typedef signed short	Int16;
typedef unsigned long	UInt32;
typedef signed long		Int32;

typedef unsigned long	EmPtr;
typedef unsigned char	Boolean;
typedef unsigned long	Result;

#ifdef  __cplusplus
extern "C" {
#endif

void MSet4( void * p, unsigned long val, unsigned long size );
void MCopy4( void * s, void * t, unsigned long size );

#ifdef  __cplusplus
}
#endif

#endif // JPEG_COMMON_H
