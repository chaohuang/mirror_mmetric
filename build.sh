#!/bin/bash

CURDIR=$( cd "$( dirname "$0" )" && pwd ); 
echo -e "\033[0;32mBuild: ${CURDIR} \033[0m";

CMAKE=""; 
if [ "$( cmake  --version 2>&1 | grep version | awk '{print $3 }' | awk -F '.' '{print $1}' )" == 3 ] ; then CMAKE=cmake; fi
if [ "$( cmake3 --version 2>&1 | grep version | awk '{print $3 }' | awk -F '.' '{print $1}' )" == 3 ] ; then CMAKE=cmake3; fi
if [ "$CMAKE" == "" ] ; then echo "Can't find cmake > 3.0"; exit; fi

print_usage()
{
  echo "$0 mpeg-pcc-mmetric building script: "
  echo "";
  echo "    Usage:" 
  echo "       -h|--help    : Display this information."  
  echo "       -o|--ouptut  : Output build directory."
  echo "       -n|--ninja   : Use Ninja"
  echo "       --debug      : Build in debug mode."
  echo "       --release    : Build in release mode."
  echo "       --doc        : Build documentation"
  echo "       --test       : Execute all tests"
  echo "       --format     : Format source code"
  echo "       --nojobs     : Disables multi-processor build on unix"
  echo "       --noomp      : Disables openmp build"
  echo "       --nocmd      : Disables mm software building"
  echo "";
  echo "    Examples:";
  echo "      $0 "; 
  echo "      $0 --debug"; 
  echo "      $0 --doc";   
  echo "      $0 --format";    
  echo "    ";  
  if [ $# != 0 ] ; then echo -e "ERROR: $1 \n"; fi
  exit 0;
}

MODE=Release
TARGETS=()
CMAKE_FLAGS=()
OUTPUT=build
case $(uname -s) in Linux*) NUMBER_OF_PROCESSORS=$( grep -c ^processor /proc/cpuinfo );; esac

while [[ $# -gt 0 ]] ; do  
  C=$1; if [[ "$C" =~ [=] ]] ; then V=${C#*=}; elif [[ $2 == -* ]] ; then  V=""; else V=$2; shift; fi;
  case "$C" in    
    -h|--help     ) print_usage;;
    -n|--ninja    ) CMAKE_FLAGS+=( "-GNinja" );; 
    -o|--output=* ) OUTPUT=${V};;
    --debug       ) MODE=Debug; CMAKE_FLAGS+=("-DCMAKE_C_FLAGS=\"-g3\"" "-DCMAKE_CXX_FLAGS=\"-g3\"" );;
    --release     ) MODE=Release;;
    --doc         ) ${CURDIR}/doc/build-doc.sh; exit;;
    --test        ) ${CURDIR}/test.sh; exit;;
    --format      ) TARGETS+=( "clang-format" );;    
    --doc         ) ${CURDIR}/doc/build-doc.sh; exit;;
    --test        ) ${CURDIR}/doc/test.sh; exit;;
    --nojobs      ) NUMBER_OF_PROCESSORS=1;;
    --noomp       ) CMAKE_FLAGS+=( "-DUSE_OPENMP=OFF" );;
    --nocmd       ) CMAKE_FLAGS+=( "-DMM_BUILD_CMD=OFF" );;
    *             ) print_usage "unsupported arguments: $C ";;
  esac
  shift;
done

CMAKE_FLAGS+=( "-DCMAKE_BUILD_TYPE=$MODE" )
CMAKE_FLAGS+=( "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" )
if ! ${CMAKE} -H${CURDIR} -B"${CURDIR}/${OUTPUT}/${MODE}" "${CMAKE_FLAGS[@]}";
then
  echo -e "\033[1;31mfailed \033[0m"
  exit 1;
fi 
echo -e "\033[0;32mdone \033[0m";

# Use custom targets
if (( ${#TARGETS[@]} ))
then 
  for TARGET in ${TARGETS[@]}
  do     
    echo -e "\033[0;32m${TARGET}: ${CURDIR} \033[0m";
    ${CMAKE} --build "${CURDIR}/${OUTPUT}/${MODE}" --target ${TARGET}
    echo -e "\033[0;32mdone \033[0m";
  done
  exit 0
fi

echo -e "\033[0;32mBuild: ${CURDIR} \033[0m";
if ! ${CMAKE} --build "${CURDIR}/${OUTPUT}/${MODE}" --config ${MODE} --parallel "${NUMBER_OF_PROCESSORS}" ;
then
  echo -e "\033[1;31mfailed \033[0m"
  exit 1;
fi 
echo -e "\033[0;32mdone \033[0m";

#EOF