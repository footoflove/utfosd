
#ifndef _UTF_DCIMAGE_H
#define _UTF_DCIMAGE_H

/////////////////////////////////////////
//
#ifdef  __cplusplus
extern "C" {
#endif
Result		UTF_DCIM_CreateImage( ImageType * imageP, unsigned short width, unsigned short height, EnvType * envP );
Result		UTF_DCIM_KillImage( ImageType * imageP, EnvType * envP );
Boolean		UTF_DCIM_IsImageCreated( ImageType * imageP );
void		UTF_DCIM_Zoom( ImageType * srcP, ImageType * trgP, unsigned short width, unsigned short height, unsigned long flag, EnvType * envP );
Result		UTF_DCIM_Rotate( ImageType *srcIP, ImageType *trgIP, unsigned short rotate, EnvType * envP );
#ifdef  __cplusplus
}
#endif


#endif // DCIMAGE_H
