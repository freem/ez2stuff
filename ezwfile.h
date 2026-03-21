#ifndef __EZWFILE_H__
#define __EZWFILE_H__
/* Extreme Zhampionship Wrestling... er, EZ2 Wave file (.ezw, .ssf) */

const char RIFF_GUY[4] = { 0x52,0x49,0x46,0x46 }; /* "RIFF" */
/* (if you have to ask why it's called "RIFF_GUY", please play more EZ2DJ) */

const char WAVE_GUY[4] = { 0x57,0x41,0x56,0x45 }; /* "WAVE" */
const char Magic_fmt[4] = { 0x66,0x6D,0x74,0x20 }; /* "fmt " */
const char Magic_data[4] = { 0x64,0x61,0x74,0x61 }; /* "data" */

/* Header structure for .ezw and .ssf files */
typedef struct{
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate; /* (sampleRate*bitsPerSample*numChannels)/8 */
	uint16_t blockAlign; /* (bitsPerSample*numChannels)/8 */
	uint16_t bitsPerSample;
	uint32_t dataSize;
} EzwHeader;

/* Header structure for Microsoft .wav files */
typedef struct{
	/*char riff[4];*/
	uint32_t filesize; /* filesize in bytes, minus 8 bytes (or in other words, how many bytes remain after this number) */
	/*char wave[4];*/
	/*char fmt[4];*/
	uint32_t fmtSize;
	uint16_t audioFormat; /* should always be 1 == PCM */
	uint16_t numChannels;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	/*char data[4];*/
	uint32_t dataSize;
} WavHeader;

#endif
