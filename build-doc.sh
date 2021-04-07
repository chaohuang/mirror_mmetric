#!/bin/bash

# this file must be executed to regenerate the README.md
# upon binary internal documentation updates
# it requires the binary to be up to date and compiled

UNAME=`uname`

# the command 
CMD=""
if [ "$(uname)" == "Linux" ]; 
then
	CMD=./bin/mm
else
	CMD=./bin/mm.exe
fi

if [ ! -f "$CMD" ]; then
    echo "Error: it requires the binary to be up to date and compiled"
	exit
fi

echo "File automatically generated by build-doc.sh and ${CMD}, do not edit manually." > README.md
echo "" >> README.md
cat  ./doc/README.md >> README.md
echo "" >> README.md
echo "# Command references" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} >> README.md
echo "\`\`\`" >> README.md
echo "" >> README.md
echo "## Analyse" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} analyse >> README.md
echo "\`\`\`" >> README.md
echo "" >> README.md
echo "## Compare" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} compare >> README.md
echo "\`\`\`" >> README.md
echo "" >> README.md
echo "## Degrade" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} degrade >> README.md
echo "\`\`\`" >> README.md
echo "## Dequantize" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} dequantize >> README.md
echo "\`\`\`" >> README.md
echo "## Normals" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} normals >> README.md
echo "\`\`\`" >> README.md
echo "## Quantize" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} quantize >> README.md
echo "\`\`\`" >> README.md
echo "## Reindex" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} reindex >> README.md
echo "\`\`\`" >> README.md
echo "## Sample" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} sample >> README.md
echo "\`\`\`" >> README.md
echo "## Sequence" >> README.md
echo "" >> README.md
echo "\`\`\`" >> README.md
${CMD} sequence >> README.md
echo "\`\`\`" >> README.md
# homogeneize line endings
dos2unix README.md
#EOF