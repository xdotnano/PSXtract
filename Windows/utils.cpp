// Copyright (C) 2014       Hykem <hykem@hotmail.com>
// Licensed under the terms of the GNU GPL, version 3
// http://www.gnu.org/licenses/gpl-3.0.txt

#include "utils.h"

// Auxiliary functions.
u16 se16(u16 i)
{
	return (((i & 0xFF00) >> 8) | ((i & 0xFF) << 8));
}

u32 se32(u32 i)
{
	return ((i & 0xFF000000) >> 24) | ((i & 0xFF0000) >>  8) | ((i & 0xFF00) <<  8) | ((i & 0xFF) << 24);
}

u64 se64(u64 i)
{
	return ((i & 0x00000000000000ff) << 56) | ((i & 0x000000000000ff00) << 40) |
		((i & 0x0000000000ff0000) << 24) | ((i & 0x00000000ff000000) <<  8) |
		((i & 0x000000ff00000000) >>  8) | ((i & 0x0000ff0000000000) >> 24) |
		((i & 0x00ff000000000000) >> 40) | ((i & 0xff00000000000000) >> 56);
}

unsigned char* strip_utf8(unsigned char *src, int size)
{
	unsigned char* ret = new unsigned char[size];
	int index = 0;
	int i;

    for (i = 0; i < size; i++)
	{
		if (src[i] == 0)
		{
			ret[index++] = '\0';
			break;
		}
		else if (src[i] <= 0x80)
			ret[index++] = src[i];
	}

    return ret;
}

bool isEmpty(unsigned char* buf, int buf_size)
{
	if (buf != NULL)
	{
		int i;
		for(i = 0; i < buf_size; i++)
		{
			if (buf[i] != 0) return false;
		}
	}
	return true;
}