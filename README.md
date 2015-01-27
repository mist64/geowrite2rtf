# geowrite2rtf

*geowrite2rtf* is a simple tool that converts a C64/C128 GEOS GeoWrite document in [.CVT format](http://unusedino.de/ec64/technical/formats/cvt.html) into RTF format. The tool can optionally write HTML or plain-text as well, though at a loss of some or all formatting.

Use a tool like [c1541](http://vice-emu.sourceforge.net/vice_12.html) or [DirMaster](http://style64.org/dirmaster) to extract the file from a D64/D71/D81/etc. disk image into a .CVT first.

## Status

*geowrite2rtf* supports:

* font size
* styles: underline, bold, reverse, italics, outline, superscript, subscript
* alignment: left, right, center, justified

*geowrite2rtf* discards:

* font faces
* insets
* tab stops
* line spacing
* headers, footers
* colors
* page size
* graphics

## License

[BSD 2-Clause](http://opensource.org/licenses/BSD-2-Clause)

## Author

Michael Steil <mist64@mac.com>
