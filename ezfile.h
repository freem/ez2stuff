#ifndef __EZFILE_H__
#define __EZFILE_H__

const char Magic_EZFF[4] = { 0x45, 0x5A, 0x46, 0x46 }; /* "EZFF" */
const char Magic_EZTR[4] = { 0x45, 0x5A, 0x54, 0x52 }; /* "EZTR" */

/* EZ2DJ 7th Trax v1.5 uses a reliably-detectable encoding method */
const char Magic_EZFF_7th_1dot5[4] = { 0x4A, 0x5B, 0x48, 0x49 }; /* "J[HI" */
/* everything after that, not so much. */

const float OldBpmFactor = 0.9972299168975069f;

typedef union {
	float f;
	uint32_t u;
} EzBpm;

/*----------------------------------------------------------------------------*/
/* .ez header data */
typedef struct{
	char magic[4];   /* "EZFF" magic */
	/* 0x00 padding and/or terminator byte not included here */

	uint8_t version; /* file format version number */

	char name[0x40]; /* chart name (0x40 bytes with terminating null) */
	/* another 0x40 bytes skipped; perhaps this was used at one time, but not now */

	uint16_t ticksPerMeasure;
	EzBpm initialBPM;
	uint16_t numTracks;
	uint32_t totalTicks; /* determined by the largest track numTicks value */
	EzBpm secondBPM;
} EzHeader;

/*----------------------------------------------------------------------------*/
enum NoteType {
	NoteType_Unknown = 0,
	NoteType_Note = 1,
	NoteType_Volume = 2,
	NoteType_BPM = 3,
	NoteType_BeatsPerMeasure = 4
};

/*
command parameters

0x01 - note
example data [v4] {40 02 00 00} {01} {07 7F 40 06 00}
example data [v6] {40 02 00 00} {01} {07 7F 40 00 06 00}
example data [v7] {40 02 00 00} {01} {07 00 7F 40 00 06 00 00}

argument count differs by version.
v4:
- (uint8_t) note number
- (uint8_t) velocity
- (uint8_t) pan
- (uint16_t) length

v5 and v6:
- (uint8_t) note number
- (uint8_t) velocity
- (uint8_t) pan
- (uint8_t) unknown/padding?
- (uint16_t) length

v7 and later:
- (uint16_t) note number
- (uint8_t) velocity
- (uint8_t) pan
- (uint8_t) unknown/padding?
- (uint16_t) length
- (uint8_t) padding/alignment byte?

--------------------------------------------------------------------------------
0x02 - volume
example data [v4] {00 00 00 00} {02} {7B 00 00 00 00}
example data [v6] {00 00 00 00} {02} {7F 00 00 00 00 00}
example data [v7] {00 00 00 00} {02} {7F 00 00 00 00 00 00 00}
1 argument, 1 byte (uint8_t): new volume
remaining bytes unused

--------------------------------------------------------------------------------
0x03 - BPM/tempo change
example data [v4] {00 00 00 00} {03} {F6 FF ED 42 00}
example data [v6] {00 00 00 00} {03} {F6 FF ED 42 00 00}
example data [v7] {00 00 00 00} {03} {F3 FF EB 42 00 00 00 00}
1 argument, 4 bytes (float): new BPM
remaining bytes unused

--------------------------------------------------------------------------------
0x04 - set Beats per Measure
example data [v4] {00 00 00 00} {04} {04 00 00 00 00}
example data [v6] {00 00 00 00} {04} {04 00 00 00 00 00}
example data [v7] {00 00 00 00} {04} {04 00 00 00 00 00 00 00}
1 argument, 1 byte (uint8_t): new beats per measure count (valid range: 0x01-0x80)
remaining bytes unused
*/

/* total number of bytes per note, including position and type */
#define BYTES_PER_NOTE_V4 10
#define BYTES_PER_NOTE_V5 11
#define BYTES_PER_NOTE_V7 13

/* note parameter length */
#define NOTE_DATA_LENGTH_V4 5
#define NOTE_DATA_LENGTH_V5 6
#define NOTE_DATA_LENGTH_V7 8

typedef struct{
	uint32_t position;
	uint8_t type;
	/* unholy nightmare union just to support different format versions */
	union{
		uint8_t v4[NOTE_DATA_LENGTH_V4];
		uint8_t v5[NOTE_DATA_LENGTH_V5];
		uint8_t v7[NOTE_DATA_LENGTH_V7];
	} data;
} Note;

/*----------------------------------------------------------------------------*/
typedef struct{
	char magic[4]; /* "EZTR" magic */
	uint16_t pad; /* 2 bytes of unknown purpose */
	char name[0x40]; /* track name (0x40 bytes with terminating null) */
	uint32_t numTicks;
	uint32_t dataSize;
	Note *notes;
} Track;

#endif
