EZ2DJ/EZ2AC File Formats
========================

## `.abm` &ndash; AmuseWorld Bitmap

(todo)

-----

## `.ez` &ndash; Note Chart Data
Internally referred to as "EZFF", based on the first four bytes of the file.

There are five known versions:
- v4 (`0x04`) &ndash; EZ2DJ 1st Tracks
- v5 (`0x05`) &ndash; EZ2DJ 1st Trax SE
- v6 (`0x06`) &ndash; EZ2DJ 2nd Trax; exported by EZSEQ.
- v7 (`0x07`) &ndash; EZ2AC Evolve; keysound index is now numerical and two bytes long.
- v8 (`0x08`) &ndash; EZ2AC Final; adds an anti-tampering pattern.

Apparently, BPMs in v4 and v5 format `.ez` files need to be multiplied by a constant
("approximately 0.9972299168975069") to be accurate.

Starting with EZ2DJ 7th Trax v1.5, `.ez` files are encoded using various methods.

### Header
- 0x00-0x03: [char*] "`EZFF`" string (0x45, 0x5A, 0x46, 0x46)
- 0x04: [byte] presumably the null terminator for the above string
- 0x05: [byte] Format version number
- 0x06: [char*] start of internal name (0x40 characters including null terminator)
- 0x86-0x87: [short] Ticks per measure
- 0x88-0x8B: [float] Initial BPM/tempo
- 0x8C-0x8D: [short] Number of tracks
- 0x8E-0x91: [uint32] Total number of ticks
- 0x92-0x95: [float] "Other" BPM/tempo

Track data follows, starting at file offset 0x96.

### Track Data
- 0x00-0x03: [char*] "`EZTR`" string (0x45, 0x5A, 0x54, 0x52)
- 0x04-0x05: always (0x00, 0x00)?
- 0x06-0x45: [char*] track name (0x40 characters including null)
- 0x46-0x49: [uint32] Number of ticks in this track
- 0x4A-0x4D: [uint32] Size of note data (in bytes)

Note data follows.

### Note Data
The note data format changes per version, mainly in the number of argument bytes.

- 0x00-0x03: [uint32] Position
- 0x04: [byte] Note Type
- 0x05: Start of parameters (length depends on version; contents depend on note type)

#### Note Types
- `0x01` &ndash; Note
- `0x02` &ndash; Volume
- `0x03` &ndash; BPM/Tempo
- `0x04` &ndash; Beats per Measure

#### Parameter Length
- v4 &ndash; 5 bytes (each note is 10 bytes long)
- v5,v6 &ndash; 6 bytes (each note is 11 bytes long)
- v7,v8 &ndash; 8 bytes (each note is 13 bytes long)

#### Note Params
- v4 &ndash; [keyindex] [velocity] [pan] [length length]
- v5,v6 &ndash; [keyindex] [velocity] [pan] [unknown] [length length]
- v7,v8 &ndash; [keyindex keyindex] [velocity] [pan] [unknown] [length length] [padding]

Velocity range is 0x00-0x7F.

Pan range is 0x00-0x7F, with 0x40 meaning center. 0x00 is presumed to be fully
panned left, 0x7F is presumed to be fully panned right.

Regular notes have a length of 6. Anything higher than that is considered a hold note.
Anything lower than that is untested.

#### Volume Params
Valid values are 0x00-0x7F.

- v4 &ndash; [volume] [0] [0] [0] [0]
- v5,v6 &ndash; [volume] [0] [0] [0] [0] [0]
- v7,v8 &ndash; [volume] [0] [0] [0] [0] [0] [0] [0]

#### BPM/Tempo Params
All BPMs are floating point.

- v4 &ndash; [bpm bpm bpm bpm] [0]
- v5,v6 &ndash; [bpm bpm bpm bpm] [0] [0]
- v7,v8 &ndash; [bpm bpm bpm bpm] [0] [0] [0] [0]

#### Beats per Measure Params
- v4 &ndash; [beats_per_measure] [0] [0] [0] [0]
- v5,v6 &ndash; [beats_per_measure] [0] [0] [0] [0] [0]
- v7,v8 &ndash; [beats_per_measure] [0] [0] [0] [0] [0] [0] [0]

-----

## `.ezi` &ndash; Instruments
Text-based format that defines the available instruments/keysounds.

There are two versions:
- "old" uses key names and octaves for keysounds, can only support up to 256 sounds.
- "new" uses indices for keysounds, can support more than 256 sounds.

(todo: what is the number between the note and filename for?)

### Old Format
Example definition: `C#0 0 filename.wav`

Possible note values are: `C`, `C#`, `D`, `D#`, `E`, `F`, `F#`, `G`, `G#`, `A`, `A#`, `B`.

Octave values range from 0 to 21. The last possible note is `D#21`.

### New Format
Example definition: `1 0 filename.wav`

The note is now a numerical index. Otherwise, the format is the same.

-----

## `.ets` &ndash; EZSEQ Track Set
This format defines the available lanes/tracks that can be used in EZSEQ.

`.pst` files from ptSequencer have a similar format.

### `[general]`
- `total` &ndash; defines the number of available tracks.

### `[Track#]`
Track numbering starts at 0.

- `type` &ndash; Track type number (see below).
- `name` &ndash; Name of this track.
- `songtrack` &ndash; Defines track number to export notes to.
- `width` &ndash; Defines width of track in pixels.
- `bold` &ndash; If set to 1, the vertical line preceding this track will be bold.

#### Track Types
- 0 &ndash; none
- 1 &ndash; White Key
- 2 &ndash; Blue Key
- 3 &ndash; Scratch (Pink)
- 4 &ndash; Pedal (Green)
- 5 &ndash; Effect (Red)
- 6 &ndash; Background

#### Songtrack Values
- 0 &ndash; Control Channel?
- 1 &ndash; ? (labeled "BG L" in older files)
- 2 &ndash; ? (labeled "BG R" in older files)
- 3 &ndash; 1P Key 1
- 4 &ndash; 1P Key 2
- 5 &ndash; 1P Key 3
- 6 &ndash; 1P Key 4
- 7 &ndash; 1P Key 5
- 8 &ndash; Effector 1
- 9 &ndash; Effector 2
- 10 &ndash; 1P Scratch
- 11 &ndash; 1P Pedal
- 12 &ndash; Effector 3
- 13 &ndash; Effector 4
- 14 &ndash; 2P Key 1
- 15 &ndash; 2P Key 2
- 16 &ndash; 2P Key 3
- 17 &ndash; 2P Key 4
- 18 &ndash; 2P Key 5
- 19 &ndash; 2P Scratch
- 20 &ndash; 2P Pedal
- 21 &ndash; Lights
- 22-31 &ndash; Background Notes

-----

## `.ezv` &ndash; "Ez2Video"
Text-based format that defines background animations. Meant to be opened by EZ2Visual.

EZ2Visual appears to recognize the following formats:
- `EVFF0.91`
- `EVFF0.92`
- `EVFF0.93`
- `EVFF0.94`

Apparently, there's also an `EVFF0.95`, with unknown changes/differences.

First line contains the format string.

- `fps` &ndash; Sets the frame rate of this animation.
- `maxkey` &ndash; Sets the maximum keyframe value for this animation.
- `layernum` &ndash; Defines the number of layers in this animation.

### Layer Definitions
```
layer:layername
{
	display=
	group=
	type=
	texcnt=
	texname=
	anikeynum=
}
```

- `display` &ndash; 0=not displayed, 1=displayed
- `group` &ndash; Group number
- `type` &ndash; Layer type number
- `texcnt` &ndash; Texture count. If this is greater than 0, that many `texname` entries are expected after this line.
- `texname` &ndash; Texture filename (sans extension)
- `anikeynum` &ndash; Sets the keyframe number for this layer.

### Frame Definitions
These are found inside of the layer definitions.
```
{
	frame=
	aniframe=
	anitype=
	delay=
	pos=
	uv=
	uvs=
	uv2=
	uvs2=
	scale=
	angle=
	color=
	mtpreset=
	points=
	rpoints=
	bezier=
	afbias=
	posbias=
	ptbias=
	angbias=
	uvbias=
	uvsbias=
	uvbias2=
	uvsbias2=
}
```

- `frame` &ndash; (int) Keyframe number.
- `aniframe` &ndash; (int) Animation frame (0=beginning, 99=end?)
- `anitype` &ndash; (int) Animation type (see below).
- `delay` &ndash; (float) Delay in frames
- `pos` &ndash; (float,float) Layer position. 320.0,240.0 is the center of the screen.
- `uv` &ndash; (float,float) Starting UV (texture mapping) position.
- `uvs` &ndash; (float,float) Starting UV (texture mapping) scale.
- `uv2` &ndash; (float,float) Ending UV (texture mapping) position.
- `uvs2` &ndash; (float,float) Ending UV (texture mapping) scale.
- `scale` &ndash; (float,float) Scale the layer to cover this size.
- `angle` &ndash; (float,float,float) X, Y, and Z rotation.
- `color` &ndash; (float,float,float,float,int,int) Floating point values determine color; last two ints determine blending modes (order: source, destination; see below for values).
- `mtpreset` &ndash; (int) Multi-texture effects preset (see below)
- `points` &ndash; (float,float float,float float,float float,float)
- `rpoints` &ndash; (float,float float,float float,float float,float)
- `bezier` &ndash; (float,float float,float) Defines a bezier curve?
- `afbias` &ndash; (float) Animation frame bias?
- `posbias` &ndash; (float) Position bias.
- `ptbias` &ndash; (float) Point bias?
- `angbias` &ndash; (float) Angular/Rotation bias.
- `uvbias` &ndash; (float) Starting UV position bias.
- `uvsbias` &ndash; (float) Starting UV scale bias.
- `uvbias2` &ndash; (float) Ending UV position bias.
- `uvsbias2` &ndash; (float) Ending UV scale bias.

#### `anitype` Values
- 0 &ndash; Stop
- 1 &ndash; Interpolation
- 2 &ndash; Once
- 3 &ndash; Loop
- 4 &ndash; Reverse Loop
- 5 &ndash; Biloop

#### `color` Blending Values
| Value | EZ2Visual Name  | StrEditor Name | OpenGL Equivalent      |
|-------|-----------------|----------------|------------------------|
| 0     | ZERO            | Zero           | GL_ZERO                |
| 1     | ONE             | One            | GL_ONE                 |
| 2     | SRCCOLOR        | SC             | GL_SRC_COLOR           |
| 3     | INVSRCCOLOR     | iSC            | GL_ONE_MINUS_SRC_COLOR |
| 4     | SRCALPHA        | SA             | GL_SRC_ALPHA           |
| 5     | INVSRCALPHA     | iSA            | GL_ONE_MINUS_SRC_ALPHA |
| 6     | DESTALPHA       | DA             | GL_DST_ALPHA           |
| 7     | INVDESTALPHA    | iDA            | GL_ONE_MINUS_DST_ALPHA |
| 8     | DESTCOLOR       | DC             | GL_DST_COLOR           |
| 9     | INVDESTCOLOR    | iDC            | GL_ONE_MINUS_DST_COLOR |
| 10    | SRCALPHASAT     | SAS            | GL_SRC_ALPHA_SATURATE  |
| 11    | BOTHSRCALPHA    | BSA            | ?                      |
| 12    | BOTHINVSRCALPHA | BiSA           | ?                      |

#### `mtpreset`/Multi-texture Effects Preset Values
All names are from EZ2Visual.
- 0 &ndash; None
- 1 &ndash; Modulate
- 2 &ndash; Modulate Alpha
- 3 &ndash; Add
- 4 &ndash; Decal Alpha
- 5 &ndash; Colored Light Map
- 6 &ndash; Inverse Colored Light Map
- 7 &ndash; Single Channel Light Map
- 8 &ndash; Modulate and Late Add
- 9 &ndash; Linear Blend using Texture Alpha
- 10 &ndash; Linear Blend using Diffuse Alpha
- 11 &ndash; Add with Smooth Saturation
- 12 &ndash; Multitexture Subtract
- 13 &ndash; Add Diffuse to Light Map then Modulate
- 14 &ndash; Detail Modulate
- 15 &ndash; Detail Add

-----

## `.ezw`, `.ssf` &ndash; Waves/Sound Files
Earlier games use `.ezw`, later games use `.ssf`. Both formats are the same.

(todo)

-----

## `.gds` &ndash; Game Description
INI-style format used to describe game modes.

The `'` character is used to start a comment.

### `[General]`
- `NumberOfSlot` &ndash; (int) Defines the total number of slots.
- `UseChainPlay` &ndash; (int) ?
- `MaxBaseStage` &ndash; (int) Maximum number of regular stages.
- `MaxBonusStage` &ndash; (int) Maximum number of bonus stages.

### `[Background]`
- `UseForceBackground` &ndash; (int) ?

### `[Slot#]`
Slot numbering starts at 1.

- `NumberOfTrack` &ndash; Defines the number of tracks in this slot.
- `Track#` &ndash; Defines a track.

Example data:
```
Track1=
{
	Key=15,16
	SongTrack=10
}
```

### `[Gauge]`
- `Type` &ndash; (int)

### `[ShowCredit]`
- `Coord1`, `Coord2`

### `[JudgmentLink]`
- `Slot#`

Example data:
```
Slot1 =
{
	NumberOfGroup=4
	Group1 =
	{
		NumberOfTrack=5
		SongTracks=3,4,5,6,7
	}
	Group2 =
	{
		NumberOfTrack=4
		SongTracks=8,9,12,13
	}
	Group3 =
	{
		NumberOfTrack=5
		SongTracks=14,15,16,17,18
	}
	Group4 =
	{
		NumberOfTrack=2
		SongTracks=11,20
	}
}
```

(todo)

-----

## `.pvi` &ndash; Panel Visuals
INI-style format used to describe game panel visuals.

### `[General]`
- `NumberOfTrack` &ndash; Number of playable tracks.

### `[Track#]`
Track numbering starts at 1.

- `Enable`
- `Coord`
- `Size`
- `BKAlphaFunc`
- `BkColor1`, `BkColor2`
- `LeftBoader`, `RightBoader` (sic)
- `PressKeyCoord`
- `PressKeyDownTexture`
- `PressKeyColor`
- `PressKeyAlphaFunc`
- `PressBarColor`
- `PressBarTexture`
- `PressBarMaxHeight`
- `PressBarGrowUpSpeed`
- `PressBarShrinkSpeed`
- `NoteAniTexture`

#### Border example data
```
LeftBoader = {
    LineWidth = 2
    LineColor = 20,20,20,255
}
RightBoader = {
    LineWidth = 0
    LineColor = 255,255,255,255
}
```

### `[MeasureLine]`
- `Enable`
- `Left`
- `Size`
- `MaxFrame`
- `AniTexture`

### `[CoolBomb]`, `[GoodBomb]`, `[LongNoteBomb]`
- `Enable`
- `HeightHotSpot`
- `Size`
- `FrameDelay`
- `MaxFrame`
- `AniTexture`

### `[Judgment]`
- `Enable`
- `HotSpot`
- `KoolStr`
- `CoolStr`
- `GoolStr`
- `MissStr`
- `FailStr`

### `[CoolCombo]`
- `Enable`
- `Font`
- `HotSpot`
- `Unit1Str`
- `Unit2Str`
- `Unit3Str`
- `Unit4Str`

### `[KeyPanel]`
- `Enable`
- `Coord`
- `Size`
- `Bitmap`

### `[MaxCoolCombo]`
- `Enable`
- `Coord`
- `FontTexture`
- `FontSize`
- `FontPitch`
- `Format`
- `AlphaFunc`

### `[Score]`
- `Enable`
- `Coord`
- `FontTexture`
- `FontSize`
- `FontPitch`
- `Format`
- `AlphaFunc`

### `[GrooveGauge]`
- `Enable`
- `BackCoord`
- `BackBitmap`
- `GaugeCoord`
- `GaugeBitmap`

### `[GrooveLight]`
- `Enable`
- `Coord`
- `Size`
- `BackTexture`
- `FrontCoord`
- `FrontSize`
- `FrontTexture`

### `[TargetBar]`
- `Enable`

Then, for each possible NoteAniTexture:

- `Coord`
- `Size`
- `AniTexture`

(todo)

-----

## `.rep` &ndash; Replay Data
File starts with `EZREPLAY_V1` string. Not fully researched yet.

-----

## `.scr` &ndash; Animation Script
Defines the timing of animations (i.e. when `.str` files are played back).
Despite sharing a file extension with Windows screen savers, they are not
implemented as screen savers.

There are multiple `.scr` formats:
- The earliest known version does not include the "SCR0" string found in later versions.
- The first appearance of `.scr` files with the "SCR0" header is in EZ2DJ 2nd Trax.

### Header (newer version)
- 0x00-0x03: [char*] "`SCR0`" string (0x53, 0x43, 0x52, 0x30)
- 0x04-0x07: [int] ?
- 0x08-0x0B: [int] ?
- 0x0C-0x0F: [int] ?
- 0x10-0x13: [int] ?
- 0x14-0x17: [int] ?

(todo)

-----

## `.str` &ndash; Animation Stream
This format is exported by Ez2Visual when using one of the "Play" options in the
"Preview" menu. This format can be opened with Tokeiburu's [StrEditor](https://rathena.org/board/topic/130296-a-more-friendly-str-editor/).

### Header
- 0x00-0x03: [char*] "`STRM`" string (0x53, 0x54, 0x52, 0x4D)
- 0x04-0x07: [int] Format version number
- 0x08-0x0B: [int] Frame rate (`fps`; 0x3C = 60 fps)
- 0x0C-0x0F: [int] Maximum key value (`maxkey`)
- 0x10-0x13: [int] Number of layers (`layernum`)

(other parts todo)
