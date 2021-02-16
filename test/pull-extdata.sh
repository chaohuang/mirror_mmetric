#!/bin/bash

# pull the datasets
rm -r extdata
mkdir -p extdata

DATA_PATH=//renwekasmb01/library24/PCC/contents
if [ "$(uname)" == "Linux" ]; 
then
	DATA_PATH=/srv/library24/PCC/contents
fi

cp -f ${DATA_PATH}/8i_idcc/longdress_vox10_face40k_map2k/longdress_vox10_1051_poisson40k_uv_map* ./extdata
cp -f ${DATA_PATH}/8i_idcc/longdress_vox10_face40k_map2k/longdress_vox10_1052_poisson40k_uv_map* ./extdata
cp -f ${DATA_PATH}/8i_idcc/longdress_vox10_face40k_map2k/longdress_vox10_1053_poisson40k_uv_map* ./extdata
cp -f ${DATA_PATH}/8i_idcc/longdress_vox10_face40k_map2k/longdress_vox10_1054_poisson40k_uv_map* ./extdata
cp -f ${DATA_PATH}/owlii/Mesh/basketball_player/basketball_player_00000001* ./extdata
cp -f ${DATA_PATH}/owlii_idcc/cpv_basketball_player/cpv_basketball_player_00000001.ply ./extdata
cp -f ${DATA_PATH}/8i_sony/longdress/Obj_textured/longdress_vox10_1051_uv* ./extdata

echo --- done

#EOF 
