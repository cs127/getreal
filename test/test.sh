#!/bin/bash

cd "$(dirname $0)"

! [ -f ../getreal ] &&
echo "Executable not found. Compile the source code first." &&
exit 16

echo ""
echo "Decrypting MUSIC0.S3M to MUSIC0_U.S3M..."
../getreal MUSIC0.S3M MUSIC0_U.S3M
echo ""
echo "Decrypting MUSIC1.S3M to MUSIC1_U.S3M..."
../getreal MUSIC1.S3M MUSIC1_U.S3M
