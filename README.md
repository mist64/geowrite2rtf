# geowrite2txt

*geowrite2txt* is a simple tool that converts a C64/C128 GEOS GeoWrite document in [.CVT format](http://unusedino.de/ec64/technical/formats/cvt.html) into a sequential text file.

Use a tool like [c1541](http://vice-emu.sourceforge.net/vice_12.html) or [DirMaster](http://style64.org/dirmaster) to extract the file from a D64/D71/D81/etc. disk image into a .CVT first.

## Bugs

*geowrite2txt* discards all rich text and graphics information. It could be extended to support lossless conversion to RTF files.

## Author

Michael Steil <mist64@mac.com>
