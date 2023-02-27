/*
 * ==========================================================================
 * getreal.c
 * v1.0.0-alpha1
 * 2023-02-27
 * ==========================================================================
 * by cs127
 * https://cs127.github.io
 * ==========================================================================
 * Command-line tool for decrypting/encrypting patterns in S3M files, as seen
 * in the music files for the 1993 demo Second Reality:
 * https://github.com/mtuomi/SecondReality/raw/master/MAIN/MUSIC0.S3M
 * https://github.com/mtuomi/SecondReality/raw/master/MAIN/MUSIC1.S3M
 * ==========================================================================
 * update v1.0.0-alpha1 / 2023-02-27: I have slightly unshittified the code.
 * ==========================================================================
 */


#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


typedef enum GRExitCode {

    // all files successfully processed
    GR_EXIT_OK                  = 0,

    // no file specified
    GR_EXIT_NO_FILE             = 1,

    // errors, added together as flags
    // each flag is set if the corresponding error is in at least one file
    GR_EXIT_IO_ERROR            = 2,
    GR_EXIT_INVALID_FILE_FORMAT = 4,
    GR_EXIT_FILE_OUT_OF_BOUNDS  = 8,

    // skill issue
    GR_EXIT_MISC                = 64

} GRExitCode;


const char CIPHER [256] = {
0x00,0x05,0x0A,0x0F,0x14,0x11,0x1E,0x1B,0x28,0x2D,0x22,0x27,0x3C,0x39,0x36,0x33,
0x50,0x55,0x5A,0x5F,0x44,0x41,0x4E,0x4B,0x78,0x7D,0x72,0x77,0x6C,0x69,0x66,0x63,
0xA0,0xA5,0xAA,0xAF,0xB4,0xB1,0xBE,0xBB,0x88,0x8D,0x82,0x87,0x9C,0x99,0x96,0x93,
0xF0,0xF5,0xFA,0xFF,0xE4,0xE1,0xEE,0xEB,0xD8,0xDD,0xD2,0xD7,0xCC,0xC9,0xC6,0xC3,
0x40,0x45,0x4A,0x4F,0x54,0x51,0x5E,0x5B,0x68,0x6D,0x62,0x67,0x7C,0x79,0x76,0x73,
0x10,0x15,0x1A,0x1F,0x04,0x01,0x0E,0x0B,0x38,0x3D,0x32,0x37,0x2C,0x29,0x26,0x23,
0xE0,0xE5,0xEA,0xEF,0xF4,0xF1,0xFE,0xFB,0xC8,0xCD,0xC2,0xC7,0xDC,0xD9,0xD6,0xD3,
0xB0,0xB5,0xBA,0xBF,0xA4,0xA1,0xAE,0xAB,0x98,0x9D,0x92,0x97,0x8C,0x89,0x86,0x83,
0x80,0x85,0x8A,0x8F,0x94,0x91,0x9E,0x9B,0xA8,0xAD,0xA2,0xA7,0xBC,0xB9,0xB6,0xB3,
0xD0,0xD5,0xDA,0xDF,0xC4,0xC1,0xCE,0xCB,0xF8,0xFD,0xF2,0xF7,0xEC,0xE9,0xE6,0xE3,
0x20,0x25,0x2A,0x2F,0x34,0x31,0x3E,0x3B,0x08,0x0D,0x02,0x07,0x1C,0x19,0x16,0x13,
0x70,0x75,0x7A,0x7F,0x64,0x61,0x6E,0x6B,0x58,0x5D,0x52,0x57,0x4C,0x49,0x46,0x43,
0xC0,0xC5,0xCA,0xCF,0xD4,0xD1,0xDE,0xDB,0xE8,0xED,0xE2,0xE7,0xFC,0xF9,0xF6,0xF3,
0x90,0x95,0x9A,0x9F,0x84,0x81,0x8E,0x8B,0xB8,0xBD,0xB2,0xB7,0xAC,0xA9,0xA6,0xA3,
0x60,0x65,0x6A,0x6F,0x74,0x71,0x7E,0x7B,0x48,0x4D,0x42,0x47,0x5C,0x59,0x56,0x53,
0x30,0x35,0x3A,0x3F,0x24,0x21,0x2E,0x2B,0x18,0x1D,0x12,0x17,0x0C,0x09,0x06,0x03
};

const char S3M_MAGIC [4] = "SCRM";


void gr_cipher(char* pattern, uint16_t psize) {

    // loop starts from bpos=2 because the first two bytes are the size

    for (uint16_t bpos = 2; bpos < psize; bpos++)
        pattern[bpos] = pattern[bpos] ^ CIPHER[bpos & 0xFF];

}


void gr_error(const char* msg, const char* path) {

    fprintf(stderr, "Error in file %s: %s\n", path, msg);

}


void gr_warn(const char* msg, const char* path) {

    fprintf(stderr, "Warning in file %s: %s\n", path, msg);

}


int main(int argc, char** argv) {

    if (argc <= 1) {
        fprintf(stderr, "Error: No files specified\n");
        return GR_EXIT_NO_FILE;
    }

    FILE* file;
    uint16_t ordnum, insnum, patnum, pidx, psize;
    size_t filesize, fileidx;
    int exitcode = GR_EXIT_OK;
    char magic [4], tmpmsg [256];

    // loop through every specified file

    for (fileidx = 1; fileidx < argc; fileidx++) {

        puts("========================================");

        printf("Processing file %s\n", argv[fileidx]);

        // load file

        if (!(file = fopen(argv[fileidx], "r+b"))) {
            gr_error(strerror(errno), argv[fileidx]);
            exitcode |= GR_EXIT_IO_ERROR;
            continue;
        }

        // get file size

        fseek(file, 0, SEEK_END);
        filesize = ftell(file);
        rewind(file); // be kind, rewind :)

        // check if the file is a valid S3M file

        fseek(file, 0x002C, SEEK_SET);
        fread(magic, 4, 1, file);

        if (memcmp(magic, S3M_MAGIC, 4)) {
            gr_error("Not a valid S3M file", argv[fileidx]);
            exitcode |= GR_EXIT_INVALID_FILE_FORMAT;
            continue;
        }

        // read number of orders, samples, and patterns

        fseek(file, 0x0020, SEEK_SET);
        fread(&ordnum, 2, 1, file);
        fread(&insnum, 2, 1, file);
        fread(&patnum, 2, 1, file);

        if (ordnum & 0x0001) {
            sprintf(tmpmsg, "OrdNum is an odd number (%d)", ordnum);
            gr_warn(tmpmsg, argv[fileidx]);
        }

        // read pattern pointers

        uint16_t ppos [patnum];
        fseek(file, 0x0060 + ordnum + insnum * 2, SEEK_SET);
        if (fread(ppos, 2, patnum, file) < patnum) {
            gr_error("List of pattern offsets is too short", argv[fileidx]);
            exitcode |= GR_EXIT_FILE_OUT_OF_BOUNDS;
            continue;
        }

        // process patterns

        printf("Processing %d patterns in file %s\n", patnum, argv[fileidx]);

        for (pidx = 0; pidx < patnum; pidx++) {

            if (ppos[pidx] << 4 >= filesize) {
                sprintf(
                    tmpmsg,
                    "Offset out of bounds for pattern %d, skipping to %d",
                    pidx, pidx + 1
                );
                gr_error(tmpmsg, argv[fileidx]);
                exitcode |= GR_EXIT_FILE_OUT_OF_BOUNDS;
                continue;
            }
            if (!ppos[pidx]) continue;

            fseek(file, ppos[pidx] << 4, SEEK_SET);
            fread(&psize, 2, 1, file);          // read first two bytes as size
            fseek(file, -2, SEEK_CUR);

            char pattern [psize];
            if (fread(pattern, psize, 1, file) < 1) {
                sprintf(
                    tmpmsg,
                    "Pattern %d ends prematurely, skipping to %d",
                    pidx, pidx + 1
                );
                gr_error(tmpmsg, argv[fileidx]);
                exitcode |= GR_EXIT_FILE_OUT_OF_BOUNDS;
                continue;
            }

            fseek(file, -psize, SEEK_CUR);
            gr_cipher(pattern, psize);          // de(/en)crypt the data
            if (fwrite(pattern, psize, 1, file) < 1) {
                sprintf(tmpmsg, "Could not write pattern %d", pidx);
                gr_error(tmpmsg, argv[fileidx]);
                exitcode |= GR_EXIT_IO_ERROR;
                continue;
            }

        }

        printf("File %s done\n", argv[fileidx]);

        fclose(file);

    }

    puts("========================================");

    return exitcode;

}
