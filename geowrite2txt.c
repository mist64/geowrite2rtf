#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

// Enable this to convert page breaks into line breaks.
// You will want this for assembly source files, for example.
//#define FF_TO_LF

int
main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s <filename>\n", basename(argv[0]));
		printf("\nThis tool converts a C64/C128 GEOS GeoWrite .CVT file into a sequential\n");
		printf("text file. All rich text and graphics information will be discarded.\n");
		exit(1);
	}

	// read the whole file into memory
	unsigned char data[1024*1024]; // highly unlikely to be bigger than this
	FILE *f = fopen(argv[1], "r");
	size_t size = fread(data, 1, sizeof(data), f);
	fclose(f);

	// this is where the payload starts in the file
	unsigned char *payload = &data[0x2FA];

	// the record data is 127 two byte records starting here
	unsigned char *record = &data[0x1FC];
	for (int i = 0; i < 127; i++) {
		unsigned char a1 = record[i * 2];
		unsigned char a2 = record[i * 2 + 1];

		// end of file
		if (a1 == 0 && (a2 == 0 || a2 == 0xFF))
			break;

		// size = number of blocks plus extra bytes
		size_t chain_size = a1 * 254 + a2;

		for (int j = 0; j < chain_size; j++) {
			unsigned char c = payload[j];
			switch (c) {
				case 0x0:
					continue;
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
					j += 4;
					continue;
				case 0x11:
					// Ruler Escape
					// TODO: We could decode it and create an RTF
					j += 26;
					continue;
				case 0x17:
					// NewCardSet Escape (i.e. font/style change)
					// TODO: We could decode it and create an RTF
					j += 3;
					continue;
			}
			printf("%c", payload[j]);
		}
		payload += chain_size;
	}
}