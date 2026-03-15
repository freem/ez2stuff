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

## `ezdec_715`, `ezenc_715`
Utilities related to encoding and decoding EZ2DJ 7th Trax v1.5 `.ez` files.

`ezdec_715` decodes files, and `ezenc_715` encodes them. The latter program
should only be used with file format v6 `.ez` files.

Usage:
- `ezdec_715 [file.ez]` to decode
- `ezenc_715 [file.ez]` to encode

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
