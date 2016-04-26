// Copyright (C) 2014       Hykem <hykem@hotmail.com>
// Licensed under the terms of the GNU GPL, version 3
// http://www.gnu.org/licenses/gpl-3.0.txt

/*
	Copyright (c) 2005  adresd
	Copyright (c) 2005  Marcus R. Brown
	Copyright (c) 2005  James Forshaw
	Copyright (c) 2005  John Kelley
	Copyright (c) 2005  Jesper Svennevid
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:
	1. Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.
	3. The names of the authors may not be used to endorse or promote products
	derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
	IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Unpack PBP comes from "PSPSDK - PSPDEV Open Source Project" (check above disclaimer).
// Decrypt PGD is based on "pgdecrypt" by tpunix. 

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

extern "C" {
	#include "libkirk/kirk_engine.h"
	#include "libkirk/amctrl.h"
	#include "libkirk/des.h"
}

#include "utils.h"

static unsigned char des_key[0x8] = {0x39, 0xF7, 0xEF, 0xA1, 0x6C, 0xCE, 0x5F, 0x4C};
static unsigned char des_iv[0x8] = {0xA8, 0x19, 0xC4, 0xF5, 0xE1, 0x54, 0xE3, 0x0B};

static unsigned char dnas_key1A90[] = {0xED, 0xE2, 0x5D, 0x2D, 0xBB, 0xF8, 0x12, 0xE5, 0x3C, 0x5C, 0x59, 0x32, 0xFA, 0xE3, 0xE2, 0x43};
static unsigned char dnas_key1AA0[] = {0x27, 0x74, 0xFB, 0xEB, 0xA4, 0xA0, 0x01, 0xD7, 0x02, 0x56, 0x9E, 0x33, 0x8C, 0x19, 0x57, 0x83};

static unsigned char pops_key[] = {0x2E, 0x41, 0x17, 0xA5, 0x32, 0xE6, 0xC4, 0x73, 0x71, 0x7B, 0x0F, 0x7A, 0x6E, 0xC0, 0xAA, 0xA5};

// Structure to describe the header of a PGD file.
typedef struct {
	unsigned char vkey[16];

	int open_flag;
	int key_index;
	int drm_type;
	int mac_type;
	int cipher_type;

	int data_size;
	int align_size;
	int block_size;
	int block_nr;
	int data_offset;
	int table_offset;

	unsigned char *buf;
} PGD_HEADER;

// Structure to describe the header of a PBP file.
typedef struct {
	char     signature[4];
	int      version;
	int      offset[8];
} PBP_HEADER;

// Correct PBP signature.
static char pbp_sig[4] = {
	0x00,
	0x50,   // P
	0x42,   // B
	0x50    // P
};

// Names of files included in a PBP.
static char *pbp_filenames[8] = {
	"PARAM.SFO",
	"ICON0.PNG",
	"ICON1.PMF",
	"PIC0.PNG",
	"PIC1.PNG",
	"SND0.AT3",
	"DATA.PSP",
	"DATA.PSAR"
};

int decrypt_pgd(unsigned char* pgd_data, int pgd_size, int flag, unsigned char* key);
int decrypt_doc(unsigned char* data, int size);
int unpack_pbp(FILE *infile);