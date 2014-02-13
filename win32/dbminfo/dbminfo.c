/* 
  libdigibooster3 example
  dbminfo: displays detailed DBM module information
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


const char* ErrorReasons[] = {
	"no error",
	"can't open file",
	"out of memory",
	"module corrupted",
	"unsupported format version",
	"data read error",
	"wrong chunk order in the module"
};


const char *notes[16] = { "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "H-", "??", "??", "??", "OF" };


void dump_songs(struct DB3Module *m)
{
	int i, j;
	struct DB3ModSong *ms;

	printf("SONGS\n");

	for (i = 0; i < m->NumSongs; i++)
	{
		ms = m->Songs[i];

		printf("\nSong title:      \"%s\"\n", ms->Name);
		printf("Playlist length: %d\n", ms->NumOrders);
		printf("Playlist:\n");

		for (j = 0; j < ms->NumOrders; j++)
		{
			printf("%8d\n", ms->PlayList[j]);
		}
	}

	printf("\n");
}



void dump_instruments(struct DB3Module *m)
{
	int i;
	struct DB3ModInstr *mi;

	printf("\nINSTRUMENTS\n\n");

	for (i = 0; i < m->NumInstr; i++)
	{
		mi = m->Instruments[i];

		printf("[$%02X] '%-30s', vol=%d, pan=%d, ", i + 1, mi->Name, mi->Volume, mi->Panning);

		if (mi->VolEnv == ENVELOPE_DISABLED) printf("volenv=none, ");
		else printf("volenv=%d, ", mi->VolEnv);

		if (mi->PanEnv == ENVELOPE_DISABLED) printf("panenv=none, ");
		else printf("panenv=%d, ", mi->PanEnv);

		if (mi->Type == ITYPE_SAMPLE)
		{
			uint32_t loop;

			struct DB3ModInstrS *mis = (struct DB3ModInstrS*)mi;

			printf("wavetable $%02X, C-4 note @ %d Hz, ", mis->SampleNum, mis->C3Freq);

			loop = mis->Flags & IF_LOOP_MASK;

			switch (loop)
			{
				case IF_NO_LOOP:
					printf("no loop");
				break;

				case IF_FORWARD_LOOP:
					printf("forward loop [%d-%d]", mis->LoopStart, mis->LoopStart + mis->LoopLen - 1);
				break;

				case IF_PINGPONG_LOOP:
					printf("pingpong loop [%d-%d]", mis->LoopStart, mis->LoopStart + mis->LoopLen - 1);
				break;
			}
		}

		printf("\n");
	}

	printf("\n");
}



void dump_wavetables(struct DB3Module *m)
{
	int i;
	struct DB3ModSample *ms;

	printf("WAVETABLES\n\n");

	for (i = 0; i < m->NumSamples; i++)
	{
		ms = m->Samples[i];

		printf("[$%02X] %9d frames\n", i, ms->Frames);
	}

	printf("\n");
}



void dump_envelope(struct DB3ModEnvelope *menv)
{
	if (menv->InstrNum != ENVELOPE_DISABLED)
	{
		int i;

		printf("\t%d points, assigned to instrument $%02X.\n", menv->NumSections + 1, menv->InstrNum);
		if (menv->SustainA != ENV_SUSTAIN_DISABLED) printf("\tsustain point 1 at %d.\n", menv->SustainA);
		if (menv->SustainB != ENV_SUSTAIN_DISABLED) printf("\tsustain point 2 at %d.\n", menv->SustainB);
		if (menv->LoopFirst != ENV_LOOP_DISABLED) printf("\tloop %d - %d\n", menv->LoopFirst, menv->LoopLast);
		printf("\tpoints:\n");

		for (i = 0; i <= menv->NumSections; i++)
		{
			if ((i & 7) == 0) printf("\t\t");
			printf("[%4d,%4d]", menv->Points[i].Position, menv->Points[i].Value);
			if ((i & 7) == 7) printf("\n");
			else if (i < menv->NumSections) printf(", ");
			else printf("\n");
		}
	}
	else printf("\tdisabled\n");
}



void dump_volume_envelopes(struct DB3Module *m)
{
	struct DB3ModEnvelope *menv;
	int i;

	printf("VOLUME ENVELOPES\n\n");

	for (i = 0; i < m->NumVolEnv; i++)
	{
		menv = &m->VolEnvs[i];
		printf("Volume envelope %d:\n", i);
		dump_envelope(menv);
	}

	printf("\n");
}



void dump_panning_envelopes(struct DB3Module *m)
{
	struct DB3ModEnvelope *menv;
	int i;

	printf("PANNING ENVELOPES\n\n");

	for (i = 0; i < m->NumPanEnv; i++)
	{
		menv = &m->PanEnvs[i];
		printf("Panning envelope %d:\n", i);
		dump_envelope(menv);
	}

	printf("\n");
}



void dump_dsp_effects(struct DB3Module *m)
{
	int track;
	uint32_t *maskptr = m->DspDefaults.EffectMask;

	printf("DSP EFFECTS\n\n");

	printf("Global echo delay:      %3d miliseconds.\n", m->DspDefaults.EchoDelay);
	printf("Global echo feedback:   %3d.\n", m->DspDefaults.EchoFeedback);
	printf("Global echo mix-in:     %3d.\n", m->DspDefaults.EchoMix);
	printf("Global echo cross-mix:  %3d.\n\n", m->DspDefaults.EchoCross);

	printf("Effect mask\n\tbit 0 = echo\n\nTrack               Mask\n--------------------------------------\n");

	for (track = 0; track < m->NumTracks; track++)
	{
		uint32_t mask, z;
		int bit;
		char a;

		mask = *maskptr++;
		z = 0x80000000;
		printf("%5d ", track);

		for (bit = 0; bit < 32; bit++)
		{
			a = '0';
			if (mask & z) a = '1';
			printf("%c", a);
			z >>= 1;
		}

		printf("\n");
	}

	printf("\n");
}


int nibble2hex(int nibble)
{
	if (nibble < 10) return nibble + 48;
	else return nibble + 55;
}


void effect2text(int comm, int param, char *text)
{
	text[0] = nibble2hex(comm);
	text[1] = nibble2hex(param >> 4);
	text[2] = nibble2hex(param & 0xF);
}


void instr2text(int instr, char *text)
{
	text[0] = nibble2hex(instr >> 4);
	text[1] = nibble2hex(instr & 0xF);
}


void print_entry(struct DB3ModEntry *me)
{
	char entry[16];

	strcpy(entry, "--- -- --- ---");

	if (me->Octave)
	{
		memcpy(entry, notes[me->Note], 2);
		if (me->Note < 12) entry[2] = me->Octave + '0';
		else if (me->Note == 15) entry[2] = 'F';
		else entry[2] = '?';
	}

	instr2text(me->Instr, &entry[4]);
	effect2text(me->Cmd1, me->Param1, &entry[7]);
	effect2text(me->Cmd2, me->Param2, &entry[11]);

	printf("%s ", entry);
}


void dump_patterns(struct DB3Module *m)
{
	int pn, rn, tn;
	struct DB3ModPatt *patt;
	struct DB3ModEntry *me;

	printf("PATTERN DUMP\n\n");

	for (pn = 0; pn < m->NumPatterns; pn++)
	{
		patt = m->Patterns[pn];
		me = patt->Pattern;
		printf("Pattern %d, %d rows:\n\n", pn, patt->NumRows);

		for (rn = 0; rn < patt->NumRows; rn++)
		{
			printf("%03d ", rn);
			for (tn = 0; tn < m->NumTracks; tn++) print_entry(me++);
			printf("\n");
		}

		printf("\n\n");
	}
}


void print_info(struct DB3Module *m)
{
	char *creator = "DigiBooster 3";

	if (m->CreatorVer == CREATOR_DIGIBOOSTER_2) creator = "DigiBooster Pro 2";

	printf("\nGENERAL INFO\n\n");
	printf("Module name:     \"%s\"\n", m->Name);
	printf("Created with:    %s.%d\n", creator, m->CreatorRev);
	printf("Songs:           %d\n", m->NumSongs);
	printf("Tracks:          %d\n", m->NumTracks);
	printf("Instruments:     %d\n", m->NumInstr);
	printf("Samples:         %d\n", m->NumSamples);
	printf("Patterns:        %d\n", m->NumPatterns);
	dump_instruments(m);
	dump_wavetables(m);
	dump_songs(m);
	dump_volume_envelopes(m);
	dump_panning_envelopes(m);
	dump_dsp_effects(m);
	dump_patterns(m);
}


int main(int argc, char *argv[])
{
	if (argc == 2)
	{
		struct DB3Module *m;
		int error;

		if (m = DB3_Load(argv[1], &error))
		{
			print_info(m);
			DB3_Unload(m);
		}
		else printf("dbminfo: Loading \"%s\" failed: %s.\n", argv[1], ErrorReasons[error]);
	}
	else printf("dbminfo: Usage: dbminfo <file>\n");

	return 0;
}