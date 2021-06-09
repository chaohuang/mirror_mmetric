#!/bin/bash

CURDIR=`dirname $0`;
echo -e "\033[0;32mBuild: $(readlink -f $CURDIR) \033[0m";

case "$(uname -s)" in
  Linux*)     MACHINE=Linux;;
  Darwin*)    MACHINE=Mac;;
  *)          MACHINE=Other
esac

MODE=Release;
CMAKE_FLAGS=;
if [ "$MACHINE" == "Linux" ] ; then NUMBER_OF_PROCESSORS=`grep -c ^processor /proc/cpuinfo`; fi

USE_OPENMP=ON

for i in "$@"
do  
  case "$i" in
		deps|Deps|deps\/       ) ./build-deps.sh; exit;;
		doc|Doc|doc\/          ) ./build-doc.sh; exit;;
		debug|Debug|DEBUG      ) MODE=Debug;;
		release|Release|RELEASE) MODE=Release;;
		nojobs|NOJOBS		   ) NUMBER_OF_PROCESSORS=1;;
		noomp|NOOMP		       ) USE_OPENMP=OFF;;
		*                      ) echo "ERROR: arguments \"$i\" not supported: option = [doc|debug|release]"; exit -1;;
  esac
done

if [[ ! -d "./deps" ]];
then
	./build-deps.sh
fi

CMAKE_FLAGS="$CMAKE_FLAGS -DCMAKE_BUILD_TYPE=$MODE -DUSE_OPENMP=$USE_OPENMP";
case "${MACHINE}" in
  Linux) cmake -B${CURDIR}/build/${MODE} -G "Unix Makefiles"               ${CMAKE_FLAGS};; 
  Mac)   cmake -B${CURDIR}/build/${MODE} -G "Xcode"                        ${CMAKE_FLAGS};;
  *)     cmake -B${CURDIR}/build/${MODE} -G "Visual Studio 16 2019" -A x64 ${CMAKE_FLAGS};;
esac

case "${MACHINE}" in
  Linux) make -C ${CURDIR}/build/${MODE} -j ${NUMBER_OF_PROCESSORS} -s;; 
  Mac)   echo "Please, open the generated xcode project and build it ";;
  *)     MSBUILD=/C/Program\ Files\ \(x86\)/Microsoft\ Visual\ Studio/2019/Professional/MSBuild/Current/Bin/msbuild.exe
         if [[ -f "$MSBUILD" ]];
         then 
           "${MSBUILD}" ./build/${MODE}/mm.sln /property:Configuration=${MODE};
         else
           echo "MsBuild not found ($MSBUILD)";
           echo "Please, open the generated visual studio solution and build it ";        
         fi
  ;;
esac 