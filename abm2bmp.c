/* abm2bmp - modified version of ezabm that only does .abm -> .bmp */
/*============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "abmfile.h"

static void Usage(char* execName){
	printf("%s - Convert .abm to .bmp\n", execName);
	printf("Usage: %s (filename.abm) [outfile.bmp]\n", execName);
}

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

	inFile = fopen(argv[1],"rb");
	if(inFile == NULL){
		perror("abm2bmp: Error attempting to open input file");
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

		/* determine formatVersion from header.dataStartAddr */
		formatVersion = FormatFromDataStartAddr(abmHeader.dataStartAddr);

		if(formatVersion == AbmFormatVersion_Unknown){
			printf("%s error: unknown format version\n",argv[0]);
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
		if(argv[2] != NULL){
			strcpy(outFilename,argv[2]);
		}
		else{
			char *inFileExt = strrchr(argv[1],'.');
			if(inFileExt != NULL){
				strncpy(outFilename,argv[1],inFileExt-argv[1]);
				strcat(outFilename,".bmp");
			}
			else{
				strcpy(outFilename,argv[1]);
				strcat(outFilename,".bmp");
			}
		}

		outFile = fopen(outFilename,"wb");
		if(outFile == NULL){
			perror("abm2bmp: Error attempting to open output file");
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
			printf("%s error: Unable to allocate memory for pixel data\n",argv[0]);
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
	return EXIT_SUCCESS;
}
