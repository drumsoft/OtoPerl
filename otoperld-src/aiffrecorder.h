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

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

typedef struct {
	FILE *fh;
	uint32_t frames;
	int framesize;
	int channels;
	unsigned char *headers;
} AiffRecorder;

AiffRecorder *AiffRecorder_create(int channels, int bits, int sampleRate);
void AiffRecorder_destroy(AiffRecorder *self);
bool AiffRecorder_open(AiffRecorder *self, const char *path);
bool AiffRecorder_write32bit(AiffRecorder *self, const uint32_t *data, int frames);
bool AiffRecorder_close(AiffRecorder *self);

