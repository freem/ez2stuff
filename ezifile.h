#ifndef __EZIFILE_H__
#define __EZIFILE_H__
#include <math.h>

/*
 * .ezi (EZ Instrument) file
 * There are two versions of .ezi files, which I will call "old" and "new".
 */

/* The "old" version assigns notes based on key and octave. */
const char* OldNoteNames[] = {
	"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};
#define OLDNOTE_NOTE_VALUES 12

/* highest note in old format is D#21 (= keysound #255; note 3 octave 21) */

/* these two define the ranges for the OldNote struct */
#define OLDNOTE_MAX_NOTE   11
#define OLDNOTE_MAX_OCTAVE 21

typedef struct{
	uint8_t note; /* 0-11 */
	uint8_t oct;  /* 0-21 */
} OldNote;

typedef struct{
	OldNote note;
	int type;
	char *filename;
} OldInstrument;

typedef struct{
	uint16_t index;
	int type;
	char *filename;
} NewInstrument;

/* Converts a keysound number (0-255) to an index to use with OldNoteNames. */
static uint8_t KeysoundIndexToNote(int index){
	if(index > 255){
		return -1;
	}
	return (index % OLDNOTE_NOTE_VALUES);
}

/* Converts a keysound number (0-255) to an octave value. */
static uint8_t KeysoundIndexToOctave(int index){
	if(index > 255){
		return -1;
	}
	return floor(index / OLDNOTE_NOTE_VALUES);
}

#endif
