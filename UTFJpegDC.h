
#ifndef _UTF_JPEGDC_H
#define _UTF_JPEGDC_H

/*
#ifndef WIN32
	#define _AASM_USE_
#endif
*/

#define DCIM_ZOOMIN			1
#define DCIM_NOZOOM			2
#define DCIM_PROGRESS		4
#define DCIM_NOTHUMB		8
#define DCIM_QUALITYZOOM	0x10

#define DCIM_Rotate90CW				0x100
#define DCIM_Rotate90CCW			0x200
#define DCIM_Rotate180				0x400
#define DCIM_IsRotate				(DCIM_Rotate90CW|DCIM_Rotate90CCW|DCIM_Rotate180)
#define DCIM_IsRotate90				(DCIM_Rotate90CW|DCIM_Rotate90CCW)

typedef	void* AllocMemPrc(unsigned long);
typedef	void FreeMemPrc(void*);
typedef long OneStepCallbackPrc(void*,unsigned long,unsigned long,unsigned long,unsigned long,unsigned long);

typedef struct
{
	// 
	AllocMemPrc			*allocP;
	FreeMemPrc			*freeP;
	OneStepCallbackPrc	*onestepP;

} EnvType;


/*
RGB to YCbCr Conversion
    
YCbCr (256 levels) can be computed directly from 8-bit RGB as follows:
Y   =     0.299  R + 0.587  G + 0.114  B
Cb  =   - 0.1687 R - 0.3313 G + 0.5    B + 128
Cr  =     0.5    R - 0.4187 G - 0.0813 B + 128


    
YCbCr to RGB Conversion
    
RGB can be computed directly from YCbCr (256 levels) as follows:
R = Y                    + 1.402   (Cr-128)
G = Y - 0.34414 (Cb-128) - 0.71414 (Cr-128)
B = Y + 1.772   (Cb-128)

Attention! 
It is necessary for you to release memory only <dataYP>, but no <dataCbCrP>.
It is allocated as follows:
{
	ImageType img;

	img.dataYP = alloc( size_of_Y_component + size_of_Cb_component + size_of_Cr_component );
	img.dataCbCrP = img.dataYP + size_of_Y_component;
}
*/
typedef struct
{
	unsigned short		width;				// for Y and CbCr
	unsigned short		height;				// for Y; <heightCbCr> = <height>/2
	unsigned short		rowByte;			// lenght of line of pixels (Y comp) in bytes
	unsigned char		*dataYP;			// Y
	unsigned char		*dataCbCrP;			// Cb, Cr

} ImageType;


typedef	unsigned char* RefreshPrc(void*,unsigned long,unsigned long,unsigned long*);
typedef	unsigned long ResizePrc(void*,unsigned long);
typedef	unsigned long GetSizePrc(void*);

typedef struct
{
	//
	void			*dataP;
	//
	RefreshPrc		*refreshP;
	ResizePrc		*resizeP;
	GetSizePrc		*getsizeP;

} StorageType;


#ifdef  __cplusplus
extern "C" {
#endif

unsigned long	UTFLoadJpeg( StorageType *srcP, ImageType *imgP, long width, long height, unsigned long flags, EnvType *envP );
unsigned long	UTFLoadThumb( StorageType *srcP, ImageType *imgP, long w, long h, unsigned long flags, EnvType *envP );
void			UTFStoreAsBMP( ImageType * imgP, void ** trgTP, unsigned long * sizeP, EnvType * envP );
void			UTFToStopDecoding();

#ifdef  __cplusplus
}
#endif


#endif //JPEGDC_H