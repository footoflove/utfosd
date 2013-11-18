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
#include "UTFMath.h"
#include "UTFGraphPort.h"

double UTFAbs(double x)
{
	if(x < 0) return (-x);

    return x;
}

float UTFMin(float data1, float data2)
{
	float temp = data1-data2;

	if(temp < 0)
	{
		return data1;
	}

	return data2;
}

float UTFMax(float data1, float data2)
{
	float temp = data1-data2;

	if(temp < 0)
	{
		return data2;
	}

	return data1;
}

float UTFSqrt(float x)
{
	float xhalf = 0.5f*x;
	int i = *(int*)&x;

	if(x <= 0) return 0;
	if(x == 1.0) return x;

	i = 0x5f3759df - (i>>1);
	x = *(float*)&i;
	x = x*(1.5f - xhalf*x*x);

	return (1/x);
}

double UTFPow(float base, WORD exponent)
{
	double iResult = 1.0;
	DWORD i;

	if(base == 0)
	{
		return 0;
	}

	for(i=0; i<exponent; i++)
	{
		iResult *= base;
	}

	return iResult;
}

WORD UTFDistance2D(int x1, int y1, int x2, int y2)
{
	int deltax = x2-x1;
	int deltay = y2-y1;
	
	return (WORD)UTFSqrt(deltax*deltax+deltay*deltay);
}

/***************************************************************************/
//Start define function UTFFactorial

#define N 1000
#define MAXDATA	10000

static int g_m=1;
static int g_b=0;
static int calculate(unsigned int *s,int n)
{ 
	unsigned long p;
	unsigned long k=0;
	int i;

	for(i=g_b; i<g_m; i++) 
	{ 
		p = (long)s[i]*(long)n+k; 
		k = p/N;
		s[i] = p-k*N;
	}

	while(!s[g_b]) g_b++;
	for(k=p/N; k;) 
	{ 
		p = k;
		k = p/N;
		s[i++] = p-k*N;
		g_m++;
	}

	return g_m;
}

WORD UTFFactorial(WORD wSrcData, char *ptrBuf, DWORD dwSize)
{
	int data_size = MAXDATA*sizeof(unsigned int);
	int result_size = MAXDATA*3+12;
	unsigned int *pdata;
	int i,m,n;
	WORD wStrLen;
	char *result;
	char buf[10];

	if(wSrcData > MAXDATA)
		return 0;

	if(!ptrBuf)
		return 0;

	if(wSrcData < 2)
	{
		strcpy(ptrBuf, "1");
		return 1;
	}

#if(RUN_PC == TRUE)
	result = (char *)malloc(result_size+data_size);
#else
	result = (char *)UTF_OSMalloc(result_size+data_size);
#endif
	if(!result) return 0;
	memset(result, 0, result_size+data_size);

	pdata = (unsigned int *)(result+result_size);
	pdata[0] = 1;

	g_m=1; g_b=0;
	for(i=2; i<=wSrcData; i++)
	{
		m = calculate(pdata, i);
	}
	
	m = wSrcData-1;
	while(!pdata[m]) m--;

	sprintf(result, "%d", pdata[m--]);
	for(i=m; i>=0; i--)
	{
		sprintf(buf, "%03d", pdata[i]);
		strcat(result, buf);
	}

	wStrLen = strlen(result);
	if(dwSize > wStrLen)
	{
		strcpy(ptrBuf, result);
	}

#if(RUN_PC == TRUE)
	free(result);
#else
	UTF_OSFree((BYTE *)result);
#endif

	return wStrLen;
}

//End define function UTFFactorial
/***************************************************************************/

double UTFExp(double x)
{
	int tag=0;
	double s=0.0,a=1.0,n=1.0;

	if(x > 709.78)
		return 0;
	if(x < -12.0)
		return 0;

	if(x<0)
	{
		tag=1;
		x = -x;
	}

	while(1)
	{
		s += a;
		a = a/n*x;
		if(a < (1e-10)) break;
		n += 1.0;
	}

	if(tag)
	{
		if(s == 0)
			return 0;
		else
			s = 1/s;
	}

	return s;
}

/***************************************************************************/
//Start define function UTFLn
static double UTFLnEx(double x)
{
	double s=0,n=1.0,a,b,t;
	double dt=2.302585092994045;
	double ct=0;

	while(x < 0.1)
	{
		x *= 10.0;
		ct += dt;
	}

	b = x-1.0;
	a = t = b;
	do
	{
		s += t;
		a *= -b;
		n += 1.0;
		t = a/n;
	}while(UTFAbs(t) > (1e-10));

	return (s-ct);
}

double UTFLn(double x)
{
	double final;

	if((x <= 0) || (x == 1.0))
		return 0;

	if(x > 1.0)
		final = -UTFLnEx(1/x);
	else
		final = UTFLnEx(x);

	return final;
}

double UTFLog(double x)
{
	double final;

	final = UTFLn(x)/2.302585092994045;

	return final;
}

////////////////////////////////////////////////////////////////////////////////
float UTFSin(float angle)
{
	float a,temp;
	float final = 0;
	WORD down = 1;
	BYTE i,number = 5;
	int signal;

	if(angle < 0) 
		temp = 360.0;
	else
		temp = -360.0;
	while((angle < 0) || (angle >= 360.0))
	{
		angle += temp;
	}

	if(angle > 180.0)
	{
		angle -= 180.0;
		signal = -1;
	}
	else
	{
		signal = 1;
	}
	if(angle > 90.0)
	{
		angle = 180.0-angle;
	}

	a = temp = angle*PI/180.0;
	for(i=0; i<number; i++)
	{
		final += a;
		a = a*temp/(float)++down;
		a = -a*temp/(float)++down;
	}

	return (final*signal);
}

float UTFCos(float angle)
{
	float temp,a=1.0;
	float final = 0;
	WORD down = 0;
	BYTE i,number = 5;
	int signal;

	if(angle < 0) 
		temp = 360.0;
	else
		temp = -360.0;
	while((angle < 0) || (angle >= 360.0))
	{
		angle += temp;
	}

	if(angle > 270.0)
	{
		angle = 360.0-angle;
		signal = 1;
	}
	else if(angle > 180.0)
	{
		signal = -1;
		angle -= 180.0;
	}
	else if(angle > 90.0)
	{
		angle = 180.0-angle;
		signal = -1;
	}
	else
	{
		signal = 1;
	}

	temp = angle*PI/180.0;
	for(i=0; i<number; i++)
	{
		final += a;
		a = a*temp/(float)++down;
		a = -a*temp/(float)++down;
	}

	return (final*signal);
}

float UTFTan(float angle)
{
	float temp;

	if(angle < 0) 
		temp = 360.0;
	else
		temp = -360.0;
	while((angle < 0) || (angle >= 360.0))
	{
		angle += temp;
	}

	if((angle == 90.0) || (angle == 270.0))
		return 0;

	return (UTFSin(angle)/UTFCos(angle));
}

float UTFCTan(float angle)
{
	float temp;

	if(angle < 0) 
		temp = 360.0;
	else
		temp = -360.0;
	while((angle < 0) || (angle >= 360.0))
	{
		angle += temp;
	}

	if((angle == 180.0) || (angle == 0))
		return 0;

	return (UTFCos(angle)/UTFSin(angle));
}

////////////////////////////////////////////////////////////////////////////////

double UTFAtoF(char *string)
{
	int i,wStrlen = strlen(string);
	WORD wCount=0, wCount2=0;
	WORD wDotPos = wStrlen;
	int bFlag = 1;
	char cTopPos=0;
	double final = 0;
	double multi = 1.0;

	for(i=0; i<wStrlen; i++)
	{
		if((string[i] < '0') || (string[i] > '9'))
		{
			if(string[i] == '.')
			{
				if(i == 0) return 0;
				wCount++;
				wDotPos = i;
			}
			else if(string[i] == '-')
			{
				if(i != 0) return 0;
				bFlag = -1;
				cTopPos = 1;
			}
			else return 0;
		}
		else wCount2++;
	}

	if((wCount > 1) || (wCount2 == 0)) 
		return 0;

	for(i=wDotPos-1; i>=cTopPos; i--)
	{
		final += (double)(multi*(string[i]-'0'));
		multi *= 10.0;
	}
	multi = 0.1;
	for(i=wDotPos+1; i<wStrlen; i++)
	{
		final += multi*(string[i]-'0');
		multi *= 0.1;
	}

	final *= (double)bFlag;

	return final;
}

////////////////////////////////////////////////////////////////////////////////

static void UTFIntegerToStr(DWORD dwData, char *string, WORD size)
{
	char buffer[24] = {0};
	char text[24] = {0};
	BYTE i,digit;

	do
	{
		digit = dwData%10;
		strcpy(buffer, text);
		sprintf(text, "%d", digit);
		strcat(text, buffer);
		dwData /= 10;
	}while(dwData);

	if(size > strlen(text))
	{
		strcpy(string, text);
	}
}

void UTFDoubleToStr(double data, char *string, WORD size)
{
	double temp1,temp2;
	int i,iStrlen;
	DWORD dwTemp;
	DWORD digit;
	char final[96];
	char text[96];

	memset(text, 0, sizeof(final));
	if(data < 0)
	{
		strcpy(text, "-");
		data = -data;
	}
	if(data >= 1000000000000000000.0)
	{
		strcpy(string, "0");
		return;
	}
	
	temp1 = data/1000000000.0;
	if(temp1 >= 1.0)
	{
		iStrlen = strlen(text);
		UTFIntegerToStr(temp1, &text[iStrlen], sizeof(text)-iStrlen);
		temp1 = atoi(&text[iStrlen]);
		temp1 *= 1000000000.0;
	}
	else
	{
		temp1 = 0;
	}
	
	temp2 = data-temp1;
	iStrlen = strlen(text);
	UTFIntegerToStr(temp2, &text[iStrlen], sizeof(text)-iStrlen);

	dwTemp = temp2;
	temp1 = temp2-dwTemp;
	if(temp1 > 0)
	{
		int iPos = -1;

		strcat(text, ".");
		for(i=0; i<10; i++)
		{
			temp1 *= 10.0;
			digit = temp1;
			digit = digit%10;
			sprintf(final, "%d", digit);
			strcat(text, final);			
		}

		dwTemp = 0;
		iStrlen = strlen(text);
		for(i=0; i<iStrlen; i++)
		{
			if((text[i] >= '0') && (text[i] <= '9'))
			{
				dwTemp++;
				if(dwTemp > 18)
				{
					text[i] = 0;
					break;
				}
			}
		}

		iStrlen = strlen(text)-1;
		for(i=iStrlen; i>=0; i--)
		{
			if(text[i] == '0')
			{
				text[i] = 0;
			}
			else break;
		}

		iStrlen = strlen(text);
		for(i=0; i<iStrlen; i++)
		{
			if(text[i] == '.')
			{
				iPos = i+1;
			}
			else if(iPos > 0)
			{
				if(text[i] > '0')
				{
					iPos = i+1;
					dwTemp = 0;
				}
				else if(++dwTemp > 4)
				{
					text[iPos] = 0;
					break;
				}
			}
		}

		iStrlen = strlen(text)-1;
		if(iStrlen >= 0)
		{
			if(text[iStrlen] == '.')
			{
				text[iStrlen] = 0;
			}
		}
	}

	memset(string, 0, size);
	if(size > strlen(text))
	{
		strcpy(string, text);
	}
}
