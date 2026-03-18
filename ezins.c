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

const char* OctaveNums = "0123456789";

int main(int argc, char** argv){
	FILE *eziFile;
	char lineBuf[512];
	uint8_t fileVer; /* 0 = old, 1 = new */
	char oldNoteStr[5]; /* only needed for old version .ezi */
	Instrument ins;

	if(argc <= 1){
		Usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	eziFile = fopen(argv[1],"r");
	if(eziFile == NULL){
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	printf("%s info for %s:\n",argv[0],argv[1]);
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

	/* Warning: This code relies on .ezi files being structured somewhat sanely. */
	char *noteName = calloc(4,1);
	while(feof(eziFile) == 0){
		fgets(lineBuf,sizeof(lineBuf),eziFile);
		/* don't process anything if we hit EOF */
		if(feof(eziFile)){
			break;
		}

		if(fileVer == 1){
			/* keysound_index type filename */
			sscanf(lineBuf,"%hi %d %s",&ins.index,&ins.type,ins.filename);
			if(ins.index < 256){
				printf("%hi (== %s%d) %d %s",ins.index,OldNoteNames[KeysoundIndexToNote(ins.index)],KeysoundIndexToOctave(ins.index),ins.type,ins.filename);
			}
			else{
				printf("%hi (n/a) %d %s",ins.index,ins.type,ins.filename);
			}
		}
		else{
			/* key_name type filename */
			sscanf(lineBuf,"%s %d %s",oldNoteStr,&ins.type,ins.filename);

			int p = strcspn(oldNoteStr,OctaveNums);
			noteName = calloc(4,1);
			strncpy(noteName,oldNoteStr,p);
			int oct = atoi(oldNoteStr+p);
			int keyIndex = 0;
			for(int i = 0; i < OLDNOTE_NOTE_VALUES; i++){
				if(strcmp(noteName,OldNoteNames[i]) == 0){
					keyIndex = i;
					break;
				}
			}
			printf("%s%d (== %d) %d %s",noteName,oct,NoteOctaveToKeysoundIndex(keyIndex,oct),ins.type,ins.filename);
		}
		printf("\n");
	}
	free(noteName);

	fclose(eziFile);
	return EXIT_SUCCESS;
}
