#questo script genera il pacchetto SMUBootLoader.mh211 da caricare sulle schede SECO per aggiornare il SMUBootLoader
clear
DATA=`date '+%y%m%d'`
estensione="_commit.mh211"
filename="SMUBootLoader_v.1.0.0_$DATA$estensione"


FILE_GPU="./ROCKCHIP_RELEASE_GPUBoot"
if [ ! -f "$FILE_GPU" ]; then
    echo "$FILE_GPU does not exist"
	cp ./aaa_MH211_CREATE_GPUBootUpdatePacket.sh ./error_ROCKCHIP_RELEASE_GPUBoot does_not_exists
	exit 1
fi


rm ./$filename
tar -czvf $filename $FILE_GPU

