#!/bin/bash

# pull the datasets
rm -r extdata
mkdir -p extdata

DATA_PATH=//renwekasmb01/library24/PCC/contents
if [ "$(uname)" == "Linux" ]; 
then 
	# use ls to force automount if possible
	ls /srv/library24
	if [ -d "/srv/library24" ]; 
	then
		DATA_PATH=/srv/library24/PCC/contents
	else 
		ls /home/library24
		if [ -d "/home/library24" ]; 
		then
			DATA_PATH=/home/library24/PCC/contents
		fi
	fi
fi

cp -f ${DATA_PATH}/8i_sony/longdress/Obj_textured/longdress_vox10_1051_uv* ./extdata
cp -f ${DATA_PATH}/8i_idcc_v1/longdress/vox10_face40k_map2k/longdress_vox10_1051_poisson40k_uv_map* ./extdata
cp -f ${DATA_PATH}/8i_idcc_v1/longdress/vox10_face40k_map2k/longdress_vox10_1052_poisson40k_uv_map* ./extdata
cp -f ${DATA_PATH}/8i_idcc_v1/longdress/vox10_face40k_map2k/longdress_vox10_1053_poisson40k_uv_map* ./extdata
cp -f ${DATA_PATH}/8i_idcc_v1/longdress/vox10_face40k_map2k/longdress_vox10_1054_poisson40k_uv_map* ./extdata
cp -f ${DATA_PATH}/8i_idcc_v1/longdress/vox10_face40k_map2k_globals.txt ./extdata
cp -f ${DATA_PATH}/owlii/Mesh/basketball_player/basketball_player_00000001* ./extdata
cp -f ${DATA_PATH}/owlii/Mesh/basketball_player/basketball_player_00000002* ./extdata
cp -f ${DATA_PATH}/owlii/Mesh/basketball_player/basketball_player_00000003* ./extdata
cp -f ${DATA_PATH}/owlii_idcc/cpv_basketball_player/cpv_basketball_player_00000001.ply ./extdata

echo --- done

#EOF 
