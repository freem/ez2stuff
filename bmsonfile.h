#ifndef __BMSONFILE_H__
#define __BMSONFILE_H__

const unsigned long BMSON_DEFAULT_RESOLUTION = 240;

typedef struct {
	unsigned long y; /* pulse num */
	float bpm; /* bpm */
} Bmson_BpmEvent;

typedef struct {
	unsigned long y; /* pulse num */
	unsigned long duration; /* stop duration (in pulses) */
} Bmson_StopEvent;

#endif
