# getreal.c

![get real](getreal.gif)

A command-line tool written in C for decrypting/encrypting patterns in
S3M files, as seen in the music files for the 1993 demo Second Reality:
* [MUSIC0.S3M](https://github.com/mtuomi/SecondReality/raw/master/MAIN/MUSIC0.S3M)
* [MUSIC1.S3M](https://github.com/mtuomi/SecondReality/raw/master/MAIN/MUSIC1.S3M)

To compile on Linux, or some other decent OS, run `make.sh`.

To compile on Windows, idk good luck with that.

Usage example (for current version, might change in the future):

```bash
getreal input.s3m output.s3m
```

If instead of writing the output to a separate file, you want the input file itself
to be replaced, you can either specify the same file name for both the input and
output, or just specify the filename once:

```bash
getreal input.s3m
```

Exit codes:

* 0: success
* 1: error while trying to process input file
* 2: error while trying to process output file
* 4: not enough CLI arguments
* 8: input file does not seem like an S3M file
* 9: attempted to read file out of bounds (e.g. due to invalid pattern offsets)
