#!/bin/bash

mkdir -p deps

cd deps

# pull the glfw source code

rm -rf glfw

GLFW_VER=3.3.2

echo --- download

curl http://github.com/glfw/glfw/releases/download/${GLFW_VER}/glfw-${GLFW_VER}.zip -L --output glfw-${GLFW_VER}.zip

echo --- unzip

unzip -q glfw-${GLFW_VER}.zip
rm -f glfw-${GLFW_VER}.zip
mv glfw-${GLFW_VER} glfw

echo --- done

#EOF 