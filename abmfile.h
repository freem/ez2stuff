#ifndef __ABMFILE_H__
#define __ABMFILE_H__

/* .abm - AmuseWorld Bitmap format */
const char ABM_HEADER[2] = { 0x41,0x57 }; /* "AW" */

/* (for internal tool use; not used by any files) */
enum AbmFormatVersion{
	AbmFormatVersion_Old  = 0,
	AbmFormatVersion_EV   = 1,
	AbmFormatVersion_NT   = 2,
	AbmFormatVersion_TT   = 3,
	AbmFormatVersion_FN   = 4,
	AbmFormatVersion_FNEX = 5,
	AbmFormatVersion_Unknown = 0xFF
};

/* old XOR constants (2nd Trax through Endless Circulation) */
/*
 * "freem, these are clearly shorts, why are you defining them as ints?"
 * The game appears to not bother with the upper two bytes of each value,
 * all of which are stored as 4 byte ints.
 *
 * Order is: data start address, width, height, bpp
 */
const int XorConst_Old[4] = {
	0x56FE, /* file offsets 0x0A-0x0D */
	0x0831, /* file offsets 0x12-0x15 */
	0x1019, /* file offsets 0x16-0x19 */
	0x1120  /* file offsets 0x1C-0x1F */
};

/* Evolve XOR constants */
const int XorConst_EV[4] = {
	0x45AE, /* file offsets 0x0A-0x0D */
	0x9AF1, /* file offsets 0x12-0x15 */
	0x1D1B, /* file offsets 0x16-0x19 */
	0x678E  /* file offsets 0x1C-0x1F */
};

/* Night Traveler XOR constants */
const int XorConst_NT[4] = {
	0x85BE, /* file offsets 0x0A-0x0D */
	0x96EC, /* file offsets 0x12-0x15 */
	0xFDEB, /* file offsets 0x16-0x19 */
	0x679E  /* file offsets 0x1C-0x1F */
};

/* Time Traveler XOR constants */
const int XorConst_TT[4] = {
	0x95AB, /* file offsets 0x0A-0x0D */
	0x45BB, /* file offsets 0x12-0x15 */
	0xAE12, /* file offsets 0x16-0x19 */
	0x78EF  /* file offsets 0x1C-0x1F */
};

/* Final XOR constants */
const int XorConst_FN[4] = {
	0x23FF, /* file offsets 0x0A-0x0D */
	0xBDC9, /* file offsets 0x12-0x15 */
	0x1F01, /* file offsets 0x16-0x19 */
	0xA97F  /* file offsets 0x1C-0x1F */
};

/* Final EX XOR constants */
const int XorConst_FNEX[4] = {
	0x109A, /* file offsets 0x0A-0x0D */
	0xCFA1, /* file offsets 0x12-0x15 */
	0x51AE, /* file offsets 0x16-0x19 */
	0xB18F  /* file offsets 0x1C-0x1F */
};

typedef struct{
	/* in most versions: bits per pixel (copied from .bmp offsets 0x1C-0x1F inclusive)
	 * Evolve, NT: total file size in bytes
	 */
	uint32_t bpp;

	uint16_t width;
	uint16_t height;
	uint32_t dataStartAddr; /* XOR'ed by a constant */
	uint32_t infoHeaderSize;
	uint32_t width2; /* XOR'ed by a constant */
	uint32_t height2; /* XOR'ed by a constant */
	uint16_t numPlanes; /* must be 1 */
	uint32_t bpp2; /* XOR'ed by a constant */
	uint16_t offset_20;
	uint32_t bitmapDataSize;
	uint32_t horizRes;
	uint32_t vertRes;
	uint32_t numPalColors;
	uint32_t numImportantColors;
} AbmHeader;

/* .bmp - Windows Device Independent Bitmap */
const char DIB_HEADER[2] = { 0x42,0x4D }; /* "BM" */

typedef struct{
	uint32_t filesize;
	uint16_t offset_06; /* "actual value depends on the application that creates the image" */
	uint16_t offset_08; /* (see offset06 comment) */
	uint32_t dataStartAddr;
	uint32_t infoHeaderSize;
	uint32_t width;
	uint32_t height;
	uint16_t numPlanes; /* must be 1 */
	uint32_t bpp;
	uint16_t compression;
	uint32_t bitmapDataSize;
	uint32_t horizRes;
	uint32_t vertRes;
	uint32_t numPalColors;
	uint32_t numImportantColors;
} DibHeader;

#endif
