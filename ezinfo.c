/* ez(2)info, ez(2)info, ez(2)info nowwwwwww */
/*============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ezfile.h"

static const char* TrackSlot[] = {
	"Control",
	"BG L",
	"BG R",
	"1P Key1",
	"1P Key2",
	"1P Key3",
	"1P Key4",
	"1P Key5",
	"Effector1",
	"Effector2",
	"1P Scratch",
	"1P Pedal",
	"Effector3",
	"Effector4",
	"2P Key1",
	"2P Key2",
	"2P Key3",
	"2P Key4",
	"2P Key5",
	"2P Scratch",
	"2P Pedal",
	"Lights",
};

static void Usage(char* execName){
	printf("%s - Prints information about .ez files\n", execName);
	printf("Usage: %s [file.ez]\n", execName);
}

int main(int argc, char** argv){
	FILE *ezFile;
	long filesize;
	char fileHeader[4]; /* first 4 bytes of file */
	EzHeader headerData;
	int noteBytes;

	if(argc <= 1){
		Usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	ezFile = fopen(argv[1],"rb");
	if(ezFile == NULL){
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}

	/* check for EZFF; file might need to be decrypted first */
	if(fgets(fileHeader,5,ezFile) != NULL){
		if(memcmp(fileHeader,Magic_EZFF,4) == 0){
			/* this is probably unnecessary */
			headerData.magic[0] = fileHeader[0];
			headerData.magic[1] = fileHeader[1];
			headerData.magic[2] = fileHeader[2];
			headerData.magic[3] = fileHeader[3];
		}
		else if(memcmp(fileHeader,Magic_EZFF_7th_1dot5,4) == 0){
			printf("This appears to be an EZ2DJ 7th Trax v1.5 encoded .ez file. Please decode the file first, if possible.\n");
			fclose(ezFile);
			exit(EXIT_FAILURE);
		}
		else{
			printf("Unexpected header values 0x%02X,0x%02X,0x%02X,0x%02X; file might need to be decoded first.\n",fileHeader[0],fileHeader[1],fileHeader[2],fileHeader[3]);
			fclose(ezFile);
			exit(EXIT_FAILURE);
		}
	}

	/* get filesize */
	fseek(ezFile, 0, SEEK_END);
	filesize = ftell(ezFile);
	fseek(ezFile, 5, SEEK_SET);

	headerData.version = (uint8_t)fgetc(ezFile);

	switch(headerData.version){
		case 4:
			noteBytes = BYTES_PER_NOTE_V4;
			break;

		case 5:
		case 6:
			noteBytes = BYTES_PER_NOTE_V5;
			break;

		case 7:
		case 8:
		default:
			noteBytes = BYTES_PER_NOTE_V7;
			break;
	}

	fgets(headerData.name,0x40,ezFile);

	/* skip 0x40 bytes */
	fseek(ezFile, 0x86, SEEK_SET); /* xxx: hardcoded */

	fread(&headerData.ticksPerMeasure,2,1,ezFile);
	fread(&headerData.initialBPM,4,1,ezFile);
	fread(&headerData.numTracks,2,1,ezFile);
	fread(&headerData.totalTicks,4,1,ezFile);
	fread(&headerData.secondBPM,4,1,ezFile);

	/* read track data */
	Track tracks[headerData.numTracks];
	for(int i = 0; i < headerData.numTracks; i++){
		fgets(tracks[i].magic,5,ezFile);
		fread(&tracks[i].pad,2,1,ezFile);
		fgets(tracks[i].name,0x41,ezFile);
		fread(&tracks[i].numTicks,4,1,ezFile);
		fread(&tracks[i].dataSize,4,1,ezFile);
		int numNotes = tracks[i].dataSize/noteBytes;

		tracks[i].notes = (Note*)calloc(numNotes, sizeof(Note));
		for(int n = 0; n < numNotes; n++){
			fread(&tracks[i].notes[n].position,4,1,ezFile);
			tracks[i].notes[n].type = (uint8_t)fgetc(ezFile);
			if(headerData.version >= 7){
				/* v7, v8 */
				fread(&tracks[i].notes[n].data.v7,NOTE_DATA_LENGTH_V7,1,ezFile);
			}
			else if(headerData.version >= 5){
				/* v5, v6 */
				fread(&tracks[i].notes[n].data.v5,NOTE_DATA_LENGTH_V5,1,ezFile);
			}
			else{
				/* v4 */
				fread(&tracks[i].notes[n].data.v4,NOTE_DATA_LENGTH_V4,1,ezFile);
			}
		}
	}

	/* if v8, some extra tampering is done to the data... */

	fclose(ezFile);

	printf("ezinfo for '%s'\n", argv[1]);
	printf("filesize:\t%li bytes\n", filesize);
	printf(".ez version:\t%i\n", headerData.version);
	if(headerData.version == 8){
		printf("Warning: v8 data may have tampered bytes; ezinfo does not currently detect this.\n");
	}
	if(headerData.name[0] != 0){
		printf("internal name:\t%s\n", headerData.name);
	}
	printf("ticks/measure:\t%hu (0x%04X)\n",headerData.ticksPerMeasure,headerData.ticksPerMeasure);
	if(headerData.version <= 5){
		union {
			float f;
			uint32_t u;
		} AdjustedBPM;
		AdjustedBPM.f = headerData.initialBPM.f * OldBpmFactor;
		printf("initial BPM:\t%f (0x%X) | adjusted: %f (0x%X)\n",
			headerData.initialBPM.f,headerData.initialBPM.u,
			AdjustedBPM.f,AdjustedBPM.u
		);
	}
	else{
		printf("initial BPM:\t%f (0x%X)\n",headerData.initialBPM.f,headerData.initialBPM.u);
	}
	printf("track count:\t%i\n",headerData.numTracks);
	printf("total ticks:\t%u (0x%08X)\n",headerData.totalTicks,headerData.totalTicks);
	if(headerData.version <= 5){
		union {
			float f;
			uint32_t u;
		} AdjustedBPM;
		AdjustedBPM.f = headerData.secondBPM.f * OldBpmFactor;
		printf("other BPM:\t%f (0x%X) | adjusted: %f (0x%X)\n",
			headerData.secondBPM.f,headerData.secondBPM.u,
			AdjustedBPM.f,AdjustedBPM.u
		);
	}
	else{
		printf("other BPM:\t%f (0x%X)\n",headerData.secondBPM.f,headerData.secondBPM.u);
	}

	printf("\n");
	/* print track data */
	for(int i = 0; i < headerData.numTracks; i++){
		int numNotes = tracks[i].dataSize/noteBytes;
		printf("--------------------\n");
		if(i < 22){
			printf("Track %d [%s]\n",i,TrackSlot[i]);
		}
		else{
			printf("Track %d [BGM]\n",i);
		}
		printf("name: %s\n",tracks[i].name);
		printf("numTicks: %u (0x%08X)\n",tracks[i].numTicks,tracks[i].numTicks);
		printf("dataSize: %u (0x%08X)\n",tracks[i].dataSize,tracks[i].dataSize);
		if(tracks[i].dataSize > 0){
			printf("num. notes: %u\n",numNotes);
		}

		for(int n = 0; n < numNotes; n++){
			printf("pos 0x%08X: type %d; data: ",tracks[i].notes[n].position,tracks[i].notes[n].type);
			switch(headerData.version){
				case 4:
					for(int p = 0; p < NOTE_DATA_LENGTH_V4; p++){
						printf("%02X ",tracks[i].notes[n].data.v4[p]);
					}
					break;

				case 5:
				case 6:
					for(int p = 0; p < NOTE_DATA_LENGTH_V5; p++){
						printf("%02X ",tracks[i].notes[n].data.v5[p]);
					}
					break;

				case 7:
				case 8:
				default:
					for(int p = 0; p < NOTE_DATA_LENGTH_V7; p++){
						printf("%02X ",tracks[i].notes[n].data.v7[p]);
					}
					break;
			}
			printf("| ");
			uint16_t len = 0;
			switch(tracks[i].notes[n].type){
				case NoteType_Note:
					printf("Note { ");
						switch(headerData.version){
							case 4:
								printf("Sound 0x%02X ",tracks[i].notes[n].data.v4[0]);
								printf("Vel 0x%02X ",tracks[i].notes[n].data.v4[1]);
								printf("Pan 0x%02X ",tracks[i].notes[n].data.v4[2]);
								len = tracks[i].notes[n].data.v4[4] << 8 | tracks[i].notes[n].data.v4[3];
								printf("Len 0x%04X",len);
								break;

							case 5:
							case 6:
								printf("Sound 0x%02X ",tracks[i].notes[n].data.v5[0]);
								printf("Vel 0x%02X ",tracks[i].notes[n].data.v5[1]);
								printf("Pan 0x%02X ",tracks[i].notes[n].data.v5[2]);
								printf("unk1 0x%02X ",tracks[i].notes[n].data.v5[3]);
								len = tracks[i].notes[n].data.v5[5] << 8 | tracks[i].notes[n].data.v5[4];
								printf("Len 0x%04X",len);
								break;

							case 7:
							case 8:
							default:
								uint16_t sound = tracks[i].notes[n].data.v7[1] << 8 | tracks[i].notes[n].data.v7[0];
								printf("Sound 0x%04X ",sound);
								printf("Vel 0x%02X ",tracks[i].notes[n].data.v7[2]);
								printf("Pan 0x%02X ",tracks[i].notes[n].data.v7[3]);
								printf("unk1 0x%02X ",tracks[i].notes[n].data.v7[4]);
								len = tracks[i].notes[n].data.v7[6] << 8 | tracks[i].notes[n].data.v7[5];
								printf("Len 0x%04X ",len);
								printf("pad 0x%02X",tracks[i].notes[n].data.v7[7]);
								break;
						}
					printf(" }");
					break;
				case NoteType_Volume:
					printf("Volume = ");
					switch(headerData.version){
						case 4:
							printf("%d",tracks[i].notes[n].data.v4[0]);
							break;

						case 5:
						case 6:
							printf("%d",tracks[i].notes[n].data.v5[0]);
							break;

						case 7:
						case 8:
						default:
							printf("%d",tracks[i].notes[n].data.v7[0]);
							break;
					}
					break;
				case NoteType_BPM:
					printf("BPM = ");
					union{
						float f;
						int i;
					} bpm;
					switch(headerData.version){
						case 4:
							bpm.i = (tracks[i].notes[n].data.v4[3] << 24) |
								(tracks[i].notes[n].data.v4[2] << 16) |
								(tracks[i].notes[n].data.v4[1] << 8) |
								tracks[i].notes[n].data.v4[0];
							break;

						case 5:
						case 6:
							bpm.i = (tracks[i].notes[n].data.v5[3] << 24) |
								(tracks[i].notes[n].data.v5[2] << 16) |
								(tracks[i].notes[n].data.v5[1] << 8) |
								tracks[i].notes[n].data.v5[0];
							break;

						case 7:
						case 8:
						default:
							bpm.i = (tracks[i].notes[n].data.v7[3] << 24) |
								(tracks[i].notes[n].data.v7[2] << 16) |
								(tracks[i].notes[n].data.v7[1] << 8) |
								tracks[i].notes[n].data.v7[0];
							break;
					}
					if(headerData.version <= 5){
						printf("%f (adjusted: %f)",bpm.f, bpm.f*OldBpmFactor);
					}
					else{
						printf("%f",bpm.f);
					}
					break;
				case NoteType_BeatsPerMeasure:
					printf("Beats per Measure = ");
					switch(headerData.version){
						case 4:
							printf("%d",tracks[i].notes[n].data.v4[0]);
							break;

						case 5:
						case 6:
							printf("%d",tracks[i].notes[n].data.v5[0]);
							break;

						case 7:
						case 8:
						default:
							printf("%d",tracks[i].notes[n].data.v7[0]);
							break;
					}
					break;
				default:
					printf("unhandled type %d\n",tracks[i].notes[n].type);
				break;
			}
			printf("\n");
		}

		/* free track note data */
		free(tracks[i].notes);
	}

	return EXIT_SUCCESS;
}
