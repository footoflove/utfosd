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

#ifndef __UTF_MEMORY_H__
#define __UTF_MEMORY_H__

#ifdef __cplusplus
extern "C" {
#endif

DWORD UTFMalloc(DWORD size);
void UTFFree(DWORD address);

#ifdef __cplusplus
}
#endif

#endif

