#!/bin/bash

CURDIR=$(readlink -f `dirname $0` );
echo -e "\033[0;32mClean: $CURDIR \033[0m";

rm -rf \
  ${CURDIR}/build/ \
  ${CURDIR}/build_*/ 

if [ "$#" -gt "0" ] 
then
  for name in glfw dmetric
  do
    if [ "$1" == "${name}" ] || [ "$1" == "all" ]
    then 
      echo -e "\033[0;32mClean: ${CURDIR}/dependencies/${name} \033[0m";
      rm -rf "${CURDIR}/dependencies/${name}";
    fi
  done
fi

#EOF