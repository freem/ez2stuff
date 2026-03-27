EZ2DJ/EZ2AC File Formats
========================
Unless otherwise stated, assume little-endian format. (EZ2DJ targeted
Windows-running x86 machines from the beginning, after all.)

## `.abm` &ndash; AmuseWorld Bitmap
Microsoft device-independent `.bmp` format with a modified header.

There are a few different versions:
- The old version was used from 2nd Trax through Endless Circulation.
- In Evolve and Night Traveler, the format was slightly changed, as well as the XOR constants.
- The old version returned to being used in Time Traveler, though the XOR constants continued to change.

### Primary Header
This describes the oldest version. Evolve and later have some differences.
- 0x00-0x01: "`AW`" string (0x41, 0x57)
- 0x02-0x05: [int] (version-dependent) Number of bits per pixel (copied from original `.bmp` offset 0x1C-0x1F) OR total file size.
- 0x06-0x07: [short] Image Width
- 0x08-0x09: [short] Image Height
- 0x0A-0x0D: [int] Data start address, XOR'ed by a constant (see below table)

In the newer version (Evolve and NT only), offset 0x02-0x05 is the total file size in bytes.

In Time Traveler, offset 0x02-0x05 has been reverted back to bits per pixel.

### Modified Windows `BITMAPINFOHEADER`
- 0x0E-0x11: [int] size of `BITMAPINFOHEADER`
- 0x12-0x15: [int] Image Width, XOR'ed by a constant (see below table)
- 0x16-0x19: [int] Image Height, XOR'ed by a constant (see below table)
- 0x1A-0x1B: [short] Number of color planes ("must be 1")
- 0x1C-0x1F: [int] Bits per pixel, XOR'ed by a constant (see below table)
- 0x20-0x21: [short] ??
- 0x22-0x25: [int] Bitmap data size
- 0x26-0x29: [int] Horizontal resolution (pixels per meter)
- 0x2A-0x2D: [int] Vertical resolution (pixels per meter)
- 0x2E-0x31: [int] Number of colors in color palette (0 for non-paletted images)
- 0x32-0x35: [int] "Number of important colors"

Pixel data follows, padded to a 4 byte boundary if necessary.

In some games, the primary header's width and/or height is set to 0.
Presumably, the game will un-XOR the width/height values from the relevant
`BITMAPINFOHEADER` offsets.

It is not currently known if paletted or compressed images exist.

### XOR Constants

| .abm File Offsets | Old      | Evolve   | NT       | TT       | Final    | Final EX   |
|-------------------|----------|----------|----------|----------|----------|------------|
| `0x0A-0x0D`       | `0x56FE` | `0x45AE` | `0x85BE` | `0x95AB` | `0x23FF` | `0x109A`   |
| `0x12-0x15`       | `0x0831` | `0x9AF1` | `0x96EC` | `0x45BB` | `0xBDC9` | `0xCFA1`   |
| `0x16-0x19`       | `0x1019` | `0x1D1B` | `0xFDEB` | `0xAE12` | `0x1F01` | `0x51AE`   |
| `0x1C-0x1F`       | `0x1120` | `0x67BE` | `0x67AE` | `0x78EF` | `0xA97F` | `0xB18F`   |

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
(Encoding information is beyond the scope of this document.)

### Header
- 0x00-0x03: [char*] "`EZFF`" string (0x45, 0x5A, 0x46, 0x46)
- 0x04: [byte] presumably the null terminator for the above string
- 0x05: [byte] Format version number (see above list)
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

General format is `[note/index] [velocity] [filename]`. I have only seen velocity
be 0 and 1. Unsure if other values are supported or not.

### Old Format
Example definition: `C#0 0 filename.wav`

Possible note values are: `C`, `C#`, `D`, `D#`, `E`, `F`, `F#`, `G`, `G#`, `A`, `A#`, `B`.

Octave values range from 0 to 21. The last possible note is `D#21`.

### New Format
Example definition: `1 0 filename.wav`

The note is now a numerical index. Otherwise, the format is the same.

-----

## Per-chart `.ini`

### `[General]`
- `Level` &ndash; Chart difficulty level.
- `MeasureScale`

### `[JudgmentDelta]`
- `Kool`
- `Cool`
- `Good`
- `Miss`

### `[GaugeUpDownRate]`
- `Cool`
- `Good`
- `Miss`
- `Fail`

### `[SongOriginalName]`
- `Name`
- `Version`

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
- `songtrack` &ndash; Defines track number to export notes to (see below).
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
Text-based format that defines background animations. Can be opened by EZ2Visual
and Tokeiburu's [StrEditor](https://rathena.org/board/topic/130296-a-more-friendly-str-editor/).

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
Earlier games use `.ezw`, later games use `.ssf`. Both formats are the same,
being derived from Microsoft `.wav` format.

### Header
 - 0x00-0x01: [short] Number of Channels
 - 0x02-0x05: [int] Sample Rate
 - 0x06-0x09: [int] "`(Sample Rate * BitsPerSample * Channels) / 8`"
 - 0x0A-0x0B: [short] "`(BitsPerSample * Channels) / 8` [1 - 8 bit mono; 2 - 8 bit stereo/16 bit mono; 4 - 16 bit stereo]"
 - 0x0C-0x0D: [short] Bits per Sample
 - 0x0E-0x11: [int] Data size

-----

## `.gds` &ndash; Game Description
INI-style format used to describe game modes.

The `'` character is used to start a comment.

### `[General]`
- `NumberOfSlot` &ndash; (int) Defines the total number of slots.
- `UseChainPlay` &ndash; (int) "Nonstop mixing" mode. Only set to 1 for Club and Space Mix in older games.
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

For `SongTrack` values, see "Songtrack Values" in the `.ets` section.

#### Key
`Key` has two values. If the second value is `-1`, there is no secondary input action.

Known values:
- 6 &ndash; Effector 1
- 7 &ndash; Effector 2
- 8 &ndash; Effector 3
- 9 &ndash; Effector 4
- 10 &ndash; 1P Key 1
- 11 &ndash; 1P Key 2
- 12 &ndash; 1P Key 3
- 13 &ndash; 1P Key 4
- 14 &ndash; 1P Key 5
- 15 &ndash; 1P Scratch 1
- 16 &ndash; 1P Scratch 2
- 17 &ndash; 1P Pedal
- 18 &ndash; 2P Key 1
- 19 &ndash; 2P Key 2
- 20 &ndash; 2P Key 3
- 21 &ndash; 2P Key 4
- 22 &ndash; 2P Key 5
- 23 &ndash; 2P Scratch 1
- 24 &ndash; 2P Scratch 2
- 25 &ndash; 2P Pedal

Values 0-5 are currently unknown.

Inputs that are not accounted for in the above list:
- 1P Start
- 2P Start
- Test Mode button
- Service credit button

### `[Gauge]`
This section appears to be optional, as a few `.gds` files lack it.
Presumably, if this section is missing, `Type` falls back to 0.

- `Type` &ndash; (int) Defines gauge type.

Known values:
- 0 &ndash; Standard Groove gauge
- 1 &ndash; Listening Rate gauge

### `[ShowCredit]`
- `Coord1`, `Coord2` &ndash; X,Y coordinates for credits display(s).

If the X coordinate is -1, the corresponding credits display is disabled.

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
INI-style format used to describe game panel visuals (and a few other things).

### `[General]`
- `NumberOfTrack` &ndash; [int] Number of track definitions.

### `[Track#]`
Track numbering starts at 1.

- `Enable` &ndash; 0=track disabled, 1=track enabled
- `Coord` &ndash; x,y coordinates.
- `Size` &ndash; width,height size.
- `BKAlphaFunc` &ndash; background alpha function (int,int)
- `BkColor1`, `BkColor2` &ndash; background colors (r,g,b,a; all ints)
- `LeftBoader`, `RightBoader` (sic) &ndash; define the borders of this track.
- `PressKeyCoord` &ndash; x,y position of pressed key texture.
- `PressKeyDownTexture` &ndash; filename of pressed key texture.
- `PressKeyColor` &ndash; pressed key texture colors (r,g,b,a; all ints)
- `PressKeyAlphaFunc` &ndash; pressed key texture alpha function (int,int)
- `PressBarColor` &ndash; pressed bar colors (r,g,b,a; all ints)
- `PressBarTexture` &ndash; filename of pressed bar texture.
- `PressBarMaxHeight`
- `PressBarGrowUpSpeed`
- `PressBarShrinkSpeed`
- `NoteAniTexture`

#### Border example data
This example creates a 2-pixel border with hex color `#141414` on the left side of the track.
The right side has no border.

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
- `Enable` &ndash; 0=disabled, 1=enabled
- `Left` &ndash; Leftmost x position.
- `Size` &ndash; width,height size.
- `MaxFrame` &ndash; maximum frame number. (Numbering starts at 0, so filenames will have 0 to MaxFrame-1)
- `AniTexture` &ndash; measure line texture path and prefix.

### `[CoolBomb]`, `[GoodBomb]`, `[LongNoteBomb]`
These define the note explosions for various judgments.

- `Enable` &ndash; 0=disabled, 1=enabled
- `HeightHotSpot`
- `Size` &ndash; width,height size.
- `FrameDelay` &ndash; number of frames to delay
- `MaxFrame` &ndash; maximum frame number. (Numbering starts at 0, so filenames will have 0 to MaxFrame-1)
- `AniTexture`

### `[Judgment]`
Standard judgments.

- `Enable` &ndash; 0=disabled, 1=enabled
- `HotSpot`
- `KoolStr` &ndash; filename of Kool animation.
- `CoolStr` &ndash; filename of Cool animation.
- `GoolStr` &ndash; filename of Good animation. (It really *does* say "Gool", I'm not trying to be funny.)
- `MissStr` &ndash; filename of Miss animation.
- `FailStr` &ndash; filename of Fail animation.

### `[JudgmentTex]`
This is the version with fast/slow indicators.

- `Enable` &ndash; 0=disabled, 1=enabled
- `KoolTex` &ndash; texture filename for Kool.
- `FastCoolTex` &ndash; texture filename for fast Cool.
- `SlowCoolTex` &ndash; texture filename for slow Cool.
- `GoodTex` &ndash; texture filename for Good.
- `MissTex` &ndash; texture filename for Miss.
- `FailTex` &ndash; texture filename for Fail.

### `[CoolCombo]`
Defines the main combo display.

- `Enable` &ndash; 0=disabled, 1=enabled
- `Font` &ndash; combo font texture path and prefix.
- `HotSpot`
- `Unit1Str` &ndash; filename of 1-digit combo animation.
- `Unit2Str` &ndash; filename of 2-digit combo animation.
- `Unit3Str` &ndash; filename of 3-digit combo animation.
- `Unit4Str` &ndash; filename of 4-digit combo animation.

### `[KeyPanel]`
- `Enable` &ndash; 0=disabled, 1=enabled
- `Coord` &ndash; x,y coordinates.
- `Size` &ndash; width,height size.
- `Bitmap` &ndash; path to panel texture.

### `[MaxCoolCombo]`
Defines the Max Combo display.

- `Enable` &ndash; 0=disabled, 1=enabled
- `Coord` &ndash; x,y coordinates.
- `FontTexture` &ndash; font texture path and prefix.
- `FontSize`
- `FontPitch`
- `Format` &ndash; `printf` format string
- `AlphaFunc` &ndash; (int,int)

### `[Score]`
Defines the Score display.

- `Enable` &ndash; 0=disabled, 1=enabled
- `Coord` &ndash; x,y coordinates.
- `FontTexture` &ndash; font texture path and prefix.
- `FontSize`
- `FontPitch`
- `Format` &ndash; `printf` format string
- `AlphaFunc` &ndash; (int,int)

### `[GrooveGauge]`
- `Enable` &ndash; 0=disabled, 1=enabled
- `BackCoord` &ndash; gauge background x,y coordinates.
- `BackBitmap`
- `GaugeCoord` &ndash; gauge foreground x,y coordinates.
- `GaugeBitmap`

### `[AudienceRating]`
Used for current Audience Rating when using the Listening Rate gauge.

- `Enable` &ndash; 0=disabled, 1=enabled
- `Coord` &ndash; x,y coordinates.
- `Font` &ndash; font filename prefix
- `FontSize` &ndash; font character size width,height
- `Format` &ndash; `printf` format string
- `AlphaFunc` &ndash; (int,int)

### `[GoalAudienceRating]`
Used for goal Audience Rating when using the Listening Rate gauge.

- `Enable` &ndash; 0=disabled, 1=enabled
- `Coord` &ndash; x,y coordinates.
- `Font` &ndash; font filename prefix
- `FontSize` &ndash; font character size width,height
- `Format` &ndash; `printf` format string
- `AlphaFunc` &ndash; (int,int)

### `[GrooveLight]`
- `Enable` &ndash; 0=disabled, 1=enabled
- `Coord` &ndash; x,y coordinates.
- `Size` &ndash; width,height size.
- `BackTexture`
- `FrontCoord`
- `FrontSize`
- `FrontTexture`

### `[TargetBar]`
Defines the target line/receptors.

- `Enable` &ndash; 0=disabled, 1=enabled

Then, for each possible `NoteAniTexture`:

- `Coord` &ndash; x,y coordinates.
- `Size` &ndash; width,height size.
- `AniTexture`

### `[SpecialNote]`
- `NoteAniTexture`

### `[RubyGauge]`
Defines the Ruby Mix life gauge.

- `Enable` &ndash; 0=disabled, 1=enabled
- `HotSpot`
- `BackStr`
- `Coord` &ndash; x,y coordinates.
- `ItemSize`
- `MaxItem`
- `MaxFrame`
- `ItemAniTexture`

### `[RubyKeyPanel]`
- `Enable` &ndash; 0=disabled, 1=enabled
- `Coord` &ndash; x,y coordinates.
- `Size` &ndash; width,height size.
- `Bitmap`
- `FailStr`

### `[ComboEffect]`
- `Enable` &ndash; 0=disabled, 1=enabled
- `Str1`
- `Str2`
- `Str3`

### `[PuzzleNote] `
- `Enable` &ndash; 0=disabled, 1=enabled
- `FadeOutAniTexture`
- `FadeInAniTexture`
- `BlinkAniTexture`
- `RandomAniTexture`
- `SuperRandomAniTexture`
- `ManiacRandomAniTexture`
- `4DAniTexture`
- `5DAniTexture`
- `BombAniTexture`
- `0_5XAniTexture`
- `2XAniTexture`
- `GaugeUpAniTexture`

### `[ComboGauge]`
- `Enable` &ndash; 0=disabled, 1=enabled
- `BackCoord`
- `BackBitmap`
- `GaugeCoord`
- `GaugeBitmap`

### `[SongSelectPane]`
- `Enable` &ndash; 0=disabled, 1=enabled
- `LeftCoord`
- `RightCoord`
- `Texture`
- `FinalTexture`

(todo)

FastSlowTex, GroupTexture

-----

## `.rep` &ndash; Replay Data
Originally used for Virtual Battle mode in EZ2DJ 6th Trax, also used for attract
mode gameplay in 7th Mix. Replay support was removed in EZ2AC Evolve.

There are two known versions, differentiated by the initial string:
- `EZREPLAY_V1`, 0x1A
- `EZREPLAY_V1.1`, 0x1A

Filename follows a pattern: `replay_[mode]_[songchart]`.

All of the pre-existing `.rep` files use `StreetMix` as `[mode]`.

Not fully researched yet.

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
- 0x04-0x07: [int] format version number?
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

if Number of layers is nonzero, that many ints will follow?

(other parts todo)

Number of textures (`texcnt`), followed by filenames (each filename's length is 0x80)

-----

## `.tgrp` &ndash; Texture Group
Found in EZ2DJ 2nd Trax. Possibly the successor to the `summary.bin` file found
in the Panel folders of 1st Trax and 1st Trax SE.

### Header
- 0x00-0x03: [char*] "`TXG0`" string (0x54, 0x58, 0x47, 0x30)

(todo)
