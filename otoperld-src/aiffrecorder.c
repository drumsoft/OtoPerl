// OtoPerl - live sound programming environment with Perl.
// OtoPerl::aiffrecorder.c - sound recorder for OtoPerl.
/*
    OtoPerl - live sound programming environment with Perl.
    Copyright (C) 2011- Haruka Kataoka

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "aiffrecorder.h"

unsigned char aiffHeaders[] = {
  'F','O','R','M',  0,0,0,0,  'A','I','F','C',
  'F','V','E','R',  0,0,0,4,  0xA2,0x80,0x51,0x40,
  'C','O','M','M',  0,0,0,44,  0,0, 0,0,0,0, 0,0, 0,0,0,0,0,0,0,0,0,0,
    'f','l','3','2', 
    21, '3','2','-','b','i','t',' ',
        'F','l','o','a','t','i','n','g',' ','P','o','i','n','t',
  'S','S','N','D',  0,0,0,0,   0,0,0,0, 0,0,0,0,
};
enum {                 // [F]ORM = 0
	ContainerSize = 4, // 4bytes signed (set after record)
	// [F]VER=12, [C]OMM = 24, 
	Channnels = 32,    // 2bytes signed
	SampleFrames = 34, // 4bytes unsined (set after record)
	SampleSize = 38,   // 2bytes signed
	SampleRate = 40,   // 10bytes exteded80, [S]SND = 76
	SoundDataSize = 80,// 4bytes signed (set after record)
	aiffHeadersSize = 92
};

// ------------------------------------------------------ private functions
void convert_extended80(unsigned char * buffer, unsigned long value);

// ---------------------------------------------- implimentation

AiffRecorder *AiffRecorder_create(int channels, int bits, int sampleRate) {
	AiffRecorder *self = (AiffRecorder *)malloc(sizeof(AiffRecorder));
	if ( !self ) {
		printf("malloc failed for aiff recording.\n");
		return NULL;
	}
	self->framesize = bits / 8;
	self->channels = channels;

	size_t length = sizeof(aiffHeaders);
	self->headers = (unsigned char *)malloc(length);
	if ( ! self->headers ) {
		printf("malloc failed for aiff recording.\n");
		return NULL;
	}
	memcpy(self->headers, aiffHeaders, length);
	*(int16_t *)(self->headers + Channnels ) = htons((int16_t)channels);
	*(int16_t *)(self->headers + SampleSize) = htons((int16_t)bits);
	convert_extended80(self->headers + SampleRate, (unsigned long)sampleRate);

	return self;
}

void AiffRecorder_destroy(AiffRecorder *self) {
	free(self->headers);
	free(self);
}

bool AiffRecorder_open(AiffRecorder *self, const char *path) {
	int r;
	self->fh = fopen(path, "w+");
	if ( ! self->fh ) {
		printf("cannot open output file: %s\n", path);
		return false;
	}
	r = fwrite(self->headers, aiffHeadersSize, 1, self->fh);
	if ( r != 1 ) {
		printf("fwrite failed for aiff header output.\n");
		return false;
	}
	self->frames = 0;
	return true;
}

bool AiffRecorder_write32bit(AiffRecorder *self, const uint32_t *data, int frames) {
	int i, r;
	uint32_t f;
	for( i = 0; i < self->channels * frames; i++ ) {
		f = htonl(data[i]);
		r = fwrite( &f, 4, 1, self->fh );
		if ( r != 1 ) {
			printf("fwrite failed for aiff sound data output.\n");
			return false;
		}
	}
	self->frames += frames;
	return true;
}

bool AiffRecorder_close(AiffRecorder *self) {
	int r;
	int32_t dataSize = self->frames * self->framesize * self->channels;
	*(int32_t *)(self->headers + ContainerSize) = 
		htonl((int32_t)(dataSize + aiffHeadersSize - 8));
	*(uint32_t *)(self->headers + SampleFrames ) = 
		htonl((uint32_t)(self->frames));
	*(int32_t *)(self->headers + SoundDataSize) = 
		htonl((int32_t)(dataSize + 8));

	r = fseek(self->fh, 0, SEEK_SET);
	if ( r ) {
		printf("fseek failed for aiff header output.\n");
		return false;
	}
	r = fwrite(self->headers, aiffHeadersSize, 1, self->fh);
	if ( r != 1 ) {
		printf("fwrite failed for aiff header output.\n");
		return false;
	}
	r = fclose(self->fh);
	if ( r ) {
		printf("fclose failed for aiff output.\n");
		return false;
	}
	return true;
}

void convert_extended80(unsigned char * buffer, unsigned long value) {
	unsigned long exp;
	unsigned char i, e;
	memset(buffer, 0, 10);
	
	exp = value;
	exp >>= 1;
	for ( e = 0; e < 32; e++ ) {
		exp >>= 1;
		if (!exp) break;
	}
	
	for ( i = 32; i; i-- ) {
		if (value & 0x80000000) break;
		value <<= 1; 
	}

	*buffer = 0x40;
	*(buffer+1) = e;
	*(uint32_t *)(buffer+2) = htonl(value);
}
