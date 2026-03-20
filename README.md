Various EZ2DJ/EZ2AC Tools
=========================

## `ezinfo`
Displays various information about (decoded) `.ez` files.
File format versions v4 to v8 are supported. I have not been able to find any
files that are v3 or earlier.

Starting with EZ2DJ 7th Trax v1.5, `.ez` files have been encoded in various ways.
This program does not handle decoding, since each game handles it differently.

If you are going to use `.ez` format v8 data with this program, it is recommended
to run it through `ezunfn` first.

Usage: `ezinfo [file.ez]`

Displays the following information:
- filesize
- `.ez` format version number
- internal track name (if any)
- ticks per measure
- initial and "other" BPM
- track count
- total ticks
- per-track data, including notes (handles commands 1-4)

## `ezins`
Displays information about (decoded) `.ezi` files. Both old and new versions
are supported.

Usage: `ezins [file.ezi]`

Displays:
- format type ("new" or "old)
- each entry, including equivalent keysound index (for old files) or key/octave pair (for new files; 0-255 only)

Converting between `.ezi` versions is not yet available, but is planned.

## `ezabm`
Utility related to `.abm` and `.bmp` files.

`--tobmp` functionality is not fully implemented. This will be fixed.

Usage:
- `ezabm --info filename.abm` to get information about an `.abm` file.
- `ezabm --tobmp version filename.abm [outfile.bmp]` to convert `.abm` to `.bmp`
- `ezabm --toabm version filename.bmp [outfile.abm]` to convert `.bmp` to `.abm`

For `--tobmp` and `--toabm`, `version` is one of the following:
- `old` &ndash; 2nd Trax through Endless Circulation
- `ev` &ndash; Evolve
- `nt` &ndash; Night Traveler
- `tt` &ndash; Time Traveler
- `fn` &ndash; Final
- `fnex` &ndash; Final EX
- `auto` &ndash; attempt to auto-detect format (`--tobmp` only)

## `ezdec_715`, `ezenc_715`
Utilities related to encoding and decoding EZ2DJ 7th Trax v1.5 files.

`ezdec_715` decodes files, and `ezenc_715` encodes them.

Usage:
- `ezdec_715 [file]` to decode
- `ezenc_715 [file]` to encode

These tools *overwrite* the input file with the modified version, so be careful.

## `ezdec_720`, `ezenc_720`
Utilities related to encoding and decoding EZ2DJ 7th Trax v2.0 `.ez` files.

Only `ezdec_720` works properly at the moment; there are some slight errors with
`ezenc_720`.

Usage:
- `ezdec_720 [file.ez]` to decode
- `ezenc_720 [file.ez]` to encode

These tools *overwrite* the input file with the modified version, so be careful.

## `ezunfn`, `ezrefn`
Utilities related to the anti-tampering pattern from (decoded) v8 `.ez` files.

`ezunfn` removes the anti-tampering pattern, while `ezrefn` adds it.

Usage:
- `ezunfn [file.ez]` to decode
- `ezrefn [file.ez]` to encode

These tools *overwrite* the input file with the modified version, so be careful.

`ezunfn` is essentially a v8 -> v7 downgrade tool, though it does not modify
the format version byte.

`ezrefn` *does* upgrade v7 format files to v8, changing the format version byte.

## Notice
I have no interest in producing tools for EZ2ON Reboot:R. If these tools happen
to work with its data, then it's a happy accident that's completely unintended.
