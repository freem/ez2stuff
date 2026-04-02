/* ezins - .ezi file utility */
/* Warning: This code relies on .ezi files being structured somewhat sanely. */
/*============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "ezifile.h"

const char OptionFlag_Info[6] = { "--info" };
const char OptionFlag_Old[5]  = { "--old" };
const char OptionFlag_New[5]  = { "--new" };
const char OptionFlag_Help[6] = { "--help" };

static void Usage(char* execName){
	printf("%s - Tool for dealing with (decoded) .ezi files\n", execName);
	printf("Usage: %s (option) (filename.ezi)\n", execName);
	printf("\n");
	printf("(option) is one of:\n");
	printf("\t--help\tDisplays program usage.\n");
	printf("\t--info\tDisplay information about .ezi file.\n");
	printf("\t--old\tConvert new format .ezi files to old format.\n");
	printf("\t--new\tConvert old format .ezi files to new format.\n");
	printf("\n");
	printf("Example usage:\n");
	printf("%s --info (filename.ezi)\n",execName);
	printf("%s --old (filename.ezi)\n",execName);
	printf("%s --new (filename.ezi)\n",execName);
}

const char* OctaveNums = "0123456789";

int main(int argc, char** argv){
	FILE *eziFile;
	char lineBuf[512];
	uint8_t fileVer; /* 0 = old, 1 = new */
	char oldNoteStr[5]; /* only needed for old version .ezi */
	Instrument ins;
	int numInstruments = 0;

	if(argc <= 1){
		Usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if(memcmp(argv[1],OptionFlag_Info,6) == 0){
		if(argc < 3){
			printf("%s error: --info requires a filename.\n",argv[0]);
			exit(EXIT_FAILURE);
		}

		eziFile = fopen(argv[2],"r");
		if(eziFile == NULL){
			perror("Error opening file");
			exit(EXIT_FAILURE);
		}

		printf("%s info for %s:\n",argv[0],argv[2]);
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

		char *noteName = calloc(4,1);
		while(feof(eziFile) == 0){
			memset(lineBuf,0,sizeof(lineBuf));
			fgets(lineBuf,sizeof(lineBuf),eziFile);

			/* some .ezi files do not include a newline at the end.
			 * we need to be able to handle these files properly.
			 */
			if(feof(eziFile)){
				if(!isalnum((int)lineBuf[0])){
					break;
				}
			}

			if(fileVer == 1){
				// keysound_index type filename
				sscanf(lineBuf,"%hi %d %s",&ins.index,&ins.type,ins.filename);
				if(ins.index < MAX_INSTRUMENTS_OLD){
					printf("%hi (== %s%d) %d %s",ins.index,OldNoteNames[KeysoundIndexToNote(ins.index)],KeysoundIndexToOctave(ins.index),ins.type,ins.filename);
				}
				else if(ins.index > MAX_INSTRUMENTS_NEW){
					printf("%hi (invalid) %d %s",ins.index,ins.type,ins.filename);
				}
				else{
					printf("%hi (n/a) %d %s",ins.index,ins.type,ins.filename);
				}
				numInstruments++;
			}
			else{
				// key_name type filename
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
				numInstruments++;
			}
			printf("\n");
		}
		free(noteName);
		printf("Number of instruments defined: %d\n",numInstruments);
		fclose(eziFile);
	}
	else if(memcmp(argv[1],OptionFlag_Old,5) == 0){
		if(argc < 3){
			printf("%s error: --old requires a filename.\n",argv[0]);
			exit(EXIT_FAILURE);
		}

		eziFile = fopen(argv[2],"r+");
		if(eziFile == NULL){
			perror("Error opening file");
			exit(EXIT_FAILURE);
		}

		/* check if file is already in the old format; if so, we do nothing */
		if(!isdigit(fgetc(eziFile))){
			printf("%s is already in the old .ezi format.\n",argv[2]);
			exit(EXIT_FAILURE);
		}

		/* check number of defined instruments */
		fseek(eziFile,0,SEEK_SET);
		while(feof(eziFile) == 0){
			memset(lineBuf,0,sizeof(lineBuf));
			fgets(lineBuf,sizeof(lineBuf),eziFile);

			if(feof(eziFile)){
				if(!isalnum((int)lineBuf[0])){
					break;
				}
			}

			sscanf(lineBuf,"%hi %d %s",&ins.index,&ins.type,ins.filename);
			numInstruments++;
		}

		if(numInstruments > MAX_INSTRUMENTS_OLD){
			printf("Error: Too many instruments defined for old format; found %d (maximum 256)\n",numInstruments);
			exit(EXIT_FAILURE);
		}

		fseek(eziFile,0,SEEK_SET);
		Instrument *instruments = calloc(numInstruments,sizeof(Instrument));
		if(instruments == NULL){
			printf("Error: Unable to allocate memory for instrument data.\n");
			exit(EXIT_FAILURE);
		}

		int curInstrument = 0;
		while(feof(eziFile) == 0){
			memset(lineBuf,0,sizeof(lineBuf));
			fgets(lineBuf,sizeof(lineBuf),eziFile);
			if(feof(eziFile)){
				if(!isalnum((int)lineBuf[0])){
					break;
				}
			}

			/* keysound_index type filename */
			sscanf(lineBuf,"%hi %d %s",&instruments[curInstrument].index,&instruments[curInstrument].type,instruments[curInstrument].filename);
			curInstrument++;
		}

		eziFile = freopen(argv[2],"w",eziFile);
		if(eziFile == NULL){
			printf("Error: Unable to re-open file for writing.\n");
			exit(EXIT_FAILURE);
		}
		for(int i = 0; i < numInstruments; i++){
			fprintf(eziFile, "%s%d %d %s\r\n",
				OldNoteNames[KeysoundIndexToNote(instruments[i].index)],
				KeysoundIndexToOctave(instruments[i].index),
				instruments[i].type,
				instruments[i].filename
			);
		}

		printf("%s: wrote output to %s\n",argv[0],argv[2]);
		free(instruments);
		fclose(eziFile);
	}
	else if(memcmp(argv[1],OptionFlag_New,5) == 0){
		if(argc < 3){
			printf("%s error: --new requires a filename.\n",argv[0]);
			exit(EXIT_FAILURE);
		}

		eziFile = fopen(argv[2],"r+");
		if(eziFile == NULL){
			perror("Error opening file");
			exit(EXIT_FAILURE);
		}

		/* check if file is already in the new format; if so, we do nothing */
		if(isdigit(fgetc(eziFile))){
			printf("%s is already in the new .ezi format.\n",argv[2]);
			exit(EXIT_FAILURE);
		}

		/* get number of defined instruments */
		fseek(eziFile,0,SEEK_SET);
		while(feof(eziFile) == 0){
			memset(lineBuf,0,sizeof(lineBuf));
			fgets(lineBuf,sizeof(lineBuf),eziFile);

			if(feof(eziFile)){
				if(!isalnum((int)lineBuf[0])){
					break;
				}
			}

			sscanf(lineBuf,"%s %d %s",oldNoteStr,&ins.type,ins.filename);
			numInstruments++;
		}

		if(numInstruments > MAX_INSTRUMENTS_NEW){
			printf("Error: Too many instruments defined for new format; found %d (maximum 2048)\n",numInstruments);
			exit(EXIT_FAILURE);
		}

		fseek(eziFile,0,SEEK_SET);
		Instrument *instruments = calloc(numInstruments,sizeof(Instrument));
		if(instruments == NULL){
			printf("Error: Unable to allocate memory for instrument data.\n");
			exit(EXIT_FAILURE);
		}

		char *noteName = calloc(4,1);
		int curInstrument = 0;
		while(feof(eziFile) == 0){
			memset(lineBuf,0,sizeof(lineBuf));
			fgets(lineBuf,sizeof(lineBuf),eziFile);
			if(feof(eziFile)){
				if(!isalnum((int)lineBuf[0])){
					break;
				}
			}

			/* key_name type filename */
			sscanf(lineBuf,"%s %d %s",oldNoteStr,&instruments[curInstrument].type,instruments[curInstrument].filename);

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
			instruments[curInstrument].index = NoteOctaveToKeysoundIndex(keyIndex,oct);
			curInstrument++;
		}

		eziFile = freopen(argv[2],"w",eziFile);
		if(eziFile == NULL){
			printf("Error: Unable to re-open file for writing.\n");
			exit(EXIT_FAILURE);
		}
		for(int i = 0; i < numInstruments; i++){
			fprintf(eziFile, "%d %d %s\r\n",instruments[i].index, instruments[i].type, instruments[i].filename);
		}

		printf("%s: wrote output to %s\n",argv[0],argv[2]);
		free(instruments);
		fclose(eziFile);
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
