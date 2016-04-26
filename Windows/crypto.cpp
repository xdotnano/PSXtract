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

#include "crypto.h"

int decrypt_pgd(unsigned char* pgd_data, int pgd_size, int flag, unsigned char* key)
{
	int result;
	PGD_HEADER PGD[sizeof(PGD_HEADER)];
	MAC_KEY mkey;
	CIPHER_KEY ckey;
	unsigned char*fkey;

	// Read in the PGD header parameters.
	memset(PGD, 0, sizeof(PGD_HEADER));

	PGD->buf = pgd_data;
	PGD->key_index = *(u32*)(pgd_data + 4);
	PGD->drm_type  = *(u32*)(pgd_data + 8);

	// Set the hashing, crypto and open modes.
	if (PGD->drm_type == 1)
	{
		PGD->mac_type = 1;
		flag |= 4;

		if(PGD->key_index > 1)
		{
			PGD->mac_type = 3;
			flag |= 8;
		}
		PGD->cipher_type = 1;
	}
	else
	{
		PGD->mac_type = 2;
		PGD->cipher_type = 2;
	}
	PGD->open_flag = flag;

	// Get the fixed DNAS key.
	fkey = NULL;
	if((flag & 0x2) == 0x2)
		fkey = dnas_key1A90;
	if((flag & 0x1) == 0x1)
		fkey = dnas_key1AA0;

	if (fkey == NULL)
	{
		printf("PGD: Invalid DNAS flag! %08x\n", flag);
		return -1;
	}

	// Test MAC hash at 0x80 (DNAS hash).
	sceDrmBBMacInit(&mkey, PGD->mac_type);
	sceDrmBBMacUpdate(&mkey, pgd_data, 0x80);
	result = sceDrmBBMacFinal2(&mkey, pgd_data + 0x80, fkey);

	if (result)
	{
		printf("PGD: Invalid 0x80 MAC hash!\n");
		return -1;
	}

	// Test MAC hash at 0x70 (key hash).
	sceDrmBBMacInit(&mkey, PGD->mac_type);
	sceDrmBBMacUpdate(&mkey, pgd_data, 0x70);

	// If a key was provided, check it against MAC 0x70.
	if (!isEmpty(key, 0x10))
	{
		result = sceDrmBBMacFinal2(&mkey, pgd_data + 0x70, key);
		if (result)
		{
			printf("PGD: Invalid 0x70 MAC hash!\n");
			return -1;
		}
		else
		{
			memcpy(PGD->vkey, key, 16);
		}
	}
	else
	{
		// Generate the key from MAC 0x70.
		bbmac_getkey(&mkey, pgd_data + 0x70, PGD->vkey);
	}

	// Decrypt the PGD header block (0x30 bytes).
	sceDrmBBCipherInit(&ckey, PGD->cipher_type, 2, pgd_data + 0x10, PGD->vkey, 0);
	sceDrmBBCipherUpdate(&ckey, pgd_data + 0x30, 0x30);
	sceDrmBBCipherFinal(&ckey);

	// Get the decryption parameters from the decrypted header.
	PGD->data_size   = *(u32*)(pgd_data + 0x44);
	PGD->block_size  = *(u32*)(pgd_data + 0x48);
	PGD->data_offset = *(u32*)(pgd_data + 0x4c);

	// Additional size variables.
	PGD->align_size = (PGD->data_size + 15) &~ 15;
	PGD->table_offset = PGD->data_offset + PGD->align_size;
	PGD->block_nr = (PGD->align_size + PGD->block_size - 1) &~ (PGD->block_size - 1);
	PGD->block_nr = PGD->block_nr / PGD->block_size;

	if ((PGD->align_size + PGD->block_nr * 16) > pgd_size)
	{
		printf("PGD: Invalid data size!\n");
		return -1;
	}

	// Test MAC hash at 0x60 (table hash).
	sceDrmBBMacInit(&mkey, PGD->mac_type);
	sceDrmBBMacUpdate(&mkey, pgd_data + PGD->table_offset, PGD->block_nr * 16);
	result = sceDrmBBMacFinal2(&mkey, pgd_data + 0x60, PGD->vkey);

	if(result)
	{
		printf("PGD: Invalid 0x60 MAC hash!\n");
		return -1;
	}

	// Decrypt the data.
	sceDrmBBCipherInit(&ckey, PGD->cipher_type, 2, pgd_data + 0x30, PGD->vkey, 0);
	sceDrmBBCipherUpdate(&ckey, pgd_data + 0x90, PGD->align_size);
	sceDrmBBCipherFinal(&ckey);

	return PGD->data_size;
}

int decrypt_doc(unsigned char* data, int size)
{
	data += 0x10;  // Skip dummy PGD header.
	size -= 0x10;  // Adjust size.

	unsigned char *out = new unsigned char[size];

	// Perform DES CBC decryption.
	des_context ctx;
	des_setkey_dec(&ctx, des_key);
	des_crypt_cbc(&ctx, DES_DECRYPT, size, des_iv, data, out);

	// Check for "DOC" header in the decrypted data.
	if ((out[0] == 0x44) &&
		(out[1] == 0x4F) &&
		(out[2] == 0x43) &&
		(out[3] == 0x20))
	{
		// Copy back the decrypted data.
		memcpy(data - 0x10, out, size);
		delete[] out;
		return 0;
	}
	else
	{
		delete[] out;
		return -1;
	}
}

int unpack_pbp(FILE *infile) 
{
	int maxbuffer = 16 * 1024 * 1024;
	PBP_HEADER header;
	int loop0;
	int total_size;

	// Get the size of the PBP
	fseek(infile, 0, SEEK_END);
	total_size = ftell(infile);
	fseek(infile, 0, SEEK_SET);

	if (total_size < 0) {
		printf("UNPACK_PBP ERROR: Could not get the input file size.\n");
		return -1;
	}

	// Read in the header
	if (fread(&header, sizeof(PBP_HEADER), 1, infile) < 0) {
		printf("UNPACK_PBP ERROR: Could not read the input file header.\n");
		return -1;
	}

	// Check the signature
	for (loop0 = 0; loop0 < sizeof(pbp_sig); loop0++) {
		if (header.signature[loop0] != pbp_sig[loop0]) {
			printf("UNPACK_PBP ERROR: Input file is not a PBP file.\n");
			return -1;
		}
	}

	// For each file in the PBP
	for (loop0 = 0; loop0 < 8; loop0++) {
		void *buffer;
		int size;

		// Get the size of this file
		if (loop0 == 7) {
			size = total_size - header.offset[loop0];
		} else {
			size = header.offset[loop0 + 1] - header.offset[loop0];
		}

		// Print out the file details
		printf("[%d] %10d bytes | %s\n", loop0, size, pbp_filenames[loop0]);

		// Skip the file if empty
		if (!size) continue;

		// Seek to the proper position in the file
		if (fseek(infile, header.offset[loop0], SEEK_SET) != 0) {
			printf("UNPACK_PBP ERROR: Could not seek in the input file.\n");
			return -1;
		}

		// Open the output file
		FILE *outfile = fopen(pbp_filenames[loop0], "wb");
		if (outfile == NULL) {
			printf("UNPACK_PBP ERROR: Could not open the output file. (%s)\n", pbp_filenames[loop0]);
			return -1;
		}

		do {
			int readsize;

			// Make sure we don't exceed the maximum buffer size
			if (size > maxbuffer) {
				readsize = maxbuffer;
			} else {
				readsize = size;
			}
			size -= readsize;

			// Create the read buffer
			buffer = malloc(readsize);
			if (buffer == NULL) {
				printf("UNPACK_PBP ERROR: Could not allocate the section data buffer. (%d)\n", readsize);
				return -1;
			}

			// Read in the data from the PBP
			if (fread(buffer, readsize, 1, infile) < 0) {
				printf("UNPACK_PBP ERROR: Could not read in the section data.\n");
				return -1;
			}

			// Write the contents of the buffer to the output file
			if (fwrite(buffer, readsize, 1, outfile) < 0) {
				printf("UNPACK_PBP ERROR: Could not write out the section data.\n");
				return -1;
			}

			// Clean up the buffer
			free(buffer);

			// Repeat if we haven't finished writing the file
		} while (size);

		// Close the output file
		if (fclose(outfile) < 0) {
			printf("UNPACK_PBP ERROR: Could not close the output file.\n");
			return -1;
		}

	}

	// Close the PBP
	if (fclose(infile) < 0) {
		printf("UNPACK_PBP ERROR: Could not close the input file.\n");
		return -1;
	}

	// Exit successful
	return 0;
}