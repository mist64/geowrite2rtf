// geowrite2txt by Michael Steil
//
// Based on this documentation:
// http://unusedino.de/ec64/technical/formats/cvt.html
// http://www.zimmers.net/geos/docs/writefile.txt

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>

// Enable this to convert page breaks into line breaks.
// You will want this for assembly source files, for example.
//#define FF_TO_LF

// You probably want to turn this on. GeoWrite files and in a \0 character.
#define SUPPRESS_NUL

// Turn this on to debug this tool... or broken files.
//#define DEBUG

#ifdef DEBUG
#undef SUPPRESS_NUL
#undef FF_TO_LF
#define debug_printf printf
#else
#define debug_printf(...)
#endif

int
main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename>\n", basename(argv[0]));
		fprintf(stderr, "\nThis tool converts a C64/C128 GEOS GeoWrite .CVT file into a sequential\n");
		fprintf(stderr, "text file. All rich text and graphics information will be discarded.\n");
		return 1;
	}

	// read the whole file into memory
	char data[1024*1024]; // highly unlikely to be bigger than this
	FILE *f = fopen(argv[1], "r");
	size_t size = fread(data, 1, sizeof(data), f);
	fclose(f);

	char *format = &data[30];
	char broken;
	if (!strcmp(format, "PRG formatted GEOS file")) {
		// broken CVT file created by DirMaster
		broken = 1;
	} else if (!strcmp(format, "PRG formatted GEOS file V1.0")) {
		broken = 0;
	} else {
		fprintf(stderr, "Unknown file format: %s\n", format);
		return 1;
	}
	debug_printf("<<<%s>>>", format);

	// this is where the payload starts in the file
	char *payload = &data[0x2FA];

	// the record data is 127 two byte records starting here
	unsigned char *record = (unsigned char *)&data[0x1FC];
	for (int i = 0; i < 61; i++) { // max 61 pages
		unsigned char a1 = record[i * 2];
		unsigned char a2 = record[i * 2 + 1];
		debug_printf("<<<chain 0x%02x/0x%02x>>>", a1, a2);
		// end of file
		if (a1 == 0x00 && a2 == 0x00)
			break;
		if (a1 == 0x00 && a2 == 0xFF)
			continue;

		// size = number of blocks plus extra bytes
		size_t chain_size;
		size_t gross_size;
		if (broken) {
			chain_size = a1 * 254 + a2;
			gross_size = chain_size;
		} else {
			chain_size = (a1 - 1) * 254 + a2 - 1;
			gross_size = a1 * 254;
		}

		for (int j = 0; j < chain_size; j++) {
			unsigned char c = payload[j];

			if (j == 0 && c == 0) {
				// Unknown Escape 0x00 (V1.1 only)
				debug_printf("<<<Unknown Escape 0x00>>>");
				j += 19;
				continue;
			}

			switch (c) {
#ifdef SUPPRESS_NUL
				case 0x0:
					continue;
#endif
#ifdef FF_TO_LF
				case 0x0c:
					printf("\n");
					continue;
#endif
				case 0x0d:
					printf("\n");
					continue;
				case 0x10:
					// Graphics Escape
					// TODO: We could decode it and create an RTF
					debug_printf("<<<Graphics Escape>>>");
					j += 4;
					continue;
				case 0x11:
					// Ruler Escape
					// TODO: We could decode it and create an RTF
					debug_printf("<<<Ruler Escape>>>");
					j += 26;
					continue;
				case 0x17:
					// NewCardSet Escape (i.e. font/style change)
					// TODO: We could decode it and create an RTF
					debug_printf("<<<NewCardSet Escape>>>");
					j += 3;
					continue;
				case 0x08:
				case 0x18:
					// Unknown Escape 0x08/0x18 (V1.1 only)
					debug_printf("<<<Unknown Escape 0x08/0x18>>>");
					j += 19;
					continue;
				case 0xF5:
					// Unknown Escape 0xF5 (V1.1 only)
					debug_printf("<<<Unknown Escape 0xF5>>>");
					j += 10;
					continue;
			}
			printf("%c", c);
		}
		payload += gross_size;
		debug_printf("<<<New Page>>>");
	}

	return 0;
}