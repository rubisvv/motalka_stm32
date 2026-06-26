/*
 * bcd_dhalt.c
 *
 *  Created on: 17 июн. 2026 г.
 *      Author: dihalt
 */

#include <bcd_dihalt.h>
#include <stdio.h>
#include "bcd_dihalt.h"

inline static divmod10_t divmodu10(uint32_t n)
//divmod10_t divmodu10(uint32_t n)
{
	divmod10_t res;
	// умножаем на 0.8
	res.quot = n >> 1;
	res.quot += res.quot >> 1;
	res.quot += res.quot >> 4;
	res.quot += res.quot >> 8;
	res.quot += res.quot >> 16;
	uint32_t qq = res.quot;
	// делим на 8
	res.quot >>= 3;
	// вычисляем остаток
	res.rem = (uint8_t)(n - ((res.quot << 1) + (qq & ~7ul)));
	// корректируем остаток и частное
	if(res.rem > 9)
	{
		res.rem -= 10;
		res.quot++;
	}
	return res;
}

//uint32_t atou(const char *buffer);

//----------------------------------------------------------
char* utoa_fast_div(uint32_t value, char *buffer)
{
	uint8_t Ind;
	Ind = 0;
	buffer += 11;
	*--buffer = 0;
	do
	{
		divmod10_t res = divmodu10(value);
		*--buffer = res.rem + '0';
		value = res.quot;
		Ind++;

	}
	while (value != 0);
	while (Ind<4)
	{
	*--buffer = 0 + '0';
	Ind++;
	}
	return buffer;
}


//----------------------------------------------------------
char* utoa_fast_div_len(uint32_t value, char *buffer, uint8_t NumLen)
{
	uint8_t Ind;
	Ind = 0;

	buffer += 11;
	*--buffer = 0;
	do
	{
		divmod10_t res = divmodu10(value);
		*--buffer = res.rem + '0';
		value = res.quot;
		Ind++;

	}
	while (value != 0 && Ind<NumLen+1);
	while (Ind<NumLen)
	{
		*--buffer = 0 + '0';
		Ind++;
	}
	return buffer;
}


//=========================================================
uint16_t atou_1(const char *buffer)
{
	char ind;
	uint16_t val;
	char Ch;

	ind = 0;
	val = 0;
	//(char)buffer--;
	while ((ind < 4) && (*buffer != 0))
	{

		Ch = *buffer;
		val = ((val + (val << 2)) << 1);
		val = val + Ch - 48;
		ind++;
		(char)buffer++;
	}

	return val;
}



