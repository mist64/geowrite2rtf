all: geowrite2rtf

clean:
	rm -f geowrite2rtf geowrite2rtf.o

geowrite2rtf: geowrite2rtf.c
	cc -o $@ $<
