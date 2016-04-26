// Copyright (C) 2014       Hykem <hykem@hotmail.com>
// Licensed under the terms of the GNU GPL, version 3
// http://www.gnu.org/licenses/gpl-3.0.txt

#define _CRT_SECURE_NO_WARNINGS

#include <direct.h>

#include "cdrom.h"
#include "lz.h"
#include "crypto.h"

// Multidisc ISO image signature.
char multi_iso_magic[0x10] = {
	0x50,  // P
	0x53,  // S
	0x54,  // T
	0x49,  // I
	0x54,  // T
	0x4C,  // L
	0x45,  // E
	0x49,  // I
	0x4D,  // M
	0x47,  // G
	0x30,  // 0
	0x30,  // 0
	0x30,  // 0
	0x30,  // 0
	0x30,  // 0
	0x30   // 0
};

// ISO image signature.
char iso_magic[0xC] = {
	0x50,  // P
	0x53,  // S
	0x49,  // I
	0x53,  // S
	0x4F,  // O
	0x49,  // I
	0x4D,  // M
	0x47,  // G
	0x30,  // 0
	0x30,  // 0
	0x30,  // 0
	0x30   // 0
};

// CUE structure
typedef struct {
	unsigned short   type;		    // Track Type = 41h for DATA, 01h for CDDA
	unsigned char    number;		// Track Number (01h to 99h)
	unsigned char    I0m;		    // INDEX 00 MM
	unsigned char    I0s;		    // INDEX 00 SS
	unsigned char    I0f;		    // INDEX 00 FF
	unsigned char	 padding;       // NULL
	unsigned char    I1m;		    // INDEX 01 MM
	unsigned char    I1s;		    // INDEX 01 SS
	unsigned char    I1f;		    // INDEX 01 FF
} CUE_ENTRY;

// CDDA table entry structure.
typedef struct {
	unsigned int     offset;
	unsigned int     size;
	unsigned char    padding[0x4];
	unsigned char	 checksum[0x4];
} CDDA_ENTRY;

// ISO table entry structure.
typedef struct {
	unsigned int     offset;
	unsigned short   size;
	unsigned short   marker;			// 0x01 or 0x00
	unsigned char	 checksum[0x10];	// First 0x10 bytes of sha1 sum of 0x10 disc sectors
	unsigned char	 padding[0x8];
} ISO_ENTRY;

// STARTDAT header structure.
typedef struct {
	unsigned char    magic[8];		// STARTDAT
	unsigned int	 unk1;			// 0x01
	unsigned int	 unk2;			// 0x01
	unsigned int	 header_size;
	unsigned int	 data_size;
} STARTDAT_HEADER;

// SIMPLE header structure.
typedef struct {
	unsigned char    magic[8];		// SIMPLE__
	unsigned int	 unk1;			// 0x64
	unsigned int	 unk2;			// 0x01
	unsigned int	 data_size;
	unsigned int	 unk3;			// 0 or chcksm
	unsigned int	 unk4;			// 0 or chcksm
} SIMPLE_HEADER;