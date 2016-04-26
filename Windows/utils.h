// Copyright (C) 2014       Hykem <hykem@hotmail.com>
// Licensed under the terms of the GNU GPL, version 3
// http://www.gnu.org/licenses/gpl-3.0.txt

#include <string.h>

typedef unsigned long long u64;
typedef unsigned int   u32;
typedef unsigned short u16;
typedef unsigned char  u8;

u16 se16(u16 i);
u32 se32(u32 i);
u64 se64(u64 i);

unsigned char* strip_utf8(unsigned char *src, int size);
bool isEmpty(unsigned char* buf, int buf_size);