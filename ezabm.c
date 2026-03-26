/* ezabm - Tool for dealing with .abm and .bmp files */
/*============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "abmfile.h"

/*
 * internal notepad:
 * this program needs to handle multiple functions:
 * + display abm info   | --info (infile.abm)
 * - convert abm to bmp | --tobmp (ver) (infile.abm) [outfile.bmp]
 * ~ convert bmp to abm | --toabm (ver) (infile.bmp) [outfile.abm]
 * if that looks simple, consider that you need to do it over 6 different formats.
 */

const char OptionFlag_Info[6]  = { "--info" };
const char OptionFlag_ToBmp[7] = { "--tobmp" };
const char OptionFlag_ToAbm[7] = { "--toabm" };
const char OptionFlag_Help[6]  = { "--help" };

const char FormatName_Old[3]  = { "old" };
const char FormatName_EV[2]   = { "ev" };
const char FormatName_NT[2]   = { "nt" };
const char FormatName_TT[2]   = { "tt" };
const char FormatName_FN[2]   = { "fn" };
const char FormatName_FNEX[4] = { "fnex" };
const char FormatName_Auto[4] = { "auto" };

const char* FormatNames[6] = {
	"old",
	"Evolve",
	"Night Traveler",
	"Time Traveler",
	"Final",
	"Final EX"
};

static void Usage(char* execName){
	printf("%s - Tool for dealing with .abm and .bmp files\n", execName);
	printf("Usage: %s (option) (params)\n", execName);
	printf("\n");
	printf("(option) is one of:\n");
	printf("\t--help              Displays program usage.\n");
	printf("\t--info              Display information about .abm file.\n");
	printf("\t--tobmp (version)   Convert .abm to .bmp; requires version\n");
	printf("\t--toabm (version)   Convert .bmp to .abm; requires version\n");
	printf("\n");
	printf("Possible values for (version):\n");
	printf("\told\t2nd Trax to Endless Circulation\n");
	printf("\tev\tEvolve\n");
	printf("\tnt\tNight Traveler\n");
	printf("\ttt\tTime Traveler\n");
	printf("\tfn\tFinal\n");
	printf("\tfnex\tFinal EX\n");
	printf("\tauto\tattempt to auto-detect (--tobmp only)\n");
	printf("\n");
	printf("Example usage:\n");
	printf("%s --info (filename.abm)\n",execName);
	printf("%s --tobmp auto (filename.abm) [outfile.bmp]\n",execName);
	printf("%s --toabm old (filename.bmp) [outfile.abm]\n",execName);
}

/* Parse version string. "auto" is not handled here. */
static int ParseVersion(char *in){
	if(memcmp(in,FormatName_Old,3) == 0){
		return AbmFormatVersion_Old;
	}
	else if(memcmp(in,FormatName_EV,2) == 0){
		return AbmFormatVersion_EV;
	}
	else if(memcmp(in,FormatName_NT,2) == 0){
		return AbmFormatVersion_NT;
	}
	else if(memcmp(in,FormatName_TT,2) == 0){
		return AbmFormatVersion_TT;
	}
	/* check for FNEX before FN to prevent false positives */
	else if(memcmp(in,FormatName_FNEX,4) == 0){
		return AbmFormatVersion_FNEX;
	}
	else if(memcmp(in,FormatName_FN,2) == 0){
		return AbmFormatVersion_FN;
	}

	return AbmFormatVersion_Unknown;
}

/* If the format version is known, get the specific XOR'ed value. */
static uint32_t GetXorValue(uint32_t inValue, int formatVersion, int xorIndex){
	/* prevent invalid index access */
	if(xorIndex > 3 || xorIndex < 0){
		return inValue;
	}

	switch(formatVersion){
		case AbmFormatVersion_Old:  return inValue ^ XorConst_Old[xorIndex];
		case AbmFormatVersion_EV:   return inValue ^ XorConst_EV[xorIndex];
		case AbmFormatVersion_NT:   return inValue ^ XorConst_NT[xorIndex];
		case AbmFormatVersion_TT:   return inValue ^ XorConst_TT[xorIndex];
		case AbmFormatVersion_FN:   return inValue ^ XorConst_FN[xorIndex];
		case AbmFormatVersion_FNEX: return inValue ^ XorConst_FNEX[xorIndex];
	}

	/* if we don't match anything, just return the input value as-is */
	return inValue;
}

/* Since dataStartAddr is always 0x36 when decoded, we can use that to determine format version. */
static int FormatFromDataStartAddr(uint32_t dataStartAddr){
	if((dataStartAddr ^ XorConst_Old[0]) == 0x36){
		return AbmFormatVersion_Old;
	}
	else if((dataStartAddr ^ XorConst_EV[0]) == 0x36){
		return AbmFormatVersion_EV;
	}
	else if((dataStartAddr ^ XorConst_NT[0]) == 0x36){
		return AbmFormatVersion_NT;
	}
	else if((dataStartAddr ^ XorConst_TT[0]) == 0x36){
		return AbmFormatVersion_TT;
	}
	else if((dataStartAddr ^ XorConst_FN[0]) == 0x36){
		return AbmFormatVersion_FN;
	}
	else if((dataStartAddr ^ XorConst_FNEX[0]) == 0x36){
		return AbmFormatVersion_FNEX;
	}
	return AbmFormatVersion_Unknown;
}

int main(int argc, char** argv){
	FILE *inFile;
	FILE *outFile;
	char fileMagic[3] = {0,0,0};
	int formatVersion = AbmFormatVersion_Unknown;

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
			perror("Error attempting to open input file");
			exit(EXIT_FAILURE);
		}

		fread(fileMagic,1,2,inFile);
		if(memcmp(fileMagic,ABM_HEADER,2) == 0){
			AbmHeader header;
			fread(&header.bpp,sizeof(uint32_t),1,inFile);
			fread(&header.width,sizeof(uint16_t),1,inFile);
			fread(&header.height,sizeof(uint16_t),1,inFile);
			fread(&header.dataStartAddr,sizeof(uint32_t),1,inFile);
			fread(&header.infoHeaderSize,sizeof(uint32_t),1,inFile);
			fread(&header.width2,sizeof(uint32_t),1,inFile);
			fread(&header.height2,sizeof(uint32_t),1,inFile);
			fread(&header.numPlanes,sizeof(uint16_t),1,inFile);
			fread(&header.bpp2,sizeof(uint32_t),1,inFile);
			fread(&header.offset_20,sizeof(uint16_t),1,inFile);
			fread(&header.bitmapDataSize,sizeof(uint32_t),1,inFile);
			fread(&header.horizRes,sizeof(uint32_t),1,inFile);
			fread(&header.vertRes,sizeof(uint32_t),1,inFile);
			fread(&header.numPalColors,sizeof(uint32_t),1,inFile);
			fread(&header.numImportantColors,sizeof(uint32_t),1,inFile);
			printf("%s info for %s:\n",argv[0],argv[2]);

			formatVersion = FormatFromDataStartAddr(header.dataStartAddr);
			if(formatVersion == AbmFormatVersion_Unknown){
				printf("Unable to determine .abm format version.\n");
			}
			else{
				printf(".abm Format Version: %s\n",FormatNames[formatVersion]);
			}

			if(formatVersion == AbmFormatVersion_EV || formatVersion == AbmFormatVersion_NT){
				printf("[0x02] Filesize: %d bytes (0x%08X)\n",header.bpp,header.bpp);
			}
			else{
				printf("[0x02] BPP: %d (0x%08X)\n",header.bpp,header.bpp);
			}
			printf("[0x06] Image Width: %d (0x%04X)\n",header.width,header.width);
			printf("[0x08] Image Height: %d (0x%04X)\n",header.height,header.height);

			printf("[0x0A] Data Start Address (encoded): 0x%08X\n",header.dataStartAddr);
			if(formatVersion != AbmFormatVersion_Unknown){
				printf("       Data Start Address (decoded): 0x%08X\n",GetXorValue(header.dataStartAddr,formatVersion,0));
			}

			printf("[0x0E] Info Header Size: 0x%08X bytes\n",header.infoHeaderSize);
			printf("[0x12] Image Width 2 (encoded): 0x%08X\n",header.width2);
			if(formatVersion != AbmFormatVersion_Unknown){
				printf("       Image Width 2 (decoded): 0x%08X\n",GetXorValue(header.width2,formatVersion,1));
			}
			printf("[0x16] Image Height 2 (encoded): 0x%08X\n",header.height2);
			if(formatVersion != AbmFormatVersion_Unknown){
				printf("       Image Height 2 (decoded): 0x%08X\n",GetXorValue(header.height2,formatVersion,2));
			}
			printf("[0x1A] Number of color planes: %d (should be 1)\n",header.numPlanes);
			printf("[0x1C] BPP (encoded): 0x%08X\n",header.bpp2);
			if(formatVersion != AbmFormatVersion_Unknown){
				printf("       BPP (decoded): 0x%08X\n",GetXorValue(header.bpp2,formatVersion,3));
			}
			printf("[0x20] Unknown: %d (0x%04X)\n",header.offset_20,header.offset_20);
			printf("[0x22] Bitmap data size: %d bytes (0x%04X)\n",header.bitmapDataSize,header.bitmapDataSize);
			printf("[0x26] Horizontal Resolution: %d pixels/meter (0x%04X)\n",header.horizRes,header.horizRes);
			printf("[0x2A] Vertical Resolution: %d pixels/meter (0x%04X)\n",header.vertRes,header.vertRes);
			printf("[0x2E] Number of palette colors: %d (0x%04X)\n",header.numPalColors,header.numPalColors);
			printf("[0x32] Number of \"important\" colors: %d (0x%04X)\n",header.numImportantColors,header.numImportantColors);
		}
		else{
			printf("Error: Unexpected ABM header magic { 0x%02X, 0x%02X }\n",fileMagic[0],fileMagic[1]);
			exit(EXIT_FAILURE);
		}

		fclose(inFile);
	}
	else if(memcmp(argv[1],OptionFlag_ToBmp,7) == 0){
		if(argc < 3){
			printf("%s --tobmp needs a format and filename.\n",argv[0]);
			exit(EXIT_FAILURE);
		}

		inFile = fopen(argv[3],"rb");
		if(inFile == NULL){
			perror("--tobmp: Error attempting to open input file");
			exit(EXIT_FAILURE);
		}

		fread(fileMagic,1,2,inFile);
		if(memcmp(fileMagic,ABM_HEADER,2) == 0){
			AbmHeader abmHeader;
			DibHeader bmpHeader;
			fread(&abmHeader.bpp,sizeof(uint32_t),1,inFile);
			fread(&abmHeader.width,sizeof(uint16_t),1,inFile);
			fread(&abmHeader.height,sizeof(uint16_t),1,inFile);
			fread(&abmHeader.dataStartAddr,sizeof(uint32_t),1,inFile);
			fread(&abmHeader.infoHeaderSize,sizeof(uint32_t),1,inFile);
			fread(&abmHeader.width2,sizeof(uint32_t),1,inFile);
			fread(&abmHeader.height2,sizeof(uint32_t),1,inFile);
			fread(&abmHeader.numPlanes,sizeof(uint16_t),1,inFile);
			fread(&abmHeader.bpp2,sizeof(uint32_t),1,inFile);
			fread(&abmHeader.offset_20,sizeof(uint16_t),1,inFile);
			fread(&abmHeader.bitmapDataSize,sizeof(uint32_t),1,inFile);
			fread(&abmHeader.horizRes,sizeof(uint32_t),1,inFile);
			fread(&abmHeader.vertRes,sizeof(uint32_t),1,inFile);
			fread(&abmHeader.numPalColors,sizeof(uint32_t),1,inFile);
			fread(&abmHeader.numImportantColors,sizeof(uint32_t),1,inFile);

			if(memcmp(argv[2],FormatName_Auto,4) == 0){
				/* determine formatVersion from header.dataStartAddr */
				formatVersion = FormatFromDataStartAddr(abmHeader.dataStartAddr);
			}
			else{
				formatVersion = ParseVersion(argv[2]);
			}

			if(formatVersion == AbmFormatVersion_Unknown){
				printf("%s ABM to BMP error: unknown format version\n",argv[0]);
				exit(EXIT_FAILURE);
			}

			/* now is the time, do it */
			bmpHeader.offset_06 = 0;
			bmpHeader.offset_08 = 0;
			bmpHeader.dataStartAddr = GetXorValue(abmHeader.dataStartAddr,formatVersion,0);
			bmpHeader.infoHeaderSize = abmHeader.infoHeaderSize;
			bmpHeader.width = GetXorValue(abmHeader.width2,formatVersion,1);
			bmpHeader.height = GetXorValue(abmHeader.height2,formatVersion,2);
			bmpHeader.numPlanes = abmHeader.numPlanes;
			bmpHeader.bpp = GetXorValue(abmHeader.bpp2,formatVersion,3);
			bmpHeader.compression = abmHeader.offset_20;
			bmpHeader.bitmapDataSize = abmHeader.bitmapDataSize;
			bmpHeader.horizRes = abmHeader.horizRes;
			bmpHeader.vertRes = abmHeader.vertRes;
			bmpHeader.numPalColors = abmHeader.numPalColors;
			bmpHeader.numImportantColors = abmHeader.numImportantColors;

			/* determine output filename */
			char *outFilename = (char*)calloc(256,sizeof(char));
			if(argv[4] != NULL){
				strcpy(outFilename,argv[4]);
			}
			else{
				char *inFileExt = strrchr(argv[3],'.');
				if(inFileExt != NULL){
					strncpy(outFilename,argv[3],inFileExt-argv[3]);
					strcat(outFilename,".bmp");
				}
				else{
					strcpy(outFilename,argv[3]);
					strcat(outFilename,".bmp");
				}
			}

			outFile = fopen(outFilename,"wb");
			if(outFile == NULL){
				perror("Error attempting to open output file");
				exit(EXIT_FAILURE);				
			}

			fputc(DIB_HEADER[0],outFile);
			fputc(DIB_HEADER[1],outFile);

			if(formatVersion == AbmFormatVersion_EV || formatVersion == AbmFormatVersion_NT){
				bmpHeader.filesize = abmHeader.bpp;
			}
			else{
				/* determine filesize later */
				bmpHeader.filesize = bmpHeader.bitmapDataSize+bmpHeader.infoHeaderSize;
			}
			fwrite(&bmpHeader.filesize,sizeof(uint32_t),1,outFile);
			fwrite(&bmpHeader.offset_06,sizeof(uint16_t),1,outFile);
			fwrite(&bmpHeader.offset_08,sizeof(uint16_t),1,outFile);
			fwrite(&bmpHeader.dataStartAddr,sizeof(uint32_t),1,outFile);
			fwrite(&bmpHeader.infoHeaderSize,sizeof(uint32_t),1,outFile);
			fwrite(&bmpHeader.width,sizeof(uint32_t),1,outFile);
			fwrite(&bmpHeader.height,sizeof(uint32_t),1,outFile);
			fwrite(&bmpHeader.numPlanes,sizeof(uint16_t),1,outFile);
			fwrite(&bmpHeader.bpp,sizeof(uint32_t),1,outFile);
			fwrite(&bmpHeader.compression,sizeof(uint16_t),1,outFile);
			fwrite(&bmpHeader.bitmapDataSize,sizeof(uint32_t),1,outFile);
			fwrite(&bmpHeader.horizRes,sizeof(uint32_t),1,outFile);
			fwrite(&bmpHeader.vertRes,sizeof(uint32_t),1,outFile);
			fwrite(&bmpHeader.numPalColors,sizeof(uint32_t),1,outFile);
			fwrite(&bmpHeader.numImportantColors,sizeof(uint32_t),1,outFile);

			if(abmHeader.bitmapDataSize == 0){
				/* need to calculate real bitmapDataSize */
				fseek(inFile,0,SEEK_END);
				long int endPos = ftell(inFile);
				bmpHeader.bitmapDataSize = endPos+0x36;
			}

			fseek(inFile,bmpHeader.dataStartAddr,SEEK_SET);
			fseek(outFile,bmpHeader.dataStartAddr,SEEK_SET);

			uint8_t *pixels = calloc(bmpHeader.bitmapDataSize,sizeof(uint8_t));
			if(pixels == NULL){
				printf("%s --tobmp error: Unable to allocate memory for pixel data\n",argv[0]);
				exit(EXIT_FAILURE);
			}
			fread(pixels,sizeof(uint8_t),bmpHeader.bitmapDataSize,inFile);
			fwrite(pixels,sizeof(uint8_t),bmpHeader.bitmapDataSize,outFile);

			if(bmpHeader.filesize == 0){
				fseek(outFile,0,SEEK_END);
				long int endPos = ftell(outFile);
				fseek(outFile,2,SEEK_SET);
				fwrite(&endPos,sizeof(uint32_t),1,outFile);
			}

			fclose(inFile);
			free(pixels);

			fclose(outFile);
			printf("%s: Wrote output to %s\n",argv[0],outFilename);
			free(outFilename);
		}
		else{
			printf("Error: Unexpected ABM header magic { 0x%02X, 0x%02X }\n",fileMagic[0],fileMagic[1]);
			exit(EXIT_FAILURE);
		}

		fclose(inFile);
	}
	else if(memcmp(argv[1],OptionFlag_ToAbm,7) == 0){
		if(argc < 3){
			printf("%s --toabm needs a format and filename.\n",argv[0]);
			exit(EXIT_FAILURE);
		}

		if(memcmp(argv[2],FormatName_Auto,4) == 0){
			printf("'auto' is not a supported format for BMP to ABM conversion; I can't read your mind.\n");
			exit(EXIT_FAILURE);
		}

		formatVersion = ParseVersion(argv[2]);
		if(formatVersion == AbmFormatVersion_Unknown){
			printf("%s BMP to ABM error: unknown format version\n",argv[0]);
			exit(EXIT_FAILURE);
		}

		inFile = fopen(argv[3],"rb");
		if(inFile == NULL){
			perror("--toabm: Error attempting to open input file");
			exit(EXIT_FAILURE);
		}

		fread(fileMagic,1,2,inFile);
		if(memcmp(fileMagic,DIB_HEADER,2) == 0){
			DibHeader bmpHeader;
			AbmHeader abmHeader;
			fread(&bmpHeader.filesize,sizeof(uint32_t),1,inFile);
			fread(&bmpHeader.offset_06,sizeof(uint16_t),1,inFile);
			fread(&bmpHeader.offset_08,sizeof(uint16_t),1,inFile);
			fread(&bmpHeader.dataStartAddr,sizeof(uint32_t),1,inFile);
			fread(&bmpHeader.infoHeaderSize,sizeof(uint32_t),1,inFile);
			fread(&bmpHeader.width,sizeof(uint32_t),1,inFile);
			fread(&bmpHeader.height,sizeof(uint32_t),1,inFile);
			fread(&bmpHeader.numPlanes,sizeof(uint16_t),1,inFile);
			fread(&bmpHeader.bpp,sizeof(uint32_t),1,inFile);
			fread(&bmpHeader.compression,sizeof(uint16_t),1,inFile);
			fread(&bmpHeader.bitmapDataSize,sizeof(uint32_t),1,inFile);
			fread(&bmpHeader.horizRes,sizeof(uint32_t),1,inFile);
			fread(&bmpHeader.vertRes,sizeof(uint32_t),1,inFile);
			fread(&bmpHeader.numPalColors,sizeof(uint32_t),1,inFile);
			fread(&bmpHeader.numImportantColors,sizeof(uint32_t),1,inFile);

			if(formatVersion == AbmFormatVersion_EV || formatVersion == AbmFormatVersion_NT){
				/* Evolve and Night Traveler handle the first few bytes differently */
				abmHeader.bpp = bmpHeader.filesize;
				abmHeader.width = 0;
				abmHeader.height = 0;
			}
			else{
				/* all other versions */
				abmHeader.bpp = bmpHeader.bpp;
				abmHeader.width = bmpHeader.width;
				abmHeader.height = bmpHeader.height;
			}

			abmHeader.dataStartAddr = GetXorValue(bmpHeader.dataStartAddr,formatVersion,0);
			abmHeader.infoHeaderSize = bmpHeader.infoHeaderSize;
			abmHeader.width2 = GetXorValue(bmpHeader.width,formatVersion,1);
			abmHeader.height2 = GetXorValue(bmpHeader.height,formatVersion,2);
			abmHeader.numPlanes = bmpHeader.numPlanes;
			abmHeader.bpp2 = GetXorValue(bmpHeader.bpp,formatVersion,3);
			abmHeader.offset_20 = bmpHeader.compression;
			abmHeader.bitmapDataSize = bmpHeader.bitmapDataSize;
			abmHeader.horizRes = bmpHeader.horizRes;
			abmHeader.vertRes = bmpHeader.vertRes;
			abmHeader.numPalColors = bmpHeader.numPalColors;
			abmHeader.numImportantColors = bmpHeader.numImportantColors;

			char *outFilename = (char*)calloc(256,sizeof(char));
			if(argv[4] != NULL){
				strcpy(outFilename,argv[4]);
			}
			else{
				char *inFileExt = strrchr(argv[3],'.');
				if(inFileExt != NULL){
					strncpy(outFilename,argv[3],inFileExt-argv[3]);
					strcat(outFilename,".abm");
				}
				else{
					strcpy(outFilename,argv[3]);
					strcat(outFilename,".abm");
				}
			}

			outFile = fopen(outFilename,"wb");
			if(outFile == NULL){
				perror("Error attempting to open output file");
				exit(EXIT_FAILURE);				
			}

			fputc(ABM_HEADER[0],outFile);
			fputc(ABM_HEADER[1],outFile);
			fwrite(&abmHeader.bpp,sizeof(uint32_t),1,outFile);
			fwrite(&abmHeader.width,sizeof(uint16_t),1,outFile);
			fwrite(&abmHeader.height,sizeof(uint16_t),1,outFile);
			fwrite(&abmHeader.dataStartAddr,sizeof(uint32_t),1,outFile);
			fwrite(&abmHeader.infoHeaderSize,sizeof(uint32_t),1,outFile);
			fwrite(&abmHeader.width2,sizeof(uint32_t),1,outFile);
			fwrite(&abmHeader.height2,sizeof(uint32_t),1,outFile);
			fwrite(&abmHeader.numPlanes,sizeof(uint16_t),1,outFile);
			fwrite(&abmHeader.bpp2,sizeof(uint32_t),1,outFile);
			fwrite(&abmHeader.offset_20,sizeof(uint16_t),1,outFile);
			if(formatVersion == AbmFormatVersion_Old){
				fputc(0,outFile);
				fputc(0,outFile);
				fputc(0,outFile);
				fputc(0,outFile);
			}
			else{
				fwrite(&abmHeader.bitmapDataSize,sizeof(uint32_t),1,outFile);
			}
			fwrite(&abmHeader.horizRes,sizeof(uint32_t),1,outFile);
			fwrite(&abmHeader.vertRes,sizeof(uint32_t),1,outFile);
			fwrite(&abmHeader.numPalColors,sizeof(uint32_t),1,outFile);
			fwrite(&abmHeader.numImportantColors,sizeof(uint32_t),1,outFile);

			uint8_t *pixels = calloc(bmpHeader.bitmapDataSize,sizeof(uint8_t));
			if(pixels == NULL){
				printf("%s --toabm error: Unable to allocate memory for pixel data\n",argv[0]);
				exit(EXIT_FAILURE);
			}
			fread(pixels,sizeof(uint8_t),bmpHeader.bitmapDataSize,inFile);
			fwrite(pixels,sizeof(uint8_t),bmpHeader.bitmapDataSize,outFile);
			fclose(outFile);
			free(pixels);
			printf("%s: Wrote output to %s\n",argv[0],outFilename);
			free(outFilename);
		}
		else{
			printf("Error: Unexpected BMP header magic { 0x%02X, 0x%02X }\n",fileMagic[0],fileMagic[1]);
			exit(EXIT_FAILURE);
		}

		fclose(inFile);
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
