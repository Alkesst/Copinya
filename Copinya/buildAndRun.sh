#!/bin/bash
echo "Compiling shell..."
if [[ !(-d Build) ]]; then
	echo Creating Build directory...
	mkdir Build
fi
cd Build
gcc ../Shell_project.c ../job_control.c -o Shell
echo "Done!"
echo "Running Shell..."
echo
./Shell
