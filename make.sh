#!/bin/bash

cd "$(dirname $0)"
mkdir -p bin
cc src/getreal.c -o bin/getreal
