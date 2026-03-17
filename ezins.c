/* ezins - .ezi file utility */
/*============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "ezifile.h"

static void Usage(char* execName){
	printf("%s - Displays information about (decoded) .ezi files\n", execName);
	printf("Usage: %s [file.ezi]\n", execName);
}

int main(int argc, char** argv){
	FILE *eziFile;
	char lineBuf[512];
	char *linePtr;
	uint8_t fileVer; /* 0 = old, 1 = new */
	OldInstrument oldIns;
	NewInstrument newIns;

	if(argc <= 1){
		Usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	eziFile = fopen(argv[1],"r");
	if(eziFile == NULL){
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	/* determine format version based on first character */
	if(isdigit(fgetc(eziFile))){
		fileVer = 1;
		printf("new version .ezi\n");
	}
	else{
		fileVer = 0;
		printf("old version .ezi\n");
	}
	fseek(eziFile,0,SEEK_SET);

	while(feof(eziFile) == 0){
		fgets(lineBuf,sizeof(lineBuf),eziFile);
		if(fileVer == 1){
			int temp;
			if(sscanf(lineBuf,"%i",&temp) != 0){
				printf("[%s%d]\t",OldNoteNames[KeysoundIndexToNote(temp)],KeysoundIndexToOctave(temp));
			}
		}

		/* process line, using space as separator token */
		linePtr = strtok(lineBuf," \r\n");
		while(linePtr != NULL){
			printf("%s\t",linePtr);
			linePtr = strtok(NULL," \r\n");
		}
		printf("\n");
	}

	fclose(eziFile);
	return EXIT_SUCCESS;
}
