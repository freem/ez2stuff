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
 * - display abm info   | --info (infile.abm)
 * - convert abm to bmp | --tobmp (ver) (infile.abm) [outfile.bmp]
 * - convert bmp to abm | --toabm (ver) (infile.bmp) [outfile.abm]
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

static void Usage(char* execName){
	printf("%s - Tool for dealing with .abm and .bmp files\n", execName);
	printf("Usage: %s (option) (params)\n", execName);
	printf("\n");
	printf("(option) is one of:\n");
	printf("\t--help\t\tDisplays program usage.\n");
	printf("\t--info\t\tDisplay information about .abm file.\n");
	printf("\t--tobmp (version)\tConvert .abm to .bmp; requires version\n");
	printf("\t--toabm (version)\tConvert .bmp to .abm; requires version\n");
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
	printf("--info (filename.abm)\n");
	printf("--tobmp old (filename.abm) [outfile.bmp]\n");
	printf("--toabm old (filename.bmp) [outfile.abm]\n");
	printf("\n");
}

int main(int argc, char** argv){
	FILE *inFile;
	FILE *outFile;
	char fileMagic[3] = {0,0,0};
	int formatVersion = AbmFormatVersion_Unknown;

	if(argc < 3){
		Usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if(memcmp(argv[1],OptionFlag_Info,6) == 0){
		inFile = fopen(argv[2],"rb");
		if(inFile == NULL){
			perror("Error attempting to open file");
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

			if((header.dataStartAddr ^ XorConst_Old[0]) == 0x36){
				formatVersion = AbmFormatVersion_Old;
				printf("Format Version: old .abm\n");
			}
			else if((header.dataStartAddr ^ XorConst_EV[0]) == 0x36){
				formatVersion = AbmFormatVersion_EV;
				printf("Format Version: Evolve .abm\n");
			}
			else if((header.dataStartAddr ^ XorConst_NT[0]) == 0x36){
				formatVersion = AbmFormatVersion_NT;
				printf("Format Version: Night Traveler .abm\n");
			}
			else if((header.dataStartAddr ^ XorConst_TT[0]) == 0x36){
				formatVersion = AbmFormatVersion_TT;
				printf("Format Version: Time Traveler .abm\n");
			}
			else if((header.dataStartAddr ^ XorConst_FN[0]) == 0x36){
				formatVersion = AbmFormatVersion_FN;
				printf("Format Version: Final .abm\n");
			}
			else if((header.dataStartAddr ^ XorConst_FNEX[0]) == 0x36){
				formatVersion = AbmFormatVersion_FNEX;
				printf("Format Version: Final EX .abm\n");
			}
			else{
				formatVersion = AbmFormatVersion_Unknown;
				printf("Unable to determine .abm version\n");
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
				printf("       Data Start Address (decoded): 0x");
				switch(formatVersion){
					case AbmFormatVersion_Old:
					default:
						printf("%08X\n",header.dataStartAddr ^ XorConst_Old[0]);
						break;

					case AbmFormatVersion_EV:
						printf("%08X\n",header.dataStartAddr ^ XorConst_EV[0]);
						break;

					case AbmFormatVersion_NT:
						printf("%08X\n",header.dataStartAddr ^ XorConst_NT[0]);
						break;

					case AbmFormatVersion_TT:
						printf("%08X\n",header.dataStartAddr ^ XorConst_TT[0]);
						break;

					case AbmFormatVersion_FN:
						printf("%08X\n",header.dataStartAddr ^ XorConst_FN[0]);
						break;

					case AbmFormatVersion_FNEX:
						printf("%08X\n",header.dataStartAddr ^ XorConst_FNEX[0]);
						break;
				}
			}

			printf("[0x0E] Info Header Size: 0x%08X bytes\n",header.infoHeaderSize);
			printf("[0x12] Image Width 2 (encoded): 0x%08X\n",header.width2);
			if(formatVersion != AbmFormatVersion_Unknown){
				printf("       Image Width 2 (decoded): 0x");
				switch(formatVersion){
					case AbmFormatVersion_Old:
					default:
						printf("%08X\n",header.width2 ^ XorConst_Old[1]);
						break;

					case AbmFormatVersion_EV:
						printf("%08X\n",header.width2 ^ XorConst_EV[1]);
						break;

					case AbmFormatVersion_NT:
						printf("%08X\n",header.width2 ^ XorConst_NT[1]);
						break;

					case AbmFormatVersion_TT:
						printf("%08X\n",header.width2 ^ XorConst_TT[1]);
						break;

					case AbmFormatVersion_FN:
						printf("%08X\n",header.width2 ^ XorConst_FN[1]);
						break;

					case AbmFormatVersion_FNEX:
						printf("%08X\n",header.width2 ^ XorConst_FNEX[1]);
						break;
				}
			}
			printf("[0x16] Image Height 2 (encoded): 0x%08X\n",header.height2);
			if(formatVersion != AbmFormatVersion_Unknown){
				printf("       Image Height 2 (decoded): 0x");
				switch(formatVersion){
					case AbmFormatVersion_Old:
					default:
						printf("%08X\n",header.height2 ^ XorConst_Old[2]);
						break;

					case AbmFormatVersion_EV:
						printf("%08X\n",header.height2 ^ XorConst_EV[2]);
						break;

					case AbmFormatVersion_NT:
						printf("%08X\n",header.height2 ^ XorConst_NT[2]);
						break;

					case AbmFormatVersion_TT:
						printf("%08X\n",header.height2 ^ XorConst_TT[2]);
						break;

					case AbmFormatVersion_FN:
						printf("%08X\n",header.height2 ^ XorConst_FN[2]);
						break;

					case AbmFormatVersion_FNEX:
						printf("%08X\n",header.height2 ^ XorConst_FNEX[2]);
						break;
				}
			}
			printf("[0x1A] Number of color planes: %d (should be 1)\n",header.numPlanes);
			printf("[0x1C] BPP (encoded): 0x%08X\n",header.bpp2);
			if(formatVersion != AbmFormatVersion_Unknown){
				printf("       BPP (decoded): 0x");
				switch(formatVersion){
					case AbmFormatVersion_Old:
					default:
						printf("%08X\n",header.bpp2 ^ XorConst_Old[3]);
						break;

					case AbmFormatVersion_EV:
						printf("%08X\n",header.bpp2 ^ XorConst_EV[3]);
						break;

					case AbmFormatVersion_NT:
						printf("%08X\n",header.bpp2 ^ XorConst_NT[3]);
						break;

					case AbmFormatVersion_TT:
						printf("%08X\n",header.bpp2 ^ XorConst_TT[3]);
						break;

					case AbmFormatVersion_FN:
						printf("%08X\n",header.bpp2 ^ XorConst_FN[3]);
						break;

					case AbmFormatVersion_FNEX:
						printf("%08X\n",header.bpp2 ^ XorConst_FNEX[3]);
						break;
				}
			}
			printf("[0x20] Unknown: %d (0x%04X)\n",header.offset_20,header.offset_20);
			printf("[0x22] Bitmap data size: %d bytes (0x%04X)\n",header.bitmapDataSize,header.bitmapDataSize);
			printf("[0x26] Horizontal Resolution: %d pixels/meter (0x%04X)\n",header.horizRes,header.horizRes);
			printf("[0x2A] Vertical Resolution: %d pixels/meter (0x%04X)\n",header.vertRes,header.vertRes);
			printf("[0x2E] Number of palette colors: %d (0x%04X)\n",header.numPalColors,header.numPalColors);
			printf("[0x32] Number of \"important\" colors: %d (0x%04X)\n",header.numImportantColors,header.numImportantColors);
		}
		else{
			printf("Error: Unexpected header magic '%s' (0x%02X, 0x%02X)\n",fileMagic,fileMagic[0],fileMagic[1]);
			exit(EXIT_FAILURE);
		}

		fclose(inFile);
	}
	else if(memcmp(argv[1],OptionFlag_ToBmp,6) == 0){
		printf(".abm -> .bmp not yet implemented\n");
		if(memcmp(argv[2],FormatName_Auto,4) == 0){
		}
		else if(memcmp(argv[2],FormatName_Old,3) == 0){
		}
		else if(memcmp(argv[2],FormatName_EV,2) == 0){
		}
		else if(memcmp(argv[2],FormatName_NT,2) == 0){
		}
		else if(memcmp(argv[2],FormatName_TT,2) == 0){
		}
		/* check for FNEX before FN to prevent false positives */
		else if(memcmp(argv[2],FormatName_FNEX,4) == 0){
		}
		else if(memcmp(argv[2],FormatName_FN,2) == 0){
		}
		else{
			printf("ABM to BMP error: unknown format '%s'\n",argv[2]);
			exit(EXIT_FAILURE);
		}
	}
	else if(memcmp(argv[1],OptionFlag_ToAbm,6) == 0){
		if(memcmp(argv[2],FormatName_Auto,4) == 0){
			printf("'auto' is not a supported format for BMP to ABM conversion; I can't read your mind.\n");
			exit(EXIT_FAILURE);
		}

		if(memcmp(argv[2],FormatName_Old,3) == 0){
		}
		else if(memcmp(argv[2],FormatName_EV,2) == 0){
		}
		else if(memcmp(argv[2],FormatName_NT,2) == 0){
		}
		else if(memcmp(argv[2],FormatName_TT,2) == 0){
		}
		/* check for FNEX before FN to prevent false positives */
		else if(memcmp(argv[2],FormatName_FNEX,4) == 0){
		}
		else if(memcmp(argv[2],FormatName_FN,2) == 0){
		}
		else{
			printf("BMP to ABM error: unknown format '%s'\n",argv[2]);
			exit(EXIT_FAILURE);
		}
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
