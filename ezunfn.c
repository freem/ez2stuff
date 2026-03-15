/* ezunfn/ezrefn - functionality related to "anti-tamper pattern" found in v8 .ez files */
/*============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ezfile.h"

static void Usage(char* execName){
#ifdef __REFN
	printf("%s - Adds anti-tamper pattern from v8 .ez files\n", execName);
#else
	printf("%s - Removes anti-tamper pattern from v8 .ez files\n", execName);
#endif
	printf("Usage: %s [file.ez]\n", execName);
}

int main(int argc, char** argv){
	FILE *ezFile;
	long filesize;
	char fileHeader[4]; /* first 4 bytes of file */
	Header headerData;

	if(argc <= 1){
		Usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	ezFile = fopen(argv[1],"r+b");
	if(ezFile == NULL){
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	/* check for EZFF; file might need to be decrypted first */
	if(fgets(fileHeader,5,ezFile) != NULL){
		if(memcmp(fileHeader,Magic_EZFF,4) != 0){
			printf("%s Error: This does not appear to be a decrypted .ez file.\n",argv[0]);
			fclose(ezFile);
			exit(EXIT_FAILURE);
		}

		/* get filesize */
		fseek(ezFile, 0, SEEK_END);
		filesize = ftell(ezFile);
		fseek(ezFile, 5, SEEK_SET);

		headerData.version = (uint8_t)fgetc(ezFile);
		if(headerData.version != 8){
#ifdef __REFN
		/* ezrefn only: if this is v7, upgrade to v8 and continue */
			if(headerData.version == 7){
				printf("%s: found v7 data, upgrading to v8.\n",argv[0]);
				fseek(ezFile,-1,SEEK_CUR);
				fputc(8,ezFile);
				/* todo: write the first 63 characters of the input filename */
			}
			else{
				printf("%s Error: File version %d != 7 or 8\n",argv[0],headerData.version);
				fclose(ezFile);
				exit(EXIT_FAILURE);
			}
#else
			printf("%s Error: File version %d != 8\n",argv[0],headerData.version);
			fclose(ezFile);
			exit(EXIT_FAILURE);
#endif
		}

		printf("%s offsets and values for %s:\n",argv[0],argv[1]);

		long offset1 = 0x1F8;
		long offset2 = 0x400;
		long offset3 = 0x5F0;
		int i = 0;
		uint8_t val,dec = 0;
		/* it's also come to my attention that you could just subtract 7
		 * from the value instead of adding 0xF9 and masking. */
		while(i < filesize){
			if(offset1 + i < filesize){
				fseek(ezFile, offset1+i, SEEK_SET);
				val = fgetc(ezFile);
#ifdef __REFN
				dec = (val+0xF9)&0xFF;
#else
				dec = (val-0xF9)&0xFF;
#endif
				printf("[1] offset 0x%lX: 0x%02X -> 0x%02X\n", offset1+i, val, dec);
				fseek(ezFile, -1, SEEK_CUR);
				fputc(dec,ezFile);
			}
			if(offset2 + i < filesize){
				fseek(ezFile, offset2+i, SEEK_SET);
				val = fgetc(ezFile);
#ifdef __REFN
				dec = (val+0xF9)&0xFF;
#else
				dec = (val-0xF9)&0xFF;
#endif
				printf("[2] offset 0x%lX: 0x%02X -> 0x%02X\n", offset2+i, val, dec);
				fseek(ezFile, -1, SEEK_CUR);
				fputc(dec,ezFile);
			}
			if(offset3 + i < filesize){
				fseek(ezFile, offset3+i, SEEK_SET);
				val = fgetc(ezFile);
#ifdef __REFN
				dec = (val+0xF9)&0xFF;
#else
				dec = (val-0xF9)&0xFF;
#endif
				printf("[3] offset 0x%lX: 0x%02X -> 0x%02X\n", offset3+i, val, dec);
				fseek(ezFile, -1, SEEK_CUR);
				fputc(dec,ezFile);
			}

			i += 0x600;
		}
		printf("%s: Wrote modified version to %s\n",argv[0],argv[1]);
	}

	fclose(ezFile);
	return EXIT_SUCCESS;
}
