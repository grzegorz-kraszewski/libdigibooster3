/* 
  libdigibooster3 example
  dbm2wav: renders DigiBooster 3 module to WAVE file
*/


/*
  Copyright (c) 2014, Grzegorz Kraszewski
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met: 

  1. Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer. 
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution. 

  This software is provided by the copyright holders and contributors "as is" and
  any express or implied warranties, including, but not limited to the implied
  warranties of merchantability and fitness for a particular purpose are
  disclaimed. In no event shall the copyright owner or contributors be liable for
  any direct, indirect, incidental, special, exemplary, or consequential damages
  (including, but not limited to, procurement of substitute goods or services;
  loss of use, data, or profits; or business interruption) however caused and
  on any theory of liability, whether in contract  strict liability or tort
  (including negligence or otherwise) arising in any way out of the use of this
  software, even if advised of the possibility of such damage.
*/


#ifndef TARGET_WIN32
#include "libdigibooster3.h"
#else
#include "../libdigibooster3/libdigibooster3.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define RENDER_BUFFER_FRAMES    100000


const char* ErrorReasons[] = {
	"no error",
	"can't open file",
	"out of memory",
	"module corrupted",
	"unsupported format version",
	"data read error",
	"wrong chunk order in the module"
};


int SongEnd = 0;
int TrailingFrames = 0;



static int little_endian_host(void)
{
	uint32_t x = 36;
	uint8_t y;

	y = *((uint8_t*)&x);
	if (y == 36) return 1;
	else return 0;
}



void fill_wave_header(uint8_t *wh, uint32_t frames, uint32_t mixfreq)
{
	uint32_t audio_len;
	uint32_t d;

	audio_len = frames << 2;                                   // stereo, 16-bit
	memcpy(&wh[0], "RIFF", 4);                                 // [00] 'RIFF' identifier
	d = audio_len + 36;
	wh[4] = d & 0xFF;                                          // [04] total 'RIFF' length
	wh[5] = (d >> 8) & 0xFF;
	wh[6] = (d >> 16) & 0xFF;
	wh[7] = (d >> 24) & 0xFF;
	memcpy(&wh[8], "WAVE", 4);                                 // [08] 'WAVE' identifier
	memcpy(&wh[12], "fmt ", 4);                                // [12] 'fmt ' chunk
	wh[16] = 16;                                               // [16] length of 'fmt ', fixed at 16 bytes
	wh[17] = 0;
	wh[18] = 0;
	wh[19] = 0;
	wh[20] = 1;                                                // [20] format tag, 0x0001 is integer PCM
	wh[21] = 0;
	wh[22] = 2;                                                // [22] number of channels: 2
	wh[23] = 0;
	wh[24] = mixfreq & 0xFF;                                   // [24] sampling rate in Hz
	wh[25] = (mixfreq >> 8) & 0xFF;
	wh[26] = (mixfreq >> 16) & 0xFF;
	wh[27] = (mixfreq >> 24) & 0xFF;
	d = mixfreq << 5;
	wh[28] = d & 0xFF;                                         // [28] average bitrate (frequency * bits per frame)
	wh[29] = (d >> 8) & 0xFF;
	wh[30] = (d >> 16) & 0xFF;
	wh[31] = (d >> 24) & 0xFF;
	wh[32] = 4;                                                // [32] audio frame align (bytes per frame)
	wh[33] = 0;
	wh[34] = 16;                                               // [34] bits per sample
	wh[35] = 0;
	memcpy(&wh[36], "data", 4);                                // [36] 'data' chunk
	wh[40] = audio_len & 0xFF;                                 // [40] length of 'data' chunk
	wh[41] = (audio_len >> 8) & 0xFF;
	wh[42] = (audio_len >> 16) & 0xFF;
	wh[43] = (audio_len >> 24) & 0xFF;
}



void buffer_endian_swap(int16_t *buffer, int samples)
{
	uint16_t x, *s = (uint16_t*)buffer;

	while (samples--)
	{
		x = *s;
		*s++ = ((x >> 8) & 0xFF) | (x << 8);
	}
}



int main(int argc, char *argv[])
{
	if (argc == 3)
	{
		struct DB3Module *m;
		void *engine;
		int error;

		if (m = DB3_Load(argv[1], &error))
		{
			if (engine = DB3_NewEngine(m, 44100, RENDER_BUFFER_FRAMES))
			{
				FILE *wav;
				int16_t *rendbuf;

				//DB3_SetCallback(engine, player_callback, NULL);

				if (rendbuf = malloc(RENDER_BUFFER_FRAMES << 2))
				{
					if (wav = fopen(argv[2], "wb"))
					{
						uint8_t wh[44];                  // wave header
						uint32_t frames, total = 0;

						fill_wave_header(wh, 0, 44100);
						fwrite(wh, 44, 1, wav);

						for (;;)
						{
							frames = DB3_Mix(engine, RENDER_BUFFER_FRAMES, rendbuf);

							if (frames > 0)
							{
								// NOTE: Most of compilers determine result of little_endian_host() at compile time
								// and optimize out the following line on little endian systems.

								if (!little_endian_host()) buffer_endian_swap(rendbuf, frames << 1);
								fwrite(rendbuf, 4, frames, wav);
								total += frames;
								printf("%d audio frames written...\r", total);
								fflush(stdout);
							}

							if (frames < RENDER_BUFFER_FRAMES) break;
						}

						fseek(wav, 0, 0);
						fill_wave_header(wh, total, 44100);
						fwrite(wh, 44, 1, wav);
						fclose(wav);
						printf("%d audio frames written.   \n", total);
					}

					free(rendbuf);
				}

				DB3_DisposeEngine(engine);
			}

			DB3_Unload(m);
		}
		else printf("dbm2wav: Loading \"%s\" failed: %s.\n", argv[1], ErrorReasons[error]);
	}
	else printf("dbm2wav: Usage: dbm2wav <module> <wavefile>\n");

	return 0;
}