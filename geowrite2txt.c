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

// You probably want to turn this on. GeoWrite files end in a \0 character.
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
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <infile> <outfile>\n", basename(argv[0]));
		fprintf(stderr, "\nThis tool converts a C64/C128 GEOS GeoWrite .CVT file into an HTML or\n");
		fprintf(stderr, "plain-text file, depending on whether <outfile> ends in '.html' or not.\n");
		fprintf(stderr, "All graphics information will be discarded.\n");
		return 1;
	}

	char *infile = argv[1];
	char *outfile = argv[2];

	// find out whether we're supposed to output HTML
	int print_html = 0;
	size_t outfile_len = strlen(outfile);
	const char *suffix = ".html";
	size_t suffix_len = strlen(suffix);
	if (outfile_len >= suffix_len && !strcmp(outfile + outfile_len - suffix_len, suffix) ){
		print_html = 1;
	}

	// read the whole file into memory
	char data[1024*1024]; // highly unlikely to be bigger than this
	FILE *f = fopen(infile, "r");
	size_t size = fread(data, 1, sizeof(data), f);
	fclose(f);
	f = fopen(outfile, "w");

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

		char style = 0;
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
				case 0x0c:
#ifdef FF_TO_LF
					fprintf(f, "\n");
					continue;
#else
					if (print_html) {
						fprintf(f, "<hr/>");
						continue;
					} else {
						break;
					}
#endif
				case 0x0d:
					if (print_html) {
						fprintf(f, "<br/>");
					} else {
						fprintf(f, "\n");
					}
					continue;
				case 0x10:
					// Graphics Escape
					// TODO: We should decode it
					debug_printf("<<<Graphics Escape>>>");
					j += 4;
					continue;
				case 0x11: {
					// Ruler Escape
					// TODO: We should decode more
					debug_printf("<<<Ruler Escape>>>");
					if (print_html) {
						unsigned char *escape = (unsigned char *)&payload[j + 1];
						unsigned char alignment = escape[22] & 3;
						char *s;
						switch (alignment) {
							case 0:
								s = "left";
								break;
							case 1:
								s = "center";
								break;
							case 2:
								s = "right";
								break;
							case 3:
								s = "justify";
								break;
						}
						fprintf(f, "<span align=\"%s\">", s);
					}
					j += 26;
					continue;
				}
				case 0x17: {
					// NewCardSet Escape (i.e. font/style change)
					unsigned char *escape = (unsigned char *)&payload[j + 1];
					debug_printf("<<<NewCardSet Escape %02x/%02x/%02x>>>", escape[0], escape[1], escape[2]);

					if (print_html) {
						int new_font = escape[0] | escape[1] << 8;
						int new_font_id = new_font >> 5;
						int new_font_size = new_font & 0x1F;
						debug_printf("<<<Font Id=%d Size=%d>>>", new_font_id, new_font_size);
						fprintf(f, "<span style=\"font-size: %dpt\">", new_font_size);

						char new_style = escape[2];
						for (int on = 0; on <= 1; on++) {
							for (int b = 1; b < 8; b++) {
								char bit = (style >> b) & 1;
								char new_bit = (new_style >> b) & 1;
								if (bit != new_bit && new_bit == on) {
									if (new_bit) {
										fprintf(f, "<");
									} else {
										fprintf(f, "</");
									}
									switch (b) {
										case 7:
											fprintf(f, "u");
											break;
										case 6:
											fprintf(f, "b");
											break;
										case 5:
											fprintf(f, "reverse"); // XXX not an actual HTML tag
											break;
										case 4:
											fprintf(f, "i");
											break;
										case 3:
											fprintf(f, "outline"); // XXX not an actual HTML tag
											break;
										case 2:
											fprintf(f, "sup");
											break;
										case 1:
											fprintf(f, "sub");
											break;
									}
									fprintf(f, ">");
								}
							}
						}
						style = new_style;
					}
					j += 3;
					continue;
				}
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
			fprintf(f, "%c", c);
		}
		payload += gross_size;
		debug_printf("<<<New Page>>>");
	}

	fclose(f);
	return 0;
}