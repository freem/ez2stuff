/* ezwssf - Tool for dealing with .ezw/.ssf and .wav files */
/*============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ezwfile.h"

const char OptionFlag_Info[6]  = { "--info" };
const char OptionFlag_ToWav[7] = { "--towav" };
const char OptionFlag_ToEzw[7] = { "--toezw" };
const char OptionFlag_Help[6]  = { "--help" };

static void Usage(char* execName){
	printf("%s - Tool for dealing with .ezw/.ssf and .wav files\n", execName);
	printf("Usage: %s (option) (params)\n", execName);
	printf("\n");
	printf("(option) is one of:\n");
	printf("\t--help\tDisplays program usage.\n");
	printf("\t--info\tDisplay information about .ezw/.ssf file.\n");
	printf("\t--towav\tConvert .ezw/.ssf to .wav\n");
	printf("\t--toezw\tConvert .wav to .ezw/.ssf\n");
	printf("\n");
	printf("Example usage:\n");
	printf("%s --info (filename.ezw)\n",execName);
	printf("%s --towav (filename.ezw) [outfile.wav]\n",execName);
	printf("%s --toezw (filename.wav) [outfile.ezw]\n",execName);
}

int main(int argc, char** argv){
	FILE *inFile;
	FILE *outFile;

	if(argc < 2){
		Usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if(memcmp(argv[1],OptionFlag_Info,6) == 0){
		if(argc < 3){
			printf("%s --info needs a filename.\n",argv[0]);
			exit(EXIT_FAILURE);
		}

		inFile = fopen(argv[2],"rb");
		if(inFile == NULL){
			perror("Error opening input file");
			exit(EXIT_FAILURE);
		}

		EzwHeader header;
		fread(&header.numChannels,sizeof(uint16_t),1,inFile);
		fread(&header.sampleRate,sizeof(uint32_t),1,inFile);
		fread(&header.byteRate,sizeof(uint32_t),1,inFile);
		fread(&header.blockAlign,sizeof(uint16_t),1,inFile);
		fread(&header.bitsPerSample,sizeof(uint16_t),1,inFile);
		fread(&header.dataSize,sizeof(uint32_t),1,inFile);
		printf("%s info for %s:\n",argv[0],argv[2]);
		printf("Channels: %d\n",header.numChannels);
		printf("Sample Rate: %d\n",header.sampleRate);
		printf("Byte Rate: %d\n",header.byteRate);
		printf("Block Align: %d\n",header.blockAlign);
		printf("Bits per Sample: %d\n",header.bitsPerSample);
		printf("Data Size: %d bytes\n",header.dataSize);

		fclose(inFile);
	}
	else if(memcmp(argv[1],OptionFlag_ToWav,7) == 0){
		if(argc < 3){
			printf("%s --towav needs an input filename.\n",argv[0]);
			exit(EXIT_FAILURE);
		}

		inFile = fopen(argv[2],"rb");
		if(inFile == NULL){
			perror("Error opening input file");
			exit(EXIT_FAILURE);
		}

		EzwHeader ezwHeader;
		fread(&ezwHeader.numChannels,sizeof(uint16_t),1,inFile);
		fread(&ezwHeader.sampleRate,sizeof(uint32_t),1,inFile);
		fread(&ezwHeader.byteRate,sizeof(uint32_t),1,inFile);
		fread(&ezwHeader.blockAlign,sizeof(uint16_t),1,inFile);
		fread(&ezwHeader.bitsPerSample,sizeof(uint16_t),1,inFile);
		fread(&ezwHeader.dataSize,sizeof(uint32_t),1,inFile);

		/* determine output filename */
		char *outFilename = (char*)calloc(256,sizeof(char));
		if(argv[3] != NULL){
			strcpy(outFilename,argv[3]);
		}
		else{
			char *inFileExt = strrchr(argv[2],'.');
			if(inFileExt != NULL){
				strncpy(outFilename,argv[2],inFileExt-argv[2]);
				strcat(outFilename,".wav");
			}
			else{
				strcpy(outFilename,argv[2]);
				strcat(outFilename,".wav");
			}
		}

		outFile = fopen(outFilename,"wb");
		if(outFile == NULL){
			perror("Error attempting to open output file");
			exit(EXIT_FAILURE);				
		}

		fputc(RIFF_GUY[0],outFile);
		fputc(RIFF_GUY[1],outFile);
		fputc(RIFF_GUY[2],outFile);
		fputc(RIFF_GUY[3],outFile);

		/* real file size is calculated later; just write 0 here for now */
		uint32_t fakeFilesize = 0;
		fwrite(&fakeFilesize,sizeof(uint32_t),1,outFile);

		fputc(WAVE_GUY[0],outFile);
		fputc(WAVE_GUY[1],outFile);
		fputc(WAVE_GUY[2],outFile);
		fputc(WAVE_GUY[3],outFile);

		fputc(Magic_fmt[0],outFile);
		fputc(Magic_fmt[1],outFile);
		fputc(Magic_fmt[2],outFile);
		fputc(Magic_fmt[3],outFile);

		/* fmtSize is 16 usually */
		uint32_t fmtSize = 16;
		fwrite(&fmtSize,sizeof(uint32_t),1,outFile);

		/* audioFormat 1 == PCM */
		uint16_t audioFormat = 1;
		fwrite(&audioFormat,sizeof(uint16_t),1,outFile);

		fwrite(&ezwHeader.numChannels,sizeof(uint16_t),1,outFile);
		fwrite(&ezwHeader.sampleRate,sizeof(uint32_t),1,outFile);
		fwrite(&ezwHeader.byteRate,sizeof(uint32_t),1,outFile);
		fwrite(&ezwHeader.blockAlign,sizeof(uint16_t),1,outFile);
		fwrite(&ezwHeader.bitsPerSample,sizeof(uint16_t),1,outFile);

		fputc(Magic_data[0],outFile);
		fputc(Magic_data[1],outFile);
		fputc(Magic_data[2],outFile);
		fputc(Magic_data[3],outFile);

		fwrite(&ezwHeader.dataSize,sizeof(uint32_t),1,outFile);

		uint8_t *sound = calloc(ezwHeader.dataSize,sizeof(uint8_t));
		if(sound == NULL){
			printf("%s --towav: Error attempting to allocate space for sound data.\n",argv[0]);
			exit(EXIT_FAILURE);
		}
		fread(sound,ezwHeader.dataSize,sizeof(uint8_t),inFile);
		fwrite(sound,ezwHeader.dataSize,sizeof(uint8_t),outFile);
		free(sound);

		uint32_t realFilesize = ezwHeader.dataSize+36;
		fseek(outFile,4,SEEK_SET);
		fwrite(&realFilesize,sizeof(uint32_t),1,outFile);		

		fclose(outFile);
		fclose(inFile);
		printf("%s --towav: wrote output to %s\n",argv[0],outFilename);
		free(outFilename);
	}
	else if(memcmp(argv[1],OptionFlag_ToEzw,7) == 0){
		if(argc < 3){
			printf("%s --toezw needs an input filename.\n",argv[0]);
			exit(EXIT_FAILURE);
		}

		inFile = fopen(argv[2],"rb");
		/* never feel this .wav */
		if(inFile == NULL){
			perror("Error opening input file");
			exit(EXIT_FAILURE);
		}

		WavHeader wavHeader;

		/* check for "RIFF" string */
		char fileMagic[4];
		fread(fileMagic,sizeof(char),4,inFile);
		if(memcmp(fileMagic,RIFF_GUY,4) != 0){
			printf("Input file does not appear to be a Microsoft .wav (no RIFF).\n");
			exit(EXIT_FAILURE);
		}

		fread(&wavHeader.filesize,sizeof(uint32_t),1,inFile);

		/* check for "WAVE" and "fmt " strings */
		fread(fileMagic,sizeof(char),4,inFile);
		if(memcmp(fileMagic,WAVE_GUY,4) != 0){
			printf("Input file does not appear to be a Microsoft .wav (no WAVE).\n");
			exit(EXIT_FAILURE);
		}

		fread(fileMagic,sizeof(char),4,inFile);
		if(memcmp(fileMagic,Magic_fmt,4) != 0){
			printf("Input file does not appear to be a Microsoft .wav (no fmt ).\n");
			exit(EXIT_FAILURE);
		}

		fread(&wavHeader.fmtSize,sizeof(uint32_t),1,inFile);

		fread(&wavHeader.audioFormat,sizeof(uint16_t),1,inFile);
		/* ensure audioFormat == 1 */
		if(wavHeader.audioFormat != 1){
			printf("ezw/ssf conversion only works with PCM format (expected 1, found %d).\n",wavHeader.audioFormat);
			exit(EXIT_FAILURE);
		}

		fread(&wavHeader.numChannels,sizeof(uint16_t),1,inFile);
		fread(&wavHeader.sampleRate,sizeof(uint32_t),1,inFile);
		fread(&wavHeader.byteRate,sizeof(uint32_t),1,inFile);
		fread(&wavHeader.blockAlign,sizeof(uint16_t),1,inFile);
		fread(&wavHeader.bitsPerSample,sizeof(uint16_t),1,inFile);

		/* check for "data" string */
		fread(fileMagic,sizeof(char),4,inFile);
		if(memcmp(fileMagic,Magic_data,4) != 0){
			printf("Input file does not appear to be a Microsoft .wav (no data).\n");
			exit(EXIT_FAILURE);
		}

		fread(&wavHeader.dataSize,sizeof(uint32_t),1,inFile);

		/* determine output filename */
		char *outFilename = (char*)calloc(256,sizeof(char));
		if(argv[3] != NULL){
			strcpy(outFilename,argv[3]);
		}
		else{
			char *inFileExt = strrchr(argv[2],'.');
			if(inFileExt != NULL){
				strncpy(outFilename,argv[2],inFileExt-argv[2]);
				strcat(outFilename,".ssf");
			}
			else{
				strcpy(outFilename,argv[2]);
				strcat(outFilename,".ssf");
			}
		}

		outFile = fopen(outFilename,"wb");
		if(outFile == NULL){
			perror("Error attempting to open output file");
			exit(EXIT_FAILURE);				
		}

		fwrite(&wavHeader.numChannels,sizeof(uint16_t),1,outFile);
		fwrite(&wavHeader.sampleRate,sizeof(uint32_t),1,outFile);
		fwrite(&wavHeader.byteRate,sizeof(uint32_t),1,outFile);
		fwrite(&wavHeader.blockAlign,sizeof(uint16_t),1,outFile);
		fwrite(&wavHeader.bitsPerSample,sizeof(uint16_t),1,outFile);
		fwrite(&wavHeader.dataSize,sizeof(uint32_t),1,outFile);

		uint8_t *sound = calloc(wavHeader.dataSize,sizeof(uint8_t));
		if(sound == NULL){
			printf("%s --toezw: Error attempting to allocate space for sound data.\n",argv[0]);
			exit(EXIT_FAILURE);
		}
		fread(sound,wavHeader.dataSize,sizeof(uint8_t),inFile);
		fwrite(sound,wavHeader.dataSize,sizeof(uint8_t),outFile);
		free(sound);

		fclose(outFile);
		fclose(inFile);
		printf("%s --toezw: wrote output to %s\n",argv[0],outFilename);
		free(outFilename);
	}
	else if(memcmp(argv[1],OptionFlag_Help,6) == 0){
		Usage(argv[0]);
		exit(EXIT_SUCCESS);
	}
	else{
		printf("%s error: Unrecognized option '%s'\n",argv[0],argv[1]);
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
