/* ez2bmson - convert {.ez, .ezi, .ini} to .bmson */
/*============================================================================*/
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ezfile.h"
#include "ezifile.h"
#include "bmsonfile.h"

const int MAX_FILENAME_LENGTH = 256;
const int LINEBUF_SIZE = 512;

/*============================================================================*/
enum ModeHintIndex {
	ModeHintIndex_5KOnly = 0,
	ModeHintIndex_5K = 1,
	ModeHintIndex_7K = 2,
	ModeHintIndex_10K = 3,
	ModeHintIndex_14K = 4,
	ModeHintIndex_Andromeda = 5
};

static const char* ModeHintNames[] = {
	"ez2-5k-only",   /* 5K Only, Turntable */
	"ez2-5k",        /* 5K Normal, 5K Ruby */
	"ez2-7k",        /* 7K Normal */
	"ez2-10k",       /* 10K Maniac, EZ2Catch */
	"ez2-14k",       /* 14K Maniac */
	"ez2-andromeda", /* Andromeda Mix */
};

static const char *ModePrefix_5KOnly    = "5keymix";
static const char *ModePrefix_5KRadio   = "5radiomix";
static const char *ModePrefix_7KStreet  = "7streetmix";
static const char *ModePrefix_10KRadio  = "10radiomix";
static const char *ModePrefix_14KRadio  = "14radiomix";
static const char *ModePrefix_Ez2Catch  = "catch";
static const char *ModePrefix_10KManiac = "clubmix";
static const char *ModePrefix_Cv2Mix    = "cv2mix"; /* requires further processing */
static const char *ModePrefix_7KRadio   = "radiomix";
static const char *ModePrefix_5KRuby    = "rubymix";
static const char *ModePrefix_Turntable = "scratchmix";
static const char *ModePrefix_14KManiac = "spacemix";
static const char *ModePrefix_5KStreet  = "streetmix";

static const char *CV2MixSuffix_5KOnly    = "-5o";
static const char *CV2MixSuffix_5KStreet  = "-5s";
static const char *CV2MixSuffix_7KStreet  = "-7s";
static const char *CV2MixSuffix_10KManiac = "-tm";
static const char *CV2MixSuffix_14KManiac = "-fm";
static const char *CV2MixSuffix_Ez2Catch  = "-ca";
static const char *CV2MixSuffix_Andromeda = "-am";
static const char *CV2MixSuffix_Hell = "-hell";

static const char *ChartSide_1P = "1p-";
static const char *ChartSide_2P = "2p-";

const char *ModeHintFromFilename(char *filename){
	if(strstr(ModePrefix_Cv2Mix,filename) != NULL){
		/* determine mode from suffix */
		if(strstr(filename,CV2MixSuffix_5KOnly) != NULL){
			return ModeHintNames[ModeHintIndex_5KOnly];
		}
		else if(strstr(filename,CV2MixSuffix_5KStreet) != NULL){
			return ModeHintNames[ModeHintIndex_5K];
		}
		else if(strstr(filename,CV2MixSuffix_7KStreet) != NULL){
			return ModeHintNames[ModeHintIndex_7K];
		}
		else if(strstr(filename,CV2MixSuffix_10KManiac) != NULL){
			return ModeHintNames[ModeHintIndex_10K];
		}
		else if(strstr(filename,CV2MixSuffix_Ez2Catch) != NULL){
			return ModeHintNames[ModeHintIndex_10K];
		}
		else if(strstr(filename,CV2MixSuffix_14KManiac) != NULL){
			return ModeHintNames[ModeHintIndex_14K];
		}
		else if(strstr(filename,CV2MixSuffix_Andromeda) != NULL){
			return ModeHintNames[ModeHintIndex_Andromeda];
		}
		else if(strstr(filename,CV2MixSuffix_Hell) != NULL){
			return ModeHintNames[ModeHintIndex_7K];
		}
	}
	else{
		/*-- 5k only --*/
		if(strstr(filename,ModePrefix_5KOnly) != NULL){
			return ModeHintNames[ModeHintIndex_5KOnly];
		}
		else if(strstr(filename,ModePrefix_Turntable) != NULL){
			return ModeHintNames[ModeHintIndex_5KOnly];
		}

		/*-- 5k standard --*/
		else if(strstr(filename,ModePrefix_5KRadio) != NULL){
			return ModeHintNames[ModeHintIndex_5K];
		}
		else if(strstr(filename,ModePrefix_5KRuby) != NULL){
			return ModeHintNames[ModeHintIndex_5K];
		}
		else if(strstr(filename,ModePrefix_5KStreet) != NULL){
			return ModeHintNames[ModeHintIndex_5K];
		}

		/*-- 7k standard --*/
		else if(strstr(filename,ModePrefix_7KRadio) != NULL){
			return ModeHintNames[ModeHintIndex_7K];
		}
		else if(strstr(filename,ModePrefix_7KStreet) != NULL){
			return ModeHintNames[ModeHintIndex_7K];
		}

		/*-- 10k maniac --*/
		else if(strstr(filename,ModePrefix_10KRadio) != NULL){
			return ModeHintNames[ModeHintIndex_10K];
		}
		else if(strstr(filename,ModePrefix_10KManiac) != NULL){
			return ModeHintNames[ModeHintIndex_10K];
		}
		else if(strstr(filename,ModePrefix_Ez2Catch) != NULL){
			return ModeHintNames[ModeHintIndex_10K];
		}

		/*-- 14k maniac --*/
		else if(strstr(filename,ModePrefix_14KRadio) != NULL){
			return ModeHintNames[ModeHintIndex_14K];
		}
		else if(strstr(filename,ModePrefix_14KManiac) != NULL){
			return ModeHintNames[ModeHintIndex_14K];
		}
	}

	return "unknown";
}

const int ModeHintToIndex(const char *modeHint){
	if(strstr(modeHint,ModeHintNames[ModeHintIndex_5KOnly]) != 0){
		return ModeHintIndex_5KOnly;
	}
	else if(strstr(modeHint,ModeHintNames[ModeHintIndex_5K]) != 0){
		return ModeHintIndex_5K;
	}
	else if(strstr(modeHint,ModeHintNames[ModeHintIndex_7K]) != 0){
		return ModeHintIndex_7K;
	}
	else if(strstr(modeHint,ModeHintNames[ModeHintIndex_10K]) != 0){
		return ModeHintIndex_10K;
	}
	else if(strstr(modeHint,ModeHintNames[ModeHintIndex_14K]) != 0){
		return ModeHintIndex_14K;
	}
	else if(strstr(modeHint,ModeHintNames[ModeHintIndex_Andromeda]) != 0){
		return ModeHintIndex_Andromeda;
	}

	return 0;
}

/*============================================================================*/
/* SongOriginalName section and its contents are ignored by design */
enum IniSectionType {
	IniSectionType_Unset = 0,
	IniSectionType_General = 1,
	IniSectionType_JudgmentDelta = 2,
	IniSectionType_GaugeUpDownRate = 3,
	IniSectionType_Other
};

/* for use with JudgmentDeltaValues */
enum JudgmentDeltaIndex {
	JudgmentDeltaIndex_Kool = 0,
	JudgmentDeltaIndex_Cool = 1,
	JudgmentDeltaIndex_Good = 2,
	JudgmentDeltaIndex_Miss = 3
};

/* for use with GaugeUpDownRateValues */
enum GaugeUpDownRateIndex {
	GaugeUpDownRateIndex_Cool = 0,
	GaugeUpDownRateIndex_Good = 1,
	GaugeUpDownRateIndex_Miss = 2,
	GaugeUpDownRateIndex_Fail = 3
};

static const char *IniSection_General = "General";
static const char *IniSection_JudgmentDelta = "JudgmentDelta";
static const char *IniSection_GaugeUpDownRate = "GaugeUpDownRate";

static const char *IniKey_General_Level = "Level";
static const char *IniKey_General_MeasureScale = "MeasureScale";

static const char *IniKey_Judge_Kool = "Kool"; /* JudgmentDelta only */
static const char *IniKey_Judge_Cool = "Cool";
static const char *IniKey_Judge_Good = "Good";
static const char *IniKey_Judge_Miss = "Miss";
static const char *IniKey_Judge_Fail = "Fail"; /* GaugeUpDownRate only */

/*============================================================================*/
const char* OctaveNums = "0123456789";

int KeysoundIndexToInstrument(uint16_t index, uint16_t count, Instrument *instruments){
	for(int i = 0; i < count; i++){
		if(instruments[i].index == index){
			return i;
		}
	}
	return -1;
}

/*============================================================================*/
/* .ez to .bmson track mappings */
const uint8_t TrackMapping_5KOnly_1P[21] = {
	0,0,0, /* control, old bgm L, old bgm R */
	11,12,13,14,15, /* left side keys */
	0,0, /* effectors 1 and 2 */
	0, /* left side scratch */
	0, /* left side pedal */
	0,0, /* effectors 3 and 4 */
	0, /* right side pedal */
	0,0,0,0,0, /* right side keys */
	0 /* right side scratch */
};

const uint8_t TrackMapping_5K_1P[21] = {
	0,0,0, /* control, old bgm L, old bgm R */
	11,12,13,14,15, /* left side keys */
	0,0, /* effectors 1 and 2 */
	1, /* left side scratch */
	10, /* left side pedal */
	0,0, /* effectors 3 and 4 */
	0, /* right side pedal */
	0,0,0,0,0, /* right side keys */
	0 /* right side scratch */
};

const uint8_t TrackMapping_7K_1P[21] = {
	0,0,0, /* control, old bgm L, old bgm R */
	11,12,13,14,15, /* left side keys */
	31,32, /* effectors 1 and 2 */
	1, /* left side scratch */
	10, /* left side pedal */
	0,0, /* effectors 3 and 4 */
	0, /* right side pedal */
	0,0,0,0,0, /* right side keys */
	0 /* right side scratch */
};

const uint8_t TrackMapping_10K_1P[21] = {
	0,0,0, /* control, old bgm L, old bgm R */
	11,12,13,14,15, /* left side keys */
	0,0, /* effectors 1 and 2 */
	1, /* left side scratch */
	10, /* left side pedal */
	0,0, /* effectors 3 and 4 */
	0, /* right side pedal */
	21,22,23,24,25, /* right side keys */
	2 /* right side scratch */
};

const uint8_t TrackMapping_14K_1P[21] = {
	0,0,0, /* control, old bgm L, old bgm R */
	11,12,13,14,15, /* left side keys */
	31,32, /* effectors 1 and 2 */
	1, /* left side scratch */
	0, /* left side pedal */
	33,34, /* effectors 3 and 4 */
	0, /* right side pedal */
	21,22,23,24,25, /* right side keys */
	2 /* right side scratch */
};

const uint8_t TrackMapping_Andromeda_1P[21] = {
	0,0,0, /* control, old bgm L, old bgm R */
	11,12,13,14,15, /* left side keys */
	31,32, /* effectors 1 and 2 */
	1, /* left side scratch */
	10, /* left side pedal */
	33,34, /* effectors 3 and 4 */
	20, /* right side pedal */
	21,22,23,24,25, /* right side keys */
	2 /* right side scratch */
};

/*----------------------------------------------------------------------------*/
/* 2p side todo */

/*============================================================================*/
static void Usage(char* execName){
printf("%s - Converts {.ez, .ezi, .ini} to .bmson\n", execName);
	printf("Usage: %s [file.ez]\n", execName);
}

int main(int argc, char** argv){
	FILE *ezFile;
	FILE *eziFile;
	FILE *iniFile;
	FILE *bmsonOutFile;
	char *ezFilename;
	char *eziFilename;
	char *iniFilename;
	char *bmsonFilename;
	char *lineBuf;
	int numInstruments = 0;

	/* .ini stuff */
	unsigned long level = 1;
	float measureScale = 1.0f;
	unsigned long JudgmentDeltaValues[4] = {0,0,0,0};
	float GaugeUpDownRateValues[4] = {0,0,0,0};

	/* .ezi stuff */
	uint8_t eziFileVersion; /* 0=old, 1=new */
	char oldNoteStr[5]; /* only needed for old version .ezi */

	/* .ez stuff */
	EzHeader EzFileHeader;
	int noteBytes;
	int numTracksUsed = 0;
	int numPlayableTracks = 0;
	int playerSide = 0; /* 0=1p, 1=2p */

	if(argc <= 1){
		Usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	ezFilename = (char*)calloc(MAX_FILENAME_LENGTH,sizeof(char));
	if(ezFilename == NULL){
		printf("%s: Error attempting to allocate memory for .ez filename\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	eziFilename = (char*)calloc(MAX_FILENAME_LENGTH,sizeof(char));
	if(eziFilename == NULL){
		printf("%s: Error attempting to allocate memory for .ezi filename\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	iniFilename = (char*)calloc(MAX_FILENAME_LENGTH,sizeof(char));
	if(iniFilename == NULL){
		printf("%s: Error attempting to allocate memory for .ini filename\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	bmsonFilename = (char*)calloc(MAX_FILENAME_LENGTH,sizeof(char));
	if(bmsonFilename == NULL){
		printf("%s: Error attempting to allocate memory for .bmson filename\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	/* accept either file.ez or file (no extension)
	 * either way, we'll need to generate other filenames.
	 */
	char *inFileExt = strrchr(argv[1],'.');
	if(inFileExt != NULL){
		/* found file extension */
		strncpy(ezFilename,argv[1],inFileExt-argv[1]);
		strcat(ezFilename,".ez");

		strncpy(eziFilename,argv[1],inFileExt-argv[1]);
		strcat(eziFilename,".ezi");

		strncpy(iniFilename,argv[1],inFileExt-argv[1]);
		strcat(iniFilename,".ini");

		strncpy(bmsonFilename,argv[1],inFileExt-argv[1]);
		strcat(bmsonFilename,".bmson");
	}
	else{
		/* no file extension given */
		strcpy(ezFilename,argv[1]);
		strcat(ezFilename,".ez");

		strcpy(eziFilename,argv[1]);
		strcat(eziFilename,".ezi");

		strcpy(iniFilename,argv[1]);
		strcat(iniFilename,".ini");

		strcpy(bmsonFilename,argv[1]);
		strcat(bmsonFilename,".bmson");
	}

	if(strstr(ezFilename,ChartSide_2P) != 0){
		playerSide = 1;
	}
	else if(strstr(ezFilename,ChartSide_1P) != 0){
		playerSide = 0;
	}
	else{
		printf("Chart filename '%s' does not specify a player; assuming 1P.\n",ezFilename);
		playerSide = 0;
	}

	/*------------------------------------------------------------------------*/
	/*-- handle .ini file --*/
	iniFile = fopen(iniFilename,"r");
	if(iniFile == NULL){
		perror("Error atempting to open .ini file");
		exit(EXIT_FAILURE);
	}

	/* for the .ini file, we're only looking for a few things; here's an example:
	 * [General]
	 * Level = 15 -> info.level (unsigned long)
	 * MeasureScale = 1.6 -> info.measure_scale
	 *
	 * [JudgmentDelta] -> info.judgement_deltas (unsigned long)
	 * Kool = 7
	 * Cool = 24
	 * Good = 56
	 * Miss = 70
	 * 
	 * [GaugeUpDownRate] -> info.life_deltas (signed float)
	 * Cool = 0.2
	 * Good = 0.1
	 * Miss = -1.3
	 * Fail = -4.0
	 */
	lineBuf = calloc(LINEBUF_SIZE,sizeof(char));
	if(lineBuf == NULL){
		printf("%s: Error attempting to allocate memory for lineBuf (.ini)\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int curIniSection = 0;
	int tokenIndex = 0;
	while(feof(iniFile) == 0){
		memset(lineBuf,0,LINEBUF_SIZE);
		fgets(lineBuf,LINEBUF_SIZE,iniFile);

		if(lineBuf[0] == '['){
			/* new section */
			if(strstr(lineBuf,IniSection_General) != NULL){
				curIniSection = IniSectionType_General;
			}
			else if(strstr(lineBuf,IniSection_JudgmentDelta) != NULL){
				curIniSection = IniSectionType_JudgmentDelta;
			}
			else if(strstr(lineBuf,IniSection_GaugeUpDownRate) != NULL){
				curIniSection = IniSectionType_GaugeUpDownRate;
			}
			else{
				curIniSection = IniSectionType_Other;
			}
		}
		else{
			/* key = value */
			if(curIniSection == IniSectionType_General){
				char *tok = strtok(lineBuf," =\r\n");
				if(tok != NULL){
					if(strstr(tok,IniKey_General_Level) != 0){
						tok = strtok(NULL," =\r\n");
						level = atol(tok);
					}
					else if(strstr(tok,IniKey_General_MeasureScale) != 0){
						tok = strtok(NULL," =\r\n");
						measureScale = atof(tok);
					}
					++tokenIndex;
				}
			}
			else if(curIniSection == IniSectionType_JudgmentDelta){
				char *tok = strtok(lineBuf," =\r\n");
				if(tok != NULL){
					if(strstr(tok,IniKey_Judge_Kool) != 0){
						tok = strtok(NULL," =\r\n");
						JudgmentDeltaValues[JudgmentDeltaIndex_Kool] = atoi(tok);
					}
					else if(strstr(tok,IniKey_Judge_Cool) != 0){
						tok = strtok(NULL," =\r\n");
						JudgmentDeltaValues[JudgmentDeltaIndex_Cool] = atoi(tok);
					}
					else if(strstr(tok,IniKey_Judge_Good) != 0){
						tok = strtok(NULL," =\r\n");
						JudgmentDeltaValues[JudgmentDeltaIndex_Good] = atoi(tok);
					}
					else if(strstr(tok,IniKey_Judge_Miss) != 0){
						tok = strtok(NULL," =\r\n");
						JudgmentDeltaValues[JudgmentDeltaIndex_Miss] = atoi(tok);
					}
				}
			}
			else if(curIniSection == IniSectionType_GaugeUpDownRate){
				char *tok = strtok(lineBuf," =\r\n");
				if(tok != NULL){
					if(strstr(tok,IniKey_Judge_Cool) != 0){
						tok = strtok(NULL," =\r\n");
						GaugeUpDownRateValues[GaugeUpDownRateIndex_Cool] = atof(tok);
					}
					else if(strstr(tok,IniKey_Judge_Good) != 0){
						tok = strtok(NULL," =\r\n");
						GaugeUpDownRateValues[GaugeUpDownRateIndex_Good] = atof(tok);
					}
					else if(strstr(tok,IniKey_Judge_Miss) != 0){
						tok = strtok(NULL," =\r\n");
						GaugeUpDownRateValues[GaugeUpDownRateIndex_Miss] = atof(tok);
					}
					else if(strstr(tok,IniKey_Judge_Fail) != 0){
						tok = strtok(NULL," =\r\n");
						GaugeUpDownRateValues[GaugeUpDownRateIndex_Fail] = atof(tok);
					}
				}
			}
		}

		if(feof(iniFile)){
			break;
		}
	}

	fclose(iniFile);
	free(iniFilename);

	/*------------------------------------------------------------------------*/
	/*-- handle .ezi file --*/
	eziFile = fopen(eziFilename,"r");
	if(eziFile == NULL){
		perror("Error atempting to open .ezi file");
		exit(EXIT_FAILURE);
	}

	memset(lineBuf,0,LINEBUF_SIZE);
	if(lineBuf == NULL){
		printf("%s: Error attempting to allocate memory for lineBuf (.ezi)\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	/* determine format version from first character */
	eziFileVersion = isdigit(fgetc(eziFile)) ? 1 : 0;
	fseek(eziFile,0,SEEK_SET);

	/* get number of instruments */
	fseek(eziFile,0,SEEK_SET);
	while(feof(eziFile) == 0){
		memset(lineBuf,0,LINEBUF_SIZE);
		fgets(lineBuf,LINEBUF_SIZE,eziFile);

		if(feof(eziFile)){
			if(!isalnum((int)lineBuf[0])){
				break;
			}
		}

		if(isalnum((int)lineBuf[0])){
			numInstruments++;
		}
	}

	Instrument instruments[numInstruments];
	memset(instruments,0,numInstruments*sizeof(Instrument));
	int curInstrument = 0;
	/* read instrument data */
	fseek(eziFile,0,SEEK_SET);
	char *noteName = calloc(4,1);
	while(feof(eziFile) == 0){
		memset(lineBuf,0,LINEBUF_SIZE);
		fgets(lineBuf,LINEBUF_SIZE,eziFile);

		/* some .ezi files do not include a newline at the end.
		 * we need to be able to handle these files properly.
		 */
		if(feof(eziFile)){
			if(!isalnum((int)lineBuf[0])){
				break;
			}
		}

		if(eziFileVersion == 1){
			// keysound_index type filename
			sscanf(lineBuf,"%hi %d %s",
				&instruments[curInstrument].index,
				&instruments[curInstrument].type,
				instruments[curInstrument].filename
			);
			++curInstrument;
		}
		else{
			// key_name type filename
			sscanf(lineBuf,"%s %d %s",
				oldNoteStr,
				&instruments[curInstrument].type,
				instruments[curInstrument].filename
			);

			int p = strcspn(oldNoteStr,OctaveNums);
			memset(noteName,0,4);
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
			++curInstrument;
		}
	}
	free(noteName);

	fclose(eziFile);
	free(eziFilename);

	/*------------------------------------------------------------------------*/
	/*-- handle .ez file --*/
	ezFile = fopen(ezFilename,"rb");
	if(ezFile == NULL){
		perror("Error atempting to open .ez file");
		exit(EXIT_FAILURE);
	}

	/* look for EZFF magic */
	char *headerBytes = calloc(4,sizeof(char));
	if(headerBytes == NULL){
		printf("%s: Error attempting to allocate memory for headerBytes (.ez)\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	fread(headerBytes,4,1,ezFile);
	if(memcmp(headerBytes,Magic_EZFF,4) != 0){
		printf("%s: Unexpected .ez header magic { 0x%02X, 0x%02X, 0x%02X, 0x%02X }\n",
			argv[0], headerBytes[0], headerBytes[1], headerBytes[2], headerBytes[3]
		);
		exit(EXIT_FAILURE);
	}
	free(headerBytes);

	fseek(ezFile,5,SEEK_SET);
	EzFileHeader.version = (uint8_t)fgetc(ezFile);
	printf(".ez format version %d\n",EzFileHeader.version);

	switch(EzFileHeader.version){
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

	/* get internal name; this is not always used, so we can't guarantee
	 * its existence for things like info.title
	 */
	fgets(EzFileHeader.name,0x40,ezFile);

	/* skip 0x40 bytes */
	fseek(ezFile, 0x86, SEEK_SET); /* xxx: hardcoded */
	fread(&EzFileHeader.ticksPerMeasure,2,1,ezFile);
	fread(&EzFileHeader.initialBPM,4,1,ezFile);
	fread(&EzFileHeader.numTracks,2,1,ezFile);
	fread(&EzFileHeader.totalTicks,4,1,ezFile);
	fread(&EzFileHeader.secondBPM,4,1,ezFile);

	/* read track data */
	Track tracks[EzFileHeader.numTracks];
	uint32_t instrumentNoteCount[numInstruments];
	memset(instrumentNoteCount,0,numInstruments*sizeof(uint32_t));

	for(int i = 0 ; i < EzFileHeader.numTracks; i++){
		fgets(tracks[i].magic,5,ezFile);
		fread(&tracks[i].pad,2,1,ezFile);
		fgets(tracks[i].name,0x41,ezFile);
		fread(&tracks[i].numTicks,4,1,ezFile);
		fread(&tracks[i].dataSize,4,1,ezFile);
		int numNotes = tracks[i].dataSize/noteBytes;

		if(tracks[i].dataSize > 0){
			++numTracksUsed;
			if(i > 2 && i < 21){
				++numPlayableTracks;
			}
		}

		tracks[i].notes = (Note*)calloc(numNotes, sizeof(Note));
		for(int n = 0; n < numNotes; n++){
			fread(&tracks[i].notes[n].position,4,1,ezFile);
			tracks[i].notes[n].type = (uint8_t)fgetc(ezFile);
			if(EzFileHeader.version >= 7){
				/* v7, v8 */
				fread(&tracks[i].notes[n].data.v7,NOTE_DATA_LENGTH_V7,1,ezFile);
			}
			else if(EzFileHeader.version >= 5){
				/* v5, v6 */
				fread(&tracks[i].notes[n].data.v5,NOTE_DATA_LENGTH_V5,1,ezFile);
			}
			else{
				/* v4 */
				fread(&tracks[i].notes[n].data.v4,NOTE_DATA_LENGTH_V4,1,ezFile);
			}

			if(tracks[i].notes[n].type == NoteType_Note){
				if(EzFileHeader.version >= 7){
					uint16_t thisKeysound = (tracks[i].notes[n].data.v7[1]) << 8 | (tracks[i].notes[n].data.v7[0] & 0xFF);
					int instIndex = KeysoundIndexToInstrument(thisKeysound,numInstruments,instruments);
					if(tracks[i].notes[n].position < EzFileHeader.totalTicks){
						instrumentNoteCount[instIndex]++;
					}
					else{
						printf("track %d weird note pos %d\n",i,tracks[i].notes[n].position);
					}
				}
				else if(EzFileHeader.version >= 5){
					int instIndex = KeysoundIndexToInstrument(tracks[i].notes[n].data.v5[0],numInstruments,instruments);
					if(tracks[i].notes[n].position < EzFileHeader.totalTicks){
						instrumentNoteCount[instIndex]++;
					}
					else{
						printf("track %d weird note pos %d\n",i,tracks[i].notes[n].position);
					}
				}
				else{
					int instIndex = KeysoundIndexToInstrument(tracks[i].notes[n].data.v4[0],numInstruments,instruments);
					if(tracks[i].notes[n].position < EzFileHeader.totalTicks){
						instrumentNoteCount[instIndex]++;
					}
					else{
						printf("track %d weird note pos %d\n",i,tracks[i].notes[n].position);
					}
				}
			}
		}
	}

	printf("[Track Counts] non-zero: %d; playable: %d\n",numTracksUsed,numPlayableTracks);
	fclose(ezFile);

	printf("initial BPM: %f\n",EzFileHeader.initialBPM.f);
	printf("ticks per measure: %d\n",EzFileHeader.ticksPerMeasure);
	printf("total ticks: %d\n",EzFileHeader.totalTicks);
	/* todo: track beats per measure, in case there's a song that changes it */
	for(int n = 0; n < tracks[0].dataSize/noteBytes; n++){
		if(tracks[0].notes[n].type == NoteType_BeatsPerMeasure){
			if(EzFileHeader.version >= 7){
				printf("beats per measure @ %d: %d\n",tracks[0].notes[n].position,tracks[0].notes[n].data.v7[0]);
			}
			else if(EzFileHeader.version >= 5){
				printf("beats per measure @ %d: %d\n",tracks[0].notes[n].position,tracks[0].notes[n].data.v5[0]);
			}
			else{
				printf("beats per measure @ %d: %d\n",tracks[0].notes[n].position,tracks[0].notes[n].data.v4[0]);
			}
		}
	}

	/*------------------------------------------------------------------------*/
	/*-- write bmson file --*/
	bmsonOutFile = fopen(bmsonFilename,"w");
	if(bmsonOutFile == NULL){
		perror("Error atempting to create .bmson file");
		exit(EXIT_FAILURE);
	}

	fputs("{\r\n",bmsonOutFile);

	/* todo: determine if we need to add any of these.
	 * if the BPM set in the control channel at position 0 matches EzFileHeader.initialBPM,
	 * then we can skip it.
	 */
	fputs("\t\"bpm_events\": [\r\n",bmsonOutFile);
	/* each bpm event
	{
		"bpm": 120,
		"y": 0
	},
	*/
	for(int n = 0; n < tracks[0].dataSize/noteBytes; n++){
		if(tracks[0].notes[n].type == NoteType_BPM){
			union{
				float f;
				int i;
			} bpm;
			switch(EzFileHeader.version){
				case 4:
					bpm.i = (tracks[0].notes[n].data.v4[3] << 24) |
						(tracks[0].notes[n].data.v4[2] << 16) |
						(tracks[0].notes[n].data.v4[1] << 8) |
						tracks[0].notes[n].data.v4[0];
					break;

				case 5:
				case 6:
					bpm.i = (tracks[0].notes[n].data.v5[3] << 24) |
						(tracks[0].notes[n].data.v5[2] << 16) |
						(tracks[0].notes[n].data.v5[1] << 8) |
						tracks[0].notes[n].data.v5[0];
					break;

				case 7:
				case 8:
				default:
					bpm.i = (tracks[0].notes[n].data.v7[3] << 24) |
						(tracks[0].notes[n].data.v7[2] << 16) |
						(tracks[0].notes[n].data.v7[1] << 8) |
						tracks[0].notes[n].data.v7[0];
					break;
			}

			if(tracks[0].notes[n].position != 0){
				if(EzFileHeader.version <= 5){
					printf("%f bpm (adjusted: %f bpm) @ %d\n",bpm.f, bpm.f*OldBpmFactor,tracks[0].notes[n].position);
				}
				else{
					printf("%f bpm @ %d\n",bpm.f,tracks[0].notes[n].position);
				}
			}
		}
	}
	fputs("\t],\r\n",bmsonOutFile);

	/*-- start info block --*/
	fputs("\t\"info\": {\r\n",bmsonOutFile);

	fputs("\t\t\"artist\": \"\",\r\n",bmsonOutFile);
	fputs("\t\t\"back_image\": \"\",\r\n",bmsonOutFile);
	fputs("\t\t\"banner_image\": \"\",\r\n",bmsonOutFile);

	fprintf(bmsonOutFile,"\t\t\"chart_name\": \"%s\",\r\n",ezFilename);

	fputs("\t\t\"eyecatch_image\": \"\",\r\n",bmsonOutFile);
	fputs("\t\t\"genre\": \"\",\r\n",bmsonOutFile);

	fprintf(bmsonOutFile,"\t\t\"init_bpm\": %f,\r\n",EzFileHeader.initialBPM.f);

	fputs("\t\t\"judge_rank\": 100,\r\n",bmsonOutFile);

	/* todo: this and the life_deltas section use uppercase in the example bmson.
	 * i'm not sure why. based on the rest of the spec, they should be lowercase,
	 * but i can't be bothered to change it yet.
	 */
	fputs("\t\t\"judgement_deltas\": {\r\n",bmsonOutFile);
	fprintf(bmsonOutFile,"\t\t\t\"COOL\": %ld,\r\n", JudgmentDeltaValues[JudgmentDeltaIndex_Cool]);
	fprintf(bmsonOutFile,"\t\t\t\"GOOD\": %ld,\r\n", JudgmentDeltaValues[JudgmentDeltaIndex_Good]);
	fprintf(bmsonOutFile,"\t\t\t\"KOOL\": %ld,\r\n", JudgmentDeltaValues[JudgmentDeltaIndex_Kool]);
	fprintf(bmsonOutFile,"\t\t\t\"MISS\": %ld\r\n", JudgmentDeltaValues[JudgmentDeltaIndex_Miss]);
	fputs("\t\t},\r\n",bmsonOutFile);

	fprintf(bmsonOutFile,"\t\t\"level\": %ld,\r\n",level);

	/* GaugeUpDownRateValues */
	fputs("\t\t\"life_deltas\": {\r\n",bmsonOutFile);
	fprintf(bmsonOutFile,"\t\t\t\"COOL\": %f,\r\n", GaugeUpDownRateValues[GaugeUpDownRateIndex_Cool]);
	fprintf(bmsonOutFile,"\t\t\t\"FAIL\": %f,\r\n", GaugeUpDownRateValues[GaugeUpDownRateIndex_Fail]);
	fprintf(bmsonOutFile,"\t\t\t\"GOOD\": %f,\r\n", GaugeUpDownRateValues[GaugeUpDownRateIndex_Good]);
	fprintf(bmsonOutFile,"\t\t\t\"MISS\": %f\r\n", GaugeUpDownRateValues[GaugeUpDownRateIndex_Miss]);
	fputs("\t\t},\r\n",bmsonOutFile);

	fprintf(bmsonOutFile,"\t\t\"measure_scale\": %f,\r\n",measureScale);

	/* mode_hint */
	char *tempName = (char *)calloc(sizeof(char),strlen(ezFilename));
	for(int i = 0; i < strlen(ezFilename); i++){
		tempName[i] = tolower(ezFilename[i]);
	}

	/* todo: there are .ez files without the proper prefixes; how to handle? */
	const char *modeHint = ModeHintFromFilename(tempName);

	/* ModeHintToIndex */
	fprintf(bmsonOutFile,"\t\t\"mode_hint\": \"%s\",\r\n",modeHint);

	fputs("\t\t\"preview_music\": \"\",\r\n",bmsonOutFile);

	/* todo: is it possible for songs to have other resolution values? */
	fprintf(bmsonOutFile,"\t\t\"resolution\": %ld,\r\n",BMSON_DEFAULT_RESOLUTION);

	fputs("\t\t\"subartists\": [\r\n",bmsonOutFile);
	fputs("\t\t],\r\n",bmsonOutFile);

	fputs("\t\t\"subtitle\": \"\",\r\n",bmsonOutFile);
	fprintf(bmsonOutFile,"\t\t\"title\": \"%s\",\r\n",ezFilename);
	fputs("\t\t\"title_image\": \"\",\r\n",bmsonOutFile);
	fputs("\t\t\"total\": 100\r\n",bmsonOutFile);
	fputs("\t},\r\n",bmsonOutFile);
	/*-- end info block --*/

	fputs("\t\"lines\": [\r\n",bmsonOutFile);
	/* each line
	{
		"y": 0
	},
	*/
	fputs("\t],\r\n",bmsonOutFile);

	/* "sound_channels":[], */
	fputs("\t\"sound_channels\": [\r\n",bmsonOutFile);
	int curInstrumentIndex = 0;
	int chartMode = ModeHintToIndex(modeHint);
	for(int i = 0; i < numInstruments; i++){
		fputs("\t\t{\r\n",bmsonOutFile);

		fprintf(bmsonOutFile,"\t\t\t\"name\": \"%s\",\r\n", instruments[i].filename);

		fputs("\t\t\t\"notes\": [\r\n",bmsonOutFile);
		/* notes are the tricky part */
		/* track 22 and up should be treated as bgm (x=0) */
		/*
		 * c (bool) continuation
		 * l (int) length (noteLen-6 for ez2 value)
		 * x (int) sound lane
		 * y (int) time in pulses
		 */
		curInstrumentIndex = instruments[i].index;
		int numInstrNotes = 0;
		for(int t = 0; t < EzFileHeader.numTracks; t++){
			/* only bother processing tracks with data */
			if(tracks[t].dataSize > 0){
				int numTrackNotes = tracks[t].dataSize/noteBytes;
				for(int n = 0; n < numTrackNotes; n++){
					if(tracks[t].notes[n].type == NoteType_Note){
						/* ignore any notes at or past the song end point */
						if(tracks[t].notes[n].position >= EzFileHeader.totalTicks){
							continue;
						}

						if(EzFileHeader.version >= 7){
							uint16_t thisKeysound = (tracks[t].notes[n].data.v7[1]) << 8 | (tracks[t].notes[n].data.v7[0] & 0xFF);
							if(thisKeysound == curInstrumentIndex){
								numInstrNotes++;
								float relativePos = (float)tracks[t].notes[n].position/(float)(EzFileHeader.ticksPerMeasure);
								uint32_t targetPulse = (uint32_t)(relativePos*(BMSON_DEFAULT_RESOLUTION*4));
								/*
								printf("track %d @ (ez: %d; bmson: %d) = keysound %d (%s)\n",
									t,tracks[t].notes[n].position,targetPulse,
									curInstrumentIndex,instruments[i].filename
								);
								*/
								fputs("\t\t\t\t{\r\n",bmsonOutFile);
					
								/* c l x y */
								fputs("\t\t\t\t\t\"c\": false,\r\n",bmsonOutFile);

								uint16_t noteLen = (tracks[t].notes[n].data.v7[6] << 8) | (tracks[t].notes[n].data.v7[5] & 0xFF);
								if(noteLen > 6){
									/* convert length */
									float relativeLength = (float)(noteLen-6)/(float)(EzFileHeader.ticksPerMeasure);
									uint32_t targetLength = (uint32_t)(relativeLength*(BMSON_DEFAULT_RESOLUTION*4));
									fprintf(bmsonOutFile,"\t\t\t\t\t\"l\": %d,\r\n",targetLength);
								}
								else{
									fputs("\t\t\t\t\t\"l\": 0,\r\n",bmsonOutFile);
								}

								if(t > 21){
									/* .ez tracks 22 and up are for BGM notes. */
									/* todo: older versions might use tracks 1 and 2 as well */
									fputs("\t\t\t\t\t\"x\": 0,\r\n",bmsonOutFile);
								}
								else{
									/* .ez track to input; this relies on chartMode and playerSide */
									if(playerSide == 1){
										/* temp */
										fputs("\t\t\t\t\t\"x\": 1,\r\n",bmsonOutFile);
									}
									else{
										int trackX = -1;
										if(chartMode == ModeHintIndex_5KOnly){
											trackX = TrackMapping_5KOnly_1P[t];
										}
										else if(chartMode == ModeHintIndex_5K){
											trackX = TrackMapping_5K_1P[t];
										}
										else if(chartMode == ModeHintIndex_7K){
											trackX = TrackMapping_7K_1P[t];
										}
										else if(chartMode == ModeHintIndex_10K){
											trackX = TrackMapping_10K_1P[t];
										}
										else if(chartMode == ModeHintIndex_14K){
											trackX = TrackMapping_14K_1P[t];
										}
										else if(chartMode == ModeHintIndex_Andromeda){
											trackX = TrackMapping_Andromeda_1P[t];
										}
										fprintf(bmsonOutFile,"\t\t\t\t\t\"x\": %d,\r\n",trackX);
									}
								}

								fprintf(bmsonOutFile,"\t\t\t\t\t\"y\": %d\r\n",targetPulse);

								if(numInstrNotes == instrumentNoteCount[i]){
									fputs("\t\t\t\t}\r\n",bmsonOutFile);
								}
								else{
									fputs("\t\t\t\t},\r\n",bmsonOutFile);
								}
							}
						}
						else if(EzFileHeader.version >= 5){
							if(tracks[t].notes[n].data.v5[0] == curInstrumentIndex){
								numInstrNotes++;
								float relativePos = (float)tracks[t].notes[n].position/(float)(EzFileHeader.ticksPerMeasure);
								uint32_t targetPulse = (uint32_t)(relativePos*(BMSON_DEFAULT_RESOLUTION*4));
								/*
								printf("track %d @ (ez: %d; bmson: %d) = keysound %d (%s)\n",
									t,tracks[t].notes[n].position,targetPulse,
									curInstrumentIndex,instruments[i].filename
								);
								*/
								fputs("\t\t\t\t{\r\n",bmsonOutFile);
					
								/* c l x y */
								fputs("\t\t\t\t\t\"c\": false,\r\n",bmsonOutFile);

								uint16_t noteLen = (tracks[t].notes[n].data.v5[5] << 8) | (tracks[t].notes[n].data.v5[4] & 0xFF);
								if(noteLen > 6){
									/* convert length */
									float relativeLength = (float)(noteLen-6)/(float)(EzFileHeader.ticksPerMeasure);
									uint32_t targetLength = (uint32_t)(relativeLength*(BMSON_DEFAULT_RESOLUTION*4));
									fprintf(bmsonOutFile,"\t\t\t\t\t\"l\": %d,\r\n",targetLength);
								}
								else{
									fputs("\t\t\t\t\t\"l\": 0,\r\n",bmsonOutFile);
								}

								if(t > 21){
									/* .ez tracks 22 and up are for BGM notes. */
									/* todo: older versions might use tracks 1 and 2 as well */
									fputs("\t\t\t\t\t\"x\": 0,\r\n",bmsonOutFile);
								}
								else{
									/* .ez track to input; this relies on chartMode and playerSide */
									if(playerSide == 1){
										/* temp */
										fputs("\t\t\t\t\t\"x\": 1,\r\n",bmsonOutFile);
									}
									else{
										int trackX = -1;
										if(chartMode == ModeHintIndex_5KOnly){
											trackX = TrackMapping_5KOnly_1P[t];
										}
										else if(chartMode == ModeHintIndex_5K){
											trackX = TrackMapping_5K_1P[t];
										}
										else if(chartMode == ModeHintIndex_7K){
											trackX = TrackMapping_7K_1P[t];
										}
										else if(chartMode == ModeHintIndex_10K){
											trackX = TrackMapping_10K_1P[t];
										}
										else if(chartMode == ModeHintIndex_14K){
											trackX = TrackMapping_14K_1P[t];
										}
										else if(chartMode == ModeHintIndex_Andromeda){
											trackX = TrackMapping_Andromeda_1P[t];
										}
										fprintf(bmsonOutFile,"\t\t\t\t\t\"x\": %d,\r\n",trackX);
									}
								}

								fprintf(bmsonOutFile,"\t\t\t\t\t\"y\": %d\r\n",targetPulse);

								if(numInstrNotes == instrumentNoteCount[i]){
									fputs("\t\t\t\t}\r\n",bmsonOutFile);
								}
								else{
									fputs("\t\t\t\t},\r\n",bmsonOutFile);
								}
							}
						}
						else{
							if(tracks[t].notes[n].data.v4[0] == curInstrumentIndex){
								numInstrNotes++;
								float relativePos = (float)tracks[t].notes[n].position/(float)(EzFileHeader.ticksPerMeasure);
								uint32_t targetPulse = (uint32_t)(relativePos*(BMSON_DEFAULT_RESOLUTION*4));
								/*
								printf("track %d @ (ez: %d; bmson: %d) = keysound %d (%s)\n",
									t,tracks[t].notes[n].position,targetPulse,
									curInstrumentIndex,instruments[i].filename
								);
								*/
								fputs("\t\t\t\t{\r\n",bmsonOutFile);

								/* c l x y */
								fputs("\t\t\t\t\t\"c\": false,\r\n",bmsonOutFile);

								uint16_t noteLen = (tracks[t].notes[n].data.v4[4] << 8) | (tracks[t].notes[n].data.v4[3] & 0xFF);
								if(noteLen > 6){
									/* convert length */
									float relativeLength = (float)(noteLen-6)/(float)(EzFileHeader.ticksPerMeasure);
									uint32_t targetLength = (uint32_t)(relativeLength*(BMSON_DEFAULT_RESOLUTION*4));
									fprintf(bmsonOutFile,"\t\t\t\t\t\"l\": %d,\r\n",targetLength);
								}
								else{
									fputs("\t\t\t\t\t\"l\": 0,\r\n",bmsonOutFile);
								}

								if(t > 21){
									/* .ez tracks 22 and up are for BGM notes. */
									/* todo: older versions might use tracks 1 and 2 as well */
									fputs("\t\t\t\t\t\"x\": 0,\r\n",bmsonOutFile);
								}
								else{
									/* .ez track to input; this relies on chartMode and playerSide */
									if(playerSide == 1){
										/* temp */
										fputs("\t\t\t\t\t\"x\": 1,\r\n",bmsonOutFile);
									}
									else{
										int trackX = -1;
										if(chartMode == ModeHintIndex_5KOnly){
											trackX = TrackMapping_5KOnly_1P[t];
										}
										else if(chartMode == ModeHintIndex_5K){
											trackX = TrackMapping_5K_1P[t];
										}
										else if(chartMode == ModeHintIndex_7K){
											trackX = TrackMapping_7K_1P[t];
										}
										else if(chartMode == ModeHintIndex_10K){
											trackX = TrackMapping_10K_1P[t];
										}
										else if(chartMode == ModeHintIndex_14K){
											trackX = TrackMapping_14K_1P[t];
										}
										else if(chartMode == ModeHintIndex_Andromeda){
											trackX = TrackMapping_Andromeda_1P[t];
										}
										fprintf(bmsonOutFile,"\t\t\t\t\t\"x\": %d,\r\n",trackX);
									}
								}

								fprintf(bmsonOutFile,"\t\t\t\t\t\"y\": %d\r\n",targetPulse);

								if(numInstrNotes == instrumentNoteCount[i]){
									fputs("\t\t\t\t}\r\n",bmsonOutFile);
								}
								else{
									fputs("\t\t\t\t},\r\n",bmsonOutFile);
								}
							}
						}
					}
				}
			}
		}
		fputs("\t\t\t]\r\n",bmsonOutFile);

		fputs("\t\t}",bmsonOutFile);
		if(i < numInstruments-1){
			fputs(",\r\n",bmsonOutFile);
		}
		else{
			fputs("\r\n",bmsonOutFile);
		}
	}
	fputs("\t],\r\n",bmsonOutFile);

	fputs("\t\"stop_events\": [\r\n",bmsonOutFile);
	/* each stop event
	{
		"duration": 120,
		"y": 0
	},
	*/
	fputs("\t],\r\n",bmsonOutFile);

	fputs("\t\"version\": \"1.0.0\"\r\n",bmsonOutFile);
	fputs("}\r\n",bmsonOutFile);

	fclose(bmsonOutFile);
	free(ezFilename);

	printf("%s: wrote output to %s\n",argv[0],bmsonFilename);
	free(bmsonFilename);
	free(lineBuf);

	for(int i = 0 ; i < EzFileHeader.numTracks; i++){
		free(tracks[i].notes);
	}

	return EXIT_SUCCESS;
}
