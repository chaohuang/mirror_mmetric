#!/bin/bash

# this file must be executed to regenerate the README.md
# upon binary internal documentation updates
# it requires the binary to be up to date and compiled

UNAME=`uname`

# the command 
CMD=""
if [ "$(uname)" == "Linux" ]; 
then
	CMD=./mm
else
	CMD=./x64/Release/mm.exe
fi

if [ ! -f "$CMD" ]; then
    echo "Error: it requires the binary to be up to date and compiled"
	exit
fi

echo "File automatically generated by readme.sh and ${CMD}, do not edit manually." > README.md
echo "" >> README.md
echo "# Compilation" >> README.md
echo "" >> README.md
echo "All dependencies are embeded (3rdparty foler)." >> README.md
echo "" >> README.md
echo "Linux:" >> README.md
echo "- make," >> README.md
echo "- output binary mm at root of filesystem." >> README.md
echo "" >> README.md
echo "Windows: " >> README.md
echo "- open solution with visual studio," >> README.md
echo "- build" >> README.md
echo "- output binary mm.exe in x64/Release" >> README.md
echo "" >> README.md
echo "# Usage" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} >> README.md
echo "\`\`\`" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} sample >> README.md
echo "\`\`\`" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} compare >> README.md
echo "\`\`\`" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} quantize >> README.md
echo "\`\`\`" >> README.md
# homogeneize line endings
dos2unix README.md
#EOF