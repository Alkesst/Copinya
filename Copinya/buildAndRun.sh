#!/bin/bash
echo "Compiling shell..."
if [[ !(-d build) ]]; then
	echo Creating Build directory...
	mkdir build
fi
cd build
gcc ../Shell_project.c ../job_control.c -o Shell
echo "Done!"
echo "Running Shell..."
echo
./Shell
