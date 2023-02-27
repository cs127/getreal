#!/bin/bash

cd "$(dirname $0)"

! [ -f ../bin/getreal ] &&
echo "Executable not found. Compile the source code first." &&
exit 16

echo ""
echo "Test:"
echo "music0.s3m will be copied to music0_d.s3m and then decrypted."
echo "music1.s3m will be copied to music1_d.s3m and then decrypted."
echo ""
cp music0.s3m music0_d.s3m
cp music1.s3m music1_d.s3m
../bin/getreal music0_d.s3m music1_d.s3m
echo ""
echo "DONE"
echo ""
